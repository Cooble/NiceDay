#include "components_imgui_access.h"
#include <imgui.h>
#include "imgui_internal.h"
#include "files/FUtil.h"
#include <shellapi.h>
#include "Atelier.h"
#include <sol/sol.hpp>
#include "Mesh.h"
#include "core/NBT.h"
#include "GlobalAccess.h"


ImVec4 GREEN_COL { 0,1,0,1 };
ImVec4 RED_COL{ 1, 0.2f, 0.2f, 1 };
namespace comp_util
{
	

#define COPPY_EE(Type) \
	if (src.has<Type>()){\
		target.emplaceOrReplace<Type>();\
		target.get<Type>() = src.get<Type>();}
	// really lame way of copying components (but is there a better one?)
	static void copyEntity(Entity src, Entity target)
	{
		COPPY_EE(TransformComponent);
		COPPY_EE(ModelComponent);
		COPPY_EE(LightComponent);
		COPPY_EE(CameraComponent);
		if (src.has<NativeScriptComponent>()) {
			
				target.emplaceOrReplace<NativeScriptComponent>();
				auto& t = target.get<NativeScriptComponent>();
				t = src.get<NativeScriptComponent>();
				t.construct(target, components_imgui_access::windows.scene);
				t.onCreate();
		}
	}
	static void saveEntity(Entity e)
	{
		NBT n;
		if (e.has<ModelComponent>()) {
			auto& model = e.get<ModelComponent>();
			auto& mesh = model.Mesh();
			auto& material = model.Material();
			auto& tag = e.get<TagComponent>();
			n.save("name", tag.name);
			n.save("mesh", mesh->data->getFilePath());
			if (!SUtil::startsWith(material->getName(), "res")) {
				MaterialLibrary::save(material, "res/models/" + material->getName() + ".mat");
				n.save("material", "res/models/" + material->getName() + ".mat");
			}
			else
				n.save("material", material->getName());
			NBT::saveToFile(std::string("res/models/") + e.get<TagComponent>().name + ".model", n);
			ND_BUG("Model Saved to: res/models/{}.model", e.get<TagComponent>().name);
		}
	}
	static Entity loadEntity(NewScene* s, const std::string& path)
	{
		NBT n;
		NBT::loadFromFile(path, n);
		auto e = s->createEntity(n["name"].c_str());
		MeshLibrary::loadOrGet(n["mesh"].string());
		MaterialLibrary::loadOrGet(n["material"].string());
		e.emplaceOrReplace<ModelComponent>(SID(n["mesh"].string()), SID(n["material"].string()));
		return e;
	}
	static void drawTexOrNo(MaterialPtr& c, int width, int height)
	{
		static auto no = TextureLib::loadOrGetTexture("res/images/no.png")->getID();
		ImGui::Image(reinterpret_cast<ImTextureID>(c ? Atelier::get().getPhoto(c)->getID() : no), { (float)width,(float)height }, { 0.f, 1.f }, { 1.f, 0.f });
	}
	static void drawTexOrNo(MeshPtr& c, int width, int height)
	{
		static auto no = TextureLib::loadOrGetTexture("res/images/no.png")->getID();
		ImGui::Image(reinterpret_cast<ImTextureID>(c ? Atelier::get().getPhoto(c)->getID() : no), { (float)width,(float)height }, { 0.f, 1.f }, { 1.f, 0.f });
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
			ImGui::Image(reinterpret_cast<ImTextureID>(Atelier::get().getPhoto(ptr)->getID()), { 32.f, 32.f }, { 0.f, 1.f }, { 1.f, 0.f });
			ImGui::EndDragDropSource();
		}
	}
	static void makeDragDropTargetMesh(Strid& meshID)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH"))
			{
				IM_ASSERT(payload->DataSize == sizeof(Strid));
				Strid payload_n = *(Strid*)payload->Data;
				auto c = MeshLibrary::get(payload_n);
				meshID = c->getID();
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
			ImGui::Image(reinterpret_cast<ImTextureID>(Atelier::get().getPhoto(ptr)->getID()), { 32.f, 32.f }, { 0.f, 1.f }, { 1.f, 0.f });
			ImGui::EndDragDropSource();
		}
	}
	static void makeDragDropTargetMaterial(Strid& matID)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL"))
			{
				IM_ASSERT(payload->DataSize == sizeof(Strid));
				Strid payload_n = *(Strid*)payload->Data;
				auto cp = MaterialLibrary::get(payload_n);
				matID = cp->getID();//todo shared ptr problem, we need to change the shared pointer object at location, not the object itself
			}
			ImGui::EndDragDropTarget();
		}
	}
	static Strid fileComboLastID = 0;

	static std::string fileCombo(const std::string& currentValue, const std::string& sourceFolder, const char** suffix, size_t suffixLength,
		StringId calledID)
	{
		ImVec4 col = FUtil::exists(currentValue) ? GREEN_COL : RED_COL;
		ImGui::TextColored(col, currentValue.c_str());

		std::vector<std::string> files;

		if (!FUtil::exists(sourceFolder))
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
				? GREEN_COL : RED_COL;
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
			for (auto& file : files)
				//if (filter.PassFilter(files[i].c_str()))
				if (file.find(filter.InputBuf) != std::string::npos&&file!=currentValue)
					if (ImGui::Selectable(file.c_str(), false))
					{
						memcpy(filter.InputBuf, file.c_str(), file.size() + 1);
						return file;
					}
		return currentValue;
	}
	
	static std::string folderCombo(const std::string& currentValue, const std::string& sourceFolder,const  std::function<bool(const std::string& folder)>& isValid,StringId calledID)
	{
		ImVec4 col = FUtil::exists(currentValue)&&isValid(currentValue) ? GREEN_COL : RED_COL;
		ImGui::TextColored(col, currentValue.c_str());

		std::vector<std::string> files;

		if (!FUtil::exists(sourceFolder))
			return currentValue;
		for (auto& file : std::filesystem::recursive_directory_iterator(ND_RESLOC(sourceFolder))) {
			auto local = ResourceMan::getLocalPath(file.path().string());
			if (isValid(local))
				files.push_back(local);
			
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
			ImVec4 col = FUtil::exists(filter.InputBuf)&&isValid(filter.InputBuf)
				? GREEN_COL : RED_COL;
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
			if (entered && FUtil::exists(filter.InputBuf)&&isValid(filter.InputBuf))
				return std::string(filter.InputBuf);
			if (entered && strlen(filter.InputBuf) == 0)
				return "";
		}

		if (strlen(filter.InputBuf) != 0)
			for (auto& file : files)
				//if (filter.PassFilter(files[i].c_str()))
				if (file.find(filter.InputBuf) != std::string::npos&& file != currentValue)//file must not be the current one
					if (ImGui::Selectable(file.c_str(), false))
					{
						memcpy(filter.InputBuf, file.c_str(), file.size() + 1);
						return file;
					}
		return currentValue;
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
	static void drawFileDialog(bool open, const char* popupName, const char* fieldName, const std::string& sourceFolder, const char** suffixes, int suffixLength, bool forbidCustomPath, const std::function<void(const char*)>& callBack)
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
			if (!strin.empty())
			{
				callBack(strin.c_str());
				ImGui::CloseCurrentPopup();
				strin = "";
			}
			ImGui::EndPopup();
		}
	}
	static void drawFileDialog(bool open, const char* popupName, const char* fieldName, const std::string& sourceFolder, const char* suffix, bool forbidCustomPath, const std::function<void(const char*)>& callBack)
	{
		drawFileDialog(open, popupName, fieldName, sourceFolder, &suffix, 1, forbidCustomPath, callBack);
	}
}
namespace components_imgui_access
{
	static bool useRad = false;
	SceneWindows windows = SceneWindows();
	TextureAtlasUV* ui_icons = nullptr;

