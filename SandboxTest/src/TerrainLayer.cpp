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
#include "scene/Colli.h"
#include "scene/components.h"
#include "scene/EditorLayer.h"
#include "scene/Material.h"
#include "scene/NewScene.h"

using namespace nd;


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

	void createGrid(Ground& map)
	{
		if (height_vbo)
		{
			delete vao;
			delete height_vbo;
			delete pos_vbo;
			delete index_buffer;
		}

		float scaler = 1.f / (map.width - 1);

		height_vbo = VertexBuffer::create(map.terrain_height.data(), map.terrain_height.size() * sizeof(float));
		height_vbo->setLayout({g_typ::FLOAT});

		auto f = std::vector<float>(map.width * map.height * 2);

		for (int y = 0; y < map.height; y++)
			for (int x = 0; x < map.width; ++x)
			{
				f[(y * map.width + x) * 2 + 0] = scaler * x;
				f[(y * map.width + x) * 2 + 1] = scaler * y;
			}

		TextureInfo info = TextureInfo().size(map.width, map.height).format(TextureFormat::RED);

		height_texture = std::shared_ptr<Texture>(Texture::create(info));
		height_texture->setPixels(map.terrain_height.data());


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
		height_vbo->changeData((char*)map.terrain_height.data(), map.terrain_height.size() * sizeof(float), 0);
		height_texture->setPixels(map.terrain_height.data());
	}
};

static TerrainMesh mesh;
static Ref<Mesh> meshPtr;
static MaterialPtr matPtr;


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
		                                           glm::vec3(0.f, 0.f, glm::half_pi<float>()));
		entit.emplaceOrReplace<ModelComponent>(meshPtr->getID(), matPtr->getID());
		m_entity = entit;
	}
	// set default camera pos
	m_editorLayer.scene().currentCamera().get<TransformComponent>().rot = {-0.594f, -2.460f, 0.f};
	m_editorLayer.scene().currentCamera().get<TransformComponent>().pos = {-12.034f, 7.118f, -5.970f};

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
		//mat->setValue("color", glm::vec4(0.f, 1.f, 0.f, 1.f));

		m_entity = m_editorLayer.scene().createEntity("dragoon");
		m_entity.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		m_entity.emplaceOrReplace<ModelComponent>(mesh->getID(), material->getID());
	}
}

void TerrainLayer::onDetach()
{
	//settings.save("x", flatCam.pos.x);
	NBT::saveToFile("terrain.settings", settings);
}

static int groundSize = 128;

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
	ImGui::End();

	/*
		static bool flatCamOpen = true;
		if (ImGui::Begin("FlatCam", &flatCamOpen))
		{
			flatCam.imGuiPropsRender();
			ImGui::Spacing();
			ImGui::SliderInt("Steps", &mandelSteps, 2, 1000);
			ImGui::SliderInt("WrapAfter", &wrapAfter, 2, 1000);
			ImGui::Spacing();
			if (quats) {
				ImGui::SliderFloat("Z", (float*)&dimensions, -1, 1);
				ImGui::SliderFloat("W", (float*)&dimensions[1], -1, 1);
			}
			if (ImGui::Checkbox("Quaternions", &quats))
			{
				if(quats)
				{
					mandelShader = Shader::create(ND_RESLOC("res/shaders/MandelBulb.shader"));

				}else
				{
					mandelShader = Shader::create(ND_RESLOC("res/shaders/MandelBrot.shader"));
				}
			}
		}
		ImGui::End();*/
}

void TerrainLayer::onRender()
{
	//MandelBrot PIe
	/*mandelShader->bind();

	std::static_pointer_cast<internal::GLShader>(mandelShader)->setUniformMat4("u_uv_trans", flatCam.getProjMatrix());
	std::static_pointer_cast<internal::GLShader>(mandelShader)->setUniform1i("u_steps", mandelSteps);
	std::static_pointer_cast<internal::GLShader>(mandelShader)->setUniform1i("u_wrapAfter", wrapAfter);
	if(quats)
		std::static_pointer_cast<internal::GLShader>(mandelShader)->setUniformVec2f("u_dimensions", dimensions);

	Effect::getDefaultVAO().bind();
	Effect::renderDefaultVAO();*/
}

