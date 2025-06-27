#include "TerrainLayer.h"

#include "DropletSim.h"
#include "EulerSim.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "imgui.h"
#include "TUtils.h"
#include "../../TestNiceDay/src/NDTests.h"
#include "graphics/API/Shader.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/Effect.h"
#include "core/NBT.h"
#include "files/FUtil.h"
#include "scene/Camm.h"
#include "scene/Colli.h"
#include "scene/components.h"
#include "scene/EditorLayer.h"
#include "scene/Material.h"
#include "scene/NewScene.h"
#include "glm/gtc/noise.hpp"


using namespace nd;


static Entity sphere;


static Droplet droplet;
static Euler euler;

static NBT settings;

TerrainLayer::TerrainLayer(nd::EditorLayer& l) : m_editorLayer(l)
{
}


struct TerrainMesh
{
	// graphical primitives
	VertexBuffer* height_vbo = nullptr;
	VertexBuffer* pos_vbo;
	VertexArray* vao;
	IndexBuffer* index_buffer;
	TexturePtr height_texture;

	std::vector<float> float_height;

	void createGrid(Ground& map)
	{
		float_height.resize(map.width * map.height);
		ZeroMemory(float_height.data(), float_height.size() * sizeof(decltype(float_height)::value_type));


		if (height_vbo)
		{
			delete vao;
			delete height_vbo;
			delete pos_vbo;
			delete index_buffer;
		}

		gfloat scaler = 1.f / (map.width - 1);

		//height_vbo = VertexBuffer::create(map.terrain_height.data(), map.terrain_height.size() * sizeof(float));
		height_vbo = VertexBuffer::create(float_height.data(), float_height.size() * sizeof(float));
		height_vbo->setLayout({g_typ::FLOAT});

		auto f = std::vector<float>(map.width * map.height * 2);

		for (int y = 0; y < map.height; y++)
			for (int x = 0; x < map.width; ++x)
			{
				f[(y * map.width + x) * 2 + 0] = scaler * x;
				f[(y * map.width + x) * 2 + 1] = scaler * y;
			}

		TextureInfo info = TextureInfo().size(map.width, map.height).format(TextureFormat::RED).wrapMode(
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
		vao->addBuffer(*height_vbo);
		vao->addBuffer(*pos_vbo);
		vao->addBuffer(*index_buffer);
	}

	void refreshHeight(Ground& map)
	{
		// convert all float to gfloat
		for (int i = 0; i < map.terrain_height.size(); i++)
			float_height[i] = map.terrain_height[i] / map.width;

		//height_vbo->changeData((char*)float_height.data(), float_height.size() * sizeof(float), 0);
		height_texture->setPixels(float_height.data());
	}

	void refreshWaterHeight(Ground& map)
	{
		// convert all float to gfloat
		for (int i = 0; i < map.water_height.size(); i++)
			float_height[i] = (map.water_height[i] + map.terrain_height[i]) / map.width;
		//height_vbo->changeData((char*)float_height.data(), float_height.size() * sizeof(float), 0);
		height_texture->setPixels(float_height.data());
	}
};

static TerrainMesh mesh;
static TerrainMesh waterMesh;
static Ref<Mesh> meshPtr;

static MaterialPtr matPtr;
static MaterialPtr waterMatPtr;

static glm::vec3 pointer_relative_pos;


void TerrainLayer::onAttach()
{
	NBT::loadFromFile("terrain.settings", settings);
	//settings.load("x", flatCam.pos.x);

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
	// set default camera pos
	m_editorLayer.scene().currentCamera().get<TransformComponent>().rot = {-0.594f, -2.460f, 0.f};
	m_editorLayer.scene().currentCamera().get<TransformComponent>().pos = {-12.034f, 7.118f, -5.970f};


	{
		//adding sphere
		auto modelMat = Material::create({
			std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
			"modelMaterial"
		});
		modelMat->setValue("color", glm::vec4(0.0, 0.0, 0, 1));
		modelMat->setValue("shines", 64.f);
		MaterialLibrary::registerMaterial(modelMat);


		auto diffusePtr = std::shared_ptr<Texture>(Texture::create(TextureInfo("res/examples/images/crate.png")));

		auto mesh = MeshLibrary::loadOrGet("res/examples/models/sphere.fbx");

		auto ent = m_editorLayer.scene().createEntity("Sphere");
		//ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.1f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), modelMat->getID());
		sphere = ent;
	}

	//adding dragoon
	if (false)
	{
		if (!FUtil::exists("res/examples/models/dragon.bin"))
		{
			ND_INFO("Building dragon binary mesh");
			MeshDataFactory::writeBinaryFile("res/examples/models/dragon.bin",
			                                 *Colli::buildMesh("res/examples/models/dragon.obj"));
		} /*
		auto material = Material::create({
		;
		);
		material->setValue("shines", 64.f);
*		 /*/

		auto material = MaterialLibrary::create({
			ShaderLib::loadOrGetShader("res/shaders/Model.shader"), "MAT", "dragonMat"
		});
		material->setValue("shines", 64.f);


		//auto mesh = MeshLibrary::buildNewMesh(
		//	MeshDataFactory::readBinaryFile(ND_RESLOC("res/examples/models/dragon.bin")));
		auto mesh = MeshLibrary::registerMesh(
			MeshDataFactory::readBinaryFile(ND_RESLOC("res/examples/models/dragon.bin")));
		//auto mesh = NewMeshFactory::buildNewMesh(data);
		//mesh.get()->inde = Topology::TRIANGLES;
		//mat->setValue("color", gvec4(0.f, 1.f, 0.f, 1.f));

		//m_entity = m_editorLayer.scene().createEntity("dragoon");
		//m_entity.emplaceOrReplace<TransformComponent>(gvec3(0.f), gvec3(1.f), gvec3(0.f));
		//m_entity.emplaceOrReplace<ModelComponent>(mesh->getID(), material->getID());
	}
}

void TerrainLayer::onDetach()
{
	//settings.save("x", flatCam.pos.x);
	NBT::saveToFile("terrain.settings", settings);
}


void TerrainLayer::onRender()
{
}

void TerrainLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::MouseScroll)
	{
		auto& event = (MouseScrollEvent&)e;

		auto rain = event.getScrollX() + event.getScrollY();

		auto w = g.width;
		auto h = g.height;
		for (int y = 1; y < h - 1; y++)
			for (int x = 1; x < w - 1; x++)
			{
				auto yy = (gfloat)y / h;
				auto xx = (gfloat)x / w;
				gvec2 pos = {xx, yy};
				if (glm::distance(pos, gvec2(pointer_relative_pos.x, pointer_relative_pos.z)) < (gfloat)0.1)
					g.water_height[x + y * w] += rain * 0.01;
			}
	}
}


