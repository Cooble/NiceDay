#include "TerrainLayer.h"

#include "DropletSim.h"
#include "EulerSim.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "imgui.h"
#include "TUtils.h"
#include "graphics/API/Shader.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/Effect.h"
#include "core/NBT.h"
#include "scene/components.h"
#include "scene/EditorLayer.h"
#include "scene/Material.h"
#include "scene/NewScene.h"
#include "glm/gtc/noise.hpp"


using namespace nd;


TerrainLayer::TerrainLayer(nd::EditorLayer& l) : m_editorLayer(l)
{
}

// Primitives for rendering heightmap
struct TerrainMesh
{
	// graphical primitives
	VertexBuffer* pos_vbo;
	VertexArray* vao;
	IndexBuffer* index_buffer;
	TexturePtr height_texture;

	std::vector<float> float_height;

	void create(EulerGround& map)
	{
		float_height.resize(map.width * map.height);
		ZeroMemory(float_height.data(), float_height.size() * sizeof(decltype(float_height)::value_type));


		if (pos_vbo)
		{
			delete vao;
			delete pos_vbo;
			delete index_buffer;
		}

		gfloat scaler = 1.f / (map.width - 1);


		auto f = std::vector<float>(map.width * map.height * 2);

		for (int y = 0; y < map.height; y++)
			for (int x = 0; x < map.width; ++x)
			{
				f[(y * map.width + x) * 2 + 0] = scaler * x;
				f[(y * map.width + x) * 2 + 1] = scaler * y;
			}

		TextureInfo info = TextureInfo().size(map.width, map.height).format(TextureFormat::RED_32F).wrapMode(
			TextureWrapMode::CLAMP_TO_EDGE);

		height_texture = std::shared_ptr<Texture>(Texture::create(info));
		height_texture->setPixels(float_height.data());


		pos_vbo = VertexBuffer::create(f.data(), f.size() * sizeof(float));
		pos_vbo->setLayout({g_typ::VEC2});

		auto indices = std::vector<uint32_t>((map.width - 1) * (map.height - 1) * 6);

		for (int y = 0; y < map.height - 1; y++)
			for (int x = 0; x < map.width - 1; ++x)
			{
				auto startIdx = (y * (map.width - 1) + x) * 6;

				indices[startIdx + 0] = (y + 1) * map.width + x;
				indices[startIdx + 1] = y * map.width + x + 1;
				indices[startIdx + 2] = y * map.width + x;


				indices[startIdx + 3] = (y + 1) * map.width + x;
				indices[startIdx + 4] = (y + 1) * map.width + x + 1;
				indices[startIdx + 5] = y * map.width + x + 1;
			}


		index_buffer = IndexBuffer::create(indices.data(), indices.size());

		vao = VertexArray::create();
		vao->addBuffer(*pos_vbo);
		vao->addBuffer(*index_buffer);
	}

	void refreshHeight(EulerGround& map)
	{
		height_texture->setPixels(map.terrain_height.data());
	}

	void refreshWaterHeight(EulerGround& map)
	{
		height_texture->setPixels(map.water_height.data());
	}
};


// Scene + Graphics
static TerrainMesh mesh;
static TerrainMesh waterMesh;
static Ref<Mesh> meshPtr;
static MaterialPtr matPtr;
static MaterialPtr waterMatPtr;
static glm::vec3 pointer_relative_pos;


// Imgui Vars
static int groundSize = 1024;
static bool toggle_render_water = false;
static bool toggle_sim_drop = false;
static bool toggle_sim_euler = false;
static int playspeed_sim_drop = 1;
static int playspeed_sim_euler = 1;
static int stopAtBalls = -1;

// Statistics
static gfloat totalGround = 0;
static gfloat currentGround = 0;
static gfloat currentSediment = 0;
static gfloat maxSediment = 0;
static gfloat currentWater = 0;
static gfloat minTerrain = 0, maxTerrain = 0;




// ========== Graphics and Scene ==========