void TerrainLayer::onEvent(Event& e) {}

void TerrainLayer::onUpdate()
{
	// well this is shit per excellance
	static int i = 0;
	//if (i++ % 60)
	//	return;

	constexpr float deltaTime = 0.00016f;
	simulate(*m_currentGround, *m_nextGround, deltaTime);

	// swap
	auto t = m_currentGround;
	m_currentGround = m_nextGround;
	m_nextGround = t;

	mesh.refreshHeight(*m_currentGround);
}

void TerrainLayer::createGround()
{
	a.resize(groundSize);
	b.resize(groundSize);

	m_currentGround = &a;
	m_nextGround = &b;

	mesh.createGrid(a);


	for (int x = 0; x < a.width; x++)
	{
		for (int y = 0; y < a.height; y++)
		{
			float xx = (float)x / a.width;
			float yy = (float)y / a.height;
			xx -= 0.5f;
			xx *= 2.f;
			yy -= 0.5f;
			yy *= 2.f;
			float d = (glm::sin(glm::sqrt(xx * xx + yy * yy) * 6) + 1) / 2;

			//mesh.map(x, y) = (x + y) / (float)mesh.map.width / 2.f;
			a.terrain_height[x + y * a.width] = d / 2;
			b.terrain_height[x + y * a.width] = d / 2;
		}
	}
	mesh.refreshHeight(a);

	static MeshData* data = nullptr;
	delete data;
	data = new MeshData;

	VertexBufferLayout layout = {g_typ::VEC2};
	data->allocate(mesh.pos_vbo->getSize(), 3 * sizeof(float), (a.width - 1) * (a.height - 1) * 6, layout);
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

				vec3 central = vec3(texture2D(mat.height_texture, a_pos).r,a_pos);

				vec2 dx = vec2(a_pos.x+0.05,a_pos.y);
				vec2 dy = vec2(a_pos.x,a_pos.y+0.05);

				float h1 = texture2D(mat.height_texture, dx).r;
				float h2 = texture2D(mat.height_texture, dy).r;

				vec3 dxx = vec3(h1,dx);
				vec3 dyy = vec3(h2,dy);

				vec3 deltaX = dxx-central;
				vec3 deltaY = dyy-central;

				vec3 nor = normalize(cross(deltaX,deltaY));
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
				float shines;
			};
			uniform MAT mat;

			in vec2 outpost;
			in vec3 v_normal;
			in vec3 v_world_pos;

			out vec4 color;
			void main()
			{
				//float foo = texture2D(mat.height_texture, outpost).r;
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


static void rainDown(Ground& now, Ground& next, float delta)
{
	constexpr float rain = 0.01f;
	// D_water += 0.01f
	for (int y = 0; y < now.height; y++)
	{
		for (int x = 0; x < now.width; x++)
		{
			auto d = glm::ivec2(x, y) - glm::ivec2(now.width / 2);

			auto d1 = now.water_height[x + y * now.width] +
				delta * (d.x * d.x + d.y * d.y < 100 || (x>now.width/2-10 && x < now.width / 2 + 10)? rain : 0);
			next.water_height[x + y * now.width] = d1;
		}
	}
}

static void flow(Ground& old, Ground& next, float deltaTime)
{
	//  gravity
	constexpr float g = 9.81f;
	// pipe length
	float l = 1.f / next.width;

	// cross section area of pipe
	float A = l * l;

	auto width = next.width;
	auto height = next.height;

	// update flux

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			auto x_l = x == 0 ? x : x - 1;
			auto x_r = x == width - 1 ? x : x + 1;

			auto y_d = y == 0 ? y : y - 1;
			auto y_u = y == height - 1 ? y : y + 1;


			auto heights = glm::vec4(
				old.terrain_height[y * width + x]
				+ next.water_height[y * width + x]);

			heights.x -=
				+old.terrain_height[y * width + x_l]
				+ next.water_height[y * width + x_l];

			heights.y -=
				+old.terrain_height[y * width + x_r]
				+ next.water_height[y * width + x_r];

			heights.z -=
				+old.terrain_height[y_d * width + x]
				+ next.water_height[y_d * width + x];

			heights.w -=
				+old.terrain_height[y_u * width + x]
				+ next.water_height[y_u * width + x];

			auto& flux = old.flux[y * width + x];
			auto& fluxNew = next.flux[y * width + x];

			fluxNew = max(glm::vec4(0.f), flux + deltaTime * heights * A * g / l);


			// CLAMP FLUX so it does not flow out of the map
			if (x == 0)
				fluxNew.x = 0;
			if (x == width - 1)
				fluxNew.y = 0;
			if (y == 0)
				fluxNew.z = 0;
			if (y == width - 1)
				fluxNew.w = 0;

			// todo water height is negative, yucky
			// todo totalOutFlux can be zero, division by zero is not nice

			auto d1 = next.water_height[y * width + x];
			float totalOutFlux = glm::compAdd(fluxNew) * deltaTime;
			float maxOutFlux = d1 * l * l;

			// cannot flow more than the water level
			auto K = glm::min(1.f, maxOutFlux / totalOutFlux);

			// Edge case outflow is zero
			if (totalOutFlux == 0.0f)
				K = 1;

			fluxNew *= K;
			ASSERT(
				!glm::isnan(fluxNew.x)&&
				!glm::isnan(fluxNew.y)&&
				!glm::isnan(fluxNew.z)&&
				!glm::isnan(fluxNew.w)
				, "Shit meanD is nan");
		}
	}

	// update water level

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			auto& flux = next.flux[y * width + x];
			float sumOut = glm::compAdd(flux);

			glm::vec4 f;
			f.x = x == 0 ? 0 : next.flux[y * width + x - 1].y;
			f.y = x == width - 1 ? 0 : next.flux[y * width + x + 1].x;
			f.z = y == 0 ? 0 : next.flux[(y - 1) * width + x].w;
			f.w = y == height - 1 ? 0 : next.flux[(y + 1) * width + x].z;
			float sumIn = glm::compAdd(f);

			float deltaVolume = (sumIn - sumOut) * deltaTime;
			float deltaHeight = deltaVolume / l / l;

			auto d1 = next.water_height[y * width + x];
			auto d2 = next.water_height[y * width + x] + deltaHeight;
			auto meanD = (d1 + d2) / 2.f;

			// water height update
			next.water_height[y * width + x] = glm::max(0.f,d2);


			// computing velocity
			float dx =
				+(x == 0 ? 0 : next.flux[y * width + x - 1].y)
				- next.flux[y * width + x].x
				+ next.flux[y * width + x].y
				- (x == width - 1 ? 0 : next.flux[y * width + x + 1].x);

			float dy =
				+(y == 0 ? 0 : next.flux[(y - 1) * width + x].w)
				- next.flux[y * width + x].z
				+ next.flux[y * width + x].w
				- (y == height - 1 ? 0 : next.flux[(y + 1) * width + x].z);

			auto& vel = next.velocity[y * width + x];
			vel = glm::vec2(dx, dy) / meanD / l / 2.f;
			if (glm::isinf(vel.x) || glm::isinf(vel.y) || glm::isnan(vel.x))
			{
				// water can be totaly dry
				vel = glm::vec2(0.f);
			}
			ASSERT(!glm::isnan(meanD), "Shit meanD is nan");
			ASSERT(!glm::isnan(dx), "Shit dx is nan");
			ASSERT(!glm::isnan(dy), "Shit dx is nan");
		}
	}
}

