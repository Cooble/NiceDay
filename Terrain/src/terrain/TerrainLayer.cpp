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

// Primitives for rendering heightmap
struct TerrainMesh
{
	MeshPtr meshPtr;
	TexturePtr terrain_texture;
	TexturePtr water_texture;

	void create(EulerGround& map)
	{
		auto mesh_data = new MeshData(
			map.width * map.height * 2,
			sizeof(float) * 2,
			(map.width - 1) * (map.height - 1) * 6,
			VertexBufferLayout{ g_typ::VEC2 });

		mesh_data->setID("terrainMesh");

		gfloat scaler = 1.f / (map.width - 1);

		auto f = (float*)mesh_data->getVertices();

		for (int y = 0; y < map.height; y++)
			for (int x = 0; x < map.width; ++x)
			{
				f[(y * map.width + x) * 2 + 0] = scaler * x;
				f[(y * map.width + x) * 2 + 1] = scaler * y;
			}

		TextureInfo info = TextureInfo().size(map.width, map.height).format(TextureFormat::RED_32F).wrapMode(
			TextureWrapMode::CLAMP_TO_EDGE);

		terrain_texture = std::shared_ptr<Texture>(Texture::create(info));
		water_texture = std::shared_ptr<Texture>(Texture::create(info));

		auto indices = (uint32_t*)mesh_data->getIndices();

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

		meshPtr = MeshLibrary::buildNewMesh(mesh_data);
		MeshLibrary::registerMesh(meshPtr);
	}

	void refreshHeight(EulerGround& map)
	{
		terrain_texture->setPixels(map.terrain_height.data());
	}

	void refreshWaterHeight(EulerGround& map)
	{
		water_texture->setPixels(map.water_height.data());
	}
};

// Static variables for UI state
static glm::vec3 pointer_relative_pos;
static bool toggle_render_water = true;
static bool toggle_sim_drop = false;
static bool toggle_sim_euler = false;
static int playspeed_sim_drop = 1;
static int playspeed_sim_euler = 1;
static int stopAtBalls = -1;

// Statistics (computed for active terrain)
static gfloat totalGround = 0;
static gfloat currentGround = 0;
static gfloat currentSediment = 0;
static gfloat maxSediment = 0;
static gfloat currentWater = 0;
static gfloat minTerrain = 0, maxTerrain = 0;

// ========== TerrainInstance Implementation ==========

void TerrainInstance::init(int size,Euler::EulerSettings* s)
{
	ground.resize(size);
	mesh = std::make_unique<TerrainMesh>();
	mesh->create(ground);
	euler_sim.init(ground, s);
}

void TerrainInstance::recreateTerrain(TerrainLayerType type)
{
	using namespace BaseGroundImgui;

	switch (type)
	{
	case TerrainLayerType::FLAT:
		BaseGround::generateFlat(ground, Flat::uiHeight);
		break;
	case TerrainLayerType::SINE:
		BaseGround::generateGroundSine(ground, Sine::uiAmplitude, Sine::uiFreq, Sine::uiPhase);
		break;
	case TerrainLayerType::PERLIN:
		BaseGround::generatePerlinMultiOctave(ground, Perlin::uiLayers, Perlin::uiScale, Perlin::uiOffset);
		break;
	case TerrainLayerType::BASIN:
		BaseGround::generateBasin(ground, Basin::uiScale, Basin::uiOffset);
		break;
	}
	ZeroMemory(ground.water_height.data(), ground.water_height.size() * sizeof(gfloat));


	//ground.new_terrain_height = ground.terrain_height;
	//ground.original_height = ground.terrain_height;
	// dont forget to re-init the euler sim
	euler_sim.init(ground, euler_sim.s);

	mesh->refreshHeight(ground);
}

void TerrainInstance::refreshMeshData()
{
	mesh->refreshHeight(ground);
	mesh->refreshWaterHeight(ground);
}

// ========== TerrainLayer Implementation ==========

TerrainLayer::TerrainLayer(nd::EditorLayer& l) : m_editor_layer(l)
{
}