// create shaders and materials for terrain and water
void TerrainLayer::createMaterial()
{

	{
		std::string vertexShader = R"(
			#version 330 core

			layout(location = 0) in vec2 a_pos;

			struct MAT {
				vec4 color;
				sampler2D height_texture;
				sampler2D terrain_texture;
				float shines;
				float width;
			};
			uniform MAT mat;


			struct GLO {

				mat4 view;
				mat4 proj;

				vec3 sunPos;

				vec3 ambient;
				vec3 diffuse;
				vec3 specular;
				vec3 camera_pos;
				//attenuation
				float constant;
				float linear;
				float quadratic;
			};
			uniform GLO glo;

			uniform mat4 world;

			out vec2 outpost;

			out vec3 v_normal;
			out vec3 v_world_pos;

			void main()
			{

				vec3 central = vec3(a_pos.x,texture2D(mat.height_texture, a_pos).r/mat.width,a_pos.y);
				const float eps = 0.02;


				float hL = texture2D(mat.height_texture, a_pos - vec2(eps, 0.0)).r/mat.width;
			    float hR = texture2D(mat.height_texture, a_pos + vec2(eps, 0.0)).r/mat.width;
			    float hD = texture2D(mat.height_texture, a_pos - vec2(0.0, eps)).r/mat.width;
			    float hU = texture2D(mat.height_texture, a_pos + vec2(0.0, eps)).r/mat.width;

				vec3 dx = vec3(2 * eps, hR - hL, 0);
				vec3 dy = vec3(0, hU - hD, 2 * eps);

				vec3 nor = normalize(cross(dy,dx));

				v_normal = (world * vec4(nor,0)).xyz;
				v_world_pos = (world * vec4(central, 1.0)).xyz;


				gl_Position = glo.proj * glo.view * vec4(v_world_pos,1.0);
				outpost=a_pos;
			}
		)";

		std::string fragmentShader = R"(
			#version 330 core


			struct GLO {

				mat4 view;
				mat4 proj;

				vec3 sunPos;

				vec3 ambient;
				vec3 diffuse;
				vec3 specular;
				vec3 camera_pos;
				//attenuation
				float constant;
				float linear;
				float quadratic;
			};
			uniform GLO glo;

						
			struct MAT {
				vec4 color;
				sampler2D height_texture;
				sampler2D terrain_texture;
				float shines;
				float width;
			};
			uniform MAT mat;

			in vec2 outpost;
			in vec3 v_normal;
			in vec3 v_world_pos;

			out vec4 color;
			void main()
			{
				vec3 nor = normalize(v_normal);
				vec3 specularColor = vec3(0.5,0.5,0.5);

				vec3 diffuseColor = vec3(0.0,0.7,0.2);
				//DIFFUSE
				vec3 toSun = normalize(glo.sunPos - v_world_pos);
				vec3 diffuseLight = glo.diffuse * max(dot(toSun, nor),0.0) * diffuseColor;

				//SPECULAR
				vec3 toCamera = normalize(glo.camera_pos - v_world_pos);
				vec3 reflection = reflect(-toCamera, nor);
				vec3 reflectiveLight = vec3(0.0);
				if(mat.shines!=0)
					reflectiveLight = glo.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), mat.shines);


				color = vec4(diffuseLight+diffuseColor*0.3 + reflectiveLight,1);				
			}
		)";

		auto shader = Shader::create(Shader::ShaderProgramSources(vertexShader, fragmentShader));
		//std::shared_ptr<internal::GLShader> shaderGL = std::dynamic_pointer_cast<internal::GLShader*>(shader);
		std::shared_ptr<internal::GLShader> bp = std::dynamic_pointer_cast<internal::GLShader>(shader);
		bp->bind();
		bp->setUniform1i("mat.height_texture", 0);
		bp->unbind();

		MaterialInfo in;
		in.shader = shader;
		in.name = "terrainMaterial";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST;
		matPtr = MaterialLibrary::create(in);
		matPtr->setValue("shines", 64.f);
		matPtr->setValue("width", 128.f);
	}

	{
		std::string waterVertexShader = R"(
		#version 330 core

		layout(location = 0) in vec2 a_pos;

		struct MAT {
			vec4 color;
			sampler2D height_texture;
			sampler2D terrain_texture;
			float shines;
			float width;
		};
		uniform MAT mat;


		struct GLO {

			mat4 view;
			mat4 proj;

			vec3 sunPos;

			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
			vec3 camera_pos;
			//attenuation
			float constant;
			float linear;
			float quadratic;
		};
		uniform GLO glo;

		uniform mat4 world;

		out vec2 outpost;

		out vec3 v_normal;
		out vec3 v_world_pos;

		void main()
		{

			vec3 central = vec3(a_pos.x,(texture2D(mat.height_texture, a_pos).r+texture2D(mat.terrain_texture, a_pos).r)/mat.width,a_pos.y);
			const float eps = 0.02;
			
			
			float hL = (texture2D(mat.height_texture, a_pos - vec2(eps, 0.0)).r+texture2D(mat.terrain_texture, a_pos - vec2(eps, 0.0)).r)/mat.width;
		    float hR = (texture2D(mat.height_texture, a_pos + vec2(eps, 0.0)).r+texture2D(mat.terrain_texture, a_pos + vec2(eps, 0.0)).r)/mat.width;
		    float hD = (texture2D(mat.height_texture, a_pos - vec2(0.0, eps)).r+texture2D(mat.terrain_texture, a_pos - vec2(0.0, eps)).r)/mat.width;
		    float hU = (texture2D(mat.height_texture, a_pos + vec2(0.0, eps)).r+texture2D(mat.terrain_texture, a_pos + vec2(0.0, eps)).r)/mat.width;	


			vec3 dx = vec3(2 * eps, hR - hL, 0);
			vec3 dy = vec3(0, hU - hD, 2 * eps);

			vec3 nor = normalize(cross(dy,dx));

			v_normal = (world * vec4(nor,0)).xyz;
			v_world_pos = (world * vec4(central, 1.0)).xyz;


			gl_Position = glo.proj * glo.view * vec4(v_world_pos,1.0);
			outpost=a_pos;
		}
		)";
		std::string waterFragmentShader = R"(
			#version 330 core
			struct GLO {
				mat4 view;
				mat4 proj;
				vec3 sunPos;
				vec3 ambient;
				vec3 diffuse;
				vec3 specular;
				vec3 camera_pos;
				//attenuation
				float constant;
				float linear;
				float quadratic;
			};
			uniform GLO glo;
						
				struct MAT {
				vec4 color;
				sampler2D height_texture;
				sampler2D terrain_texture;
				float shines;
				float width;
			};
			uniform MAT mat;
			in vec2 outpost;
			in vec3 v_normal;
			in vec3 v_world_pos;
			out vec4 color;
			void main()
			{

				float waterHeight = texture2D(mat.terrain_texture, outpost).r/mat.width;
				if (waterHeight < 0.005)
				{
					discard;
				}

				vec3 nor = normalize(v_normal);
				vec3 specularColor = vec3(0.5,0.5,0.5);

				vec3 diffuseColor = vec3(0.0,0.1,0.9);
				//DIFFUSE
				vec3 toSun = normalize(glo.sunPos - v_world_pos);
				vec3 diffuseLight = glo.diffuse * max(dot(toSun, nor),0.0) * diffuseColor;

				//SPECULAR
				vec3 toCamera = normalize(glo.camera_pos - v_world_pos);
				vec3 reflection = reflect(-toCamera, nor);
				vec3 reflectiveLight = vec3(0.0);
				if(mat.shines!=0)
					reflectiveLight = glo.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), mat.shines);

				color = vec4(diffuseLight+diffuseColor*0.3 + reflectiveLight,min(0.6,waterHeight*10));
			}
		)";

		auto waterShader = Shader::create(Shader::ShaderProgramSources(waterVertexShader, waterFragmentShader));
		std::shared_ptr<internal::GLShader> bp = std::dynamic_pointer_cast<internal::GLShader>(waterShader);
		bp->bind();
		bp->setUniform1i("mat.height_texture", 0);
		bp->setUniform1i("mat.terrain_texture", 1);
		bp->unbind();


		MaterialInfo in;
		in.shader = waterShader;
		in.name = "terrainMaterialWater";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST | MaterialFlags::FLAG_BLEND;
		waterMatPtr = MaterialLibrary::create(in);
		waterMatPtr->setValue("shines", 16.f);
		waterMatPtr->setValue("width", 128.f);
	}
}

