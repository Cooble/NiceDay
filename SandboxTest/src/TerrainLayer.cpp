#include "TerrainLayer.h"

#include "event/MouseEvent.h"
#include "core/App.h"
#include "imgui.h"
#include "MandelBrotLayer.h"
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

static gfloat interpolate2D(const std::vector<gfloat>& data, int width, int height, gfloat x, gfloat y)
{
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	x0 = glm::clamp(x0, 0, width - 1);
	y0 = glm::clamp(y0, 0, height - 1);
	x1 = glm::clamp(x1, 0, width - 1);
	y1 = glm::clamp(y1, 0, height - 1);

	gfloat xLerp = x - x0;
	gfloat yLerp = y - y0;

	gfloat h00 = data[y0 * width + x0];
	gfloat h01 = data[y0 * width + x1];
	gfloat h10 = data[y1 * width + x0];
	gfloat h11 = data[y1 * width + x1];
	return h00 * (1 - xLerp) * (1 - yLerp) +
		h01 * (xLerp) * (1 - yLerp) +
		h10 * (yLerp) * (1 - xLerp) +
		h11 * (xLerp) * (yLerp);
}
static gvec2 gradAt(std::vector<gfloat>& scalarField, int x, int y, int w)
{
	auto x0 = glm::clamp(x, 0, w - 1);
	auto y0 = glm::clamp(y, 0, w - 1);
	auto x1 = glm::clamp(x + 1, 0, w - 1);
	auto y1 = glm::clamp(y + 1, 0, w - 1);
	auto gradX = scalarField[y0 * w + x1] - scalarField[y0 * w + x0];
	auto gradY = scalarField[y1 * w + x0] - scalarField[y0 * w + x0];
	return { gradX, gradY };
}
static gvec2 interPol(gvec2 v00, gvec2 v01, gvec2 v10, gvec2 v11, gvec2 pos)
{
	return v00 * (1 - pos.x) * (1 - pos.y) +
		v01 * (pos.x) * (1 - pos.y) +
		v10 * (pos.y) * (1 - pos.x) +
		v11 * (pos.x) * (pos.y);
}


static Entity sphere;
struct Droplet
{
	gvec2 pos;
	gvec2 direction;
	gfloat speed;
	gvec2 grad;
	gfloat sediment;
	gfloat water;
	gfloat oldHeight;

	// debug
	gfloat newHeight;
	gfloat heightDiff;
	gfloat capacity;
	gfloat toDeposit;
	gfloat toErode;


	static constexpr gfloat pMomentum = 0.05;
	static constexpr gfloat pMinSlope = 0.01;
	static constexpr gfloat pCapacity = 4;
	static constexpr gfloat pDeposition = 0.3;
	static constexpr gfloat pErosion = 0.3;
	static constexpr gfloat pEvaporation = 0.01;
	static constexpr gfloat pGravity = 4;

	void init(Ground& g)
	{
		auto w = g.width;
		auto h = g.height;
		pos.x = std::rand() % w;
		pos.y = std::rand() % h;
		oldHeight = interpolate2D(g.terrain_height, w, h, pos.x, pos.y);
		sediment = 0;
		water = 1;
		direction = gvec2(0, 0.1);
		speed = 0;
	}

