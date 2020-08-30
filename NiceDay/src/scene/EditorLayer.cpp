#include "EditorLayer.h"
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
#include "files/FUtil.h"
#include "script/NativeScript.h"
#include "graphics/TextureAtlas.h"
#include "graphics/Effect.h"
#include "core/ImGuiLayer.h"

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

static glm::vec3 screenToRay(const glm::vec2& src, const glm::mat4 proj, const glm::mat4& view, const glm::vec2& screenResolution)
{
	glm::vec2 s = src;
	s /= screenResolution;
	s = s * 2.f - 1.f;

	auto p = glm::inverse(proj) * glm::vec4(s, 0, 1);
	p.w = 0;//we care only about direction
	p.z = -1;//looking towards
	return  glm::normalize(glm::vec3(glm::inverse(view) * p));
}
/*
 * Solves equation: A*a+B*b+C*c = D
 * Returns vec3(a,b,c)
 */
static glm::vec3 matrixSolver(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D)
{
	glm::vec4 X(A.x, B.x, C.x, D.x);
	glm::vec4 Y(A.y, B.y, C.y, D.y);
	glm::vec4 Z(A.z, B.z, C.z, D.z);

	Z -= Z.x / Y.x * Y;
	Y -= Y.x / X.x * X;
	Z -= Z.y / Y.y * Y;
	X -= X.z / Y.z * Y;
	Y -= Y.z / Z.z * Z;
	X -= X.y / Y.y * Y;

	glm::vec3 ret = { X.w / X.x, Y.w / Y.y, Z.w / Z.z };
	return ret;
}
static glm::vec3 getClosestPointOnLine(
	const glm::vec3& lineDir, const glm::vec3& linePos,
	const glm::vec3& secondDir, const glm::vec3& secondPos)
{
	auto abc = matrixSolver(secondDir, glm::normalize(glm::cross(glm::normalize(secondDir), glm::normalize(lineDir))), -lineDir, linePos - secondPos);
	/*{
		auto& tra = sphere0.get<TransformComponent>();
		tra.pos = secondPos + secondDir * abc.x;
		tra.recomputeMatrix();
	}
	{
		auto& tra = sphere1.get<TransformComponent>();
		tra.pos = linePos - lineDir * abc.z;
		tra.recomputeMatrix();
	}*/
	/*{
		auto& tra = line1.get<TransformComponent>();
		ND_INFO("cros {}", glm::cross(secondDir, lineDir));
		auto t = glm::normalize(glm::cross(secondDir, lineDir));
		auto r = sqrt(t.x * t.x + t.y * t.y + t.z * t.z);
		tra.rot.p = glm::atan(t.y / t.x);
		tra.rot.t = glm::acos(t.z / r);
		tra.pos = secondPos + secondDir * abc.z;
		tra.recomputeMatrix();
	}
	{
		auto& tra = line2.get<TransformComponent>();
		auto t = lineDir;
		auto r = sqrt(t.x * t.x + t.y * t.y + t.z * t.z);
		tra.rot.t = glm::atan(t.y / t.x);
		tra.rot.p = glm::acos(t.z / r);
		tra.pos = linePos + lineDir * abc.z;
		tra.recomputeMatrix();
	}*/
	return abc.z * lineDir + linePos;
}



float thic = 2;

