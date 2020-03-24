#include "ndpch.h"
#include "LuaLayer.h"
#include "imgui.h"

#include "event/KeyEvent.h"
#include <GLFW/glfw3.h>
#include "memory/stack_allocator.h"
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>
#include "audio/Player.h"
#include "imguifiledialog/ImGuiFileDialog.h"

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

#define LUACON_ERROR_PREF "[error]"
#define LUACON_TRACE_PREF "[trace]"
#define LUACON_INFO_PREF "[info_]"
#define LUACON_WARN_PREF "[warn_]"

static NBT* s_settings;
static const char* s_settings_file = "lua_editor_settings.dat";

struct FileOpen
{
	std::string filePath;
	std::string name;
	std::string text;
	bool saved;
};

static std::vector<FileOpen> s_files;
static bool s_needs_settings_load = false;
static int s_currentFileIndex = 0;


static char text[1024 * 32];
static const char* textExample =
	"--Basic Lua Script\n"
	"a = 10\n"
	"b= 35\n"
	"c = (a+b)*b/2\n"
	"\n"
	"ND_TRACE(c)\n\0";

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct LuaConsole
{
	LuaLayer* layer;
	char InputBuf[256];
	ImVector<char*> Items;
	ImVector<const char*> Commands;
	ImVector<char*> History;
	int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter Filter;
	bool AutoScroll;
	bool ScrollToBottom;
	bool hasNewCommandPending = false;
	bool show_script_editor = false;

	LuaConsole(LuaLayer* layer): layer(layer)
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("CLASSIFY");
		// "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
		AutoScroll = true;
		ScrollToBottom = true;
	//	AddLog("Welcome to Dear ImGui!");
	}

	~LuaConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	// Portable helpers
	static int Stricmp(const char* str1, const char* str2)
	{
		int d;
		while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
		{
			str1++;
			str2++;
		}
		return d;
	}

	static int Strnicmp(const char* str1, const char* str2, int n)
	{
		int d = 0;
		while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
		{
			str1++;
			str2++;
			n--;
		}
		return d;
	}

	static char* Strdup(const char* str)
	{
		size_t len = strlen(str) + 1;
		void* buf = malloc(len);
		IM_ASSERT(buf);
		return (char*)memcpy(buf, (const void*)str, len);
	}

	static void Strtrim(char* str)
	{
		char* str_end = str + strlen(str);
		while (str_end > str && str_end[-1] == ' ') str_end--;
		*str_end = 0;
	}

	void ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
		ScrollToBottom = true;
	}

	void AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
		if (AutoScroll)
			ScrollToBottom = true;
	}


	void Draw(bool* p_open)
	{
		static bool keepDefaultFile = false;
		static bool fileSaved = true;
		static bool scriptEditOpen = false;
		if (s_files.empty())
		{
			s_files.push_back({"", "unsaved", "--something interesting", false});
			s_currentFileIndex = 0;
		}
		if (show_script_editor)
		{
			ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Script Editor", &show_script_editor))
			{
				ImGui::TextColored({0, 1, 0, 1},
				                   (s_files[s_currentFileIndex].filePath + (s_files[s_currentFileIndex].saved ? "" : " *")).
				                   c_str());

				// Note: we are using a fixed-sized buffer for simplicity here. See ImGuiInputTextFlags_CallbackResize
				// and the code in misc/cpp/imgui_stdlib.h for how to setup InputText() for dynamically resizing strings.


				if (ImGui::SmallButton("Run"))
				{
					layer->runScriptInConsole(nullptr, text);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("AsyncRun"))
				{
					std::string s = text;
					s = "local asddfaqq = function()\n" + s + "\nend\nregisterCoroutine(asddfaqq)";
					layer->runScriptInConsole(nullptr, s.c_str());
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("Clear"))
				{
					s_files[s_currentFileIndex].text = "";
					s_files[s_currentFileIndex].saved = false;
					ZeroMemory(text, IM_ARRAYSIZE(text));
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("New"))
				{
					int length = strlen(text);
					s_files[s_currentFileIndex].text = std::string(text);

					s_files.push_back({"", "unsaved", "--example", false});
					s_currentFileIndex = s_files.size() - 1;
					ZeroMemory(text, IM_ARRAYSIZE(text));
					memcpy(&text, s_files[s_currentFileIndex].text.c_str(),
					       strlen(s_files[s_currentFileIndex].text.c_str()));
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Close"))
				{
					s_files.erase(s_files.begin() + s_currentFileIndex);
					if (s_files.empty())
					{
						s_files.push_back({"", "unsaved", "--example", false});
						s_currentFileIndex = 0;
						ZeroMemory(text, IM_ARRAYSIZE(text));
						memcpy(&text, s_files[s_currentFileIndex].text.c_str(),
						       strlen(s_files[s_currentFileIndex].text.c_str()));
					}
					else
					{
						if (s_currentFileIndex == s_files.size())
							s_currentFileIndex--;
						ZeroMemory(text, IM_ARRAYSIZE(text));
						memcpy(&text, s_files[s_currentFileIndex].text.c_str(),
						       strlen(s_files[s_currentFileIndex].text.c_str()));
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Save"))
				{
					if (!s_files[s_currentFileIndex].filePath.empty())
					{
						FILE* file = fopen(s_files[s_currentFileIndex].filePath.c_str(), "w");
						if (file)
						{
							size_t length = strlen(text);
							size_t written = 0;
							while (length != 0)
							{
								written += fwrite(text + written, 1, length, file);
								length -= written;
							}
							fclose(file);
							keepDefaultFile = true;
						}
						s_files[s_currentFileIndex].saved = true;
					}
					else
					{
						keepDefaultFile = false;
						ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".lua\0\0", ".");
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Save As"))
				{
					keepDefaultFile = true;
					ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".lua\0\0", ".");
				}
				// display
				if (ImGuiFileDialog::Instance()->FileDialog("SaveFileDlgKey"))
				{
					// action if OK
					if (ImGuiFileDialog::Instance()->IsOk == true)
					{
						std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
						std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

						FILE* file = fopen(filePathName.c_str(), "w");
						if (file)
						{
							auto index = strlen(text);
							size_t length = index + 1;
							size_t written = 0;
							while (length != 0)
							{
								written += fwrite(text + written, 1, length, file);
								length -= written;
							}
							fclose(file);

							if (s_files[s_currentFileIndex].filePath == "")
							{
								s_files[s_currentFileIndex].filePath = filePathName;
								s_files[s_currentFileIndex].name = filePathName.substr(filePath.size() + 1);
								s_files[s_currentFileIndex].saved = true;
							}
						}
					}
					// close
					ImGuiFileDialog::Instance()->CloseDialog("SaveFileDlgKey");
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Open"))
				{
					ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Open File", ".lua\0\0", ".");
				}
				// display
				if (ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
				{
					// action if OK
					if (ImGuiFileDialog::Instance()->IsOk == true)
					{
						int length = strlen(text);
						s_files[s_currentFileIndex].text = std::string(text);

						std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
						std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

						FILE* file = fopen(filePathName.c_str(), "r");
						if (file)
						{
							//ZeroMemory(text, IM_ARRAYSIZE(text));
							size_t lengthRead = 0;
							size_t currentRead;
							while ((currentRead = fread(text + lengthRead, 1, 4096, file)) > 0)
							{
								lengthRead += currentRead;
							}
							fclose(file);
							ZeroMemory(text+ lengthRead, IM_ARRAYSIZE(text)- lengthRead);

							s_files.push_back({filePathName, "", "", true});
							s_currentFileIndex = s_files.size() - 1;
							s_files[s_currentFileIndex].name = filePathName.substr(filePath.size() + 1);
							s_files[s_currentFileIndex].text = std::string(text);
						}
					}
					// close
					ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
				}
				ImGui::SameLine();
				ImGui::SmallButton("       ");
				for (int i = 0; i < s_files.size(); ++i)
				{
					auto& file = s_files[i];
					ImGui::SameLine();
					std::string title = file.name;
					bool selected = i == s_currentFileIndex;
					if (selected)
					{
						title = "[" + title + "]";
						ImGui::PushStyleColor(ImGuiCol_Button, {0.f, 1.f, 0.f, 1.f});
						ImGui::PushStyleColor(ImGuiCol_Text, {0.f, 0.f, 0.f, 1.f});
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.f, 1.f, 0.f, 1.f});
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.f, 1.f, 0.f, 1.f});
					}
					if (ImGui::SmallButton(title.c_str()))
					{
						if (i != s_currentFileIndex)
						{
							int length = strlen(text);
							s_files[s_currentFileIndex].text = std::string(text);
							s_currentFileIndex = i;
							ZeroMemory(text, IM_ARRAYSIZE(text));
							memcpy(&text, s_files[s_currentFileIndex].text.c_str(),
							       strlen(s_files[s_currentFileIndex].text.c_str()));
						}
					}
					if (selected)
					{
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
					}
				}
				static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
				//ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", (unsigned int*)&flags, ImGuiInputTextFlags_ReadOnly);
				//ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", (unsigned int*)&flags, ImGuiInputTextFlags_AllowTabInput);
				//ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", (unsigned int*)&flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
				if (ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text),
				                              ImVec2(-1.0f, -1.f), flags))
					s_files[s_currentFileIndex].saved = false;
			}
			ImGui::End();
		}
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Lua Console", p_open, ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}

		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		// Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font size.
		
		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::Button("Script Editor"))
			{
				show_script_editor = !show_script_editor;
			}
			/*if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("Script editor", NULL, &show_script_editor);

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) //needs to be here otherwise no menu will be visible
			{
				ImGui::EndMenu();
			}*/
			ImGui::EndMenuBar();
		}

		/*if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("Script editor", NULL, &show_script_editor);
				ImGui::MenuItem("smth else", NULL, &show_script_editor);
				ImGui::MenuItem("Script e45ditor", NULL, &show_script_editor);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}*/

		// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
		// Here we create a context menu only available from the title bar.
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Close Console"))
				*p_open = false;
			ImGui::EndPopup();
		}

		//ImGui::TextWrapped("Lua Engine Interface");

		//ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

		// TODO: display items starting from the bottom

		if (ImGui::SmallButton("Clear")) { ClearLog(); }
		ImGui::SameLine();
		bool copy_to_clipboard = ImGui::SmallButton("Copy");
		ImGui::SameLine();
		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;

		ImGui::Separator();

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			if (ImGui::Checkbox("Auto-scroll", &AutoScroll))
				if (AutoScroll)
					ScrollToBottom = true;
			ImGui::EndPopup();
		}
		/*
		// Options, Filter
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();*/

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		// 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
		                  ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		if (copy_to_clipboard)
			ImGui::LogToClipboard();
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!Filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			bool removeLabel = false;
			if (strstr(item, LUACON_ERROR_PREF))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.3f, 0.3f, 1.f));
				pop_color = true;
			}
			if (strstr(item, LUACON_TRACE_PREF))
			{
				removeLabel = true;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
				pop_color = true;
			}
			if (strstr(item, LUACON_INFO_PREF))
			{
				removeLabel = true;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.5f, 0.f, 1.f));
				pop_color = true;
			}
			if (strstr(item, LUACON_WARN_PREF))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.65f, 0.f, 1.f));
				pop_color = true;
			}
			else if (strncmp(item, ">> ", 3) == 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
				pop_color = true;
			}
			ImGui::TextUnformatted(item + (removeLabel ? +7 : 0));
			if (pop_color)
				ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
			ImGui::LogFinish();
		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf),
		                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion |
		                     ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy(s, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget


		ImGui::End();
	}

	void ExecCommand(const char* command_line)
	{
		AddLog(">> %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));
		hasNewCommandPending = true;
		/*
				// Process command
				if (Stricmp(command_line, "CLEAR") == 0)
				{
					ClearLog();
				}
				else if (Stricmp(command_line, "HELP") == 0)
				{
					AddLog("Commands:");
					for (int i = 0; i < Commands.Size; i++)
						AddLog("- %s", Commands[i]);
				}
				else if (Stricmp(command_line, "HISTORY") == 0)
				{
					int first = History.Size - 10;
					for (int i = first > 0 ? first : 0; i < History.Size; i++)
						AddLog("%3d: %s\n", i, History[i]);
				}
				else
				{
					AddLog("Unknown command: '%s'\n", command_line);
				}
				*/
		// On commad input, we scroll to bottom even if AutoScroll==false
		ScrollToBottom = true;
	}

	const char* retrieveLastCommand()
	{
		hasNewCommandPending = false;
		return History[History.size() - 1];
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
	// In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		LuaConsole* console = (LuaConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			{
				// Example of TEXT COMPLETION

				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf)
				{
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				// Build a list of candidates
				ImVector<const char*> candidates;
				for (int i = 0; i < Commands.Size; i++)
					if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
						candidates.push_back(Commands[i]);

				if (candidates.Size == 0)
				{
					// No match
					AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
				}
				else if (candidates.Size == 1)
				{
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]);
					data->InsertChars(data->CursorPos, " ");
				}
				else
				{
					// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
					int match_len = (int)(word_end - word_start);
					for (;;)
					{
						int c = 0;
						bool all_candidates_matches = true;
						for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0)
					{
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
					}

					// List matches
					AddLog("Possible matches:\n");
					for (int i = 0; i < candidates.Size; i++)
						AddLog("- %s\n", candidates[i]);
				}

				break;
			}
		case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = HistoryPos;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (HistoryPos == -1)
						HistoryPos = History.Size - 1;
					else if (HistoryPos > 0)
						HistoryPos--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (HistoryPos != -1)
						if (++HistoryPos >= History.Size)
							HistoryPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != HistoryPos)
				{
					const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
		}
		return 0;
	}
};