void TerrainLayer::createMaterial()
{
	std::string vertexShader = R"(
			#version 330 core

			layout(location = 0) in float a_height;
			layout(location = 1) in vec2 a_pos;

			struct MAT {
				vec4 color;
				sampler2D height_texture;
				float shines;
				sampler2D terrain_texture;

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

				vec3 central = vec3(a_pos.x,texture2D(mat.height_texture, a_pos).r,a_pos.y);
				const float eps = 0.02;


				float hL = texture2D(mat.height_texture, a_pos - vec2(eps, 0.0)).r;
			    float hR = texture2D(mat.height_texture, a_pos + vec2(eps, 0.0)).r;
			    float hD = texture2D(mat.height_texture, a_pos - vec2(0.0, eps)).r;
			    float hU = texture2D(mat.height_texture, a_pos + vec2(0.0, eps)).r;

				vec3 dx = vec3(2 * eps, hR - hL, 0);
				vec3 dy = vec3(0, hU - hD, 2 * eps);

				vec3 nor = normalize(cross(dy,dx));


				//if (dot(nor, vec3(0,1,0)) < 0)
				//	nor = -nor;

				v_normal = (world * vec4(nor,0)).xyz;
				v_world_pos = (world * vec4(central, 1.0)).xyz;


				gl_Position = glo.proj * glo.view * vec4(v_world_pos,1.0);
				outpost=a_pos;
			}
		)";
	{
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
				float shines;
				sampler2D terrain_texture;
			};
			uniform MAT mat;

			in vec2 outpost;
			in vec3 v_normal;
			in vec3 v_world_pos;

			out vec4 color;
			void main()
			{
				//gfloat foo = texture2D(mat.height_texture, outpost).r;
				//color = vec4(foo,foo,foo,1);


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
		bp->setUniform1i("height_texture", 0);
		bp->unbind();

		MaterialInfo in;
		in.shader = shader;
		in.name = "terrainMaterial";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST;
		matPtr = MaterialLibrary::create(in);
		matPtr->setValue("shines", 64.f);
		// also set the height texture sometime in the future
	}

	//create water material
	{
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
				float shines;
				sampler2D terrain_texture;

			};
			uniform MAT mat;
			in vec2 outpost;
			in vec3 v_normal;
			in vec3 v_world_pos;
			out vec4 color;
			void main()
			{

				float waterHeight = texture2D(mat.height_texture, outpost).r;
				float terrainHeight = texture2D(mat.terrain_texture, outpost).r;
				if (waterHeight < terrainHeight)
				{
					discard;
				}
				float diff = waterHeight - terrainHeight;
				diff = max(diff*1000, 1.0);
				


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



				color = vec4(diffuseLight+diffuseColor*0.3 + reflectiveLight,diff*0.4);

			}
		)";

		auto waterShader = Shader::create(Shader::ShaderProgramSources(vertexShader, waterFragmentShader));
		std::shared_ptr<internal::GLShader> bp = std::dynamic_pointer_cast<internal::GLShader>(waterShader);
		bp->bind();
		bp->setUniform1i("height_texture", 0);
		bp->setUniform1i("terrain_texture", 1);
		bp->unbind();


		MaterialInfo in;
		in.shader = waterShader;
		in.name = "terrainMaterialWater";
		in.structName = "MAT";
		in.flags = MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST | MaterialFlags::FLAG_BLEND;
		waterMatPtr = MaterialLibrary::create(in);
		waterMatPtr->setValue("shines", 16.f);
	}
}



