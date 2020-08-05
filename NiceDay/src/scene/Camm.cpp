#include "Camm.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "GLFW/glfw3.h"
#include "core/App.h"
#include "imgui.h"


Cam::Cam(Entity e):entity(e),position(nullptr),angles(nullptr)
{
}


/*void Cam::imGuiPropsRender()
{
	SceneObject::imGuiPropsRender();
	float angle = glm::degrees(fov);
	glm::vec3 angs = glm::degrees((*angles));

	ImGui::Checkbox("LookingThrough", &lookingThrough);
	ImGui::Spacing();
	ImGui::SliderFloat3("Pos", (float*)&pos, -0.1f, 0.5f);
	ImGui::SliderFloat3("Pitch/Yaw/Roll", (float*)&angs, -180, 180);
	ImGui::SliderFloat("FOV", (float*)&angle, 10, 180);
	ImGui::Spacing();
	ImGui::SliderFloat("TurnSpeed", &turnSpeed, 0.001f, 0.01f);
	ImGui::SliderFloat("Speed", (float*)&speed, 0.1f, 0.8f);

	(*angles) = glm::radians(angs);
	fov= glm::radians(angle);

}*/

PlayerCam::PlayerCam(Entity e):Cam(e)
{
}

EditorCam::EditorCam(Entity e):Cam(e)
{
}


void Cam::refreshData()
{
	angles = reinterpret_cast<Angles*>(&entity.get<TransformComponent>().rot);
	position = &entity.get<TransformComponent>().pos;
}
void PlayerCam::onEvent(Event& e)
{
	refreshData();
	auto press = dynamic_cast<KeyPressEvent*>(&e);
	if (press)
	{
		if (press->getKey() == GLFW_KEY_W)
		{
			fullRotation = !fullRotation;
			if (fullRotation)
			{
				auto center = App::get().getWindow()->getDimensions() / 2.f;
				fullRotation = true;
				App::get().getWindow()->setCursorPolicy(Window::CURSOR_DISABLED);
				App::get().getWindow()->setCursorPos(center);
				currentMouseCoords = center;
				camRot = (*angles);
			}
			else
			{
				auto center = App::get().getWindow()->getDimensions() / 2.f;
				App::get().getWindow()->setCursorPolicy(Window::CURSOR_ENABLED);
				App::get().getWindow()->setCursorPos(center);
			}
		}
	}
	else if (e.getEventType() == Event::EventType::MouseMove)
	{
		if (fullRotation)
		{
			auto m = dynamic_cast<MouseMoveEvent*>(&e);
			auto delta = m->getPos() - currentMouseCoords;
			(*angles).pitch = camRot.x - delta.y / App::get().getWindow()->getWidth() * mouseSensitivity;
			(*angles).yaw = camRot.y - delta.x / App::get().getWindow()->getWidth() * mouseSensitivity;

			if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
			{
				//reset cursor loc when got to border angle
				currentMouseCoords.y = m->getY();
				(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
				camRot.x = (*angles).pitch;
			}

			(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
			//modulo from -180 to 180
			if ((*angles).yaw > 3.14159f)
				(*angles).yaw -= 2 * 3.14159f;
			else if ((*angles).yaw < -3.14159f)
				(*angles).yaw += 2 * 3.14159f;
			if ((*angles).roll > 3.14159f)
				(*angles).roll -= 2 * 3.14159f;
			else if ((*angles).roll < -3.14159f)
				(*angles).roll += 2 * 3.14159f;
		}
	}
}

void PlayerCam::onUpdate()
{
	refreshData();
	glm::vec3 go(0, 0, 0);

	if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
		go.x += 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
		go.x -= 1;


	if (App::get().getInput().isKeyPressed(GLFW_KEY_SPACE))
		go.y += 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT_SHIFT))
		go.y -= 1;


	if (App::get().getInput().isKeyPressed(GLFW_KEY_UP))
		go.z -= 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_DOWN))
		go.z += 1;

	if (go != glm::vec3(0, 0, 0))
		this->go(glm::normalize(go) * speed);
}

void PlayerCam::go(const glm::vec3& relativeDirection)
{
	auto rel = relativeDirection;
	float yy = rel.y;
	rel.y = 0;
	glm::quat q((*angles));
	auto mm = glm::toMat4(q);

	glm::vec4 v = glm::vec4(rel, 0);
	auto t = mm * v;
	t.y = yy;
	(*position) += glm::vec3(t);
}