extern "C" {
int luaopen_customodul(lua_State* L); // declare the wrapped module
}

#define LUA_EXTRALIBS {"customodul",luaopen_customodul},

static LuaLayer* s_lua_layer;

static int nd_info(lua_State* L)
{
	auto string = lua_tostring(L, 1);
	ND_INFO(string);
	s_lua_layer->printToLuaConsole(L, (nd::temp_string(LUACON_INFO_PREF) + string).c_str());
	return 0;
}

static int nd_trace(lua_State* L)
{
	auto string = lua_tostring(L, 1);
	ND_TRACE(string);
	s_lua_layer->printToLuaConsole(L, (nd::temp_string(LUACON_TRACE_PREF) + string).c_str());
	return 0;
}

static int nd_error(lua_State* L)
{
	auto string = lua_tostring(L, 1);
	ND_ERROR(string);
	s_lua_layer->printToLuaConsole(L, (nd::temp_string(LUACON_ERROR_PREF) + string).c_str());
	return 0;
}

static int nd_warn(lua_State* L)
{
	auto string = lua_tostring(L, 1);
	ND_WARN(string);
	s_lua_layer->printToLuaConsole(L, (nd::temp_string(LUACON_WARN_PREF) + string).c_str());
	return 0;
}

static int catch_panic(lua_State* L)
{
	auto message = lua_tostring(L, -1);
	ND_ERROR("LUA PANIC ERROR: {}", message);
	return 0;
}