// completely recreate terrain: mesh, height texture, etc., resizing ground
void TerrainLayer::createGround()
{
	g.resize(groundSize);

	mesh.create(g);
	waterMesh.create(g);


	recreateTerrain(g);


	static MeshData* data = nullptr;
	delete data;
	data = new MeshData;

	VertexBufferLayout layout = { g_typ::VEC2 };
	data->allocate(mesh.pos_vbo->getSize(), 3 * sizeof(gfloat), (g.width - 1) * (g.height - 1) * 6, layout);
	data->setID("terrainMesh");
	{
		meshPtr = std::make_shared<Mesh>();
		meshPtr->data = data;

		meshPtr->indexData.count = data->getIndicesCount();
		meshPtr->indexData.offset = 0;
		// this might be the most disgusting thing i ever did, but it cannot be nullptr since it checks during draw call,
		// it should not read from it though, >)
		meshPtr->indexData.indexBuffer = (IndexBuffer*)0x42;
		int index = 0;
		for (auto& e : mesh.pos_vbo->getLayout().getElements())
		{
			meshPtr->vertexData.declaration.addElement(index, e.typ, VertexType::POS);
			meshPtr->vertexData.binding.setBinding(index++, mesh.pos_vbo);
		}
		meshPtr->vao_temp = mesh.vao;
	}
	MeshLibrary::registerMesh(meshPtr);

	// must not forge to update texture to newer version as well
	matPtr->setValue("height_texture", mesh.height_texture);
	// must not forge to update texture to newer version as well
	waterMatPtr->setValue("height_texture", mesh.height_texture);
	waterMatPtr->setValue("terrain_texture", waterMesh.height_texture);

}