void TerrainLayer::createMaterials()
{
	std::string vertexShader = R"(
			#version 330 core

			layout(location = 0) in vec2 a_pos;

			struct MAT {
				vec4 color;
				sampler2D water_texture;
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
				vec3 central = vec3(a_pos.x,texture2D(mat.terrain_texture, a_pos).r/mat.width,a_pos.y);
				const float eps = 0.02;

				float hL = texture2D(mat.terrain_texture, a_pos - vec2(eps, 0.0)).r/mat.width;
			    float hR = texture2D(mat.terrain_texture, a_pos + vec2(eps, 0.0)).r/mat.width;
			    float hD = texture2D(mat.terrain_texture, a_pos - vec2(0.0, eps)).r/mat.width;
			    float hU = texture2D(mat.terrain_texture, a_pos + vec2(0.0, eps)).r/mat.width;

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
				float constant;
				float linear;
				float quadratic;
			};
			uniform GLO glo;
						
			struct MAT {
				vec4 color;
				sampler2D water_texture;
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
				
				vec3 toSun = normalize(glo.sunPos - v_world_pos);
				vec3 diffuseLight = glo.diffuse * max(dot(toSun, nor),0.0) * diffuseColor;

				vec3 toCamera = normalize(glo.camera_pos - v_world_pos);
				vec3 reflection = reflect(-toCamera, nor);
				vec3 reflectiveLight = vec3(0.0);
				if(mat.shines!=0)
					reflectiveLight = glo.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), mat.shines);

				color = vec4(diffuseLight+diffuseColor*0.3 + reflectiveLight,1);				
			}
		)";

	std::string waterVertexShader = R"(
		#version 330 core

		layout(location = 0) in vec2 a_pos;

		struct MAT {
			vec4 color;
			sampler2D water_texture;
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
			vec3 central = vec3(a_pos.x,(texture2D(mat.water_texture, a_pos).r+texture2D(mat.terrain_texture, a_pos).r)/mat.width,a_pos.y);
			const float eps = 0.02;
			
			float hL = (texture2D(mat.water_texture, a_pos - vec2(eps, 0.0)).r+texture2D(mat.terrain_texture, a_pos - vec2(eps, 0.0)).r)/mat.width;
		    float hR = (texture2D(mat.water_texture, a_pos + vec2(eps, 0.0)).r+texture2D(mat.terrain_texture, a_pos + vec2(eps, 0.0)).r)/mat.width;
		    float hD = (texture2D(mat.water_texture, a_pos - vec2(0.0, eps)).r+texture2D(mat.terrain_texture, a_pos - vec2(0.0, eps)).r)/mat.width;
		    float hU = (texture2D(mat.water_texture, a_pos + vec2(0.0, eps)).r+texture2D(mat.terrain_texture, a_pos + vec2(0.0, eps)).r)/mat.width;	

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
				float constant;
				float linear;
				float quadratic;
			};
			uniform GLO glo;
						
			struct MAT {
				vec4 color;
				sampler2D water_texture;
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
				float waterHeight = texture2D(mat.water_texture, outpost).r/mat.width;
				if (waterHeight < 0.005)
				{
					discard;
				}

				vec3 nor = normalize(v_normal);
				vec3 specularColor = vec3(0.5,0.5,0.5);
				vec3 diffuseColor = vec3(0.0,0.1,0.9);
				
				vec3 toSun = normalize(glo.sunPos - v_world_pos);
				vec3 diffuseLight = glo.diffuse * max(dot(toSun, nor),0.0) * diffuseColor;

				vec3 toCamera = normalize(glo.camera_pos - v_world_pos);
				vec3 reflection = reflect(-toCamera, nor);
				vec3 reflectiveLight = vec3(0.0);
				if(mat.shines!=0)
					reflectiveLight = glo.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), mat.shines);

				color = vec4(diffuseLight+diffuseColor*0.3 + reflectiveLight,max(0.2,min(0.65,waterHeight*10)));
			}
		)";

	// Create terrain material
	{
		auto shader = Shader::create(Shader::ShaderProgramSources(vertexShader, fragmentShader));
		MaterialInfo in;
		in.shader = shader;
		in.name = "terrainMaterialVanilla";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST;

		m_terrain_vanilla.m_terrain_material = MaterialLibrary::create(in);
		m_terrain_vanilla.m_terrain_material->setValue("shines", 64.f);
		m_terrain_vanilla.m_terrain_material->setValue("width", 128.f);

		in.name = "terrainMaterialSimd";
		m_terrain_simd.m_terrain_material = MaterialLibrary::create(in);
		m_terrain_simd.m_terrain_material->setValue("shines", 64.f);
		m_terrain_simd.m_terrain_material->setValue("width", 128.f);
	}

	// Create water material
	{
		auto waterShader = Shader::create(Shader::ShaderProgramSources(waterVertexShader, waterFragmentShader));
		MaterialInfo in;
		in.shader = waterShader;
		in.name = "waterMaterialVanilla";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST | MaterialFlags::FLAG_BLEND;
		m_terrain_vanilla.m_water_material = MaterialLibrary::create(in);
		m_terrain_vanilla.m_water_material->setValue("shines", 16.f);
		m_terrain_vanilla.m_water_material->setValue("width", 128.f);

		in.name = "waterMaterialSimd";
		m_terrain_simd.m_water_material = MaterialLibrary::create(in);
		m_terrain_simd.m_water_material->setValue("shines", 16.f);
		m_terrain_simd.m_water_material->setValue("width", 128.f);
	}
}

