#pragma once
#include "ndpch.h"
#include "NewScene.h"
#include "script/NativeScript.h"

struct CameraComponent;

struct CameraController:NativeScript
{
	float mouseSensitivity = 2.5f;
	float speed = 0.4f;
	float turnSpeed = 0.01;
	struct Angles
	{
		union {
			struct
			{
				float pitch;
				float yaw;
				float roll;
			};
			glm::vec3 rot;
		};
		Angles() = default;
		Angles(glm::vec3 rot) :rot(rot) {}
		operator glm::vec3() { return rot; }
	};
	Angles* angles;
	glm::vec3* position;
	glm::vec2 mouseCorrdsOffset=glm::vec2(0.f);
	glm::vec3 camRot;

	glm::mat4 getLookingMat()
	{
		glm::quat q(*angles);
		return  glm::toMat4(q);
	}
	glm::vec3 facingDirection()
	{
		glm::quat q(*angles);
		return  glm::vec3(glm::toMat4(q) * glm::vec4(0, 0, -1, 0));
	}
	glm::mat4 getViewMatrix()
	{
		auto out = glm::rotate(glm::mat4(1.f), -(*angles).pitch, { 1, 0, 0 });
		out = glm::rotate(out, -(*angles).yaw, { 0, 1, 0 });
		out = glm::rotate(out, -(*angles).roll, { 0, 0, 1 });

		return glm::translate(out, -(*position));
	}

	void loadComponentsData();
};
struct PlayerCameraController :CameraController
{
	bool fullRotation = false;
	void onEvent(Event& e);

	void onUpdate();

	void go(const glm::vec3& relativeDirection);
	
};
struct EditCameraController:CameraController
{
	float lastdepth = 0;
	float metersPerPixel = 0.01f;
	glm::vec3 camPos;
	glm::vec3 farPoint;
	const float pointDistance = 10;

	glm::vec2 startCursor;
	bool fullRotation = false;
	bool fullMove = false;
	bool fullRotRelative = false;
	bool fullRotRelativeOverride = false;//no point to rotate around, use rotation instead

	void onEvent(Event& e);
	void onUpdate();

	void go(const glm::vec3& relativeDirection);
};