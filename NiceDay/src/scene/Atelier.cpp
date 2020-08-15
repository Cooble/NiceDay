#include "Atelier.h"
#include "graphics/API/FrameBuffer.h"
#include "Mesh.h"
#include "Colli.h"
#include "graphics/Effect.h"
#include "imgui.h"


void Atelier::init()
{
	m_fbo = FrameBuffer::create(FrameBufferInfo().multiSample(4));
	m_fbo->createBindSpecialAttachment(FBAttachment::DEPTH_STENCIL, { 256,256 });
	m_background = TextureLib::loadOrGetTexture("res/models/material_bg.png");
	m_sphere = MeshLibrary::buildNewMesh(Colli::buildMesh(ND_RESLOC("res/models/sphere.fbx")));

	static UniformLayout envLayout;
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
	m_enviroment = Material::create({ nullptr, "GLO", "Enviroment", &envLayout});

	
	m_default_material = Material::create({ ShaderLib::loadOrGetShader("res/shaders/Model.shader"),"MAT","DefaultMaterial" });
	m_default_material->setValue("color", glm::vec4(0.5f,0.5f,0.5f,1.f));
	m_default_material->setValue("shines", 64.f);

	LightComponent l = LightComponent();
	m_env.ambient = l.ambient;
	m_env.diffuse = l.diffuse;
	m_env.specular = l.specular;
	m_env.constant = l.constant;
	m_env.linear = l.linear;
	m_env.quadratic = l.quadratic;
	m_env.sunPos = { -5.f,5.f,5.f };
	

	m_env.camera_pos = glm::vec3(0, 0, 2.7f);
	m_env.view = glm::translate(glm::mat4(1.f), -m_env.camera_pos);
	m_env.proj = glm::perspective(glm::quarter_pi<float>(),1.0f, 1.f, 100.f);
}
void Atelier::snapshot(TexturePtr& photo, MaterialPtr& mat)
{
	Gcon.enableDepthTest(false);
	Gcon.disableBlend();

	m_fbo->attachTexture(photo.get(), 0);
	m_fbo->clear(BuffBit::COLOR | BuffBit::DEPTH, { 0,0,0.5f,1 });
	Effect::render(m_background.get(), m_fbo);
	
	Gcon.enableDepthTest(true);
	Gcon.enableBlend();
	
	
	//bind mat vars
	mat->bind();

	//bind enviroment vars
	m_enviroment->setRaw(m_env);

	m_enviroment->bind(0, mat->getShader());
	m_sphere->vao_temp->bind();

	//bind other vars
	auto s = std::static_pointer_cast<GLShader>(mat->getShader());
	//if(mod.material->getShader()->getLayout().getLayoutByName("world"))
	s->setUniformMat4("world", glm::mat4(1.f));

	auto flags = mat->getFlags();
	
	//Gcon.depthMask(flags & MaterialFlags::FLAG_DEPTH_MASK);
	//Gcon.enableCullFace(flags & MaterialFlags::FLAG_CULL_FACE);
	//Gcon.enableDepthTest(flags & MaterialFlags::FLAG_DEPTH_TEST);
	
	if (m_sphere->indexData.exists())
		Gcon.cmdDrawElements(m_sphere->data->getTopology(), m_sphere->indexData.count);
	else
		Gcon.cmdDrawArrays(m_sphere->data->getTopology(), m_sphere->vertexData.binding.count);

	//Gcon.depthMask(true);
	//Gcon.enableCullFace(true);
	//Gcon.enableDepthTest(true);
}
void Atelier::snapshot(TexturePtr& photo, MeshPtr& mesh)
{
	Gcon.enableDepthTest(false);
	Gcon.disableBlend();

	m_fbo->attachTexture(photo.get(), 0);
	m_fbo->clear(BuffBit::COLOR | BuffBit::DEPTH, { 0,0,0.5f,1 });
	Effect::render(m_background.get(), m_fbo);

	Gcon.enableDepthTest(true);
	Gcon.enableBlend();


	//bind mat vars
	m_default_material->bind();

	//bind enviroment vars
	m_enviroment->setRaw(m_env);

	m_enviroment->bind(0, m_default_material->getShader());
	mesh->vao_temp->bind();

	//bind other vars
	auto s = std::static_pointer_cast<GLShader>(m_default_material->getShader());
	//if(mod.material->getShader()->getLayout().getLayoutByName("world"))
	glm::mat4 trans(1.f);
	auto aabb = mesh->data->getAABB();
	float maxDim = glm::max(glm::max(aabb.getDim().x, aabb.getDim().y), aabb.getDim().z);
	trans = glm::scale(trans, glm::vec3(1.f / maxDim));
	trans = glm::translate(trans, -aabb.getCenter());
	trans = trans * glm::eulerAngleYXZ(-glm::quarter_pi<float>()/2.f, -glm::quarter_pi<float>()/2.f, 0.f);
	
	s->setUniformMat4("world", trans);

	auto flags = m_default_material->getFlags();

	//Gcon.depthMask(flags & MaterialFlags::FLAG_DEPTH_MASK);
	//Gcon.enableCullFace(flags & MaterialFlags::FLAG_CULL_FACE);
	//Gcon.enableDepthTest(flags & MaterialFlags::FLAG_DEPTH_TEST);
	Gcon.enableCullFace(false);//ensure that obj will be always visible no matter the rotation

	if (mesh->indexData.exists())
		Gcon.cmdDrawElements(mesh->data->getTopology(), mesh->indexData.count);
	else
		Gcon.cmdDrawArrays(mesh->data->getTopology(), mesh->vertexData.binding.count);
	Gcon.enableCullFace(true);
}