// ========== Terrain Generation ==========

enum class TerrainLayerType
{
	FLAT,
	SINE,
	PERLIN,
	BASIN
};

static TerrainLayerType terrainLayerType = TerrainLayerType::FLAT;

void TerrainLayer::recreateTerrain(EulerGround& g)  
{  
   using namespace BaseGroundImgui;  

   switch (terrainLayerType)  
   {  
   case TerrainLayerType::FLAT:  
       BaseGround::generateFlat(g, Flat::uiHeight);  
       break;  
   case TerrainLayerType::SINE:  
       BaseGround::generateGroundSine(g, Sine::uiAmplitude, Sine::uiFreq, Sine::uiPhase);  
       break;  
   case TerrainLayerType::PERLIN:  
       BaseGround::generatePerlinMultiOctave(g, Perlin::uiLayers, Perlin::uiScale, Perlin::uiOffset);  
       break;  
   case TerrainLayerType::BASIN:  
       BaseGround::generateBasin(g, Basin::uiScale, Basin::uiOffset);  
       break;  
   }  
   Droplet::balls = 0;  
   mesh.refreshHeight(g);  
   ZeroMemory(g.water_height.data(), g.water_height.size() * sizeof(gfloat));  
   waterMesh.refreshWaterHeight(g);  
}




// ========== TerrainLayer ==========

void TerrainLayer::onAttach()
{
	createMaterial();
	createGround();

	// terrain
	{
		auto entit = m_editorLayer.scene().createEntity("terrain");
		entit.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(10.f),
		                                           glm::vec3(0.f, 0.f, 0.f));
		entit.emplaceOrReplace<ModelComponent>(meshPtr->getID(), matPtr->getID());
		m_entity = entit;
	}
	// water
	{
		auto entit = m_editorLayer.scene().createEntity("terrainWater");
		entit.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(10.f),
		                                           glm::vec3(0.f, 0.f, 0.f));
		entit.emplaceOrReplace<ModelComponent>(meshPtr->getID(), waterMatPtr->getID());
		m_water_entity = entit;
	}
	//adding sphere
	{
		auto modelMat = Material::create({
			std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/scene/shaders/Model.shader")), "MAT",
			"modelMaterial"
		});
		modelMat->setValue("color", glm::vec4(0.0, 0.0, 0, 1));
		modelMat->setValue("shines", 64.f);
		MaterialLibrary::registerMaterial(modelMat);


		auto mesh = MeshLibrary::loadOrGet("res/scene/models/sphere.fbx");

		m_sphere = m_editorLayer.scene().createEntity("Sphere");
		//ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		m_sphere.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.1f), glm::vec3(0.f));
		m_sphere.emplaceOrReplace<ModelComponent>(mesh->getID(), modelMat->getID());
	}


	// set default camera pos
	m_editorLayer.scene().currentCamera().get<TransformComponent>().pos = {-4.72f, 13.41f, -6.36f};
	m_editorLayer.scene().currentCamera().get<TransformComponent>().rot = {-0.54f, -2.44f, 0.f};
}