struct EditorHUD
{
	enum class MODE
	{
		TRANSLATE,
		ROTATE,
		SCALE
	} mode;
	bool enable=true;
	Entity selectedEntity;
	FrameBuffer* fbo;
	ShaderPtr shader;
	glm::mat4 world_trans;
	MeshPtr cubeMesh;
	float rescale = 1;
	glm::vec4 color;
	bool dragging = false;
	glm::vec3 oldPos;
	glm::vec3 oldPosOffset;
	enum DIR
	{
		X,Y,Z,NONE
	} dir=NONE;
	glm::vec3 getDir(DIR d) {
		switch (d) {
			case X: return { 1,0,0 };
			case Y: return { 0,1,0 };
			case Z: return { 0,0,1 };
			default: return {0,0,0};
	} }
	void init()
	{
		shader = Shader::create("res/shaders/Arrow.shader");
		
		fbo = FrameBuffer::create(FrameBufferInfo().defaultTarget(1920, 1080, TextureFormat::RGBA).special(FBAttachment::DEPTH_STENCIL));
		cubeMesh = MeshLibrary::buildNewMesh(MeshDataFactory::buildCube(0.001f* thic));
	}
	void onScreenResize(int width,int height)
	{
		fbo->resize(width, height);
	}
	void renderCross(const glm::mat4& trans)
	{
		auto sh = std::static_pointer_cast<GLShader>(shader);
		sh->setUniform4f("color", 1, 0, 0, 1);
		sh->setUniformMat4("world", trans *glm::translate(glm::mat4(1.f),glm::vec3(0.1* thic/2,0,0))* glm::scale(glm::mat4(1.f), glm::vec3(100/ thic, 1, 1)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);
		sh->setUniform4f("color", 0, 1, 0, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0.0,0.1* thic/2, 0)) * glm::scale(glm::mat4(1.f), glm::vec3(1, 100 / thic, 1)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);
		sh->setUniform4f("color", 0, 0, 1, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0.1* thic/2)) * glm::scale(glm::mat4(1.f), glm::vec3(1, 1, 100 / thic)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);
	}
	
	void render()
	{
		if (!enable||!selectedEntity)
			return;
		fbo->bind();
		fbo->clear(BuffBit::COLOR|BuffBit::DEPTH,{0,0,0,0});
		cubeMesh->vao_temp->bind();
		shader->bind();
		auto sh = std::static_pointer_cast<GLShader>(shader);
		renderCross(env.proj * env.view * world_trans* glm::scale(glm::mat4(1.f), glm::vec3(rescale)));
		auto t = glm::mat4(glm::mat3(env.view));
		t[3][3] = 1;
		renderCross(glm::translate(glm::mat4(1.f), glm::vec3(0.9, 0.9, -0.1))*t );


		glm::ivec2 pos = APin().getMouseLocation();
		auto tex = fbo->getAttachment();
		pos = glm::clamp(pos, glm::ivec2(0, 0), glm::ivec2(tex->width(), tex->height()));
		GLCall(glReadPixels(pos.x,tex->height()-pos.y, 1, 1, GL_RGBA, GL_FLOAT, &color));

		ND_IMGUI_VIEW("HUD", fbo->getAttachment());
		Gcon.enableDepthTest(false);
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		Effect::render(fbo->getAttachment(), Renderer::getDefaultFBO());
		Renderer::getDefaultFBO()->bind();
	}

	void onUpdate()
	{
		if (!enable||!selectedEntity)
			return;

		world_trans = glm::translate(glm::mat4(1.f),selectedEntity.get<TransformComponent>().pos);
		rescale = glm::distance(selectedEntity.get<TransformComponent>().pos,env.camera_pos);

		//ND_BUG("loc {} {}", APin().getMouseLocation().x, APin().getMouseLocation().y);
		if (!dragging) {
			if (APin().isMouseFreshlyPressed(MouseCode::LEFT))
			{
				if (color == glm::vec4(0, 0, 0, 0))
					return;
				dir = NONE;
				if (color == glm::vec4(0, 0, 1, 1))
					dir = Z;
				else if (color == glm::vec4(0, 1, 0, 1))
					dir = Y;
				else if (color == glm::vec4(1, 0, 0, 1))
					dir = X;
				if (dir != NONE)
				{
					dragging = true;
					oldPos = selectedEntity.get<TransformComponent>().pos;
					auto s = screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());

					oldPosOffset = selectedEntity.get<TransformComponent>().pos - getClosestPointOnLine(getDir(dir), oldPos, s, env.camera_pos);
				}
			}
		}else
		{
			if (!APin().isMousePressed(MouseCode::LEFT))
			{
				dragging = false;
				
			}else
			{
				auto s = screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());

				selectedEntity.get<TransformComponent>().pos = getClosestPointOnLine(getDir(dir), oldPos, s, env.camera_pos)+ oldPosOffset;
				selectedEntity.get<TransformComponent>().recomputeMatrix();
			}
		}
		
	}

	
	
};
static EditorHUD hud;