	bool step(Ground& g)
	{
		auto w = g.width;
		auto h = g.height;

		// 2. gradient
		grad = interPol(
			gradAt(g.terrain_height, (int)pos.x, (int)pos.y, w),
			gradAt(g.terrain_height, (int)pos.x, (int)(pos.y + 1), w),
			gradAt(g.terrain_height, (int)(pos.x + 1), (int)pos.y, w),
			gradAt(g.terrain_height, (int)(pos.x + 1), (int)(pos.y + 1), w),
			pos - (gvec2)glm::ivec2(pos)
		);


		// 3. update velocity
		auto dirRaw = direction * (1 - pMomentum) - grad * pMomentum;
		direction = glm::normalize(dirRaw);

		// handle result of normalize(0)
		if (glm::length2(dirRaw)<0.000000000001)
			direction = { 0,1 };
		

		auto oldPos = glm::ivec2(pos);
		// 4. move to next cell based on velocity
		pos += normalize(direction);

		auto newPos = glm::ivec2(pos);
		if (clamp(newPos, 0, w - 1) != newPos)
			return false; //we are out of map

		auto newHeight = interpolate2D(g.terrain_height, w, h, pos.x, pos.y);
		auto heightDiff = newHeight - oldHeight;


		capacity = glm::max(-heightDiff, pMinSlope) * speed * pCapacity;

		toDeposit = glm::max((gfloat)0, (sediment - capacity) * pDeposition);
		toErode = glm::max((gfloat)0, (capacity - sediment) * pErosion);


		auto& oldHeightPtr = g.terrain_height[oldPos.y * w + oldPos.x];

		// cannot deposit sediment on old location higher than current location
		toDeposit = glm::clamp(toDeposit, (gfloat)0, heightDiff);

		// deposit some sediment for which there is now no capacity at oldPos
		oldHeightPtr += toDeposit;
		sediment -= toDeposit;

		// cannot erode old location lower than current loc
		toErode = glm::clamp(toErode, (gfloat)0, -heightDiff);

		// erode some sediment
		oldHeightPtr -= toErode;
		sediment += toErode;


		speed = glm::sqrt(glm::max((gfloat)0,speed * speed - heightDiff * pGravity));

		if (speed < 0.00000001)//tno speed pick some random direction
			direction = gvec2(rand() % 64 -32,rand() % 64 -32);

		water *= 1 - pEvaporation;

		oldHeight = newHeight;

		return water>0;// if no water is left this is the end
	}
};

static Droplet droplet;

static NBT settings;

TerrainLayer::TerrainLayer(nd::EditorLayer& l) : m_editorLayer(l) {}


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


static gfloat myPerlin(gvec2 uv)
{
	// Use glm::perlin (which expects a vec2)
	auto out =
		glm::perlin(uv) +
		glm::perlin((uv - (gfloat)20) * (gfloat)2.f) * (gfloat)0.5f +
		glm::perlin((uv - (gfloat)1235.4) * (gfloat)4.f) * (gfloat)0.25f;

	// normalize to [-1, 1]
	out /= 1.75f;

	// normalize to [0, 1]
	out = (out + 1.f) / 2.f;

	return out;
}

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


void TerrainLayer::onRender() {}

void TerrainLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::MouseScroll)
	{
		auto& event = (MouseScrollEvent&)e;

		auto rain = event.getScrollX() + event.getScrollY();

		auto w = a.width;
		auto h = a.height;
		for (int y = 1; y < h - 1; y++)
			for (int x = 1; x < w - 1; x++)
			{
				auto yy = (gfloat)y / h;
				auto xx = (gfloat)x / w;
				gvec2 pos = {xx, yy};
				if (glm::distance(pos, gvec2(pointer_relative_pos.x, pointer_relative_pos.z)) < (gfloat)0.1)
					m_currentGround->water_height[x + y * w] += rain * 0.01;
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
				const float eps = 0.05;


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


static bool e_rain = false;
static bool e_flow = false;
static bool e_erosion = false;
static bool e_evaporation = false;
static bool e_landslide = false;

static int groundSize = 128;
static gfloat totalGround = 0;
static gfloat currentGround = 0;
static gfloat currentSediment = 0;
static gfloat maxSediment = 0;
static gfloat currentWater = 0;
static gfloat minTerrain = 0, maxTerrain = 0;


static gfloat K_rain = 0.01f;
static gfloat K_g = 9.81f;
// Sediment Capacity
static gfloat K_sediment_capacity = 0.05f;
// Dissolving constant 
static gfloat K_s_dissolving = 0.1f;
// Depositing constant
static gfloat K_d_depositing = 0.03f;
// Evaporation constant
static gfloat K_evaporation = 0.03f;
static gfloat K_tilt_minimum = 0.15f;
static gfloat K_landSlideSpeed = 20.5f;
static gfloat K_landSlideCutoffAngle = 0.80f;


template <typename T>
static void InputGFloat(const char* label, T* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f",
                        ImGuiInputTextFlags flags = 0)
{
	if constexpr (std::is_same_v<T, float>)
	{
		ImGui::InputFloat(label, v, step, step_fast, format, flags);
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		ImGui::InputDouble(label, v, step, step_fast, format, flags);
	}
	else static_assert(false, "Unsupported type for InputGFloat");
}

void TerrainLayer::onImGuiRender()
{
	static bool showBase = true;

	if (!ImGui::Begin("WorldInfo", &showBase))
	{
		ImGui::End();
		return;
	}
	static bool showChunks = true;
	ImGui::Checkbox("Show Chunks", &showChunks);

	ImGui::InputInt("Ground Size", &groundSize);
	if (ImGui::Button("Recreate Ground"))
		createGround();


	ImGui::SeparatorText("Simulation");
	ImGui::PushID("Drop Sim");
	if (ImGui::Button("Init"))
		droplet.init(*m_currentGround);

	static bool stepSucces = true;
	if (ImGui::Button("Step"))
		stepSucces = droplet.step(*m_currentGround);

	ImGui::Text("Droplet pos: %.1f,%.1f", droplet.pos.x, droplet.pos.y);
	ImGui::Text("Droplet height %.1f", interpolate2D(m_currentGround->terrain_height,a.width,a.height,droplet.pos.x,droplet.pos.y));
	ImGui::Text("Droplet direction %.1f,%.1f", droplet.direction.x, droplet.direction.y);
	ImGui::Text("Droplet speed %.1f", droplet.speed);
	ImGui::Text("Droplet grad %.1f,%.1f", droplet.grad.x, droplet.grad.y);

	ImGui::Text("Droplet sediment %.1f", droplet.sediment);
	ImGui::Text("Droplet water %.1f", droplet.water);
	ImGui::Text("Droplet old height %.1f", droplet.oldHeight);
	ImGui::Text("Droplet capacity %.1f", droplet.capacity);
	ImGui::Text("Droplet to deposit %.1f", droplet.toDeposit);
	ImGui::Text("Droplet to erode %.1f", droplet.toErode);
	ImGui::PopID();

	ImGui::Checkbox("Rain", &e_rain);
	ImGui::Checkbox("Flow", &e_flow);
	ImGui::Checkbox("Erosion", &e_erosion);
	ImGui::Checkbox("Evaporation", &e_evaporation);
	ImGui::Checkbox("Landslide", &e_landslide);

	ImGui::SeparatorText("Info");
	ImGui::Text("Initial Ground: %.1f", totalGround);
	ImGui::Text("Ground Diff: %.1f", totalGround - currentGround);
	ImGui::Text("Water: %.1f", currentWater);
	ImGui::Text("Ground: %.1f", currentGround);
	ImGui::Text("Sediment: %.1f", currentSediment);
	ImGui::Text("Max Sediment: %.1f", maxSediment);
	ImGui::Text("Total Soil: %.1f", currentGround + currentSediment);
	ImGui::Text("Min/Max Terrain Height: %.3f/%.3f", minTerrain, maxTerrain);
	ImGui::Text("Relative pos on: %.3f,%.3f,%.3f", pointer_relative_pos.x, pointer_relative_pos.y,
	            pointer_relative_pos.z);

	ImGui::SeparatorText("Settings");
	ImGui::PushID("Settings");

	InputGFloat("Rain", &K_rain, 0, 0, "%.6f");
	InputGFloat("Gravity", &K_g, 0, 0, "%.6f");
	InputGFloat("Sediment Capacity", &K_sediment_capacity, 0, 0, "%.6f");
	InputGFloat("Dissolving", &K_s_dissolving, 0, 0, "%.6f");
	InputGFloat("Depositing", &K_d_depositing, 0, 0, "%.6f");
	InputGFloat("Evaporation", &K_evaporation, 0, 0, "%.6f");
	InputGFloat("Tilt Minimum", &K_tilt_minimum, 0, 0, "%.6f");
	InputGFloat("Land Slide Speed", &K_landSlideSpeed, 0, 0, "%.6f");
	InputGFloat("Land Slide Cutoff Angle", &K_landSlideCutoffAngle, 0, 0, "%.6f");

	ImGui::PopID();

	ImGui::SeparatorText("Looking At");
	ImGui::PushID("LookingAt");

	auto idx =
		glm::clamp((int)(pointer_relative_pos.x * a.width), 0, a.width - 1) +
		glm::clamp((int)(pointer_relative_pos.z * a.height), 0, a.height - 1) * a.width;


	ImGui::Text("Terrain Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * a.width,
	            m_currentGround->terrain_height[idx], (float)pointer_relative_pos.z * a.height);
	ImGui::Text("Water Height: %.3f, %.3f, %.3f", (float)pointer_relative_pos.x * a.width,
	            m_currentGround->water_height[idx], (float)pointer_relative_pos.z * a.height);

	ImGui::PopID();
	ImGui::End();
}


void generateGroundSine(Ground& a, Ground& b, gfloat& totalGround)
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
			b.terrain_height[x + y * a.width] = d;

			totalGround += d;
		}
	}
}