static int nd_run_lua_file_script(lua_State* L)
{
	auto string = lua_tostring(L, 1);
	s_lua_layer->runScriptFromFile(L, string);
	return 0;
}

static int nd_exit(lua_State* L)
{
	s_lua_layer->closeConsole();
	return 0;
}

static int vec2_to_string(lua_State* L)
{
	s_lua_layer->closeConsole();
	return 0;
}

//returns table with pairs of path and isDirectory
static int ls(lua_State* L)
{
	if (!lua_isstring(L, 1))
	{
		ND_WARN("invalid lua argument");
		return 0;
	}
	auto string = lua_tostring(L, 1);
	if (!std::filesystem::is_directory(string))
	{
		ND_WARN("{} is not directory",string);
		return 0;
	}

	lua_newtable(L);
	int i = 0;
	for (auto& p : std::filesystem::directory_iterator(string))
	{
		lua_pushnumber(L, i + 1); // parent table index
		lua_newtable(L);
		lua_pushstring(L, "path");
		lua_pushstring(L, p.path().generic_string().c_str());
		lua_settable(L, -3);
		lua_pushstring(L, "isDirectory");
		lua_pushboolean(L, p.is_directory());
		lua_settable(L, -3);
		lua_settable(L, -3);
		i++;
	}
	return 1;
}