static float interpolate2D(const std::vector<float>& data, int width, int height, float x, float y)
{
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	if (x1 >= width)
		x1 = width - 1;
	if (y1 >= height)
		y1 = height - 1;
	float xLerp = x - x0;
	float yLerp = y - y0;
	float h00 = data[y0 * width + x0];
	float h01 = data[y0 * width + x1];
	float h10 = data[y1 * width + x0];
	float h11 = data[y1 * width + x1];
	return h00 * (1 - xLerp) * (1 - yLerp) +
		h01 * (xLerp) * (1 - yLerp) +
		h10 * (yLerp) * (1 - xLerp) +
		h11 * (xLerp) * (yLerp);
}

static void erosion(Ground& now, Ground& next, float deltaTime)
{
	// Sediment Capacity
	constexpr float K_c = 0.01f;

	// Dissolving constant 
	constexpr float K_s = 0.01f;

	// Depositing constant
	constexpr float K_d = 0.01f;

	// Evaporation constant
	constexpr float K_e = 0.01f;


	// Erosion & Deposition

	for (int y = 0; y < next.height; ++y)
	{
		for (int x = 0; x < next.width; ++x)
		{
			auto& vel = next.velocity[y * next.width + x];
			auto& sed = now.sediment[y * next.width + x];

			auto& height = next.terrain_height[y * now.width + x];
			float heightRight = x == next.width - 1 ? height : next.terrain_height[y * next.width + x + 1];
			//float heightLeft = x == 0 ? height : now.terrain_height[y * now.width + x - 1];
			float heightUp = y == next.height - 1 ? height : next.terrain_height[(y + 1) * next.width + x];

			glm::vec2 gradient = glm::vec2(heightRight, heightUp) - height;

			// compute local tilt angle
			float angle = glm::atan(glm::length(gradient));

			float capacity = K_c * glm::length(vel) * glm::sin(angle);

			// is water has still more capacity to dissolve something
			if (capacity > sed)
			{
				// dissolving
				float amount = K_s * (capacity - sed);
				height -= amount;
				sed += amount;
			}
			else
			{
				// depositing
				float amount = K_d * (sed - capacity);
				height += amount;
				sed -= amount;
			}
		}
	}

	float max = 0;
	// Sediment transport

	for (int y = 0; y < next.height; ++y)
	{
		for (int x = 0; x < next.width; ++x)
		{
			max = glm::max(max, next.velocity[y * next.width + x].x * deltaTime);
			max = glm::max(max, next.velocity[y * next.width + x].y * deltaTime);

			float velx = next.velocity[y * next.width + x].x;
			float vely = next.velocity[y * next.width + x].y;

			float fx = (float)x - velx * deltaTime;
			float fy = (float)y - vely * deltaTime;


			next.sediment[x * next.width + y] = interpolate2D(now.sediment, now.width, now.height,
			                                                  fx, fy);
		}
	}
	//ND_BUG("Max {}", max);


	float maxWater = 0;
	float minWater = 0;
	// Evaporation
	for (int y = 0; y < now.height; ++y)
	{
		for (int x = 0; x < now.width; ++x)
		{
			auto& height = next.water_height[y * now.width + x];

			height *= 1 - K_e * deltaTime;
			maxWater = glm::max(maxWater, height);
			minWater = glm::min(minWater, height);
		}
	}
	//ND_BUG("Water {}", maxWater);
	//ND_BUG("Water min{}", minWater);
}

void TerrainLayer::simulate(Ground& now, Ground& next, float delta)
{
	if (false)
	{
		static float time = 0;
		time += 0.05f;
		for (int x = 0; x < now.width; x++)
		{
			for (int y = 0; y < now.height; y++)
			{
				float xx = (float)x / now.width;
				float yy = (float)y / now.height;
				xx -= 0.5f;
				yy -= 0.5f;
				float d = (glm::sin(glm::sqrt(xx * xx + yy * yy) * 6 + time) + 1) / 2;
				next.terrain_height[x + y * now.width] = d / 2;
			}
		}
	}

	rainDown(now, next, delta);
	flow(now, next, delta);
	erosion(now, next, delta);
}
