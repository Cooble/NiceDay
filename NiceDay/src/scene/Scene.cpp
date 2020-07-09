#include "Scene.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/GContext.h"
#include "Camm.h"
#include "core/App.h"
#include "imgui.h"
#include "graphics/API/FrameBuffer.h"
#include "platform/OpenGL/GLRenderer.h"

void GModel::set(Mesh* mesh)
{
	this->mesh = mesh;
	vbo = VertexBuffer::create(mesh->getVertices(), mesh->getVerticesSize());
	vbo->setLayout(mesh->getLayout());
	if (mesh->getIndicesCount())
		vio = IndexBuffer::create((uint32_t*)mesh->getIndices(), mesh->getIndicesCount());
	vao = VertexArray::create();
	vao->bind();
	vao->addBuffer(*vbo);
	if (mesh->getIndicesCount())
		vao->addBuffer(*vio);
	vao->unbind();
}

void SceneObject::imGuiPropsRender()
{
	char buff[50];
	sprintf(buff, "%s: %s", typeToString(), name.c_str());
	ImGui::TextColored(ImVec4(0, 1, 0, 1), buff);
	ImGui::SameLine();
	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();
}

const char* SceneObject::typeToString()
{
	static const char* model = "Model";
	static const char* light = "Light";
	static const char* cam = "Cam";
	if (type == ObjectType::MODEL)
		return model;
	if (type == ObjectType::LIGHT)
		return light;
	if (type == ObjectType::CAM)
		return cam;
	return nullptr;
}

glm::mat4 Model::createWorldMatrix()
{
	auto out = glm::rotate(glm::mat4(1.f), rot.x, {1, 0, 0});
	out = glm::rotate(out, rot.y, {0, 1, 0});
	out = glm::rotate(out, rot.z, {0, 0, 1});
	out = glm::translate(out, pos);
	return glm::scale(out, scale);
}

void Model::imGuiPropsRender()
{
	SceneObject::imGuiPropsRender();
	ImGui::SliderFloat3("Pos", (float*)&pos, -10, 10);
	ImGui::SliderFloat3("Scale", (float*)&scale, -10, 10);
	ImGui::SliderFloat3("Rotation", (float*)&rot, -10, 10);
	ImGui::ColorEdit4("Color", (float*)&color);
}

void Light::imGuiPropsRender()
{
	SceneObject::imGuiPropsRender();

	ImGui::SliderFloat3("Pos", (float*)&pos, -10, 10);
	float f[3];
	f[0] = constant;
	f[1] = linear;
	f[2] = quadratic;
	if (ImGui::InputFloat3("Const/Lin/Quad", f, "%.5f"))
	{
		constant = f[0];
		linear = f[1];
		quadratic = f[2];
	}

	ImGui::ColorEdit3("Ambient", (float*)&ambient);
	ImGui::ColorEdit3("Diffuse", (float*)&diffuse);
	ImGui::ColorEdit3("Specular", (float*)&specular);
}

Light::Light(std::string name): SceneObject(ObjectType::LIGHT, std::move(name))
{
	pos = {};
	ambient = {0.1f, 0.1f, 0.1f};
	diffuse = {0.5f, 0.5f, 0.5f};
	specular = {1.f, 1.f, 1.f};
	constant = 1.f;
	linear = 0.0014f;
	quadratic = 0.000007f;
}

Scene::Scene()
{
	modelShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/Model.shader"));
	modelShader->bind();
	dynamic_cast<GLShader*>(modelShader)->setUniform1i("u_material.diffuse", 0);
	dynamic_cast<GLShader*>(modelShader)->setUniform1i("u_material.specular", 1);
	dynamic_cast<GLShader*>(modelShader)->setUniform1f("u_material.shines", 64);

	modelShader->unbind();
	cubeMapShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/CubeMap.shader"));


	auto cam = new PlayerCam("PlayerCam");
	auto cam2 = new EditorCam("EditCam");
	m_cam_look_idx = 0;
	m_cam_event_idx = 0;

	addObject(cam);
	addObject(cam2);

	Light* l = new Light("Lux");
	addObject(l);

	{
		//adding wire model
		GModel* wire = new GModel;
		wire->set(MeshFactory::buildWirePlane(40, 40));
		Model* w = new Model("wire");
		w->model = wire;
		w->color = {0.5f, 0.5f, 0.5f, 1};
		w->scale = {4, 1, 4};
		w->shines = 64;
		wire->topology = Topology::LINES;
		addObject(w);
		m_wire = w;
	}


	{
		TimerStaper t("Loading of cubemap took");
		//adding cube_map
		auto* skyTex = Texture::create(TextureInfo(TextureType::_CUBE_MAP, ND_RESLOC("res/images/skymap2/*.png")));
		auto sky = new GModel;
		sky->set(MeshFactory::buildCube());
		auto w = new Model("skybox");
		w->model = sky;
		w->scale = glm::vec3(40);
		w->model->textures.push_back(skyTex);
		w->model->shader = CUBE_MAP_SHADER;
		addObject(w);
	}
}