void generateGroundBasin(Ground& a, Ground& b, gfloat& totalGround)
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
			b.terrain_height[x + y * a.width] = d;

			totalGround += d;
		}
	}
}


std::vector<gfloat> perlinMap;
std::vector<gfloat> originalHeightMap;

static void precomputePerlin(int size)
{
	perlinMap.resize(size * size);

	for (int x = 0; x < size; x++)
		for (int y = 0; y < size; y++)
		{
			auto idx = y * size + x;
			perlinMap[idx] = myPerlin(gvec2((gfloat)x / size, (gfloat)y / size) * (gfloat)10);
		}
}

void TerrainLayer::createGround()
{
	a.resize(groundSize);
	b.resize(groundSize);
	originalHeightMap.resize(groundSize * groundSize);

	precomputePerlin(groundSize);

	m_currentGround = &a;
	m_nextGround = &b;

	mesh.createGrid(a);
	waterMesh.createGrid(a);

	totalGround = 0;

	generateGroundBasin(a, b, totalGround);

	// save it for reference
	originalHeightMap = a.terrain_height;

	mesh.refreshHeight(a);
	waterMesh.refreshWaterHeight(a);

	static MeshData* data = nullptr;
	delete data;
	data = new MeshData;

	VertexBufferLayout layout = {g_typ::VEC2};
	data->allocate(mesh.pos_vbo->getSize(), 3 * sizeof(gfloat), (a.width - 1) * (a.height - 1) * 6, layout);
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

void TerrainLayer::onUpdate()
{
	// well this is shit per excellance
	static int i = 0;
	//if (i++ % 60)
	//	return;

	constexpr gfloat deltaTime = 0.004f;


	for (int i = 0; i < 4; i++)
	{
		simulate(*m_currentGround, *m_nextGround, deltaTime);

		// swap
		auto t = m_currentGround;
		m_currentGround = m_nextGround;
		m_nextGround = t;
	}
	calculateStatistics(*m_currentGround);


	mesh.refreshHeight(*m_currentGround);
	waterMesh.refreshWaterHeight(*m_currentGround);

	auto cam = m_editorLayer.scene().currentCamera();
	auto worldPos = cam.get<TransformComponent>().pos + m_editorLayer.screenToWorld(APin().getMouseLocation()) *
		m_editorLayer.scene().getLookingDepth();

	// now figure out where on mesh the point is
	auto meshWorldMatrix = m_entity.get<TransformComponent>();

	pointer_relative_pos = glm::vec3(glm::vec4(worldPos, 1) * glm::inverse(meshWorldMatrix.trans));


	gvec3 totalPos = gvec3(droplet.pos.x / m_currentGround->width, m_currentGround->terrain_height[(int)(droplet.pos.y)*m_currentGround->width+(int)droplet.pos.x]/m_currentGround->width, droplet.pos.y / m_currentGround->width);
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


static void doAll(Ground& old, Ground& next, gfloat dt)
{
	const gfloat A_PIPE = 0.6f;
	const gfloat G = 9.81f;
	const gfloat L_PIPE = 1.f;
	const gfloat LL = 1.f;
	const gfloat K_max_disolve = 0.1;

	auto w = next.width;
	auto h = next.height;


	// 1. rain
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto d = glm::ivec2(x, y) - glm::ivec2(w / 2);

			gfloat increase = (d.x * d.x + d.y * d.y < 100 || (x > w / 2 - 10 && x < w / 2 + 10))
				                  ? K_rain
				                  : 0;

			//increase = K_rain;
			auto d1 = old.water_height[x + y * w] + dt * increase;
			next.water_height[x + y * w] = d1;
		}

	// totalHeight = terrain_height + water_height
	auto totalHeight = old.terrain_height;
	for (size_t i = 0; i < totalHeight.size(); i++)
		totalHeight[i] += next.water_height[i];

	// 2. flux
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto deltaH = gvec4(
				totalHeight[idx] - totalHeight[y * w + x - 1],
				totalHeight[idx] - totalHeight[y * w + x + 1],
				totalHeight[idx] - totalHeight[(y - 1) * w + x],
				totalHeight[idx] - totalHeight[(y + 1) * w + x]);

			auto fluxFactor = dt * A_PIPE / L_PIPE * G;
			next.flux[idx] = glm::max(gvec4(0.f), old.flux[idx] + deltaH * fluxFactor);

			auto sumF = glm::compAdd(next.flux[idx]);

			if (sumF > 0)
			{
				auto waterVolume = next.water_height[idx] * LL * LL;
				auto outVolume = sumF * dt;
				auto adjustmentFactor = glm::min((gfloat)1, waterVolume / outVolume);

				next.flux[idx] *= adjustmentFactor;
			}
		}

	// 3. water height

	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto sumIn =
				+next.flux[y * w + x - 1].y
				+ next.flux[y * w + x + 1].x
				+ next.flux[(y - 1) * w + x].w
				+ next.flux[(y + 1) * w + x].z;
			auto sumOut = glm::compAdd(next.flux[idx]);

			auto deltaV = (sumIn - sumOut) * dt;
			auto deltaH = deltaV / (LL * LL);
			next.water_height[idx] = glm::max((gfloat)0.f, next.water_height[idx] + deltaH);
			auto meanH = next.water_height[idx] - deltaH / 2.f;

			if (meanH > 0)
			{
				auto fluxX =
					+next.flux[y * w + x - 1].y
					- next.flux[idx].x
					+ next.flux[idx].y
					- next.flux[y * w + x + 1].x;
				auto fluxY =
					+next.flux[(y - 1) * w + x].w
					- next.flux[idx].z
					+ next.flux[idx].w
					- next.flux[(y + 1) * w + x].z;
				next.velocity[idx] = gvec2(fluxX, fluxY) / (meanH * LL);
			}
			else
				next.velocity[idx] = gvec2(0.f);
		}

	// 4. erosion
	constexpr gfloat erosionClamp = 10;

	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto gradX = (next.terrain_height[y * w + x + 1] - next.terrain_height[y * w + x - 1]) / 2;
			auto gradY = (next.terrain_height[(y + 1) * w + x] - next.terrain_height[(y - 1) * w + x]) / 2;

			auto grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
			auto sin_local_tilt = glm::sqrt(grade / (1 + grade));

			sin_local_tilt = glm::max(sin_local_tilt, K_tilt_minimum);

			auto capacity = K_sediment_capacity * glm::length(next.velocity[idx]) * sin_local_tilt * glm::min(
				(gfloat)1, next.water_height[idx]);


			//auto perlinFactor = myPerlin(gvec2((gfloat)x / w, (gfloat)y / h));
			auto perlinFactor = perlinMap[idx];

			// the deeper the harder to dissolve
			auto depthFactor = 1 / (1 + originalHeightMap[idx] - next.terrain_height[idx]);
			if (originalHeightMap[idx] - next.terrain_height[idx] < 0)
				depthFactor = 1;

			if (capacity > old.sediment[idx])
			{
				auto dSoil = K_s_dissolving * (capacity - old.sediment[idx]) * perlinFactor * depthFactor;

				// limit dissolve
				dSoil = glm::min(dSoil, K_max_disolve);

				// limit dissolve to the terrain height
				dSoil = glm::min(dSoil, old.terrain_height[idx]);

				next.terrain_height[idx] = old.terrain_height[idx] - dSoil;
				old.sediment[idx] = old.sediment[idx] + dSoil;
			}
			else
			{
				auto dSoil = K_d_depositing * (old.sediment[idx] - capacity);

				// limit deposit
				dSoil = glm::min(dSoil, K_max_disolve);

				next.terrain_height[idx] = old.terrain_height[idx] + dSoil;
				old.sediment[idx] = old.sediment[idx] - dSoil;
			}
		}

	// 5. sediment transport
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			gfloat velx = next.velocity[idx].x;
			gfloat vely = next.velocity[idx].y;

			gfloat fx = (gfloat)x - velx * dt;
			gfloat fy = (gfloat)y - vely * dt;


			next.sediment[idx] = interpolate2D(old.sediment, w, h, fx, fy);
			//next.sediment[idx] = old.sediment[idx];
		}

	// 6. evaporation
	if (e_evaporation)
		for (int y = 1; y < h - 1; y++)
			for (int x = 1; x < w - 1; x++)
			{
				auto idx = y * w + x;
				next.water_height[idx] *= 1 - K_evaporation * dt;

				// remove the incredibly small values
				constexpr gfloat evaporationEpsilon = 0.001;
				if (next.water_height[idx] < evaporationEpsilon)
					next.water_height[idx] = 0;
			}

	// 7. landslide
	if (e_landslide)
		for (int y = 1; y < h - 1; y++)
			for (int x = 1; x < w - 1; x++)
			{
				auto idx = y * w + x;

				auto& height = next.terrain_height[idx];

				auto heightN = gvec2(
					next.terrain_height[y * next.width + x + 1],
					next.terrain_height[(y + 1) * next.width + x]);

				auto delta = heightN - height;

				auto takeN = K_landSlideSpeed * dt * delta;

				auto signs = glm::sign(takeN);
				takeN = glm::max(glm::abs(takeN) - K_landSlideCutoffAngle, (gfloat)0.f) * signs;
				// should produce something like
				//                   /
				//                  /
				//   -------+-------
				//  /
				// /

				height += glm::compAdd(takeN);
				next.terrain_height[y * w + x + 1] -= takeN.x;
				next.terrain_height[(y + 1) * w + x] -= takeN.y;
			}
}



static void dropAll(Ground& g, int steps)
{
	Droplet d;
	d.init(g);
	for (int i = 0; i < steps && d.step(g); ++i);
}

void TerrainLayer::simulate(Ground& now, Ground& next, gfloat delta)
{
	//doAll(now, next, delta);
	next = now;
	//dropAll(next, 128);
}