void TerrainLayer::createTerrainInstances()
{
	// Initialize vanilla terrain
	m_terrain_vanilla.euler_sim.sim_type = Euler::OPENCL;
	m_terrain_vanilla.init(m_ground_size, &m_euler_settings);
	m_terrain_vanilla.recreateTerrain(m_terrain_type);

	// Initialize SIMD terrain
	m_terrain_simd.euler_sim.sim_type = Euler::SIMD_PARALLEL;
	m_terrain_simd.init(m_ground_size, &m_euler_settings);
	m_terrain_simd.recreateTerrain(m_terrain_type);

	// Create entities for vanilla terrain (left side)
	{
		auto entit = m_editor_layer.scene().createEntity("terrain_vanilla");
		entit.emplaceOrReplace<TransformComponent>(
			glm::vec3(-6.f, 0.f, 0.f),
			glm::vec3(10.f),
			glm::vec3(0.f, 0.f, 0.f));
		entit.emplaceOrReplace<ModelComponent>(
			m_terrain_vanilla.mesh->meshPtr->getID(),
			m_terrain_vanilla.m_terrain_material->getID());
		m_terrain_vanilla.terrain_entity = entit;
		m_terrain_vanilla.terrain_entity.get<TagComponent>().enabled = m_enable_vanilla;

		auto water_entit = m_editor_layer.scene().createEntity("water_vanilla");
		water_entit.emplaceOrReplace<TransformComponent>(
			glm::vec3(-6.f, 0.f, 0.f),
			glm::vec3(10.f),
			glm::vec3(0.f, 0.f, 0.f));
		water_entit.emplaceOrReplace<ModelComponent>(
			m_terrain_vanilla.mesh->meshPtr->getID(),
			m_terrain_vanilla.m_water_material->getID());
		m_terrain_vanilla.water_entity = water_entit;
		m_terrain_vanilla.water_entity.get<TagComponent>().enabled = m_enable_vanilla;

	}

	// Create entities for SIMD terrain (right side)
	{
		auto entit = m_editor_layer.scene().createEntity("terrain_simd");
		entit.emplaceOrReplace<TransformComponent>(
			glm::vec3(6.f, 0.f, 0.f),
			glm::vec3(10.f),
			glm::vec3(0.f, 0.f, 0.f));
		entit.emplaceOrReplace<ModelComponent>(
			m_terrain_simd.mesh->meshPtr->getID(),
			m_terrain_simd.m_terrain_material->getID());
		m_terrain_simd.terrain_entity = entit;
		m_terrain_simd.terrain_entity.get<TagComponent>().enabled = m_enable_simd;

		auto water_entit = m_editor_layer.scene().createEntity("water_simd");
		water_entit.emplaceOrReplace<TransformComponent>(
			glm::vec3(6.f, 0.f, 0.f),
			glm::vec3(10.f),
			glm::vec3(0.f, 0.f, 0.f));
		water_entit.emplaceOrReplace<ModelComponent>(
			m_terrain_simd.mesh->meshPtr->getID(),
			m_terrain_simd.m_water_material->getID());
		m_terrain_simd.water_entity = water_entit;
		m_terrain_simd.water_entity.get<TagComponent>().enabled = m_enable_simd;
	}

	// Update vanilla material textures
	m_terrain_vanilla.m_terrain_material->setValue("terrain_texture", m_terrain_vanilla.mesh->terrain_texture);
	m_terrain_vanilla.m_terrain_material->setValue("water_texture", m_terrain_vanilla.mesh->water_texture);

	m_terrain_vanilla.m_water_material->setValue("terrain_texture", m_terrain_vanilla.mesh->terrain_texture);
	m_terrain_vanilla.m_water_material->setValue("water_texture", m_terrain_vanilla.mesh->water_texture);

	// Update SIMD material textures
	m_terrain_simd.m_terrain_material->setValue("terrain_texture", m_terrain_simd.mesh->terrain_texture);
	m_terrain_simd.m_terrain_material->setValue("water_texture", m_terrain_simd.mesh->water_texture);

	m_terrain_simd.m_water_material->setValue("terrain_texture", m_terrain_simd.mesh->terrain_texture);
	m_terrain_simd.m_water_material->setValue("water_texture", m_terrain_simd.mesh->water_texture);

	
}