void EditorCam::onEvent(Event& e)
{
	refreshData();
	
	auto press = dynamic_cast<KeyPressEvent*>(&e);
	if (press)
	{
		if (press->getKey() == GLFW_KEY_LEFT_CONTROL && !fullRotation && !fullMove)
		{
			auto center = App::get().getWindow()->getDimensions() / 2.f;
			startCursor = App::get().getInput().getMouseLocation();
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_DISABLED);
			App::get().getWindow()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			fullRotation = true;
		}
		else if (press->getKey() == GLFW_KEY_LEFT_SHIFT && !fullRotation && !fullMove)
		{
			auto center = App::get().getWindow()->getDimensions() / 2.f;
			startCursor = App::get().getInput().getMouseLocation();
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_DISABLED);
			App::get().getWindow()->setCursorPos(center);
			currentMouseCoords = center;
			camPos = (*position);
			fullMove = true;
		}
		else if (press->getKey() == GLFW_KEY_LEFT_ALT && !fullRotation && !fullMove && !fullRotRelative)
		{
			auto center = App::get().getWindow()->getDimensions() / 2.f;
			startCursor = App::get().getInput().getMouseLocation();
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_DISABLED);
			App::get().getWindow()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			farPoint = (*position) + facingDirection() * pointDistance;
			fullRotRelative = true;
		}
	}
	auto rel = dynamic_cast<KeyReleaseEvent*>(&e);
	if (rel)
	{
		if (rel->getKey() == GLFW_KEY_LEFT_CONTROL && fullRotation)
		{
			fullRotation = false;
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_ENABLED);
			App::get().getWindow()->setCursorPos(startCursor);
		}
		else if (rel->getKey() == GLFW_KEY_LEFT_SHIFT && fullMove)
		{
			fullMove = false;
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_ENABLED);
			App::get().getWindow()->setCursorPos(startCursor);
		}
		else if (rel->getKey() == GLFW_KEY_LEFT_ALT && fullRotRelative)
		{
			fullRotRelative = false;
			App::get().getWindow()->setCursorPolicy(Window::CURSOR_ENABLED);
			App::get().getWindow()->setCursorPos(startCursor);
		}
	}
	else if (e.getEventType() == Event::EventType::MouseMove && fullRotation)
	{
		auto m = dynamic_cast<MouseMoveEvent*>(&e);
		auto delta = m->getPos() - currentMouseCoords;
		(*angles).pitch = camRot.x - delta.y / App::get().getWindow()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / App::get().getWindow()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			currentMouseCoords.y = m->getY();
			(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
			camRot.x = (*angles).pitch;
		}

		(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
		//modulo from -180 to 180
		if ((*angles).yaw > 3.14159f)
			(*angles).yaw -= 2 * 3.14159f;
		else if ((*angles).yaw < -3.14159f)
			(*angles).yaw += 2 * 3.14159f;
		if ((*angles).roll > 3.14159f)
			(*angles).roll -= 2 * 3.14159f;
		else if ((*angles).roll < -3.14159f)
			(*angles).roll += 2 * 3.14159f;
	}
	else if (e.getEventType() == Event::EventType::MouseMove && fullMove)
	{
		auto m = dynamic_cast<MouseMoveEvent*>(&e);
		auto delta = (m->getPos() - currentMouseCoords) / 50.f;
		(*position)= camPos;
		go({-delta.x * speed, delta.y * speed, 0});
	}
	else if (e.getEventType() == Event::EventType::MouseMove && fullRotRelative)
	{
		auto m = dynamic_cast<MouseMoveEvent*>(&e);
		auto delta = m->getPos() - currentMouseCoords;
		(*angles).pitch = camRot.x - delta.y / App::get().getWindow()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / App::get().getWindow()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			currentMouseCoords.y = m->getY();
			(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
			camRot.x = (*angles).pitch;
		}

		(*angles).pitch = glm::clamp((*angles).pitch, -3.14159f / 2, 3.14159f / 2); //limit rot from -90 to 90
		//modulo from -180 to 180
		if ((*angles).yaw > 3.14159f)
			(*angles).yaw -= 2 * 3.14159f;
		else if ((*angles).yaw < -3.14159f)
			(*angles).yaw += 2 * 3.14159f;
		if ((*angles).roll > 3.14159f)
			(*angles).roll -= 2 * 3.14159f;
		else if ((*angles).roll < -3.14159f)
			(*angles).roll += 2 * 3.14159f;

		(*position) = farPoint - pointDistance * facingDirection();
	}
	else if (e.getEventType() == Event::EventType::MouseScroll)
	{
		auto m = dynamic_cast<MouseScrollEvent*>(&e);
		go({0, 0, -m->getScrollY() * speed});
	}
}

void EditorCam::onUpdate()
{
	refreshData();
	
	glm::vec3 go(0, 0, 0);

	if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
		go.x += 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
		go.x -= 1;


	if (App::get().getInput().isKeyPressed(GLFW_KEY_SPACE))
		go.y += 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT_SHIFT))
		go.y -= 1;


	if (App::get().getInput().isKeyPressed(GLFW_KEY_UP))
		go.z -= 1;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_DOWN))
		go.z += 1;

	if (go != glm::vec3(0, 0, 0))
		this->go(glm::normalize(go) * speed);
}

void EditorCam::go(const glm::vec3& relativeDirection)
{
	glm::quat q((*angles));
	auto mm = glm::toMat4(q);

	glm::vec4 v = glm::vec4(relativeDirection, 0);
	auto t = mm * v;
	(*position) += glm::vec3(t);
}