static int groundSize = 128;
static gfloat totalGround = 0;
static gfloat currentGround = 0;
static gfloat currentSediment = 0;
static gfloat maxSediment = 0;
static gfloat currentWater = 0;
static gfloat minTerrain = 0, maxTerrain = 0;


static bool toggle_sim_drop = false;
static bool toggle_sim_euler = false;
static int playspeed_sim_drop = 1;
static int playspeed_sim_euler = 1;


void TerrainLayer::onImGuiRender()
{
	using namespace ter;
	static bool showBase = true;

	if (!ImGui::Begin("WorldInfo", &showBase))
	{
		ImGui::End();
		return;
	}
	ImGui::InputInt("Ground Size", &groundSize);
	if (ImGui::Button("Recreate Ground"))
		createGround();


	if(ImGui::CollapsingHeader("DropSim"))
	{
		static bool stepSucces = true;

		ImGui::SeparatorText("Simulation");
		ImGui::PushID("Drop Sim");
		if (ImGui::Button("Init")) {
			stepSucces = true;
			droplet.init(g);
		}

		ImGui::BeginDisabled(!stepSucces);
		if (ImGui::Button("Step"))
			stepSucces = droplet.step(g);
		ImGui::EndDisabled();


		if (ImGui::Button("One Whole Drop"))
		{
			stepSucces = true;
			droplet.init(g);
			while (droplet.step(g));
		}
		ImGui::PushStyleColor(ImGuiCol_Button, toggle_sim_drop ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			toggle_sim_drop ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			toggle_sim_drop ? ImVec4(0.1f, 0.6f, 0.1f, 1.0f) : ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
		if (ImGui::Button("Play"))
		{
			toggle_sim_drop = !toggle_sim_drop;
		}
		ImGui::PopStyleColor(3);

		int min = 1;
		int max = 1000000;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_drop, &min, &max, "%ld", ImGuiSliderFlags_Logarithmic);


		ImGui::Text("Droplet height %.1f",
			ter::interpolate2D(g.terrain_height, g.width, g.height, droplet.pos.x, droplet.pos.y));

		ImGui::PopID();

		droplet.imguiRender();

	}
	if (ImGui::CollapsingHeader("EulerSim"))
	{
		if (ImGui::Button("Init"))
			euler.init(g);
		if (ImGui::Button("Step"))
			euler.step(g);

		ImGui::PushStyleColor(ImGuiCol_Button, toggle_sim_euler ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) : ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			toggle_sim_euler ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			toggle_sim_euler ? ImVec4(0.1f, 0.6f, 0.1f, 1.0f) : ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

		toggle_sim_euler ^= ImGui::Button("Play");
		ImGui::PopStyleColor(3);

		int min = 1;
		int max = 100;
		ImGui::SliderScalar("Speed", ImGuiDataType_U32, &playspeed_sim_euler, &min, &max, "%ld", ImGuiSliderFlags_Logarithmic);


		euler.imguiRender();
	}



	if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ImGuiAddTableRow("Initial Ground", "%.1f", totalGround);
		ImGuiAddTableRow("Ground Diff", "%.1f", totalGround - currentGround);
		ImGuiAddTableRow("Water", "%.1f", currentWater);
		ImGuiAddTableRow("Ground", "%.1f", currentGround);
		ImGuiAddTableRow("Sediment", "%.1f", currentSediment);
		ImGuiAddTableRow("Max Sediment", "%.1f", maxSediment);
		ImGuiAddTableRow("Total Soil", "%.1f", currentGround + currentSediment);
		ImGuiAddTableRow("Min/Max Terrain", "%.3f / %.3f", minTerrain, maxTerrain);
		ImGuiAddTableRow("Pointer Pos (Rel)", "%.3f, %.3f, %.3f", pointer_relative_pos.x, pointer_relative_pos.y, pointer_relative_pos.z);
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
	ImGui::End();
}