void TerrainLayer::recreateBothTerrains()
{
	Droplet::balls = 0;
	m_terrain_vanilla.recreateTerrain(m_terrain_type);
	m_terrain_simd.recreateTerrain(m_terrain_type);
}

TerrainInstance& TerrainLayer::getActiveTerrainInstance()
{
	return m_terrain_vanilla; // For mouse interaction, use vanilla terrain
}

void TerrainLayer::onAttach()
{
	createMaterials();
	createTerrainInstances();

	// Add sphere for droplet visualization
	{
		auto modelMat = Material::create({
			std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/scene/shaders/Model.shader")), "MAT",
			"modelMaterial"
			});
		modelMat->setValue("color", glm::vec4(0.0, 0.0, 0, 1));
		modelMat->setValue("shines", 64.f);
		MaterialLibrary::registerMaterial(modelMat);

		auto mesh = MeshLibrary::loadOrGet("res/scene/models/sphere.fbx");

		m_sphere = m_editor_layer.scene().createEntity("Sphere");
		m_sphere.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.1f), glm::vec3(0.f));
		m_sphere.emplaceOrReplace<ModelComponent>(mesh->getID(), modelMat->getID());
	}
	// add second light
	{
		auto ent = m_editor_layer.scene().createEntity("Light2");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(16.f,10,10), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<LightComponent>();
	}

	// Set default camera position
	m_editor_layer.scene().currentCamera().get<TransformComponent>().pos = { -4.72f, 13.41f, -6.36f };
	m_editor_layer.scene().currentCamera().get<TransformComponent>().rot = { -0.54f, -2.44f, 0.f };
}

void TerrainLayer::onRender()
{
	// Update shader widths
	m_terrain_vanilla.m_terrain_material->setValue("width", (float)m_terrain_vanilla.ground.width);
	m_terrain_vanilla.m_water_material->setValue("width", (float)m_terrain_vanilla.ground.width);

	m_terrain_simd.m_terrain_material->setValue("width", (float)m_terrain_simd.ground.width);
	m_terrain_simd.m_water_material->setValue("width", (float)m_terrain_simd.ground.width);
	

	// Refresh mesh data for both terrains
	if (m_enable_vanilla)
		m_terrain_vanilla.refreshMeshData();
	if (m_enable_simd)
		m_terrain_simd.refreshMeshData();
	

	// Control water rendering
	m_terrain_vanilla.water_entity.get<TagComponent>().enabled = toggle_render_water && m_enable_vanilla;
	m_terrain_simd.water_entity.get<TagComponent>().enabled = toggle_render_water && m_enable_simd;

	// Control SIMD terrain visibility
	m_terrain_vanilla.terrain_entity.get<TagComponent>().enabled = m_enable_vanilla;
	m_terrain_simd.terrain_entity.get<TagComponent>().enabled = m_enable_simd;
	m_terrain_simd.water_entity.get<TagComponent>().enabled = m_enable_simd && toggle_render_water;
}