static Strid toStrid(MaterialPtr& mat)
{
	char c[9];
	*(uint64_t*)c = (uint64_t)mat.get();//unique identifier is pointer
	c[8] = 0;
	return StringId(c)();
}
static Strid toStrid(MeshPtr& mat)
{
	char c[9];
	*(uint64_t*)c = (uint64_t)mat.get();//unique identifier is pointer
	c[8] = 0;
	return StringId(c)();
}

TexturePtr Atelier::getPhoto(MaterialPtr& mat)
{
		auto id = toStrid(mat);
	auto it = m_photos.find(id);
	if(it==m_photos.end())
	{
		auto out = std::shared_ptr<Texture>(Texture::create(TextureInfo().size(AtelierDim::width,AtelierDim::height)));
		m_photos[id] = out;
		m_pending_work.push_back(mat);
		return out;
	}
	return m_photos[id];
}
TexturePtr Atelier::getPhoto(MeshPtr& mat)
{
	//ImGui::DragFloat3("Pos", glm::value_ptr(m_env.camera_pos));
	//m_env.view = glm::translate(glm::mat4(1.f), -m_env.camera_pos);

	auto id = toStrid(mat);
	auto it = m_photos.find(id);
	if (it == m_photos.end())
	{
		auto out = std::shared_ptr<Texture>(Texture::create(TextureInfo().size(AtelierDim::width, AtelierDim::height)));
		m_photos[id] = out;
		m_pending_work_mesh.push_back(mat);
		return out;
	}
	//m_pending_work_mesh.push_back(mat);
	return m_photos[id];
}

TexturePtr Atelier::assignPhotoWork(MaterialPtr& mat)
{
	m_pending_work.push_back(mat);
	return getPhoto(mat);
}
TexturePtr Atelier::assignPhotoWork(MeshPtr& mat)
{
	m_pending_work_mesh.push_back(mat);
	return getPhoto(mat);
}

void Atelier::makePendingPhotos()
{
	for (auto& ptr : m_pending_work)
		snapshot(m_photos[toStrid(ptr)], ptr);
	for (auto& ptr : m_pending_work_mesh)
		snapshot(m_photos[toStrid(ptr)], ptr);
	m_pending_work.clear();
	m_pending_work_mesh.clear();
}
