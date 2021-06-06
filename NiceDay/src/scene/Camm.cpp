#include "Camm.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "core/FakeWindow.h"


namespace nd {

void CameraController::loadComponentsData()
{
	angles = reinterpret_cast<Angles*>(&getComponent<TransformComponent>().rot);
	position = &getComponent<TransformComponent>().pos;
}

void PlayerCameraController::onEvent(Event& e)
{
	loadComponentsData();
	auto press = dynamic_cast<KeyPressEvent*>(&e);
	if (press)
	{
		if (press->getKey() == KeyCode::W)
		{
			fullRotation = !fullRotation;
			if (fullRotation)
			{
				auto center = APwin()->getDimensions() / 2.f;
				fullRotation = true;
				APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
				APwin()->setCursorPos(center);
				mouseCorrdsOffset = center;
				camRot = (*angles);
			}
			else
			{
				auto center = APwin()->getDimensions() / 2.f;
				APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
				APwin()->setCursorPos(center);
			}
		}
	}
	else if (e.getEventType() == Event::EventType::MouseMove)
	{
		if (fullRotation)
		{
			auto m = dynamic_cast<MouseMoveEvent*>(&e);
			auto delta = m->getPos() - mouseCorrdsOffset;
			(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
			(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

			if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
			{
				//reset cursor loc when got to border angle
				mouseCorrdsOffset.y = m->getY();
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

	getComponent<CameraComponent>().viewMatrix = getViewMatrix();
}

void PlayerCameraController::onUpdate()
{
	loadComponentsData();
	glm::vec3 go(0, 0, 0);

	if (APin().isKeyPressed(KeyCode::RIGHT))
		go.x += 1;
	if (APin().isKeyPressed(KeyCode::LEFT))
		go.x -= 1;


	if (APin().isKeyPressed(KeyCode::SPACE))
		go.y += 1;
	if (APin().isKeyPressed(KeyCode::RIGHT_SHIFT))
		go.y -= 1;


	if (APin().isKeyPressed(KeyCode::UP))
		go.z -= 1;
	if (APin().isKeyPressed(KeyCode::DOWN))
		go.z += 1;

	if (go != glm::vec3(0, 0, 0))
		this->go(glm::normalize(go) * speed);
	getComponent<CameraComponent>().viewMatrix = getViewMatrix();
}

void PlayerCameraController::go(const glm::vec3& relativeDirection)
{
	auto rel = relativeDirection;
	float yy = rel.y;
	rel.y = 0;
	glm::quat q((*angles));
	auto mm = glm::toMat4(q);

	auto v = glm::vec4(rel, 0);
	auto t = mm * v;
	t.y = yy;
	(*position) += glm::vec3(t);
}

void EditCameraController::onEvent(Event& e)
{
	if (scene->currentCamera() != entity)
		return;
	loadComponentsData();
	/*auto& navBar = dynamic_cast<FakeWindow*>(APwin())->getNavigationBar();


	if (navBar.freshPress)
	{
		if (navBar.rotationActive && !fullRotation)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			fullRotation = true;
		}
		else if (navBar.moveActive && !fullMove)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camPos = (*position);
			fullMove = true;
		}
		else if (navBar.lockActive && !fullRotRelative)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			farPoint = (*position) + facingDirection() * pointDistance;
			fullRotRelative = true;
		}
	}

	if (navBar.freshRelease)
	{
		fullRotation = false;
		fullMove = false;
		fullRotRelative = false;
		APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
		APwin()->setCursorPos(startCursor);
	}
	else if (fullRotation)
	{
		auto delta = navBar.drag;
		(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			//currentMouseCoords.y = m->getY();
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
	else if (fullMove)
	{
		auto delta = navBar.drag / 50.f;
		(*position) = camPos;
		go({ -delta.x * speed, delta.y * speed, 0 });
	}
	else if (fullRotRelative)
	{
		auto delta = navBar.drag / 50.f;

		(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			//currentMouseCoords.y = m->getY();
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
	else if (navBar.scrollActive)
	{

		go({ 0, 0, navBar.drag.y/20.f * speed });
	}*/
	getComponent<CameraComponent>().viewMatrix = getViewMatrix();
}

/*
void EditCameraController::onEvent(Event& e)
{
	if (scene->currentCamera() != entity)
		return;
	loadComponentsData();
	auto& navBar = dynamic_cast<FakeWindow*>(APwin())->getNavigationBar();

	auto press = dynamic_cast<KeyPressEvent*>(&e);
	if (press)
	{
		if (press->getKey() == KeyCode::LEFT_CONTROL && !fullRotation && !fullMove)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			fullRotation = true;
		}
		else if (press->getKey() == KeyCode::LEFT_SHIFT && !fullRotation && !fullMove)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camPos = (*position);
			fullMove = true;
		}
		else if (press->getKey() == KeyCode::LEFT_ALT && !fullRotation && !fullMove && !fullRotRelative)
		{
			auto center = APwin()->getDimensions() / 2.f;
			startCursor = APin().getMouseLocation();
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			APwin()->setCursorPos(center);
			currentMouseCoords = center;
			camRot = (*angles);
			farPoint = (*position) + facingDirection() * pointDistance;
			fullRotRelative = true;
		}
	}
	auto rel = dynamic_cast<KeyReleaseEvent*>(&e);
	if (rel)
	{
		if (rel->getKey() == KeyCode::LEFT_CONTROL && fullRotation)
		{
			fullRotation = false;
			APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
			APwin()->setCursorPos(startCursor);
		}
		else if (rel->getKey() == KeyCode::LEFT_SHIFT && fullMove)
		{
			fullMove = false;
			APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
			APwin()->setCursorPos(startCursor);
		}
		else if (rel->getKey() == KeyCode::LEFT_ALT && fullRotRelative)
		{
			fullRotRelative = false;
			APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
			APwin()->setCursorPos(startCursor);
		}
	}
	else if (e.getEventType() == Event::EventType::MouseMove && fullRotation)
	{
		auto m = dynamic_cast<MouseMoveEvent*>(&e);
		auto delta = m->getPos() - currentMouseCoords;
		(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

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
		(*position) = camPos;
		go({ -delta.x * speed, delta.y * speed, 0 });
	}
	else if (e.getEventType() == Event::EventType::MouseMove && fullRotRelative)
	{
		auto m = dynamic_cast<MouseMoveEvent*>(&e);
		auto delta = m->getPos() - currentMouseCoords;
		(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

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
		go({ 0, 0, -m->getScrollY() * speed });
	}
	getComponent<CameraComponent>().viewMatrix = getViewMatrix();
}*/

void EditCameraController::onUpdate()
{
	if (scene->currentCamera() != entity)
		return;
	loadComponentsData();

	auto& navBar = dynamic_cast<FakeWindow*>(APwin())->getNavigationBar();

	if (navBar.freshPress)
	{
		mouseCorrdsOffset = glm::vec2(0.f);
		startCursor = APin().getMouseLocation();
		if (navBar.rotationActive && !fullRotation)
		{
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			camRot = (*angles);
			fullRotation = true;
		}
		else if (navBar.moveActive && !fullMove)
		{
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			camPos = (*position);
			fullMove = true;
		}
		else if (navBar.lockActive && !fullRotRelative)
		{
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			camRot = (*angles);
			//farPoint = (*position) + facingDirection() * pointDistance;
			lastdepth = scene->getLookingDepth();
			fullRotRelativeOverride = lastdepth >= getComponent<CameraComponent>().Far - 0.1f;
			farPoint = (*position) + facingDirection() * lastdepth;
			fullRotRelative = true;
		}
		else if (navBar.scrollActive)
		{
			APwin()->setCursorPolicy(Window::CURSOR_DISABLED);
			camPos = (*position);
		}
	}

	if (navBar.freshRelease)
	{
		fullRotation = false;
		fullMove = false;
		fullRotRelative = false;
		fullRotRelativeOverride = false;
		APwin()->setCursorPolicy(Window::CURSOR_ENABLED);
		APwin()->setCursorPos(startCursor);
	}
	else if (fullRotation || fullRotRelativeOverride)
	{
		auto delta = navBar.drag;
		(*angles).pitch = camRot.x - (delta.y - mouseCorrdsOffset.y) / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			mouseCorrdsOffset.y = delta.y;
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
	else if (fullMove)
	{
		auto delta = navBar.drag * metersPerPixel;
		(*position) = camPos;
		go({-delta.x, delta.y, 0});
	}
	else if (fullRotRelative && !fullRotRelativeOverride)
	{
		auto delta = navBar.drag;

		(*angles).pitch = camRot.x - delta.y / APwin()->getWidth() * mouseSensitivity;
		(*angles).yaw = camRot.y - delta.x / APwin()->getWidth() * mouseSensitivity;

		if ((*angles).pitch < -3.14159f / 2 || (*angles).pitch > 3.14159f / 2)
		{
			//reset cursor loc when got to border angle
			//currentMouseCoords.y = m->getY();
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


		(*position) = farPoint - lastdepth * facingDirection();
	}
	else if (navBar.scrollActive)
	{
		(*position) = camPos;
		go({0, 0, navBar.drag.y * metersPerPixel});
	}


	/*glm::vec3 go(0, 0, 0);
	if (APin().isKeyPressed(KeyCode::RIGHT))
		go.x += 1;
	if (APin().isKeyPressed(KeyCode::LEFT))
		go.x -= 1;


	if (APin().isKeyPressed(KeyCode::SPACE))
		go.y += 1;
	if (APin().isKeyPressed(KeyCode::RIGHT_SHIFT))
		go.y -= 1;


	if (APin().isKeyPressed(KeyCode::UP))
		go.z -= 1;
	if (APin().isKeyPressed(KeyCode::DOWN))
		go.z += 1;

	if (go != glm::vec3(0, 0, 0))
		this->go(glm::normalize(go) * speed);*/
	getComponent<CameraComponent>().viewMatrix = getViewMatrix();
}

void EditCameraController::go(const glm::vec3& relativeDirection)
{
	glm::quat q((*angles));
	auto mm = glm::toMat4(q);

	auto v = glm::vec4(relativeDirection, 0);
	auto t = mm * v;
	(*position) += glm::vec3(t);
}
}