	void Image(ImTextureID image, const TextureAtlasUVCoords& coords, ImVec2 size)
	{
		ImGui::Image(image, size, *(ImVec2*)(&coords.min), *(ImVec2*)(&coords.max));
	}

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

	
		
		if (useRad) {
			
			ImGui::DragFloat3("rotation", (float*)&c.rot, 0.05f, -glm::two_pi<float>(), glm::two_pi<float>());
		}else
		{
			auto deg = glm::degrees(c.rot);
			if(ImGui::DragFloat3("rotation", (float*)&deg, 0.05f, -180.f, 180.f))
				c.rot = glm::radians(deg);//refresh only and only if change has occurred, otherwise due to imprecision value would keep changing
		}
		c.recomputeMatrix();
		ImGui::SameLine();
		ImGui::Checkbox(useRad ? "Rad" : "Deg", &useRad);
	}

	void draw(Entity e, ModelComponent& c)
	{
		//=========================================
		ImGui::Text("MATERIAL");


		auto& material = c.Material();
		comp_util::drawTexOrNo(material, AtelierDim::width, AtelierDim::height);
		if (material && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			windows.material = material;
			windows.open_material = true;
		}
		if (material)
			comp_util::makeDragDropSource(material);
		comp_util::makeDragDropTargetMaterial(c.material);

		{
			auto& material = c.Material();

			if (material)
				ImGui::TextColored({ 0, 1, 1, 1 }, material->getName().c_str());
			else ImGui::TextColored({ 1, 0, 0, 1 }, "No Material Bound");
		}

		//=========================================
		ImGui::Text("MESH");

		auto& mesh = c.Mesh();
		comp_util::drawTexOrNo(mesh, AtelierDim::width, AtelierDim::height);
		comp_util::makeDragDropTargetMesh(c.mesh);

		{
			auto& mesh = c.Mesh();

			if (mesh)
				comp_util::makeDragDropSource(mesh);
			if (mesh)
				ImGui::TextColored({ 0, 1, 1, 1 }, mesh->getName().c_str());
			else ImGui::TextColored({ 1, 0, 0, 1 }, "No Mesh Bound");
		}
	}

	void draw(Entity e, CameraComponent& c)
	{
		auto angle = glm::degrees(c.fov);
		if (ImGui::SliderFloat("Fov", &angle, 10, 170))
			c.fov = glm::radians(angle);
		ImGui::SliderFloat("Near", &c.Near, 0.5f, 200.f);
		ImGui::SliderFloat("Far", &c.Far, 0.5f, 200.f);
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
			ImGui::SetColumnWidth(0, 20.f);
			ImGui::SetColumnWidth(1, 80.f);
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
			ImGui::SetColumnWidth(-1, 100.f);
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

	static std::string textureCombo(const std::string& currentCombo,TextureType type = TextureType::_2D)
	{
		if (type == TextureType::_2D) {
			auto id = std::filesystem::exists(ND_RESLOC(currentCombo))
				? TextureLib::loadOrGetTexture(currentCombo)->getID()
				: TextureLib::loadOrGetTexture("res/images/no.png")->getID();
			if (id)
				ImGui::Image(reinterpret_cast<ImTextureID>(id), { (float)AtelierDim::width, (float)AtelierDim::height }, { 0.f, 1.f }, { 1.f, 0.f });
			//else ImGui::TextColored({ 1,0,0,1 }, "Image %s not found", currentCombo.c_str());

			char c[5];
			*(uint32_t*)&c[0] = ImGui::GetActiveID();
			c[4] = 0;

			auto s = ".png";
			auto ret = comp_util::fileCombo(currentCombo, "res", &s, 1, SIDS("textureCombo")/*.concat(c)*/);

			return ret;
		}
		if(type==TextureType::_CUBE_MAP)
		{
			auto lastSlash = currentCombo.find_last_of('/');
			std::string src = currentCombo;
			if(lastSlash!=std::string::npos)
				src = currentCombo.substr(0, lastSlash);
			auto s = comp_util::folderCombo(src, "res", [](auto& string) {
				return FUtil::exists(string + "/px.png") && FUtil::exists(string + "/nx.png") &&
					FUtil::exists(string + "/py.png") && FUtil::exists(string + "/ny.png") &&
					FUtil::exists(string + "/pz.png") && FUtil::exists(string + "/nz.png");
				 }, SIDS("texturecubecombo"));
			
			return s + (s.empty()?"":"/*.png");
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
		ImGui::SetNextWindowSize({ 900.f,500.f }, ImGuiCond_Once);
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

		ImGui::Image(reinterpret_cast<ImTextureID>(Atelier::get().getPhoto(c)->getID()), { (float)AtelierDim::width,(float)AtelierDim::height }, { 0.f, 1.f }, { 1.f, 0.f });
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
					if (p->getLayout().getLayoutByName("MAT"))
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

				auto newTex = textureCombo(texture ? texture->getFilePath() : "", element.type == g_typ::TEXTURE_2D ? TextureType::_2D : TextureType::_CUBE_MAP);
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
	bool drawEntityManager()
	{
		static bool open = true;
		bool newModel = false;
		bool newLight = false;
		bool newCamera= false;
		bool loadModel = false;
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
				if (ImGui::BeginMenu("Model")) {
					loadModel = ImGui::MenuItem("Load");
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			comp_util::drawStringDialog(newModel, "newModel", "Name", [](const char* c)
			{
					auto ent = windows.scene->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<ModelComponent>();
					ent.get<TagComponent>().enabled = false;
			});
			comp_util::drawStringDialog(newLight, "newLight", "Name", [](const char* c)
				{
					auto ent = windows.scene->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<LightComponent>();
				});
			comp_util::drawStringDialog(newCamera, "newCamera", "Name", [](const char* c)
				{
					auto ent = windows.scene->createEntity(c);
					ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
					ent.emplaceOrReplace<CameraComponent>(glm::mat4(1.f), glm::quarter_pi<float>(), 1.f, 100.f);
					ent.emplaceOrReplace<NativeScriptComponent>();
					auto& script = ent.get<NativeScriptComponent>();
					script.bind<EditCameraController>();
					script.construct(ent, windows.scene);
					script.onCreate();
				});
			comp_util::drawFileDialog(loadModel, "loadModel", "Name", "res/models", ".model", true, [](const char* c)
			{
					auto ent = comp_util::loadEntity(windows.scene, c);
					ND_BUG("Loaded entity from {}", c);
			});
			
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (ImGui::TreeNode("Objects"))
			{
				ImGui::Columns(2);

				int i = 0;
				auto tagView = windows.scene->reg().view<TagComponent>();
				for (auto ent : tagView)
				{
					auto& tag = tagView.get<TagComponent>(ent);
					ImGui::PushID(i++);
					auto entity = windows.scene->wrap(ent);
					bool selectedi = windows.selectedEntity == entity;

					if (selectedi)
						ImGui::PushStyleColor(ImGuiCol_Text, { 0, 1, 0, 1 });
					if (ImGui::TreeNodeEx(tag(), ImGuiTreeNodeFlags_OpenOnArrow))
					{
						if (selectedi)
							ImGui::PopStyleColor();
						//ImGui::Text("Blemc");
						ImGui::TreePop();
					}
					else if (selectedi) 
						ImGui::PopStyleColor();
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						windows.selectedEntity = entity;
					auto nameid = (std::string("entityPop ") + tag.name).c_str();
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						ImGui::OpenPopup(nameid);
					bool copyFlag = false;
					bool delFlag = false;
					if(ImGui::BeginPopup(nameid))
					{
						copyFlag = ImGui::MenuItem("Copy");
						delFlag = ImGui::MenuItem("Delete");
						if(ImGui::MenuItem("Save"))
						{
							comp_util::saveEntity(entity);
						}
						ImGui::EndPopup();
					}

					comp_util::drawStringDialog(copyFlag, (std::string("entityPop ") + tag.name).c_str(), "Name", [entity](const char* c)
						{
							auto ent = windows.scene->createEntity(c);
							comp_util::copyEntity(entity, ent);//not work for cameras
							
						});
					
					ImGui::NextColumn();
					bool enabl = tag.enabled;
					ImGui::Checkbox("##enabledee", &enabl); ImGui::SameLine();
					if (entity.has<ModelComponent>())
					{
						auto& model = entity.get<ModelComponent>();
						auto& material = model.Material();
						if (material && model.Mesh())//enable only if both material and mesh are bound
							tag.enabled = enabl;

						comp_util::drawTexOrNo(material, ImGui::GetItemRectSize().y, ImGui::GetItemRectSize().y);
						comp_util::makeDragDropSource(material);
						comp_util::makeDragDropTargetMaterial(model.material);
						
						if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						{
							windows.open_material = true;
							windows.material = model.Material();
						}
					}
					else if (entity.has<CameraComponent>())//set viewing to this camera
					{
						bool lookThrough = windows.activeCamera == entity;
						Image(lookThrough ? SIDS("eye_on") : SIDS("eye_off"), *ui_icons, { 25.f,25.f });
						//ImGui::Image((ImTextureID)(lookThrough ? eyeOnID : eyeOffID), { 25,25 },  { 0.f, 1.f }, { 1.f, 0.f });
						if (ImGui::IsItemClicked())
							windows.activeCamera = entity;

					}
					ImGui::NextColumn();
					ImGui::PopID();
					if (delFlag) {
						if (windows.selectedEntity == entity)
							windows.selectedEntity = Entity::null;
						entity.destroy();
					}
				}
				ImGui::Columns(1);

				if (ImGui::Selectable("None", false))
				{
					windows.selectedEntity = Entity::null;
					//tabsOpened.clear();
				}
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (windows.selectedEntity)
			{
				auto& tag = windows.selectedEntity.get<TagComponent>();
				auto selected = windows.selectedEntity;
				//ImGui::PushID(tag.operator()());
				ImGui::TextColored({ 0.f, 1.f, 0.f, 1.f }, tag.operator()());
				ImGuiTabBarFlags tab_bar_flags = (ImGuiTabBarFlags_FittingPolicyDefault_) | (
					false ? ImGuiTabBarFlags_Reorderable : 0);
				if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
				{
					//if (opt_reorderable)
					//	NotifyOfDocumentsClosedElsewhere(app);
					// Submit Tabs
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
		comp_util::drawStringDialog(clickOnNew, "MaterNew?", "Texture Name", [](const char* c){
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
			ImGui::Image(reinterpret_cast<ImTextureID>(Atelier::get().getPhoto(material)->getID()), { 128.f, 128.f }, { 0.f, 1.f }, { 1.f, 0.f });
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup(c);
			else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				windows.material = material;
				windows.open_material = true;
			}
			comp_util::makeDragDropSource(material);
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
		comp_util::drawFileDialog(clickOnNew, "MaterNew?", "Texture Name","res/models", suffixes,4,true, [](const char* c) {
			MeshLibrary::loadOrGet(std::string(c));
			});

		auto& list = MeshLibrary::getList();

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
			ImGui::Image(reinterpret_cast<ImTextureID>(Atelier::get().getPhoto(new_mesh)->getID()), { 128.f, 128.f }, { 0.f, 1.f }, { 1.f, 0.f });
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup(c);
			/*else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				windows.material = new_mesh;
				windows.open_material = true;
			}*/
			comp_util::makeDragDropSource(new_mesh);
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
						MeshLibrary::remove(new_mesh->getID());

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

	bool drawQuantizeDialog(bool enabled)
	{
		static bool ena=false;
		if (enabled)
			ena =!ena;
		if (ena) {
			if (ImGui::Begin("Quantization", &ena))
			{
				ImGui::DragFloat("pos", windows.quantizationPos, 0.1f);
				ImGui::DragFloat("scale", windows.quantizationScale, 0.1f);
				if (useRad) {

					ImGui::DragFloat("rot", windows.quantizationRot, 0.1f);
				}
				else
				{
					auto deg = glm::degrees(*windows.quantizationRot);
					if (ImGui::DragFloat("rotation", (float*)&deg, 1.f))
						*windows.quantizationRot = glm::radians(deg);//refresh only and only if change has occurred, otherwise due to imprecision value would keep changing
				}
				ImGui::SameLine();
				ImGui::Checkbox(useRad ? "Rad" : "Deg", &useRad);
			}
			ImGui::End();
		}
		return true;
	}
	bool drawToolPanel()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 10));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,10 });
		ImGui::SetNextWindowSize(ImVec2(52, 0), ImGuiCond_Always);
		ImGui::Begin("Tools",0, ImGuiWindowFlags_NoDecoration);
		Image(windows.transformOperation==TRANSOP_MOVE ? SIDS("move_on") : SIDS("move_off"), *ui_icons, { 50.f,50.f });
		if (ImGui::IsItemClicked())windows.transformOperation = TRANSOP_MOVE;

		Image(windows.transformOperation == TRANSOP_SCALE ? SIDS("scale_on") : SIDS("scale_off"), *ui_icons, { 50.f,50.f });
		if (ImGui::IsItemClicked())windows.transformOperation = TRANSOP_SCALE;

		Image(windows.transformOperation == TRANSOP_ROTATE ? SIDS("rotate_on") : SIDS("rotate_off"), *ui_icons, { 50.f,50.f });
		if (ImGui::IsItemClicked())windows.transformOperation = TRANSOP_ROTATE;

		Image(SIDS("quantize"), *ui_icons, { 50.f,50.f });
		drawQuantizeDialog(ImGui::IsItemClicked());
		
	
		ImGui::PopStyleVar(3);
		ImGui::End();
		return true;
	}

	void SceneWindows::drawWindows()
	{
		if (open_material)
			open_material = drawWindow(material);
		drawEntityManager();
		drawMMBrowser();
		drawToolPanel();
		
	}

	void SceneWindows::init()
	{
		ui_icons = &GlobalAccess::ui_icons;
	}
}