void TerrainLayer::onEvent(Event& e)
{
	// Apply brush to terrain
	if (e.getEventType() == Event::MousePress)
	{
		auto& event = (MousePressEvent&)e;
		if (event.getButton() != MouseCode::RIGHT)
			return;

		auto& activeGround = getActiveTerrainInstance().ground;
		auto pBigger = pointer_relative_pos * (float)activeGround.width;
		auto pp = glm::vec2(pBigger.x, pBigger.z);

		int radiusRatio = 6;
		int radius = activeGround.width / radiusRatio;
		constexpr float amount = 5.f;

		int startX = std::max(0, (int)pp.x - radius);
		int startY = std::max(0, (int)pp.y - radius);
		int endX = std::min(activeGround.width - 1, (int)pp.x + radius);
		int endY = std::min(activeGround.height - 1, (int)pp.y + radius);

		for (int y = startY; y <= endY; y++)
			for (int x = startX; x <= endX; x++)
			{
				auto distance = glm::distance(pp, glm::vec2(x, y));
				distance /= radius;
				activeGround.terrain_height[x + y * activeGround.width] += amount * glm::max(0.f, 1 - distance);
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

	ImGui::SeparatorText("Comparison Mode");
	ImGui::Checkbox("Enable SIMD", &m_enable_simd);
	ImGui::Checkbox("Enable OPENCL", &m_enable_vanilla);
	
	ImGui::SetItemTooltip("Show both vanilla and SIMD implementations side by side");

	ImGui::SeparatorText("Simulation");
	ImGui::SetItemTooltip("Two simulation methods available\nFor both methods you can adjust simulation speed");

	bool eulerSimOpen = ImGui::CollapsingHeader("EulerSim");
	ImGui::SetItemTooltip(
		"Every cell is connected to its 4 neighbors and transfers water and soil to them\nComputationally expensive, not recommended to tinker with sizes bigger than 256x256");

	if (eulerSimOpen)
	{
		if (ImGui::Button("Init"))
		{
			if (m_enable_vanilla)
				m_terrain_vanilla.euler_sim.init(m_terrain_vanilla.ground, &m_euler_settings);
			if (m_enable_simd)
				m_terrain_simd.euler_sim.init(m_terrain_simd.ground,&m_euler_settings);
		}
		ImGui::SetItemTooltip("Restarts sim, clears water");

		if (ImGui::Button("Step"))
		{
			if (m_enable_vanilla)
				m_terrain_vanilla.euler_sim.step(m_terrain_vanilla.ground);
			if (m_enable_simd)
				m_terrain_simd.euler_sim.step(m_terrain_simd.ground);
		}

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
			recreateBothTerrains();
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");

		int min = 1;
		int max = 50;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_euler, &min, &max, "%ld");
		ImGui::Checkbox("Render Water", &toggle_render_water);

		m_terrain_vanilla.euler_sim.imguiRender();
	}

	bool dropSimOpen = ImGui::CollapsingHeader("Droplet Sim");
	ImGui::SetItemTooltip(
		"Drop randomly spawns on the terrain and simulate erosion by moving around, picking up soil and depositing it elsewhere");

	if (dropSimOpen)
	{
		static bool stepSucces = true;

		ImGui::PushID("Drop Sim");
		if (ImGui::Button("Init"))
		{
			stepSucces = true;
			m_droplet.init(m_terrain_vanilla.ground);
		}
		ImGui::SetItemTooltip("Place a new droplet on the terrain");

		ImGui::BeginDisabled(!stepSucces);
		if (ImGui::Button("Step"))
			stepSucces = m_droplet.step(m_terrain_vanilla.ground);
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Move drop by one step");

		if (ImGui::Button("One Whole Drop"))
		{
			stepSucces = true;
			m_droplet.init(m_terrain_vanilla.ground);
			while (m_droplet.step(m_terrain_vanilla.ground));
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
			recreateBothTerrains();
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");

		int min = 1;
		int max = 1000000;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_drop, &min, &max, "%ld",
			ImGuiSliderFlags_Logarithmic);

		ImGui::InputInt("Stop at # balls", &stopAtBalls);
		ImGui::SetItemTooltip(
			"Stop simulation after this many balls are spawned\nSet to -1 to disable stopping at balls");
		if (stopAtBalls == Droplet::balls)
			toggle_sim_drop = false;

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

	ImGui::Text("Center Vanilla Sed: %.5f", (float)m_terrain_vanilla.ground.sediment[700]);
	ImGui::Text("Center SIMD    Sed: %.5f", (float)m_terrain_simd.ground.sediment[700]);

	ImGui::Text("Center Vanilla NewSed: %.5f", (float)m_terrain_vanilla.ground.new_sediment[700]);
	ImGui::Text("Center SIMD    NewSed: %.5f", (float)m_terrain_simd.ground.new_sediment[700]);

	ImGui::Text("Center Vanilla Terrain Height: %.5f", (float)m_terrain_vanilla.ground.terrain_height[700]);
	ImGui::Text("Center SIMD    Terrain Height: %.5f", (float)m_terrain_simd.ground.terrain_height[700]);

	
	ImGui::SeparatorText("Looking At");
	ImGui::PushID("LookingAt");

	auto& activeGround = getActiveTerrainInstance().ground;
	auto idx =
		glm::clamp((int)(pointer_relative_pos.x * activeGround.width), 0, activeGround.width - 1) +
		glm::clamp((int)(pointer_relative_pos.z * activeGround.height), 0, activeGround.height - 1) * activeGround.width;

	ImGui::Text("Terrain Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * activeGround.width,
		activeGround.terrain_height[idx], (float)pointer_relative_pos.z * activeGround.height);
	ImGui::Text("Water Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * activeGround.width,
		activeGround.water_height[idx], (float)pointer_relative_pos.z * activeGround.height);

	ImGui::PopID();
}

void TerrainLayer::onImGuiRenderGenerator()
{
	using namespace BaseGroundImgui;

	ImGui::SeparatorText("Terrain Generation");
	ImGui::SetItemTooltip(
		"This section allows you to configure the terrain before starting any simulation\nAdjusting any parameter here will reset the entire mesh, removing all simulation progress");

	{
		ImGui::InputInt("Terrain Resolution", &m_ground_size);
		ImGui::SetItemTooltip(
			"Horizontal Mesh Resolution\nNumber of cells in a row\nShould be a power of 2\nTo apply changes press \"Apply Resolution\"");

		if (ImGui::Button("Apply Resolution"))
		{
			createTerrainInstances();
		}
		ImGui::SetItemTooltip("Resizes terrain to \"Terrain Resolution\"");

		ImGui::SameLine();
		if (ImGui::Button("Reset Simulation"))
			recreateBothTerrains();
		ImGui::SetItemTooltip("Resets the terrain to the initial state, removing all simulation progress");

		static int countDownToApplyChanges = 0;
		bool dirty = false;

		if (Perlin::Show())
		{
			dirty = true;
			m_terrain_type = TerrainLayerType::PERLIN;
		}
		if (Basin::Show())
		{
			dirty = true;
			m_terrain_type = TerrainLayerType::BASIN;
		}
		if (Sine::Show())
		{
			dirty = true;
			m_terrain_type = TerrainLayerType::SINE;
		}
		if (Flat::Show())
		{
			dirty = true;
			m_terrain_type = TerrainLayerType::FLAT;
		}

		if (dirty)
			countDownToApplyChanges = 3;

		if (countDownToApplyChanges > 0)
		{
			countDownToApplyChanges--;
			if (!countDownToApplyChanges)
				recreateBothTerrains();
		}
	}
}

void TerrainLayer::onUpdate()
{
	ND_PROFILE_METHOD();

	simulate();

	calculateStatistics(m_terrain_vanilla.ground);

	auto cam = m_editor_layer.scene().currentCamera();
	auto camPos = cam.get<TransformComponent>().pos;
	auto mousePx = APin().getMouseLocation();
	auto worldPos = camPos + m_editor_layer.screenToWorld(mousePx) * m_editor_layer.getDepthAtScreen(
		APin().getMouseLocation());

	// Figure out where on mesh the point is
	auto meshWorldMatrix = m_terrain_vanilla.terrain_entity.get<TransformComponent>();
	pointer_relative_pos = glm::vec3(glm::inverse(meshWorldMatrix.trans) * glm::vec4(worldPos, 1));

	// Update sphere position for droplet visualization
	auto& g = m_terrain_vanilla.ground;
	gvec3 totalPos = gvec3(m_droplet.pos.x / g.width,
		g.terrain_height[glm::clamp(
			(int)(m_droplet.pos.y) * g.width + (int)m_droplet.pos.x, 0,
			g.width * g.width - 1)] / g.width,
		m_droplet.pos.y / g.width);
	m_sphere.get<TransformComponent>().pos = glm::vec3(meshWorldMatrix.trans * glm::vec4(totalPos, 1));
}

void TerrainLayer::simulate()
{
	if (toggle_sim_drop)
	{
		for (int i = 0; i < playspeed_sim_drop; i++)
		{
			if (!m_droplet.step(m_terrain_vanilla.ground))
			{
				if (stopAtBalls == Droplet::balls)
					return;
				m_droplet.init(m_terrain_vanilla.ground);
			}
		}
	}
	else if (toggle_sim_euler)
	{
		if (m_enable_vanilla)
			m_terrain_vanilla.euler_sim.refreshParams(m_terrain_vanilla.ground, m_euler_settings);
		if (m_enable_simd)
			m_terrain_simd.euler_sim.refreshParams(m_terrain_simd.ground, m_euler_settings);


		for (int i = 0; i < playspeed_sim_euler; i++)
		{
			if (m_enable_vanilla)
				m_terrain_vanilla.euler_sim.step(m_terrain_vanilla.ground);
			if (m_enable_simd)
				m_terrain_simd.euler_sim.step(m_terrain_simd.ground);
		}

		if (m_enable_vanilla)
			m_terrain_vanilla.euler_sim.stepRender(m_terrain_vanilla.ground);
		if (m_enable_simd)
			m_terrain_simd.euler_sim.stepRender(m_terrain_simd.ground);
	}
}

void TerrainLayer::runDropBenchmark(int passes)
{
	ND_INFO("Running performance matrix for droplet sim with {} passes", passes);

	static const int kTerrain[] = { 256, 512, 1024, 2048 };
	static const int kBalls[] = { 5000, 10000, 20000, 50000, 100000 };

	NBT results;

	for (int terrain : kTerrain)
	{
		m_ground_size = terrain;
		createTerrainInstances();
		BaseGround::generatePerlinMultiOctave(
			m_terrain_vanilla.ground,
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
				Droplet::balls = 0;
				m_droplet.init(m_terrain_vanilla.ground);

				TimerStaper timer("dropletPass");

				while (Droplet::balls < balls)
				{
					if (!m_droplet.step(m_terrain_vanilla.ground))
						m_droplet.init(m_terrain_vanilla.ground);
				}

				totalUS += timer.getUS();
			}

			terrainRow[std::to_string(balls)] = totalUS / passes / 1000;
		}

		results[std::to_string(terrain)] = std::move(terrainRow);
	}

	ND_INFO("Saving results to perf.json");
	NBT::saveToFile("perf.json", results);
}

void TerrainLayer::calculateStatistics(EulerGround& g)
{
	return;
	currentGround = 0;
	currentWater = 0;
	minTerrain = std::numeric_limits<gfloat>::max();
	maxTerrain = 0;
	currentSediment = 0;

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