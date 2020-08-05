#pragma once
#include "ndpch.h"
#include "NewScene.h"

struct CameraComponent;
class Event;
struct Cam 
{
	float speed = 0.4f;
	float turnSpeed = 0.01;
	Entity entity;

	struct Angles
	{
		union{
			struct
			{
				float pitch;
				float yaw;
				float roll;
			};
			glm::vec3 rot;
		};
		Angles() = default;
		Angles(glm::vec3 rot):rot(rot){}
		operator glm::vec3() { return rot; }
	};
	Angles* angles;
	glm::vec3* position;
	void refreshData();
	Cam(Entity e);
	virtual ~Cam() = default;

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
	virtual glm::mat4 getViewMatrix()
	{
		auto out = glm::rotate(glm::mat4(1.f), -(*angles).pitch, { 1, 0, 0 });
		out = glm::rotate(out, -(*angles).yaw, { 0, 1, 0 });
		out = glm::rotate(out, -(*angles).roll, { 0, 0, 1 });

		return glm::translate(out, -(*position));
	}

	virtual void go(const glm::vec3& relativeDirection) {}

	virtual void onEvent(Event& e) {}

	virtual void onUpdate() {}
};

struct PlayerCam : Cam
{
	float mouseSensitivity = 2.5f;
	glm::vec2 currentMouseCoords;
	glm::vec3 camRot;

	bool fullRotation = false;
	PlayerCam(Entity e);
	void onEvent(Event& e) override;


	void onUpdate() override;

	void go(const glm::vec3& relativeDirection) override;
};

struct EditorCam : Cam
{
	float mouseSensitivity = 2.5f;
	glm::vec2 currentMouseCoords;
	glm::vec3 camRot;
	glm::vec3 camPos;
	glm::vec3 farPoint;
	const float pointDistance = 10;

	glm::vec2 startCursor;
	bool fullRotation = false;
	bool fullMove = false;
	bool fullRotRelative = false;

	EditorCam(Entity e);
	void onEvent(Event& e) override;

	void onUpdate() override;

	void go(const glm::vec3& relativeDirection) override;
};