void TerrainLayer::onRender()
{
	// update shader 
	matPtr->setValue("width", (float)g.width);
	waterMatPtr->setValue("width", (float)g.width);

	// send heightmap to texture
	mesh.refreshHeight(g);
	if (toggle_render_water)
		waterMesh.refreshWaterHeight(g);

	// disable water scene component
	m_water_entity.get<TagComponent>().enabled = toggle_render_water;
}

void TerrainLayer::onEvent(Event& e)
{
	// use right mouse button to apply brush terrain
	if (e.getEventType() == Event::MousePress)
	{
		auto& event = (MousePressEvent&)e;
		if (event.getButton() != MouseCode::RIGHT)
			return;


		auto pBigger = pointer_relative_pos * (float)g.width;
		auto pp = glm::vec2(pBigger.x, pBigger.z);

		int radiusRatio = 6;
		int radius = g.width / radiusRatio;
		constexpr float amount = 5.f;


		int startX = std::max(0, (int)pp.x - radius);
		int startY = std::max(0, (int)pp.y - radius);
		int endX = std::min(g.width - 1, (int)pp.x + radius);
		int endY = std::min(g.height - 1, (int)pp.y + radius);


		for (int y = startY; y <= endY; y++)
			for (int x = startX; x <= endX; x++)
			{
				auto distance = glm::distance(pp, glm::vec2(x, y));
				distance /= radius;
				g.terrain_height[x + y * g.width] += amount * glm::max(0.f, 1 - distance);
			}
	}
}