void generateGroundSine(Ground& a, gfloat& totalGround)
{
	for (int x = 0; x < a.width; x++)
	{
		for (int y = 0; y < a.height; y++)
		{
			gfloat xx = (gfloat)x / a.width;
			gfloat yy = (gfloat)y / a.height;
			xx -= 0.5f;
			xx *= 2.f;
			yy -= 0.5f;
			yy *= 2.f;
			gfloat d = (glm::sin(glm::sqrt(xx * xx + yy * yy) * 6) + 1) / 2;

			d /= 2;
			//mesh.map(x, y) = (x + y) / (gfloat)mesh.map.width / 2.f;
			d *= 0.9f;
			d += 0.1f * glm::max(x, y) / a.width;

			d *= a.width;

			a.terrain_height[x + y * a.width] = d;
			totalGround += d;
		}
	}
}

void generateGroundBasin(Ground& a, gfloat& totalGround)
{
	for (int x = 0; x < a.width; x++)
	{
		for (int y = 0; y < a.height; y++)
		{
			gfloat xx = (gfloat)x / a.width;
			gfloat yy = (gfloat)y / a.height;


			auto delta = glm::max(glm::abs(xx - 0.5), glm::abs(yy - 0.5));

			gfloat d = delta * 0.75 + 0.25;

			d *= a.width;

			a.terrain_height[x + y * a.width] = d;

			totalGround += d;
		}
	}
}


void TerrainLayer::createGround()
{
	g.resize(groundSize);


	mesh.createGrid(g);
	waterMesh.createGrid(g);

	totalGround = 0;

	//generateGroundBasin(a, b, totalGround);
	generateGroundSine(g, totalGround);


	mesh.refreshHeight(g);
	waterMesh.refreshWaterHeight(g);

	static MeshData* data = nullptr;
	delete data;
	data = new MeshData;

	VertexBufferLayout layout = {g_typ::VEC2};
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

		for (auto& e : mesh.height_vbo->getLayout().getElements())
		{
			meshPtr->vertexData.declaration.addElement(index, e.typ, VertexType::POS);
			meshPtr->vertexData.binding.setBinding(index++, mesh.height_vbo);
		}
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

	waterMatPtr->setValue("height_texture", waterMesh.height_texture);
	waterMatPtr->setValue("terrain_texture", mesh.height_texture);
}

static void calculateStatistics(Ground& g);


void TerrainLayer::simulate(Ground& g, gfloat delta)
{
	if (toggle_sim_drop)
	{
		for (int i = 0; i < playspeed_sim_drop; i++)
		{
			if (!droplet.step(g))
				droplet.init(g);
		}
	}
	else if (toggle_sim_euler)
	{
		for (int i = 0; i < playspeed_sim_euler; i++)
		{
			euler.step(g);
		}
	}
}


void TerrainLayer::onUpdate()
{
	constexpr gfloat deltaTime = 0.004f;


	simulate(g, deltaTime);

	calculateStatistics(g);



	mesh.refreshHeight(g);
	waterMesh.refreshWaterHeight(g);

	auto cam = m_editorLayer.scene().currentCamera();
	auto worldPos = cam.get<TransformComponent>().pos + m_editorLayer.screenToWorld(APin().getMouseLocation()) *
		m_editorLayer.scene().getLookingDepth();

	// now figure out where on mesh the point is
	auto meshWorldMatrix = m_entity.get<TransformComponent>();

	pointer_relative_pos = glm::vec3(glm::vec4(worldPos, 1) * glm::inverse(meshWorldMatrix.trans));


	gvec3 totalPos = gvec3(droplet.pos.x / g.width,
	                       g.terrain_height[glm::clamp(
		                       (int)(droplet.pos.y) * g.width + (int)droplet.pos.x, 0,
		                       g.width * g.width - 1)] / g.width,
	                       droplet.pos.y / g.width);
	sphere.get<TransformComponent>().pos = glm::vec3(meshWorldMatrix.trans * glm::vec4(totalPos, 1));
}


static void calculateStatistics(Ground& g)
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


