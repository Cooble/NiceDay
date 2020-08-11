#include "components_imgui_access.h"
#include <imgui.h>
#include "imgui_internal.h"
#include "files/FUtil.h"
#include <shellapi.h>
#include "Atelier.h"
#include <sol/sol.hpp>
#include "Colli.h"
#include "Mesh.h"

namespace components_imgui_access
{
	static void drawTexOrNo(MaterialPtr& c,int width,int height)
	{
		static auto no = TextureLib::loadOrGetTexture("res/images/no.png")->getID();
		ImGui::Image((ImTextureID)(c ? Atelier::get().getPhoto(c)->getID() : no), { (float)width,(float)height }, { 0,1 }, { 1,0 });
	}
	static void drawTexOrNo(MeshPtr& c, int width, int height)
	{
		static auto no = TextureLib::loadOrGetTexture("res/images/no.png")->getID();
		ImGui::Image((ImTextureID)(c ? Atelier::get().getPhoto(c)->getID() : no), { (float)width,(float)height }, { 0,1 }, { 1,0 });
	}
	static void makeDragDropSource(MeshPtr& ptr)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None | ImGuiDragDropFlags_SourceAllowNullID))
		{
			// Set payload to carry the index of our item (could be anything)
			auto id = ptr->getID();
			ImGui::SetDragDropPayload("MESH", &id, sizeof(Strid));

			// Display preview (could be anything, e.g. when dragging an image we could decide to display
			// the filename and a small preview of the image, etc.)
			ImGui::Text(ptr->getName().c_str());
			ImGui::SameLine();
			ImGui::Image((ImTextureID)Atelier::get().getPhoto(ptr)->getID(), { 32, 32 }, { 0, 1 }, { 1, 0 });
			ImGui::EndDragDropSource();
		}
	}
	static void makeDragDropTarget(MeshPtr& ptr)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH"))
			{
				IM_ASSERT(payload->DataSize == sizeof(Strid));
				Strid payload_n = *(Strid*)payload->Data;
				ptr = MeshFactory::get(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
	static void makeDragDropSource(MaterialPtr& ptr)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None | ImGuiDragDropFlags_SourceAllowNullID))
		{
			// Set payload to carry the index of our item (could be anything)
			auto id = ptr->getID();
			ImGui::SetDragDropPayload("MATERIAL", &id, sizeof(Strid));

			// Display preview (could be anything, e.g. when dragging an image we could decide to display
			// the filename and a small preview of the image, etc.)
			ImGui::Text(ptr->getName().c_str());
			ImGui::SameLine();
			ImGui::Image((ImTextureID)Atelier::get().getPhoto(ptr)->getID(), { 32, 32 }, { 0, 1 }, { 1, 0 });
			ImGui::EndDragDropSource();
		}
	}
	static void makeDragDropTarget(MaterialPtr& ptr)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL"))
			{
				IM_ASSERT(payload->DataSize == sizeof(Strid));
				Strid payload_n = *(Strid*)payload->Data;
				ptr = MaterialLibrary::get(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
	
	SceneWindows windows = SceneWindows();

	void draw(Entity e, LightComponent& c)
	{
		ImGui::ColorEdit3("ambient", (float*)&c.ambient);
		ImGui::ColorEdit3("diffuse", (float*)&c.diffuse);
		ImGui::ColorEdit3("specular", (float*)&c.specular);
		ImGui::Spacing();
		ImGui::SliderFloat("constant", &c.constant, 0.01, 1);
		ImGui::SliderFloat("linear", &c.linear, 0.01, 1);
		ImGui::SliderFloat("quadratic", &c.quadratic, 0.01, 1);
	}

	void draw(Entity e, TransformComponent& c)
	{
		ImGui::DragFloat3("pos", (float*)&c.pos, 0.05f);
		ImGui::DragFloat3("scale", (float*)&c.scale, 0.05f);
		ImGui::DragFloat3("rotation", (float*)&c.rot, 0.05f, -glm::two_pi<float>(), glm::two_pi<float>());
		c.recomputeMatrix();
	}

	void draw(Entity e, ModelComponent& c)
	{
		//=========================================
		ImGui::Text("MATERIAL");
		
		
		drawTexOrNo(c.material, AtelierDim::width, AtelierDim::height);
		if (c.material && ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			windows.material = c.material;
			windows.open_material = true;
		}
		if (c.material)
			makeDragDropSource(c.material);
		makeDragDropTarget(c.material);


		if(c.material)
			ImGui::TextColored({ 0, 1, 1, 1 } ,c.material->getName().c_str());
		else ImGui::TextColored({ 1, 0, 0, 1 }, "No Material Bound");

		//=========================================
		ImGui::Text("MESH");
	
		drawTexOrNo(c.mesh, AtelierDim::width, AtelierDim::height);
		makeDragDropTarget(c.mesh);
		if (c.mesh)
			makeDragDropSource(c.mesh);
		if (c.mesh)
			ImGui::TextColored({ 0, 1, 1, 1 }, c.mesh->getName().c_str());
		else ImGui::TextColored({ 1, 0, 0, 1 }, "No Mesh Bound");
	}

	void draw(Entity e, CameraComponent& c)
	{
		auto angle = glm::degrees(c.fov);
		if (ImGui::SliderFloat("Fov", &angle, 10, 180))
			c.fov = glm::radians(angle);
		ImGui::SliderFloat("Near", &c.Near, 0.5f, 100.f);
		ImGui::SliderFloat("Far", &c.Far, 0.5f, 100.f);
	}

	bool drawWindow(Mesh& c)
	{
		return true;
	}

	static Strid fileComboLastID = 0;

	static std::string fileCombo(const std::string& currentValue, const std::string& sourceFolder, const char** suffix,size_t suffixLength,
		StringId calledID)
	{
		ImVec4 col = std::filesystem::exists(ND_RESLOC(currentValue)) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0.2, 0.2, 1);
		ImGui::TextColored(col, currentValue.c_str());

		std::vector<std::string> files;

		if (!std::filesystem::exists(ND_RESLOC(sourceFolder)))
			return currentValue;
		for (auto& file : std::filesystem::recursive_directory_iterator(ND_RESLOC(sourceFolder)))
			for (int i = 0; i < suffixLength; ++i)
			{
				if (SUtil::endsWith(file.path().string(), suffix[i]))
				{
					files.push_back(ResourceMan::getLocalPath(file.path().string()));
					break;
				}
			}
			
		static ImGuiTextFilter filter;
		if (calledID() != fileComboLastID)
		{
			fileComboLastID = calledID();
			if (currentValue.size())
				memcpy((void*)filter.InputBuf, currentValue.c_str(), currentValue.size() + 1);
			else filter.InputBuf[0] = 0;
		}
		{
			ImVec4 col = std::filesystem::exists(ND_RESLOC(std::string(filter.InputBuf)))
				? ImVec4(0, 1, 0, 1)
				: ImVec4(1, 0.2, 0.2, 1);
			static bool changedeeed = false;
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			bool entered = ImGui::InputText("Filter (inc,-exc)", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf),
				ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_CallbackCharFilter,
				[](ImGuiInputTextCallbackData* data)
				{
					changedeeed = true;
					return 0;
				});
			ImGui::PopStyleColor();

			if (changedeeed)
				filter.Build();
			changedeeed = false;
			if (entered && std::filesystem::exists(ND_RESLOC(std::string(filter.InputBuf))))
				return std::string(filter.InputBuf);
			if (entered && strlen(filter.InputBuf) == 0)
				return "";
		}

		if (strlen(filter.InputBuf) != 0)
			for (int i = 0; i < files.size(); i++)
				//if (filter.PassFilter(files[i].c_str()))
				if (files[i].find(filter.InputBuf) != std::string::npos)
					if (ImGui::Selectable(files[i].c_str(), false))
					{
						memcpy(filter.InputBuf, files[i].c_str(), files[i].size() + 1);
						return files[i];
					}
		return currentValue;
	}

	static std::string shaderCombo(const std::string& currentCombo)
	{
		// Using the generic BeginCombo() API, you have full control over how to display the combo contents.
		// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
		// stored in the object itself, etc.)
		std::vector<std::string> shaders;
		if (!std::filesystem::exists(ResourceMan::getResPath() + "/shaders"))
			return currentCombo;
		for (auto& file : std::filesystem::recursive_directory_iterator(ResourceMan::getResPath() + "/shaders"))
		{
			if (SUtil::endsWith(file.path().string(),".shader"))
			{
				shaders.push_back(ResourceMan::getLocalPath(file.path().string()));
			}
		}
		/*std::vector<const char*> shaderss;
		shaderss.resize(shaders.size());
		for (int i = 0; i < shaders.size(); ++i)
			shaderss[i] = shaders[i].c_str();*/

		int item_current_idx = 0; // Here our selection data is an index.
		for (int i = 0; i < shaders.size(); ++i)
		{
			if (shaders[i] == currentCombo)
			{
				item_current_idx = i;
				break;
			}
		}
		if (ImGui::BeginCombo("Shader ", shaders[item_current_idx].c_str()))
		{
			for (int n = 0; n < shaders.size(); n++)
			{
				const bool is_selected = (item_current_idx == n);
				if (ImGui::Selectable(shaders[n].c_str(), is_selected))
				{
					item_current_idx = n;
				}
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		auto path = ND_RESLOC(shaders[item_current_idx]);
		if (std::filesystem::exists(path) && ImGui::Button("Open"))
		{
			SUtil::replaceWith(path, '/', '\\');
			ShellExecute(0, 0, path.c_str(), 0, 0, SW_SHOW);
		}
		return shaders[item_current_idx];
	}

	static void drawShaderStats(const ShaderPtr& shader)
	{
		auto& layout = shader->getLayout();
		auto layoutVertex = layout.getLayoutByName("VERTEX");

		auto layoutMat = layout.getLayoutByName("MAT");
		auto layoutGlo = layout.getLayoutByName("GLO");
		//ImGui::TextColored({ 0.3,0.3,1,1 }, shader->getFilePath().substr(shader->getFilePath().find_last_of("/") + 1).c_str());
		if (layoutMat) {
			ImGui::Text("MATERIAL");
			ImGui::Separator();
			ImGui::Columns(2, 0, false);
			ImGui::SetColumnWidth(-10, 100);
			int i = 0;
			for (auto& val : layoutMat->elements)
			{
				ImGui::Text(GTypes::getName(val.type)); ImGui::NextColumn();
				if (val.arraySize == 1)
					ImGui::Text(val.name.c_str());
				else
					ImGui::Text("%s[%i]", val.name.c_str(), val.arraySize);

				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
		if (layoutVertex) {
			ImGui::NewLine();
			ImGui::Text("VERTEX");
			ImGui::Separator();
			ImGui::Columns(3, 0, false);
			ImGui::SetColumnWidth(0, 20);
			ImGui::SetColumnWidth(1, 80);
			int i = 0;
			for (auto& val : layoutVertex->elements)
			{
				ImGui::Text("%i", i++); ImGui::NextColumn();
				ImGui::Text(GTypes::getName(val.type)); ImGui::NextColumn();
				ImGui::Text(val.name.c_str()); ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
		if (layoutGlo) {
			ImGui::NewLine();
			ImGui::Text("GLOBALS");
			ImGui::Separator();
			ImGui::Columns(2, 0, false);
			ImGui::SetColumnWidth(-1, 100);
			int i = 0;
			for (auto& val : layoutGlo->elements)
			{
				ImGui::Text(GTypes::getName(val.type)); ImGui::NextColumn();
				if (val.arraySize == 1)
					ImGui::Text(val.name.c_str());
				else
					ImGui::Text("%s[%i]", val.name.c_str(), val.arraySize);

				ImGui::NextColumn();
			}
		}

	}

	static std::string textureCombo(const std::string& currentCombo)
	{
		auto id = std::filesystem::exists(ND_RESLOC(currentCombo))
			? TextureLib::loadOrGetTexture(currentCombo)->getID()
			: TextureLib::loadOrGetTexture("res/images/no.png")->getID();
		if (id)
			ImGui::Image((ImTextureID)id, { AtelierDim::width,AtelierDim::height }, { 0, 1 }, { 1, 0 });
		//else ImGui::TextColored({ 1,0,0,1 }, "Image %s not found", currentCombo.c_str());

		char c[5];
		*(uint32_t*)&c[0] = ImGui::GetActiveID();
		c[4] = 0;

		auto s = ".png";
		auto ret = fileCombo(currentCombo, "res",&s, 1, StringId("textureCombo")/*.concat(c)*/);

		return ret;
	}


	// opens and begins a popup to request user input string, callback will be called when enter is hit and entered string is not empty
	static void drawStringDialog(bool open, const char* popupName, const char* fieldName, const std::function<void(const char*)>& callBack)
	{
		//static Strid lastPopupName = 0;
		static char buff[256];

		if (open) {
			ImGui::OpenPopup(popupName);
			buff[0] = 0;
		}

		/*if(lastPopupName!=SID(popupName))
		{
			buff[0] = 0;
			lastPopupName = SID(popupName);
		}*/

		if (ImGui::BeginPopup(popupName))
		{
			if (fieldName)
				ImGui::Text(fieldName);

			ImGui::SetKeyboardFocusHere(0);
			if (ImGui::InputText(fieldName, buff, IM_ARRAYSIZE(buff), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (buff[0])
					callBack(buff);
				buff[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	// opens and begins a popup to request user input file, callback will be called when enter is hit and entered string is not empty
	static void drawFileDialog(bool open, const char* popupName, const char* fieldName, const std::string& sourceFolder, const char** suffixes,int suffixLength, bool forbidCustomPath,const std::function<void(const char*)>& callBack)
	{
		static std::string strin;
		int focus = 0;
		if (open) {
			ImGui::OpenPopup(popupName);
			strin = "";
			focus = 5;
		}

		if (ImGui::BeginPopup(popupName))
		{
			if (fieldName)
				ImGui::Text(fieldName);

			//if(focus-- == 0)
			//	ImGui::SetKeyboardFocusHere(0);
			strin = fileCombo(strin, sourceFolder, suffixes, suffixLength, popupName);
			if(!strin.empty())
			{
				callBack(strin.c_str());
				ImGui::CloseCurrentPopup();
				strin = "";
			}
			ImGui::EndPopup();
		}
	}

#define PROP_DRAW(imguiFunc,Type,imguiType)\
	{\
	auto ptr = (Type*)c->getPointerToValueFullName(element.name.c_str());\
	if (element.arraySize == 1)\
	{\
		change |= ImGui::imguiFunc(element.name.c_str(), (imguiType*)ptr);\
	}\
	else\
		for (int i = 0; i < element.arraySize; ++i)\
			change |= ImGui::imguiFunc((element.name + "[" + std::to_string(i) + "]").c_str(), (imguiType*)(ptr + i));\
	}
	static Material* lastmaterial = nullptr;

	bool drawWindow(MaterialPtr& c)
	{
		static char buff[256];
		static bool editNameActive = false;
		static bool editNameFirstFocus = false;
		static int selectedElement = -1;


		//todo use splitterBehavior instead
		bool keepOpen = true;
		bool change = false;
		static ImGuiID dock_left_collumn = 0;
		static ImGuiID dock_right = 0;
		if (lastmaterial != c.get())
		{
			lastmaterial = c.get();
			change = true; //rerender
			memcpy(buff, c->getName().c_str(), c->getName().size() + 1);
			editNameActive = false;
			editNameFirstFocus = false;
			selectedElement = 0;
		}

		//ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ 900,500 }, ImGuiCond_Once);
		ImGui::Begin("Material Editor", &keepOpen/*, ImGuiWindowFlags_NoDocking*/);

		if (!keepOpen)
		{
			ImGui::End();
			dock_left_collumn = 0;
			return false;
		}

		ImGuiID dockspace_id = ImGui::GetID("MaterialDock");

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);

		if (dock_left_collumn == 0)
		{
			ImGui::DockBuilderRemoveNodeChildNodes(dockspace_id);
			ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.32f, &dock_left_collumn, &dock_right);

			ImGui::DockBuilderDockWindow("RightMaterial", dock_right);
			ImGui::DockBuilderDockWindow("LeftMaterial", dock_left_collumn);

			ImGui::DockBuilderGetNode(dock_right)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			ImGui::DockBuilderGetNode(dock_left_collumn)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

			ImGui::DockBuilderFinish(dockspace_id);
		}
		ImGui::End();
		ImGui::Begin("LeftMaterial");

		ImGui::Image((ImTextureID)Atelier::get().getPhoto(c)->getID(), { AtelierDim::width,AtelierDim::height }, { 0, 1 }, { 1, 0 });
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup("materialPop");
		if (ImGui::BeginPopup("materialPop"))
		{
			if (ImGui::MenuItem("Save Material"))
			{
				MaterialLibrary::save(c, std::string("res/models/") + c->getName() + ".mat");
			}
			ImGui::EndPopup();
		}

		static int wasFirst = 0;
		if (editNameActive)
		{
			if (editNameFirstFocus) {
				ImGui::SetKeyboardFocusHere(0);
				editNameFirstFocus = false;
				wasFirst = 2;

			}
			bool entered = ImGui::InputText("Rename", buff, IM_ARRAYSIZE(buff), ImGuiInputTextFlags_EnterReturnsTrue);
			if (!ImGui::IsItemActive() && !wasFirst)
				editNameActive = false;
			if (wasFirst)wasFirst--;
			if (entered)
			{
				if (strlen(buff) == 0)
					memcpy(buff, c->getName().c_str(), c->getName().size() + 1);
				else
					c->setName(buff);
				editNameActive = false;
			}

		}
		else {
			ImGui::TextColored({ 0, 1, 0, 1 }, c->getName().c_str());
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				editNameActive = true;
				editNameFirstFocus = true;
			}

		}


		ImGui::Separator();

		const int shaderId = -2;
		for (int i = 0; i < c->getLayout().elements.size(); ++i)
		{
			auto& element = c->getLayout().elements[i];
			bool s = i == selectedElement;
			ImGui::Selectable(c->toShortNameString(element.name).c_str(), &s);
			if (s)
				selectedElement = i;
		}
		bool b = selectedElement == shaderId;;
		ImGui::Selectable("Shader", &b);
		if (b)
			selectedElement = shaderId;
		ImGui::End();

		//ImGui::SetNextWindowDockID(dock_right, ImGuiCond_Always);
		ImGui::Begin("RightMaterial");
		static bool invalidShader = false;
		static bool noAPI = false;
		static std::string shaderP;
		if (selectedElement == shaderId)
		{
			auto newValue = shaderCombo(c->getShader()->getFilePath());
			if (newValue != c->getShader()->getFilePath() || ImGui::Button("Reload"))
			{
				try
				{
					auto p = Shader::create(newValue);
					if (!p->getLayout().structs.empty())
					{
						c->setShader(p);
						change = true;
						noAPI = false;
					}
					else
					{
						noAPI = true;
						shaderP = newValue;
					}
					invalidShader = false;
				}
				catch (...)
				{
					shaderP = newValue;
					invalidShader = true;
				}
			}
			if (ImGui::Button("Open folder")) {
				std::string s = ResourceMan::getResPath();
				s = "explorer " + s + "/shaders";
				SUtil::replaceWith(s, '/', '\\');
				system(s.c_str());
			}
			if (invalidShader)
				ImGui::TextColored({ 1, 0, 0, 1 }, "Error parsing shader %s", shaderP.c_str());
			else if (noAPI)
				ImGui::TextColored({ 1, 0.2f, 0, 1 }, "Shader does not have proper layout: %s", shaderP.c_str());

			if (c->getShader())
			{
				ImGui::NewLine();
				drawShaderStats(c->getShader());
			}
		}
		else if (selectedElement != -1)
		{
			auto& element = c->getLayout().elements[selectedElement];
			ImGui::PushID(element.name.c_str());
			switch (element.type)
			{
			case g_typ::UNSIGNED_INT:
				PROP_DRAW(InputInt, uint32_t, int);
				break;
			case g_typ::FLOAT:
				PROP_DRAW(InputFloat, float, float);
				break;
			case g_typ::VEC2:
				PROP_DRAW(InputFloat2, glm::vec2, float);
				break;
			case g_typ::VEC3:
				PROP_DRAW(InputFloat3, glm::vec3, float);
				ImGui::Separator();
				PROP_DRAW(ColorEdit3, glm::vec3, float);
				break;
			case g_typ::VEC4:
				PROP_DRAW(InputFloat4, glm::vec4, float);
				ImGui::Separator();
				PROP_DRAW(ColorEdit4, glm::vec4, float);
				break;
			case g_typ::INT:
				PROP_DRAW(InputInt, int, int);
				break;
			case g_typ::IVEC2:
				PROP_DRAW(InputInt2, glm::ivec2, int);
				break;
			case g_typ::IVEC3:
				PROP_DRAW(InputInt3, glm::ivec3, int);
				break;
			case g_typ::IVEC4:
				PROP_DRAW(InputInt4, glm::ivec4, int);
				break;
			case g_typ::TEXTURE_CUBE:
			case g_typ::TEXTURE_2D:
			{
				auto& texture = c->getValueFullName<TexturePtr>(element.name.c_str());

				auto newTex = textureCombo(texture ? texture->getFilePath() : "");
				if (newTex.size() && (!texture || newTex != texture->getFilePath()))
				{
					auto newT = TextureLib::loadOrGetTexture(
						newTex, element.type == g_typ::TEXTURE_2D ? TextureType::_2D : TextureType::_CUBE_MAP);
					if (newT)
					{
						texture = newT;
						change = true;
					}
				}
				if (ImGui::Button("Clear") || (newTex.empty() && texture))
				{
					texture = std::shared_ptr<Texture>(nullptr);
					change = true;
				}


				break;
			}
			}
			ImGui::PopID();
		}

		ImGui::End();
		if (change)
		{
			Atelier::get().assignPhotoWork(c);
		}
		return true;
	}

#define COMPO_ENTRY_IMGUI(Typ,name)\
	if (selected.has<Typ>())\
if (ImGui::BeginTabItem(name, &open,ImGuiTabItemFlags_NoCloseButton))\
{\
	components_imgui_access::draw(selected, selected.get<Typ>());\
	ImGui::EndTabItem();\
}
	bool drawEntityManager(NewScene& c,Entity& lookThroughCam)
	{
		static auto eyeOnId = TextureLib::loadOrGetTexture("res/models/eye_on.png")->getID();
		static auto eyeOffId = TextureLib::loadOrGetTexture("res/models/eye_off.png")->getID();
		auto eyeOnID = eyeOnId;
		auto eyeOffID = eyeOffId;
		static bool open = true;
		auto sci = &c;
		static Entity selected = Entity::null;
		bool newModel = false;
		bool newLight = false;
		bool newCamera= false;
		NewScene* sc = &c;
		//static defaultable_vector<int> tabsOpened;
		if (ImGui::Begin("Scene", &open, ImGuiWindowFlags_MenuBar)){
			if (ImGui::BeginMenuBar()){
				if (ImGui::BeginMenu("Entity")){
					if (ImGui::BeginMenu("Add")){
						newModel = ImGui::MenuItem("Model");
						newLight = ImGui::MenuItem("Light");
						newCamera = ImGui::MenuItem("Camera");
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			
			drawStringDialog(newModel, "newModel", "Name", [sc](const char* c)
			{
					auto ent = sc->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<ModelComponent>(nullptr, nullptr);
					ent.get<TagComponent>().enabled = false;
			});
			drawStringDialog(newLight, "newLight", "Name", [sc](const char* c)
				{
					auto ent = sc->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<LightComponent>();
				});
			drawStringDialog(newCamera, "newCamera", "Name", [sc](const char* c)
				{
					auto ent = sc->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<CameraComponent>(glm::mat4(1.f), glm::quarter_pi<float>(), 1.f, 100.f);
					auto editCam = (EditorCam*)malloc(sizeof(EditorCam));
					ent.emplaceOrReplace<PointerComponent>(new(editCam)EditorCam(ent));
				});
			
			
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (ImGui::TreeNode("Objects"))
			{
				ImGui::Columns(2);

				int i = 0;
				c.reg().each([sci, &i,&lookThroughCam,eyeOffID,eyeOnID](const entt::entity ent)
					{
						ImGui::PushID(i++);
						auto entity = sci->wrap(ent);
						bool selectedi = selected == entity;
						auto& tag = entity.get<TagComponent>();

						if (selectedi)
							ImGui::PushStyleColor(ImGuiCol_Text, { 0, 1, 0, 1 });
						if (ImGui::TreeNodeEx(tag(), ImGuiTreeNodeFlags_OpenOnArrow))
						{
							if (selectedi)
								ImGui::PopStyleColor();
							//ImGui::Text("Blemc");
							ImGui::TreePop();
						}
						else if (selectedi) ImGui::PopStyleColor();
						if (ImGui::IsItemClicked())
						{
							selected = entity;
						}
						ImGui::NextColumn();
						bool enabl = tag.enabled;
						ImGui::Checkbox("##enabledee", &enabl);ImGui::SameLine();
						if (entity.has<ModelComponent>())
						{
							auto& model = entity.get<ModelComponent>();
							auto& material = model.material;
							if(material && model.mesh)//enable only if both material and mesh are bound
								tag.enabled = enabl;

							drawTexOrNo(material, ImGui::GetItemRectSize().y, ImGui::GetItemRectSize().y);
							makeDragDropSource(material);
							makeDragDropTarget(material);
							if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								windows.open_material = true;
								windows.material = material;
							}
						}else if(entity.has<CameraComponent>())//set viewing to this camera
						{
							bool lookThrough = lookThroughCam == entity;
							ImGui::Image((ImTextureID)(lookThrough ? eyeOnID : eyeOffID), { 25,25 }, { 0,1 }, { 1,0 });
							if(ImGui::IsItemClicked())
								lookThroughCam = entity;
							
						}
						ImGui::NextColumn();
						ImGui::PopID();
					});
				ImGui::Columns(1);

				if (ImGui::Selectable("None", false))
				{
					selected = Entity::null;
					//tabsOpened.clear();
				}
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (selected)
			{
				auto& tag = selected.get<TagComponent>();
				//ImGui::PushID(tag.operator()());
				ImGui::TextColored({ 0.f, 1.f, 0.f, 1.f }, tag.operator()());
				ImGuiTabBarFlags tab_bar_flags = (ImGuiTabBarFlags_FittingPolicyDefault_) | (
					false ? ImGuiTabBarFlags_Reorderable : 0);
				if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
				{
					//if (opt_reorderable)
					//	NotifyOfDocumentsClosedElsewhere(app);
					// Submit Tabs
					int tabIdx = 0;
					bool open = true;
					COMPO_ENTRY_IMGUI(TransformComponent, "Trans");
					COMPO_ENTRY_IMGUI(LightComponent, "Light");
					COMPO_ENTRY_IMGUI(CameraComponent, "Camera");
					COMPO_ENTRY_IMGUI(ModelComponent, "Model");
					ImGui::EndTabBar();
				}
				//ImGui::PopID();
			}
		}
		ImGui::End();
		return true;
	}
	bool drawMaterialManager(bool clickOnNew)
	{
		drawStringDialog(clickOnNew, "MaterNew?", "Texture Name", [](const char* c){
			MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/Model.shader"),"MAT",c });
		});

		auto& list = MaterialLibrary::getList();

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		int ind = 1;
		for (auto& mater : list)
		{
			auto& material = mater.second;
			ImGui::PushID(ind);
			//popup id
			char c[10 + 4 + 1]{ "drawMatPop" };
			*(int*)(c + 10) = ind++;
			c[14] = 0;

			auto& b = MaterialLibrary::isDirty(material->getID());
			if (b) {
				Atelier::get().assignPhotoWork(material);
				b = false;
			}

			ImGui::BeginGroup();
			ImGui::Image((ImTextureID)Atelier::get().getPhoto(material)->getID(), { 128, 128 }, { 0, 1 }, { 1, 0 });
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup(c);
			else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				windows.material = material;
				windows.open_material = true;
			}
			makeDragDropSource(material);
			ImGui::TextColored({ 0, 1, 1, 1 }, material->getName().c_str());
			ImGui::EndGroup();
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ImGui::GetItemRectSize().x; // Expected position if next button was on same line
			bool sameLine = next_button_x2 < window_visible_x2;

			if (ImGui::BeginPopup(c))
			{
				if (ImGui::MenuItem("Save"))
				{
					MaterialLibrary::save(material, std::string("res/models/") + material->getName() + ".mat");
				}
				if (ImGui::MenuItem("Copy"))
				{
					int index = 0;
					while (true)
					{
						std::string possibleName = material->getName() + " Copy" + std::to_string(++index);
						auto t = MaterialLibrary::getByName(possibleName);
						if (!t)
						{
							MaterialLibrary::copy(material, possibleName);
							break;
						}
					}
					ImGui::EndPopup();
					ImGui::PopID();
					break;//we need to break loop because iterator is invalidated by MaterialLibrary::copy

				}
				if (ImGui::MenuItem("Open"))
				{
					windows.material = material;
					windows.open_material = true;
				}
				if (ImGui::MenuItem("Delete"))
				{
					int expectedCount = 1;
					if (windows.material == material)
						expectedCount = 2;
					if (material.use_count() == expectedCount) {
						MaterialLibrary::remove(material->getID());
						if (expectedCount == 2)
						{
							windows.material = nullptr;
							windows.open_material = false;
						}
					}

					ImGui::EndPopup();
					ImGui::PopID();
					break;//we need to break loop because iterator is invalidated by MaterialLibrary::remove
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
			if (sameLine)
				ImGui::SameLine();
		}
		return true;
	}

	bool drawModelManager(bool clickOnNew)
	{
		const char* suffixes[]{ ".fbx",".obj",".dae",".bin" };
		drawFileDialog(clickOnNew, "MaterNew?", "Texture Name","res/models", suffixes,4,true, [](const char* c) {
			MeshFactory::loadOrGet(std::string(c));
			});

		auto& list = MeshFactory::getList();

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;


		int ind = 1;
		for (auto& mater : list)
		{
			auto& new_mesh = mater.second;
			ImGui::PushID(ind);
			//popup id
			char c[10 + 4 + 1]{ "drawMeshPop" };
			*(int*)(c + 10) = ind++;
			c[14] = 0;

			ImGui::BeginGroup();
			ImGui::Image((ImTextureID)Atelier::get().getPhoto(new_mesh)->getID(), { 128, 128 }, { 0, 1 }, { 1, 0 });
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup(c);
			/*else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				windows.material = new_mesh;
				windows.open_material = true;
			}*/
			makeDragDropSource(new_mesh);
			ImGui::TextColored({ 0, 1, 1, 1 }, new_mesh->getName().c_str());
			ImGui::EndGroup();
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ImGui::GetItemRectSize().x; // Expected position if next button was on same line
			bool sameLine = next_button_x2 < window_visible_x2;

			if (ImGui::BeginPopup(c))
			{
				if(!SUtil::endsWith(new_mesh->getName(),".bin")&&ImGui::MenuItem("Export as BIN"))
				{
					MeshDataFactory::writeBinaryFile(new_mesh->getName().substr(0, new_mesh->getName().find_last_of('.')+1) + "bin", *new_mesh->data);
				}
				if (ImGui::MenuItem("Delete"))
				{
					if (new_mesh.use_count() == 1)
						MeshFactory::remove(new_mesh->getID());

					ImGui::EndPopup();
					ImGui::PopID();
					break;//we need to break loop because iterator is invalidated by MaterialLibrary::remove
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
			if (sameLine)
				ImGui::SameLine();
		}
		return true;
	}
	
	bool drawMMBrowser()
	{
		static bool textureOrModel = true;
		static bool keepOpen = true;
		ImGui::Begin("MMBrowser", &keepOpen, ImGuiWindowFlags_MenuBar);
		bool newClick = false;
		if (ImGui::BeginMenuBar())
		{
			if(textureOrModel)
			{
				if (ImGui::BeginMenu("Texture"))
				{
					newClick = ImGui::MenuItem("New", "");
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Mesh"))
					textureOrModel = false;
			}else
			{
				if (ImGui::BeginMenu("Mesh"))
				{
					newClick = ImGui::MenuItem("New", "");
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Texture"))
					textureOrModel = true;
			}
			ImGui::EndMenuBar();
		}
		if (textureOrModel)
			drawMaterialManager(newClick);
		else drawModelManager(newClick);
		ImGui::End();
		return true;
	}

	void SceneWindows::drawWindows()
	{
		if (open_material)
			open_material = drawWindow(material);
		if (open_mesh)
			open_mesh = drawWindow(*mesh);
		drawMMBrowser();
		
	}
}