void Scene::addModelInternal(Model* model)
{
	bool found = false;
	for (int i = 0; i < m_modelos.size(); ++i)
	{
		if (m_modelos[i] == model->model)
		{
			found = false;
			break;
		}
	}
	if (!found)
		m_modelos.push_back(model->model);
	m_models.push_back(model);
}

void Scene::addCamInternal(Cam* cam)
{
	m_cameras.push_back(cam);
}


void Scene::addObject(SceneObject* object)
{
	m_scene_objects.push_back(object);
	if (object->type == ObjectType::MODEL)
		addModelInternal((Model*)object);
	else if (object->type == ObjectType::LIGHT)
		addLightInternal((Light*)object);
	else if (object->type == ObjectType::CAM)
		addCamInternal((Cam*)object);
}

void Scene::addLightInternal(Light* light)
{
	m_lights.push_back(light);
}

void Scene::removeObject(SceneObject* object)
{
	m_scene_objects.erase(std::find(m_scene_objects.begin(), m_scene_objects.end(), object));

	if (object->type == ObjectType::MODEL)
		m_models.erase(std::find(m_models.begin(), m_models.end(), object));
	else if (object->type == ObjectType::LIGHT)
		m_lights.erase(std::find(m_lights.begin(), m_lights.end(), object));
}

void Scene::update()
{
	m_cameras[m_cam_event_idx]->onUpdate();

	//wire reposition
	glm::vec3 pos = -20.f + glm::floor(getLookCam()->pos / 4.f) * 4.f;
	pos.y = 0;
	m_wire->pos = pos;
}

void Scene::onEvent(Event& e)
{
	m_cameras[m_cam_event_idx]->onEvent(e);
}

void Scene::render()
{
	clearDisplay(); //binds fbo

	glm::mat4 proj = glm::perspective(getLookCam()->fov,
	                                  (GLfloat)App::get().getWindow()->getWidth() / (GLfloat)App::get()
	                                                                                         .getWindow()->getHeight(),
	                                  1.f, 100.0f);
	glm::mat4 view = getLookCam()->getViewMatrix();

	renderSkyBox(proj, view);

	modelShader->bind();
	//cam stuff
	{
		dynamic_cast<GLShader*>(modelShader)->setUniformVec3f("u_camera_pos", getLookCam()->pos);
		dynamic_cast<GLShader*>(modelShader)->setUniformMat4("u_viewMat", view);
		dynamic_cast<GLShader*>(modelShader)->setUniformMat4("u_projMat", proj);
	}


	//light stuff
	{
		auto& licht = *m_lights[0];
		dynamic_cast<GLShader*>(modelShader)->setUniformVec3f("u_light.pos", licht.pos);
		dynamic_cast<GLShader*>(modelShader)->setUniformVec3f("u_light.ambient", licht.ambient);
		dynamic_cast<GLShader*>(modelShader)->setUniformVec3f("u_light.diffuse", licht.diffuse);
		dynamic_cast<GLShader*>(modelShader)->setUniformVec3f("u_light.specular", licht.specular);
		dynamic_cast<GLShader*>(modelShader)->setUniform1f("u_light.constant", licht.constant);
		dynamic_cast<GLShader*>(modelShader)->setUniform1f("u_light.linear", licht.linear);
		dynamic_cast<GLShader*>(modelShader)->setUniform1f("u_light.quadratic", licht.quadratic);
	}


	//model stuff
	for (auto& mod : m_models)
	{
		auto& model = *mod;
		if (!model.visible)
			continue;
		if (model.model->shader != MODEL_SHADER)
			continue;
		dynamic_cast<GLShader*>(modelShader)->setUniformMat4("u_worldMat", model.createWorldMatrix());
		dynamic_cast<GLShader*>(modelShader)->setUniform1f("u_material.shines", model.shines);
		dynamic_cast<GLShader*>(modelShader)->setUniformVec4f("u_material.color", model.color);
		model.model->vao->bind();
		int i = 0;
		for (auto texture : model.model->textures)
			texture->bind(i++);

		if (model.model->vio)
			Gcon.cmdDrawElements(model.model->topology, model.model->vio->getCount());
		else
			Gcon.cmdDrawArrays(model.model->topology,
			                   model.model->vbo->getSize() / model.model->vbo->getLayout().getStride());
	}
}

