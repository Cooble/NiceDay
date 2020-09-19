#pragma once
#include "ndpch.h"
#include "glm/gtx/euler_angles.hpp"
#include "scene/Material.h"
#include "event/Event.h"
#include "Mesh.h"

struct TransformComponent
{
	glm::vec3 pos;
	glm::vec3 scale;
	glm::vec3 rot;
	glm::mat4 trans;

	void recomputeMatrix()
	{
		trans = glm::mat4(1.f);
		trans = glm::translate(trans, pos);
		trans = trans* glm::eulerAngleYXZ(rot.x, rot.y, rot.z);
		trans = glm::scale(trans, scale);
	}
	
	TransformComponent() = default;
	TransformComponent(const glm::vec3& p, const glm::vec3& s, const glm::vec3& r) :pos(p), scale(s), rot(r), trans(glm::mat4(1.f)) { recomputeMatrix(); }

};

struct TagComponent
{
	const static int MAX_LENGTH = 30;
	char name[MAX_LENGTH];
	bool enabled = true;

	TagComponent() :name("UNSPECIFIED"){}
	TagComponent(const char* n)
	{
		int size = strlen(n);
		ASSERT(size <= MAX_LENGTH, "Too long tag");
		memcpy(&name, n, size+1);
	}
	const char* operator()() const { return name; }
};
struct ModelComponent
{
	Strid mesh=0;
	Strid material=0;
	
	MeshPtr& Mesh() { return MeshLibrary::get(mesh); }
	MaterialPtr& Material() { return MaterialLibrary::get(material); }
	
	ModelComponent() = default;
	ModelComponent(Strid mesh, Strid mat) :mesh(mesh), material(mat){}

};

struct LightComponent
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	//attenuation
	float constant;
	float linear;
	float quadratic;

	LightComponent()
	{
		ambient = { 0.1f, 0.1f, 0.1f };
		diffuse = { 0.5f, 0.5f, 0.5f };
		specular = { 1.f, 1.f, 1.f };
		constant = 1.f;
		linear = 0.0014f;
		quadratic = 0.000007f;
	}
};
struct CameraComponent
{
	glm::mat4 viewMatrix;
	float fov;
	float Near=1;
	float Far=100;
	enum CameraType
	{
		EDITOR,
		PLAYER
	}type= CameraType::EDITOR;
};


ND_HAS_MEMBER_METHOD_PREPARE(onCreate);
ND_HAS_MEMBER_METHOD_PREPARE(onDestroy);
ND_HAS_MEMBER_METHOD_PREPARE(onUpdate);
ND_HAS_MEMBER_METHOD_PREPARE(onEvent);

class Entity;
class NewScene;
class Event;

struct NativeScriptComponent
{
	void* ptr=nullptr;
	std::function<void(void**,Entity&,NewScene*)> constructorFunc;
	std::function<void(void*)> destructorFunc;
	std::function<void(void*)> onCreateFunc;
	std::function<void(void*)> onDestroyFunc;
	std::function<void(void*)> onUpdateFunc;
	std::function<void(void*,Event&)> onEventFunc;

	template<typename T>
	void bind()
	{
		constructorFunc = [](void** ptr, Entity& entity,NewScene* scene) {*ptr = new T(); ((T*)(*ptr))->entity = entity; ((T*)(*ptr))->scene = scene; };
		destructorFunc = [](void* ptr) {delete (T*)ptr; };

		if constexpr (ND_HAS_MEMBER_METHOD(T,onCreate))
			onCreateFunc = [](void* ptr) {((T*)ptr)->onCreate(); };
		if constexpr (ND_HAS_MEMBER_METHOD(T, onDestroy))
			onDestroyFunc = [](void* ptr) {((T*)ptr)->onDestroy(); };
		if constexpr (ND_HAS_MEMBER_METHOD(T, onUpdate))
			onUpdateFunc = [](void* ptr) {((T*)ptr)->onUpdate(); };
		if constexpr (ND_HAS_MEMBER_METHOD(T, onEvent))
			onEventFunc = [](void* ptr, Event& e) {((T*)ptr)->onEvent(e); };

	}
	void onCreate() { if(onCreateFunc)onCreateFunc(ptr); }
	void onUpdate() { if (onUpdateFunc)onUpdateFunc(ptr); }
	void onDestroy() { if (onDestroyFunc)onDestroyFunc(ptr); }
	void onEvent(Event& e){ if (onEventFunc)onEventFunc(ptr,e); }
	void construct(Entity& entity,NewScene* scene) { constructorFunc(&ptr,entity,scene); }
	void destruct() { destructorFunc(ptr); }
};

