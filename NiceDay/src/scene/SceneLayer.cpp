#include "SceneLayer.h"
#include "graphics/API/Texture.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "MeshData.h"
#include "Colli.h"
#include "graphics/API/VertexArray.h"
#include "graphics/GContext.h"
#include "NewScene.h"
#include "components.h"
#include "scene/Mesh.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/API/FrameBuffer.h"
#include "core/App.h"
#include "Camm.h"
#include "platform/OpenGL/GLRenderer.h"
#include "components_imgui_access.h"
#include "Atelier.h"


struct Env
{
	glm::mat4 view;
	glm::mat4 proj;

	glm::vec3 sunPos;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 camera_pos;

	float constant;
	float linear;
	float quadratic;
} env;

UniformLayout envLayout;
static MaterialPtr enviroment;
static Entity currentCamera;
EditorCam* editCam;

static void onPointerComponentDestroyed(entt::registry& reg, entt::entity ent)
{
	auto& t = reg.get<PointerComponent>(ent);
	if (t.ptr)
		free(t.ptr);
}

void SceneLayer::onAttach()
{
	auto t =Atelier::get();//just init atelier
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/ignore/OpenSans-Regular.ttf").c_str(), 20);

	//adding JP (merging with current font) -> nihongode ok kedo, mada nai to omou
	/*ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/NotoSansCJKjp-Medium.otf").c_str(), 20, &config, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->Build();*/

	m_scene = new NewScene;
	m_scene->reg().on_destroy<PointerComponent>().connect<&onPointerComponentDestroyed>();


	envLayout.name = "GLO";
	envLayout.prefixName = "glo";
	envLayout.emplaceElement(g_typ::MAT4, 1, "glo.view");
	envLayout.emplaceElement(g_typ::MAT4, 1, "glo.proj");

	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.sunPos");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.ambient");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.diffuse");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.specular");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.camera_pos");

	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.constant");
	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.linear");
	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.quadratic");
	enviroment = Material::create({nullptr, "GLO", "Enviroment", &envLayout});


	auto modelMat = Material::create({
		std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
		"modelMaterial"
	});
	modelMat->setValue("color", glm::vec4(1.0, 1.0, 0, 1));
	modelMat->setValue("shines", 64.f);


	auto crate1Mesh = MeshFactory::buildNewMesh(Colli::buildMesh("res/models/cube.fbx"));


	if (!std::filesystem::exists(ND_RESLOC("res/models/dragon.bin")))
	{
		ND_INFO("Building dragon binary mesh");
		MeshDataFactory::writeBinaryFile(
			"res/models/dragon.bin", *Colli::buildMesh("res/models/dragon.obj"));
	}
	//adding cubemap
	{
		//adding cube_map
		auto skyTex = Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap2/*.png"));
		auto mesh = MeshFactory::buildNewMesh(MeshDataFactory::buildCube(40.f));

		auto flags = MaterialFlags::DEFAULT_FLAGS;
		flags = (~(MaterialFlags::FLAG_CULL_FACE | MaterialFlags::FLAG_DEPTH_MASK) & flags) | MaterialFlags::FLAG_CHOP_VIEW_MAT_POS;
		auto mat = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial" ,nullptr,flags });
		mat->setValue("cubemap", std::shared_ptr<Texture>(skyTex));
		auto ent = m_scene->createEntity("SkyBox");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh, mat);
	}
	//adding crate
	{
		//auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));
		//auto specular = Texture::create(TextureInfo("res/models/crate_specular.png"));

		//auto mesh = NewMeshFactory::buildNewMesh(Colli::buildMesh(ND_RESLOC("res/models/cube.fbx")));
		auto mesh = crate1Mesh;

		/*auto mat = modelMat->copy("crateMaterial");
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));
		mat->setValue("specular", std::shared_ptr<Texture>(specular));
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		MaterialLibrary::save(mat, "res/crateM.mat");*/
		auto mat = MaterialLibrary::load("res/models/crateM.mat");

		auto ent = m_scene->createEntity("Crate");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(1.f), glm::vec3(100.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh, mat);
	}
	//adding sphere
	{
		auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));

		auto mesh = MeshFactory::buildNewMesh(Colli::buildMesh("res/models/sphere.fbx"));

		auto mat = MaterialLibrary::copy(modelMat,"SphereMat");
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

		auto ent = m_scene->createEntity("Sphere");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f,5.f,0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh, mat);
	}
	//adding dragoon
	/*{
	auto dragonMesh = NewMeshFactory::buildNewMesh(MeshFactory::readBinaryFile(ND_RESLOC("res/models/dragon.bin")));
		//auto mesh = NewMeshFactory::buildNewMesh(data);
		auto mesh = dragonMesh;
		mesh->topology = Topology::TRIANGLES;
		auto mat = MaterialLibrary::copy(modelMat,"dragoonMat");
		//mat->setValue("color", glm::vec4(0.f, 1.f, 0.f, 1.f));
		MaterialLibrary::save(mat, "res/drag.mat");

		auto ent = m_scene->createEntity("Dragoon");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh, mat);
	}*/
	
	//adding light
	{
		auto ent = m_scene->createEntity("Light");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<LightComponent>();
	}
	//adding camera
	{
		auto ent = m_scene->createEntity("Cam");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<CameraComponent>(glm::mat4(1.f), glm::quarter_pi<float>(), 1.f, 100.f);
		editCam = (EditorCam*)malloc(sizeof(EditorCam));
		ent.emplaceOrReplace<PointerComponent>(new(editCam)EditorCam(ent));
		currentCamera = ent;
	}
}