void LuaLayer::print_error(lua_State* state)
{
	// The error message is on top of the stack.
	// Fetch it, print it and then pop it off the stack.
	const char* message = lua_tostring(state, -1);
	printToLuaConsole(state, (nd::temp_string(LUACON_ERROR_PREF) + message).c_str());
	ND_ERROR("LUA ERROR:\n {}", message);
	lua_pop(state, 1);
}

class A
{
protected:
	std::string name;

public:
	A(std::string s)
	{
		this->name = s;
	}

	std::string getName()
	{
		return this->name;
	}

	void printName()
	{
		printf("Hello, my name is %s!\n", this->name.c_str());
	}
};

static glm::vec2 addVec2(glm::vec2& a, glm::vec2 b)
{
	return a + b;
}

static void consumeVector(glm::vec2& vec)
{
	ND_INFO("we have a vec2 {},{}", vec.x, vec.y);
}

static glm::vec3 ad(const glm::vec3& a, const glm::vec3& b)
{
	return glm::vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static int playSound(lua_State* L)
{
	auto numberOfArgs = lua_gettop(L);
	float volume = 1;
	float pitch = 1;
	if (numberOfArgs == 0)
	{
		ND_ERROR("Lua error invaliad number of args");
		return 0;
	}
	const char* c = lua_tostring(L, 1);
	if (numberOfArgs > 1)
		volume = lua_tonumber(L, 2);
	if (numberOfArgs > 2)
		pitch = lua_tonumber(L, 3);
	Sounder::get().playSound(c, volume, pitch);

	return 0;
}

static int playMusic(lua_State* L)
{
	auto numberOfArgs = lua_gettop(L);
	float volume = 1;
	float pitch = 1;
	if (numberOfArgs == 0)
	{
		ND_ERROR("Lua error invaliad number of args");
		return 0;
	}
	const char* c = lua_tostring(L, 1);
	if (numberOfArgs > 1)
		volume = lua_tonumber(L, 2);
	if (numberOfArgs > 2)
		pitch = lua_tonumber(L, 3);
	Sounder::get().playMusic(c, volume, pitch);

	return 0;
}

static std::string s_couroutineFileSrc;
void LuaLayer::onAttach()
{
	//settings save and load
	{
		ZeroMemory(text, IM_ARRAYSIZE(text));
		s_settings = NBT::create();
		BasicIStream ss = ND_BASICISTREAM_IN(s_settings_file);
		if (ss.isOpen())
		{
			
			s_settings->deserialize(&ss);
			s_currentFileIndex = s_settings->get("current_file_index",(int)0);
			for (int i = 0; i < s_settings->get("files_size", 0); ++i)
			{
				s_files.emplace_back();
				auto& t = s_files[s_files.size() - 1];
				t.filePath = s_settings->get<std::string>("filepath" + nd::to_string(i));
				t.name = s_settings->get<std::string>("name" + nd::to_string(i));
				t.saved = s_settings->get<bool>("saved" + nd::to_string(i));
				t.text = s_settings->get<std::string>("text" + nd::to_string(i));
				if(i==s_currentFileIndex)
				{
					ZeroMemory(text, IM_ARRAYSIZE(text));
					memcpy(text, t.text.c_str(), t.text.size()+1);
				}
			}
		}
	}
	
	s_lua_layer = this;
	m_console = new LuaConsole(this);


	m_L = luaL_newstate();
	luaopen_base(m_L); // load basic libs (eg. print)
	luaL_openlibs(m_L);
	luaopen_customodul(m_L);


	lua_atpanic(m_L, catch_panic);
	lua_register(m_L, "ND_TRACE", nd_trace);
	lua_register(m_L, "ND_INFO", nd_info);
	lua_register(m_L, "ND_WARN", nd_warn);
	lua_register(m_L, "ND_ERROR", nd_error);
	lua_register(m_L, "ND_RUN_FILE", nd_run_lua_file_script);
	lua_register(m_L, "runf", nd_run_lua_file_script);
	lua_register(m_L, "exit", nd_exit);
	lua_register(m_L, "playSound", playSound);
	lua_register(m_L, "playMusic", playMusic);
	lua_register(m_L, "ls", ls);

	loadGlmVec();
	loadSoundHandle();
	

	s_couroutineFileSrc = readStringFromFile(ND_RESLOC("res/lua/coroutines.lua").c_str());
	runScriptFromFile(m_L, "res/lua/startup.lua");
	
	
}

void LuaLayer::onDetach()
{
	BasicIStream ss = ND_BASICISTREAM_OUT(s_settings_file);
	s_settings->clear();
	if (ss.isOpen())
	{
		if(s_currentFileIndex!=-1)
		s_files[s_currentFileIndex].text = std::string(text);
		s_settings->set("files_size", (int)s_files.size());
		s_settings->set("current_file_index", s_currentFileIndex);
		for (int i = 0; i < s_files.size(); ++i)
		{
			auto& file = s_files[i];
			s_settings->set("filepath" + nd::to_string(i), file.filePath);
			s_settings->set("name" + nd::to_string(i), file.name);
			s_settings->set("saved" + nd::to_string(i), file.saved);
			s_settings->set("text" + nd::to_string(i), file.text);
		}
		
		s_settings->serialize(&ss);
	}
	else
		ND_WARN("cannot save lua settings");

	lua_close(m_L);
	delete m_console;
}

void LuaLayer::onImGuiRender()
{
	if (m_show_imgui_console)
		m_console->Draw(&m_show_imgui_console);
}

void LuaLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto m = dynamic_cast<KeyPressEvent&>(e);
		if (m.getKey() == GLFW_KEY_L && m.isControlPressed())
		{
			m_show_imgui_console = !m_show_imgui_console;
		}
	}
}

