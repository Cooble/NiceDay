#pragma once
#include "ndpch.h"
#include "Scene.h"
class Event;
struct Cam :SceneObject
{
	bool lookingThrough=false;
	float speed = 0.4f;
	float turnSpeed = 0.01;
	glm::vec3 pos = glm::vec3(0, 0, 0);
	float fov=glm::quarter_pi<float>();

	union
	{
		struct
		{
			float pitch;
			float yaw;
			float roll;
		};

		glm::vec3 angles;
	};

	constexpr glm::vec3& getPos() { return pos; }
	constexpr glm::vec3& getRotation() { return angles; }


	Cam(std::string name);
	virtual ~Cam() = default;
	void imGuiPropsRender() override;

	glm::mat4 getLookingMat()
	{
		glm::quat q(angles);
		return  glm::toMat4(q);
	}
	glm::vec3 facingDirection()
	{
		glm::quat q(angles);
		return  glm::vec3(glm::toMat4(q) * glm::vec4(0, 0, -1, 0));
	}
	virtual glm::mat4 getViewMatrix()
	{
		auto out = glm::rotate(glm::mat4(1.f), -pitch, { 1, 0, 0 });
		out = glm::rotate(out, -yaw, { 0, 1, 0 });
		out = glm::rotate(out, -roll, { 0, 0, 1 });

		return glm::translate(out, -pos);
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
	PlayerCam(std::string name);
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

	EditorCam(std::string name);
	void onEvent(Event& e) override;

	void onUpdate() override;

	void go(const glm::vec3& relativeDirection) override;
};