void SceneLayer::onDetach()
{
}

void SceneLayer::onUpdate()
{
	
	editCam->onUpdate();
}

void SceneLayer::onRender()
{
	Atelier::get().makePendingPhotos();
	Renderer::getDefaultFBO()->bind();
	Gcon.enableCullFace(true);
	Gcon.enableDepthTest(true);
	//Renderer::getDefaultFBO()->clear(BuffBit::COLOR, { 0,1,0,1 });
	auto& camCom = currentCamera.get<CameraComponent>();
	auto& transCom = currentCamera.get<TransformComponent>();
	env.camera_pos = transCom.pos;

	env.view = editCam->getViewMatrix();
	env.proj = glm::perspective(camCom.fov,
	                            (float)App::get().getWindow()->getWidth() / (float)App::get()
	                                                                                   .getWindow()->getHeight(),
	                            camCom.Near, camCom.Far);

	auto lights = m_scene->view<LightComponent>();
	for (auto light : lights)
	{
		auto& l = lights.get(light);
		env.ambient = l.ambient;
		env.diffuse = l.diffuse;
		env.specular = l.specular;
		env.constant = l.constant;
		env.linear = l.linear;
		env.quadratic = l.quadratic;
		env.sunPos = m_scene->wrap(light).get<TransformComponent>().pos;
	}
	auto models = m_scene->group<TransformComponent, ModelComponent>();
	int index = 0;
	for (auto model : models)
	{
		if(!m_scene->reg().get<TagComponent>(model).enabled)
			continue;
		//auto& [trans, mod] = models.get<TransformComponent,ModelComponent>(model);
		auto& trans = models.get<TransformComponent>(model);
		trans.recomputeMatrix();
		auto& mod = models.get<ModelComponent>(model);

		//bind mat vars
		mod.material->bind();

		//bind enviroment vars
		enviroment->setRaw(env);
		enviroment->bind(0, mod.material->getShader());
		mod.mesh->vao_temp->bind();

		//bind other vars
		auto s = std::static_pointer_cast<GLShader>(mod.material->getShader());
		//if(mod.material->getShader()->getLayout().getLayoutByName("world"))
		s->setUniformMat4("world", trans.trans);

		auto flags = mod.material->getFlags();
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(flags & MaterialFlags::FLAG_DEPTH_MASK);
			Gcon.enableCullFace(flags & MaterialFlags::FLAG_CULL_FACE);
			Gcon.enableDepthTest(flags & MaterialFlags::FLAG_DEPTH_TEST);
			if(flags& MaterialFlags::FLAG_CHOP_VIEW_MAT_POS)
				s->setUniformMat4("glo.view", glm::mat4(glm::mat3(env.view)) * glm::scale(glm::mat4(1.f), trans.scale));
		}
		

		if (mod.mesh->indexData.exists())
			Gcon.cmdDrawElements(mod.mesh->data->getTopology(), mod.mesh->indexData.count);
		else
			Gcon.cmdDrawArrays(mod.mesh->data->getTopology(), mod.mesh->vertexData.binding.count);

		//reset if neccessary
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(true);
			Gcon.enableCullFace(true);
			Gcon.enableDepthTest(true);
		}
	}
}



void SceneLayer::onImGuiRender()
{
	auto lastCam = currentCamera;
	components_imgui_access::drawEntityManager(*m_scene,currentCamera);
	if (lastCam != currentCamera) {
		editCam = (EditorCam*)currentCamera.get<PointerComponent>().ptr;
		lastCam = currentCamera;
	}
	
	components_imgui_access::windows.drawWindows();
}

void SceneLayer::onEvent(Event& e)
{
	editCam->onEvent(e);
	//auto& com = currentCamera.get<EventConsumerComponent>().consumer;
	//com(currentCamera,e);
}