void LuaLayer::printToLuaConsole(lua_State* L, const char* c)
{
	if (m_show_imgui_console)
		m_console->AddLog("%s\n", c);
}

void LuaLayer::runScriptInConsole(lua_State* L, const char* c)
{
	luaL_loadstring(m_L, c);
	auto result = lua_pcall(m_L, 0, LUA_MULTRET, 0);

	if (result != LUA_OK)
	{
		print_error(m_L);
		return;
	}
}

void LuaLayer::runScriptFromFile(lua_State* L, const nd::temp_string& filePath)
{
	int result = luaL_loadfile(m_L, ND_RESLOC(std::string(filePath)).c_str());
	if (result != LUA_OK)
	{
		print_error(m_L);
		return;
	}
	result = lua_pcall(m_L, 0, LUA_MULTRET, 0);

	if (result != LUA_OK)
	{
		print_error(m_L);
	}
}

void LuaLayer::loadGlmVec()
{
	luabridge::getGlobalNamespace(m_L)
		.beginClass<glm::vec2>("glmvec2")
		.addConstructor<void(*)(float, float)>()
		.addData("x", &glm::vec2::x)
		.addData("y", &glm::vec2::y)
		.endClass();

	luabridge::getGlobalNamespace(m_L)
		.beginClass<glm::vec3>("glmvec3")
		.addConstructor<void(*)(float, float, float)>()
		.addData("x", &glm::vec3::x)
		.addData("y", &glm::vec3::y)
		.addData("z", &glm::vec3::z)
		.endClass();
	luabridge::getGlobalNamespace(m_L)
		.beginClass<glm::vec4>("glmvec4")
		.addConstructor<void(*)(float, float, float, float)>()
		.addData("x", &glm::vec4::x)
		.addData("y", &glm::vec4::y)
		.addData("z", &glm::vec4::z)
		.addData("w", &glm::vec4::w)
		.endClass();
}

