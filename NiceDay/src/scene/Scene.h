#pragma once
#include "ndpch.h"
#include "Mesh.h"
#include "graphics/API/VertexArray.h"
#include "graphics/API/Texture.h"
#include "graphics/GContext.h"


enum SceneShader
{
	MODEL_SHADER,
	CUBE_MAP_SHADER
};
enum class ObjectType
{
	MODEL,
	LIGHT,
	CAM
};
struct GModel {
	VertexBuffer* vbo=nullptr;
	VertexArray* vao=nullptr;
	IndexBuffer* vio=nullptr;
	Mesh* mesh=nullptr;
	std::vector<Texture*> textures;
	Topology topology = Topology::TRIANGLES;
	SceneShader shader=MODEL_SHADER;
	void set(Mesh* mesh);
};
struct SceneObject
{
	const ObjectType type;
	std::string name;
	bool visible = true;
	SceneObject(ObjectType type,std::string name):type(type),name(name){}
	virtual void imGuiPropsRender();
	const char* typeToString();
	virtual ~SceneObject() = default;
	
};
struct Model: SceneObject
{
	Model(std::string s):SceneObject(ObjectType::MODEL,std::move(s)){}
	GModel* model;
	
	glm::vec3 pos={};
	glm::vec3 rot={};
	glm::vec3 scale={1,1,1};

	float shines=64;
	glm::vec4 color={};

	glm::mat4 createWorldMatrix();

	void imGuiPropsRender() override;
};
struct Light:SceneObject
{
	glm::vec3 pos;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	//attenuation
	float constant;
	float linear;
	float quadratic;

	Light(std::string name);
	void imGuiPropsRender() override;
};
class Shader;
class Event;
struct Cam;
class Scene
{
private:
	std::vector<SceneObject*> m_scene_objects;
	std::vector<GModel*> m_modelos;
	std::vector<Model*> m_models;
	std::vector<Light*> m_lights;
	std::vector<Cam*> m_cameras;
	
	Shader* modelShader;
	Shader* cubeMapShader;
	int m_cam_look_idx;
	int m_cam_event_idx;
	Model* m_wire;
	
	void addModelInternal(Model* model);
	void addLightInternal(Light* light);
	void addCamInternal(Cam* cam);

public:
	Scene();
	void addObject(SceneObject* object);
	void removeObject(SceneObject* object);
	void update();
	void onEvent(Event& e);
	void render();
	void imGuiRender();
	void clearDisplay();
	void renderSkyBox(const glm::mat4& proj, const glm::mat4 view);

	Cam* getLookCam() { return m_cameras[m_cam_look_idx]; }
	Cam* getEventCam() { return m_cameras[ m_cam_event_idx]; }
	
};

