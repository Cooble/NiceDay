#include "TerrainLayer.h"

#include "event/MouseEvent.h"
#include "core/App.h"
#include "imgui.h"
#include "MandelBrotLayer.h"
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

TerrainLayer::TerrainLayer(nd::EditorLayer& l): m_editorLayer(l) {}



struct HeightMap {
	std::vector<float> data;
	int width;
	int height;

	HeightMap(int w, int h)
		: width(w), height(h)
	{
		data.resize(w * h);
		ZeroMemory(data.data(), data.size() * sizeof(float));
	}
	HeightMap()
		: width(0), height(0)
	{
	}

	float& operator()(int x, int y)
	{
		return data[x + y * width];
	}

};

struct TerrainMesh {
	HeightMap map;

	// graphical primitives
	VertexBuffer* height_vbo;
	VertexBuffer* pos_vbo;
	VertexArray* vao;
	IndexBuffer* index_buffer;
	TexturePtr height_texture;


	// requires heightmap to be set
	void createGrid()
	{
		float scaler = 1.f/(map.width-1);

		height_vbo = VertexBuffer::create(map.data.data(), map.data.size() * sizeof(float));
		height_vbo->setLayout({ g_typ::FLOAT});

		auto f = std::vector<float>(map.width * map.height * 2); 

		for (int y = 0; y < map.height; y++)
			for (int x = 0; x < map.width; ++x)
			{
				f[(y * map.width + x) * 2 + 0] = scaler * x;
				f[(y * map.width + x) * 2 + 1] = scaler * y;
			}

		TextureInfo info = TextureInfo().size(map.width, map.height).format(TextureFormat::RED);

		height_texture = std::shared_ptr<Texture>(Texture::create(info));
		height_texture->setPixels(map.data.data());


		pos_vbo = VertexBuffer::create(f.data(), f.size() * sizeof(float));
		pos_vbo->setLayout({ g_typ::VEC2});

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

	void refreshHeight()
	{
		height_vbo->changeData((char*)map.data.data(), map.data.size() * sizeof(float),0);
		height_texture->setPixels(map.data.data());
	}
};
static TerrainMesh mesh;

void TerrainLayer::onAttach()
{
	NBT::loadFromFile("terrain.settings", settings);
	//settings.load("x", flatCam.pos.x);

	MeshPtr tempMesh;
	// terrain
	if (true){
		// data
		constexpr int MESH_SIZE = 128;
		mesh.map = HeightMap(MESH_SIZE,MESH_SIZE);
		mesh.createGrid();

		for(int x=0;x<mesh.map.width;x++)
		{
			for (int y = 0; y < mesh.map.height; y++)
			{
				float xx = (float)x / mesh.map.width;
				float yy = (float)y / mesh.map.height;
				xx -= 0.5f;
				xx *= 2.f;
				yy -= 0.5f;
				yy *= 2.f;
				float d =(glm::sin(glm::sqrt(xx * xx + yy * yy)*6)+1)/2;

				//mesh.map(x, y) = (x + y) / (float)mesh.map.width / 2.f;
				mesh.map(x, y) = d/2;
			}
		}
		mesh.refreshHeight();


		auto data = new MeshData;
		VertexBufferLayout layout = { g_typ::VEC2 };
		data->allocate(mesh.pos_vbo->getSize(), 3 * sizeof(float), (mesh.map.width - 1) * (mesh.map.height - 1) * 6, layout);
		data->setID("terrainMesh");

		Ref<Mesh> meshPtr;
		{
			meshPtr = std::make_shared<Mesh>();
			meshPtr->data = data;
			
			meshPtr->indexData.count = data->getIndicesCount();
			meshPtr->indexData.offset = 0;
			// this might be the most disgusting thing i ever did, but it cannot be nullptr since it checks during draw call,
			// it should not read from it though, >)
			meshPtr->indexData.indexBuffer =(IndexBuffer*) 0x42;
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
		in.flags =  MaterialFlags::FLAG_DEPTH_MASK | MaterialFlags::FLAG_DEPTH_TEST;
		auto mat = MaterialLibrary::create(in);
		mat->setValue("height_texture", mesh.height_texture);
		mat->setValue("shines", 64.f);

		


		auto entit = m_editorLayer.scene().createEntity("terrain");
		entit.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(10.f), glm::vec3(0.f,0.f,glm::half_pi<float>()));
		entit.emplaceOrReplace<ModelComponent>(meshPtr->getID(), mat->getID());
		m_entity = entit;
	}
	// set default camera pos
	m_editorLayer.scene().currentCamera().get<TransformComponent>().rot = { -0.594f,-2.460f,0.f };
	m_editorLayer.scene().currentCamera().get<TransformComponent>().pos = { -12.034f,7.118f,-5.970f };

	//adding dragoon
	if(false){
		if (!FUtil::exists("res/examples/models/dragon.bin"))
		{
			ND_INFO("Building dragon binary mesh");
			MeshDataFactory::writeBinaryFile("res/examples/models/dragon.bin", *Colli::buildMesh("res/examples/models/dragon.obj"));
		}/*
		auto material = Material::create({
		;
		);
		material->setValue("shines", 64.f);
*		 /*/
	
		auto material = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/Model.shader"), "MAT", "dragonMat" });
		material->setValue("shines", 64.f);


		//auto mesh = MeshLibrary::buildNewMesh(
		//	MeshDataFactory::readBinaryFile(ND_RESLOC("res/examples/models/dragon.bin")));
		auto mesh = MeshLibrary::registerMesh(MeshDataFactory::readBinaryFile(ND_RESLOC("res/examples/models/dragon.bin")));
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

void TerrainLayer::onImGuiRender()
{
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
	//m_entity.get<TransformComponent>().rot.x += 0.01f;
}