void LuaLayer::loadSoundHandle()
{
	luabridge::getGlobalNamespace(m_L)
		.beginClass<SoundHandle>("Sound")
		.addConstructor<void(*)()>()
		.addFunction("open",&SoundHandle::open)
		.addFunction("play",&SoundHandle::play)
		.addFunction("pause",&SoundHandle::pause)
		.addFunction("stop",&SoundHandle::stop)
		.addFunction("setLoop",&SoundHandle::setLoop)
		.addFunction("l",&SoundHandle::setLoop)
		.addFunction("setPitch",&SoundHandle::setPitch)
		.addFunction("p",&SoundHandle::setPitch)
		.addFunction("setVolume",&SoundHandle::setVolume)
		.addFunction("v",&SoundHandle::setVolume)
		.addFunction("isPlaying",&SoundHandle::isPlaying)
		.endClass();
	
	luabridge::getGlobalNamespace(m_L)
		.beginClass<MusicHandle>("Music")
		.addConstructor<void(*)()>()
		.addFunction("open", &MusicHandle::open)
		.addFunction("play", &MusicHandle::play)
		.addFunction("pause", &MusicHandle::pause)
		.addFunction("stop", &MusicHandle::stop)
		.addFunction("setLoop", &MusicHandle::setLoop)
		.addFunction("l", &MusicHandle::setLoop)
		.addFunction("setPitch", &MusicHandle::setPitch)
		.addFunction("p", &MusicHandle::setPitch)
		.addFunction("setVolume", &MusicHandle::setVolume)
		.addFunction("v", &MusicHandle::setVolume)
		.addFunction("isPlaying", &MusicHandle::isPlaying)
		.endClass();
}

void LuaLayer::onUpdate()
{
	if (m_console->hasNewCommandPending)
	{
		auto command = m_console->retrieveLastCommand();
		luaL_loadstring(m_L, command);
		auto result = lua_pcall(m_L, 0, LUA_MULTRET, 0);

		if (result != LUA_OK)
		{
			print_error(m_L);
			return;
		}
	}
	//call coroutes
	luaL_loadstring(m_L, s_couroutineFileSrc.c_str());
	auto result = lua_pcall(m_L, 0, LUA_MULTRET, 0);

	if (result != LUA_OK)
	{
		print_error(m_L);
		return;
	}
}