void TerrainLayer::onImGuiRender()
{
	static bool showBase = true;

	if (!ImGui::Begin("Terrain Eroder", &showBase))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("tabsi"))
	{
		if (ImGui::BeginTabItem("Generators"))
		{
			onImGuiRenderGenerator();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Simulators"))
		{
			onImGuiRenderSimulator();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void TerrainLayer::onImGuiRenderSimulator()
{
	using namespace ter;
	// ===================== SIMULATORS =====================
	ImGui::SeparatorText("Simulation");
	ImGui::SetItemTooltip("Two simulation methods available\nFor both methods you can adjust simulation speed");

	bool eulerSimOpen = ImGui::CollapsingHeader("EulerSim");
	ImGui::SetItemTooltip("Every cell is connected to its 4 neighbors and transfers water and soil to them\nComputationally expensive, not recommended to tinker with sizes bigger than 256x256");
	if (eulerSimOpen)
	{
		if (ImGui::Button("Init"))
			m_euler.init(g);
		ImGui::SetItemTooltip("Restarts sim, clears water");
		if (ImGui::Button("Step"))
			m_euler.step(g);

		ImGui::PushStyleColor(ImGuiCol_Button,
			toggle_sim_euler ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			toggle_sim_euler ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			toggle_sim_euler ? ImVec4(0.1f, 0.6f, 0.1f, 1.0f) : ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

		toggle_sim_euler ^= ImGui::Button("Play");
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::Button("Reset Simulation"))
			recreateTerrain(g);
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");


		int min = 1;
		int max = 10;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_euler, &min, &max, "%ld");
		ImGui::Checkbox("Render Water", &toggle_render_water);

		m_euler.imguiRender();
	}

	bool dropSimOpen = ImGui::CollapsingHeader("Droplet Sim");
	ImGui::SetItemTooltip("Drop randomly spawns on the terrain and simulate erosion by moving around, picking up soil and depositing it elsewhere");
	if (dropSimOpen)
	{
		static bool stepSucces = true;

		ImGui::PushID("Drop Sim");
		if (ImGui::Button("Init"))
		{
			stepSucces = true;
			m_droplet.init(g);
		}
		ImGui::SetItemTooltip("Place a new droplet on the terrain");

		ImGui::BeginDisabled(!stepSucces);
		if (ImGui::Button("Step"))
			stepSucces = m_droplet.step(g);
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Move drop by one step");


		if (ImGui::Button("One Whole Drop"))
		{
			stepSucces = true;
			m_droplet.init(g);
			while (m_droplet.step(g));
		}
		ImGui::SetItemTooltip("Run the whole 1 drop simulation from spawning it to evaporation");

		ImGui::PushStyleColor(ImGuiCol_Button,
		                      toggle_sim_drop ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
		                      toggle_sim_drop ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
		                      toggle_sim_drop ? ImVec4(0.1f, 0.6f, 0.1f, 1.0f) : ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
		if (ImGui::Button("Play"))
		{
			toggle_sim_drop = !toggle_sim_drop;
		}
		ImGui::PopStyleColor(3);
		ImGui::SetItemTooltip("Run the simulation based on the set speed");
		ImGui::SameLine();
		if (ImGui::Button("Reset Simulation"))
			recreateTerrain(g);
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");


		int min = 1;
		int max = 1000000;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_drop, &min, &max, "%ld",
		                    ImGuiSliderFlags_Logarithmic);


		ImGui::InputInt("Stop at # balls", &stopAtBalls);
		ImGui::SetItemTooltip("Stop simulation after this many balls are spawned\nSet to -1 to disable stopping at balls");
		if (stopAtBalls == Droplet::balls)
			toggle_sim_drop = false;


		//ImGui::Text("Droplet height %.1f",
		//	ter::interpolate2D(g.terrain_height, g.width, g.height, m_droplet.pos.x, m_droplet.pos.y));

		ImGui::PopID();

		m_droplet.imguiRender();


		if (ImGui::Button("RunBenchMark"))
			runDropBenchmark(10);
		ImGui::SetItemTooltip("Run the benchmark\nWill make app unresponsive for a while!\nWatch console for results");
	}


	ImGui::SeparatorText("Stats");
	if (ImGui::BeginTable(
		"Stats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ImGuiAddTableRow("Initial Ground", "%.1f", totalGround);
		ImGuiAddTableRow("Ground Diff", "%.1f", totalGround - currentGround);
		ImGuiAddTableRow("Water", "%.1f", currentWater);
		ImGuiAddTableRow("Ground", "%.1f", currentGround);
		ImGuiAddTableRow("Sediment", "%.1f", currentSediment);
		ImGuiAddTableRow("Max Sediment", "%.1f", maxSediment);
		ImGuiAddTableRow("Total Soil", "%.1f", currentGround + currentSediment);
		ImGuiAddTableRow("Min/Max Terrain", "%.3f / %.3f", minTerrain, maxTerrain);
		ImGuiAddTableRow("Pointer Pos (Rel)", "%.3f, %.3f, %.3f", pointer_relative_pos.x, pointer_relative_pos.y,
		                 pointer_relative_pos.z);
		ImGui::EndTable();
	}

	ImGui::SeparatorText("Looking At");
	ImGui::PushID("LookingAt");

	auto idx =
		glm::clamp((int)(pointer_relative_pos.x * g.width), 0, g.width - 1) +
		glm::clamp((int)(pointer_relative_pos.z * g.height), 0, g.height - 1) * g.width;


	ImGui::Text("Terrain Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * g.width,
	            g.terrain_height[idx], (float)pointer_relative_pos.z * g.height);
	ImGui::Text("Water Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * g.width,
	            g.water_height[idx], (float)pointer_relative_pos.z * g.height);

	ImGui::PopID();
}

void TerrainLayer::onImGuiRenderGenerator()
{
	using namespace BaseGroundImgui;

	// ===================== BASE TERRAIN GENERATION =====================
	ImGui::SeparatorText("Terrain Generation");
	ImGui::SetItemTooltip(
		"This section allows you to configure the terrain before starting any simulation\nAdjusting any parameter here will reset the entire mesh, removing all simulation progress");
	{
		ImGui::InputInt("Terrain Resolution", &groundSize);
		ImGui::SetItemTooltip(
			"Horizontal Mesh Resolution\nNumber of cells in a row\nShould be a power of 2\nTo apply changes press \"Apply Resolution\"");

		if (ImGui::Button("Apply Resolution"))
			createGround();
		ImGui::SetItemTooltip("Resizes terrain to \"Terrain Resolution\"");


		ImGui::SameLine();
		if (ImGui::Button("Reset Simulation"))
			recreateTerrain(g);
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");


		static int countDownToApplyChanges = 0;
		// terrain gen

		bool dirty = false;

		if (Perlin::Show())
		{
			dirty = true;
			terrainLayerType = TerrainLayerType::PERLIN;
		}
		if (Basin::Show())
		{
			dirty = true;
			terrainLayerType = TerrainLayerType::BASIN;
		}
		if (Sine::Show())
		{
			dirty = true;
			terrainLayerType = TerrainLayerType::SINE;
		}
		if (Flat::Show())
		{
			dirty = true;
			terrainLayerType = TerrainLayerType::FLAT;
		}

		if (dirty)
			countDownToApplyChanges = 3;


		if (countDownToApplyChanges > 0)
		{
			countDownToApplyChanges--;
			if (!countDownToApplyChanges)
				recreateTerrain(g);
		}
	}
}

void TerrainLayer::onUpdate()
{
	ND_PROFILE_METHOD();

	simulate(g);

	calculateStatistics(g);

	auto cam = m_editorLayer.scene().currentCamera();
	auto camPos = cam.get<TransformComponent>().pos;
	auto mousePx = APin().getMouseLocation();
	auto worldPos = camPos + m_editorLayer.screenToWorld(mousePx) * m_editorLayer.getDepthAtScreen(APin().getMouseLocation());

	// now figure out where on mesh the point is
	auto meshWorldMatrix = m_entity.get<TransformComponent>();

	pointer_relative_pos = glm::vec3(glm::inverse(meshWorldMatrix.trans) * glm::vec4(worldPos, 1));


	gvec3 totalPos = gvec3(m_droplet.pos.x / g.width,
		g.terrain_height[glm::clamp(
			(int)(m_droplet.pos.y) * g.width + (int)m_droplet.pos.x, 0,
			g.width * g.width - 1)] / g.width,
		m_droplet.pos.y / g.width);
	m_sphere.get<TransformComponent>().pos = glm::vec3(meshWorldMatrix.trans * glm::vec4(totalPos, 1));
}




// ========== Simulation ==========

void TerrainLayer::simulate(EulerGround& g)
{
	if (toggle_sim_drop)
	{
		for (int i = 0; i < playspeed_sim_drop; i++)
		{
			if (!m_droplet.step(g))
			{
				if (stopAtBalls == Droplet::balls)
					return;
				m_droplet.init(g);
			}
		}
	}
	else if (toggle_sim_euler)
	{
		for (int i = 0; i < playspeed_sim_euler; i++)
		{
			m_euler.step(g);
		}
	}
}

void TerrainLayer::runDropBenchmark(int passes)
{
	ND_INFO("Running performance matrix for droplet sim with {} passes", passes);

	static const int kTerrain[] = {256, 512, 1024, 2048};
	static const int kBalls[] = {5000, 10000, 20000, 50000, 100000};

	NBT results;

	for (int terrain : kTerrain)
	{
		groundSize = terrain;
		createGround();
		BaseGround::generatePerlinMultiOctave(
			g,
			BaseGroundImgui::Perlin::uiLayers,
			BaseGroundImgui::Perlin::uiScale,
			BaseGroundImgui::Perlin::uiOffset);

		NBT terrainRow;

		ND_TRACE("Testing terrain size: {}", terrain);

		for (int balls : kBalls)
		{
			uint64_t totalUS = 0;

			for (int p = 0; p < passes; ++p)
			{
				/* ------------ one full pass ------------ */
				Droplet::balls = 0; // reset static counter (if it is static)
				m_droplet.init(g); // start fresh

				TimerStaper timer("dropletPass"); // start stopwatch

				while (Droplet::balls < balls)
				{
					if (!m_droplet.step(g)) // droplet came to rest
						m_droplet.init(g); // start a new one
				}

				totalUS += timer.getUS(); // stop stopwatch
			}

			terrainRow[std::to_string(balls)] = totalUS / passes / 1000; // mean ms
		}

		results[std::to_string(terrain)] = std::move(terrainRow);
	}

	ND_INFO("Saving results to perf.json");
	NBT::saveToFile("perf.json", results);
}

void TerrainLayer::calculateStatistics(EulerGround& g)
{
	currentGround = 0;
	currentWater = 0;
	minTerrain = std::numeric_limits<gfloat>::max();
	maxTerrain = 0;
	currentSediment = 0;
	//maxSediment = 0;
	// calculate the ground height
	for (int y = 0; y < g.height; y++)
		for (int x = 0; x < g.width; x++)
		{
			auto idx = y * g.width + x;
			currentGround += g.terrain_height[idx];
			currentWater += g.water_height[idx];
			currentSediment += g.sediment[idx];
			minTerrain = glm::min(minTerrain, g.terrain_height[idx]);
			maxTerrain = glm::max(maxTerrain, g.terrain_height[idx]);
			maxSediment = glm::max(maxSediment, g.sediment[idx]);
		}
}