static glm::vec3 worldToScreen(const glm::vec3& src,const glm::mat4 proj,const glm::mat4& view,const glm::vec2& screenResolution)
{
	glm::vec4 s = view * glm::vec4(src.x,src.y,src.z,1);
	auto depth = s.z;
	s = proj*s;
	s /= s.w;
	s.x = (s.x + 1.f) / 2.f;
	s.y = (s.y + 1.f) / 2.f;
	auto v = glm::vec2(s) * screenResolution;
	return glm::vec3(v, depth);
}

static Entity sphere0;
static Entity sphere1;
static Entity sphere2;


UniformLayout envLayout;
static MaterialPtr enviroment;

static void onCamComponentDestroyed(entt::registry& reg, entt::entity ent)
{

}

struct WireMoveScript :NativeScript
{
	void onUpdate()
	{
		auto& transform = getComponent<TransformComponent>();
		auto& camTrans = scene->currentCamera().get<TransformComponent>();

		bool rec = false;
		if (abs((transform.pos.x + 20 * 4) - camTrans.pos.x) > 4) {
			transform.pos.x = camTrans.pos.x - 20 * 4;
			rec = true;
		}
		if (abs((transform.pos.z + 20 * 4) - camTrans.pos.z) > 4) {
			transform.pos.z = camTrans.pos.z - 20 * 4;
			rec = true;
		}
		if (rec)
			transform.recomputeMatrix();

	}
};
static float centerDepth;
static float* depth_buff;
static Entity sphere;