void Scene::renderSkyBox(const glm::mat4& proj, const glm::mat4 view)
{
	cubeMapShader->bind();
	dynamic_cast<GLShader*>(cubeMapShader)->setUniformMat4("u_projMat", proj);

	Gcon.enableDepthTest(false);
	GLCall(glDepthMask(GL_FALSE));
	GLCall(glDisable(GL_CULL_FACE));
	for (int i = 0; i < m_models.size(); ++i)
	{
		auto& model = *m_models[i];
		auto& gModel = *model.model;
		if (gModel.shader != CUBE_MAP_SHADER)
			continue;
		if (!model.visible)
			continue;

		//draw sky
		dynamic_cast<GLShader*>(cubeMapShader)->setUniformMat4("u_viewMat",
		                                                       glm::mat4(glm::mat3(view)) * glm::scale(
			                                                       glm::mat4(1.f), model.scale));
		gModel.vao->bind();
		for (auto texture : gModel.textures)
			texture->bind(i++);

		if (gModel.vio)
			Gcon.cmdDrawElements(gModel.topology, gModel.vio->getCount());
		else
			Gcon.cmdDrawArrays(gModel.topology, gModel.vbo->getSize() / gModel.vbo->getLayout().getStride());
	}
	GLCall(glDepthMask(GL_TRUE));
	GLCall(glEnable(GL_CULL_FACE));
	Gcon.enableDepthTest(true);

}

void Scene::imGuiRender()
{
	static bool open = true;
	if (ImGui::Begin("Sceene", &open, ImGuiWindowFlags_NoNav))
	{
		bool lookEdit = m_cam_look_idx == 1;
		if (ImGui::Checkbox("Look editor", &lookEdit))
			m_cam_look_idx = lookEdit ? 1 : 0;

		bool eventEdit = m_cam_event_idx == 1;
		if (ImGui::Checkbox("Event editor", &eventEdit))
			m_cam_event_idx = eventEdit ? 1 : 0;

		auto cam = m_cameras[m_cam_event_idx];

		
		static int selected = -1;
		if (ImGui::TreeNode("Objects"))
		{
			for (int n = 0; n < m_scene_objects.size(); n++)
			{
				char buf[64];

				sprintf(buf, "%s: %s", m_scene_objects[n]->typeToString(), m_scene_objects[n]->name.c_str());
				bool selectedi = n == selected;
				if (selectedi)
					ImGui::PushStyleColor(ImGuiCol_Text, {0, 1, 0, 1});
				if (ImGui::TreeNodeEx(buf, ImGuiTreeNodeFlags_OpenOnArrow))
				{
					if (selectedi)
						ImGui::PopStyleColor();
					//ImGui::Text("Blemc");
					ImGui::TreePop();
				}
				else if (selectedi) ImGui::PopStyleColor();
				if (ImGui::IsItemClicked())
				{
					selected = n;
				}
				ImGui::SameLine(200);

				ImGui::Checkbox("##noid", &m_scene_objects[n]->visible);
			}
			if (ImGui::Selectable("None", false))
				selected = -1;
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (selected != -1)
		{
			ImGui::PushID("obj");
			m_scene_objects[selected]->imGuiPropsRender();
			ImGui::PopID();
		}
		for (int i = 0; i < m_cameras.size(); ++i)
		{
			//new cam has been switched to
			if (i != m_cam_look_idx && m_cameras[i]->lookingThrough)
			{
				m_cameras[m_cam_look_idx]->lookingThrough = false;
				m_cam_look_idx = i;
				m_cam_event_idx = i;
				break;
			}
		}
	}
	ImGui::End();
}

void Scene::clearDisplay()
{
	//display stuff
	Renderer::getDefaultFBO()->bind();
	Gcon.enableDepthTest(true);
	Gcon.enableBlend();
	GLCall(glEnable(GL_CULL_FACE));
	Renderer::getDefaultFBO()->clear(BuffBit::DEPTH | BuffBit::COLOR, {0.2f, 0.2f, 0.2f, 1.f});
}
