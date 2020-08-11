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
		trans = glm::translate(glm::mat4(1.f), pos);
		trans = glm::scale(trans, scale);
		trans = trans* glm::eulerAngleYXZ(rot.x, rot.y, rot.z);
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
	MeshPtr mesh;
	MaterialPtr material;
	ModelComponent() = default;
	ModelComponent(MeshPtr mesh, MaterialPtr ptr) :mesh(mesh), material(ptr){}

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
	
};
// points somewhere
// free() will be called on component destruction
// 
// for this to work this ptr needs to point to memory allocated by malloc()!
// and destructor wont be called whatsoever...
struct PointerComponent
{
	void* ptr=nullptr;
	PointerComponent() = default;
	PointerComponent(void* ptr):ptr(ptr){}
};