void EditorLayer::onAttach()
{
	hud.init();
	auto screenRes = glm::vec2(100, 100);
	auto worldPos = glm::vec3(2, 5, 10);
	auto proj = glm::perspective(glm::quarter_pi<float>(), screenRes.x / screenRes.y, 1.f, 100.f);
	auto view = glm::translate(glm::mat4(1.f),glm::vec3(1, 1, 1));
	auto world = glm::mat4(1.f);
	//auto screenTrans = worldToScreen(worldPos, proj, view, world, screenRes);
	//auto back = screenToWorld(screenTrans,proj,view,world,screenRes);

	
	depth_buff = &centerDepth;
	//depth_buff = (float*)malloc(1920 * 1080 * sizeof(float));
	components_imgui_access::windows.init();
	auto t = Atelier::get();//just init atelier
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/ignore/OpenSans-Regular.ttf").c_str(), 20);

	//adding JP (merging with current font) -> nihongode ok kedo, mada nai to omou
	/*ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/NotoSansCJKjp-Medium.otf").c_str(), 20, &config, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->Build();*/
	m_scene = new NewScene;
	m_scene->reg().on_destroy<CameraComponent>().connect<&onCamComponentDestroyed>();
	components_imgui_access::windows.scene = m_scene;


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
	enviroment = Material::create({ nullptr, "GLO", "Enviroment", &envLayout });


	auto modelMat = Material::create({
		std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
		"modelMaterial"
		});
	modelMat->setValue("color", glm::vec4(1.0, 1.0, 0, 1));
	modelMat->setValue("shines", 64.f);


	auto crate1Mesh = MeshLibrary::loadOrGet("res/models/cube.fbx");


	auto simpleMat = MaterialLibrary::create({
	std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
	"simpleColorMat"
		});
	simpleMat->setValue("color", glm::vec4(1.0, 1.0, 0.5, 1));
	simpleMat->setValue("shines", 64.f);
	
	if (!FUtil::exists("res/models/dragon.bin"))
	{
		ND_INFO("Building dragon binary mesh");
		MeshDataFactory::writeBinaryFile(
			"res/models/dragon.bin", *Colli::buildMesh("res/models/dragon.obj"));
	}
	//adding cubemap
	{
		//adding cube_map
		auto mesh = MeshLibrary::registerMesh(MeshDataFactory::buildCube(40.f));

		auto flags = MaterialFlags::DEFAULT_FLAGS;
		flags = (~(MaterialFlags::FLAG_CULL_FACE | MaterialFlags::FLAG_DEPTH_MASK) & flags) | MaterialFlags::FLAG_CHOP_VIEW_MAT_POS;

		ND_INFO("Loading cubemaps");
		auto mat = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial2" ,nullptr,flags });
		mat->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap2/*.png"))));
		/*auto mat1 = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial3" ,nullptr,flags });
		mat1->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap3/*.png"))));
		auto mat2 = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial4" ,nullptr,flags });
		mat2->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap4/*.png"))));*/
		
		auto ent = m_scene->createEntity("SkyBox");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
	}
	
	//adding crate
	{
		//auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));
		//auto specular = Texture::create(TextureInfo("res/models/crate_specular.png"));

		//auto mesh = NewMeshFactory::buildNewMesh(Colli::buildMesh(ND_RESLOC("res/models/cube.fbx")));

		/*auto mat = modelMat->copy("crateMaterial");
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));
		mat->setValue("specular", std::shared_ptr<Texture>(specular));
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		MaterialLibrary::save(mat, "res/crateM.mat");*/
		auto mat = MaterialLibrary::loadOrGet("res/models/crateM.mat");

		auto ent = m_scene->createEntity("Crate");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(1.f), glm::vec3(100.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(crate1Mesh->getID(), mat->getID());
	}
	
	//adding sphere
	{
		auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));

		auto mesh = MeshLibrary::loadOrGet("res/models/sphere.fbx");

		auto mat = MaterialLibrary::copy(modelMat, "SphereMat");
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

		auto ent = m_scene->createEntity("Sphere");
		//ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
		{

			auto mat = MaterialLibrary::copy(simpleMat, "SphereMat0");
			mat->setValue("color", glm::vec4(1.0, 0.0, 0, 1));
			mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));
			
			sphere0 = m_scene->createEntity("Sphere0");
			sphere0.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
			sphere0.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			{
				auto mat = MaterialLibrary::copy(simpleMat, "SphereMat1");
				mat->setValue("color", glm::vec4(0.0, 1.0, 0, 1));
				mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

				sphere1 = m_scene->createEntity("Sphere1");
				sphere1.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
				sphere1.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			}
			{
				auto mat = MaterialLibrary::copy(simpleMat, "SphereMat2");
				mat->setValue("color", glm::vec4(0.0, 0.0, 1, 1));
				mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

				sphere2 = m_scene->createEntity("Sphere2");
				sphere2.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
				sphere2.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			}
		}
		sphere = ent;
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
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1,10.f,1), glm::vec3(0.f));
		ent.emplaceOrReplace<LightComponent>();
	}
	//adding camera
	{
		auto ent = m_scene->createEntity("Cam");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<CameraComponent>(glm::mat4(1.f), glm::quarter_pi<float>(), 1.f, 100.f);
		m_scene->currentCamera() = ent;
		ent.emplaceOrReplace<NativeScriptComponent>();
		auto& script = ent.get<NativeScriptComponent>();
		script.bind<EditCameraController>();
	}
	//adding wire
	{
		auto ent = m_scene->createEntity("Wire");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(4.f), glm::vec3(0.f));

		auto mesh = MeshLibrary::registerMesh(MeshDataFactory::buildWirePlane(40, 40));
		auto mat = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/Model.shader"),"MAT","WireMat" ,nullptr,MaterialFlags::DEFAULT_FLAGS });
		mat->setValue("shines", 0.f);
		mat->setValue("color", glm::vec4(0.5f, 0.5f, 0.5f, 1));


		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
		ent.emplaceOrReplace<NativeScriptComponent>();
		auto& script = ent.get<NativeScriptComponent>();
		script.bind<WireMoveScript>();
	}
}

void EditorLayer::onDetach()
{
}

void EditorLayer::onUpdate()
{
	hud.onUpdate();
	auto view = m_scene->reg().view<NativeScriptComponent>();
	for (auto entity : view)
	{
		auto& script = view.get(entity);
		if (!script.ptr) {
			script.construct(m_scene->wrap(entity), m_scene);
			script.onCreate();
		}
		script.onUpdate();
	}
}
static glm::vec2 minMaxCam;
void EditorLayer::onRender()
{

	Atelier::get().makePendingPhotos();
	Renderer::getDefaultFBO()->bind();
	Gcon.enableCullFace(true);
	Gcon.enableDepthTest(true);
	Entity camEntity = m_scene->currentCamera();
	//Renderer::getDefaultFBO()->clear(BuffBit::COLOR, { 0,1,0,1 });
	auto& camCom = camEntity.get<CameraComponent>();
	minMaxCam = { camCom.Near,camCom.Far };
	auto& transCom = camEntity.get<TransformComponent>();
	env.camera_pos = transCom.pos;

	env.view = camCom.viewMatrix;
	env.proj = glm::perspective(camCom.fov,
		(float)APwin()->getWidth() / (float)App::get()
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
		if (!m_scene->reg().get<TagComponent>(model).enabled)
			continue;
		//auto& [trans, mod] = models.get<TransformComponent,ModelComponent>(model);
		auto& trans = models.get<TransformComponent>(model);
		trans.recomputeMatrix();
		auto& mod = models.get<ModelComponent>(model);
		auto& material = mod.Material();
		auto& mesh = mod.Mesh();
		//bind mat vars
		material->bind();

		//bind enviroment vars
		enviroment->setRaw(env);
		enviroment->bind(0, material->getShader());
		mesh->vao_temp->bind();

		//bind other vars
		auto s = std::static_pointer_cast<GLShader>(material->getShader());
		//if(mod.material->getShader()->getLayout().getLayoutByName("world"))
		s->setUniformMat4("world", trans.trans);

		auto flags = material->getFlags();
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(flags & MaterialFlags::FLAG_DEPTH_MASK);
			Gcon.enableCullFace(flags & MaterialFlags::FLAG_CULL_FACE);
			Gcon.enableDepthTest(flags & MaterialFlags::FLAG_DEPTH_TEST);
			if (flags & MaterialFlags::FLAG_CHOP_VIEW_MAT_POS)
				s->setUniformMat4("glo.view", glm::mat4(glm::mat3(env.view)) * glm::scale(glm::mat4(1.f), trans.scale));
		}


		if (mesh->indexData.exists())
			Gcon.cmdDrawElements(mesh->data->getTopology(), mesh->indexData.count);
		else
			Gcon.cmdDrawArrays(mesh->data->getTopology(), mesh->vertexData.binding.count);

		//reset if neccessary
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(true);
			Gcon.enableCullFace(true);
			Gcon.enableDepthTest(true);
		}
	}
	auto size = Renderer::getDefaultFBO()->getSize();
	GLCall(glReadPixels(size.x/2, size.y/2,1,1, GL_DEPTH_COMPONENT, GL_FLOAT, depth_buff));
	m_scene->getLookingDepth() = getCurrentDepth();

	hud.render();

	
}
// get world distance based on depth pixel value d
static float transformDepth(float d, float min, float max) {
	return (max * min / (max - min)) / (-d + max / (max - min));
}

void EditorLayer::onImGuiRender()
{
	components_imgui_access::windows.activeCamera = m_scene->currentCamera();
	components_imgui_access::windows.drawWindows();
	hud.selectedEntity = components_imgui_access::windows.selectedEntity;

	m_scene->currentCamera() = components_imgui_access::windows.activeCamera;
	static bool b = true;
	ImGui::Begin("Ray", &b);
	
	ImGui::End();
	/*auto s = screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y -APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
	//ImGui::Text("x=%f y=%f z=%f", s.x, s.y, s.z);
	ImGui::ColorEdit4("color", (float*)&hud.color);

	//sphere.get<TransformComponent>().pos = s * 10.f + env.camera_pos;
	static glm::vec3 defPos = sphere.get<TransformComponent>().pos;
	sphere.get<TransformComponent>().pos = getClosestPointOnLine(glm::vec3(1,0 ,0), glm::vec3(0.0f), s,env.camera_pos);
	sphere.get<TransformComponent>().recomputeMatrix();*/

	
}

void EditorLayer::onEvent(Event& e)
{
	auto view = m_scene->reg().view<NativeScriptComponent>();
	for (auto entity : view)
	{
		auto& script = view.get(entity);
		if (!script.ptr)
			script.construct(m_scene->wrap(entity), m_scene);
		script.onEvent(e);
	}
}

float EditorLayer::getCurrentDepth() {
	//float f = *(depth_buff + APwin()->getWidth() * APwin()->getHeight() / 2);
	float f = *depth_buff;
	return transformDepth(f, minMaxCam.x, minMaxCam.y);
}

void EditorLayer::onWindowResize(int width, int height)
{
	
	hud.onScreenResize(width, height);
}

