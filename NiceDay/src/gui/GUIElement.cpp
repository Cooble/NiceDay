#include "ndpch.h"
#include "GUIElement.h"
#include "GUIContext.h"
#include "GLFW/glfw3.h"


void GUIElement::checkFocus(MouseMoveEvent& e)
{
	if (contains(e.getX(), e.getY()))
	{
		if (!has_focus && !e.handled) //if event has been handled we are in a different unfocused window
		{
			has_focus = true;
			onMyEvent(MouseFocusGain(e.getX(), e.getY()));
			e.handled = true;
			//GUIContext::get().submitBroadcastEvent(MouseFocusLost(this->id, GUIElement_InvalidNumber));
		}
	}
	else if (has_focus) {
		has_focus = false;
		onMyEvent(MouseFocusLost(e.getX(), e.getY()));
	}
}


static GEID getGUIID()
{
	static GEID currentId = 0;
	return currentId++;
}

GUIElement::GUIElement(GETYPE type): id(getGUIID()), type(type)
{
	pos = {0, 0};
	dim = {0, 0};
	setPadding(0);
}

GUIElement::~GUIElement()
{
	for (auto child : children)
	{
		delete child;
	}
}

bool GUIElement::contains(float xx, float yy) const
{
	if (isNotSpacial()) //lives inside whole parent
		return true;
	auto& stack = GUIContext::get().getStackPos();
	xx -= stack.x;
	yy -= stack.y;
	return xx >= x && xx < x + width && yy >= y && yy < y + height;
}

void GUIElement::onEvent(Event& e)
{
	//get all broadcast types
	switch (e.getEventType())
	{
	case Event::EventType::MouseMove:
	case Event::EventType::MouseDrag:
	case Event::EventType::MouseRelease:
		GUIContext::get().pushPos(x, y);
		for (auto child : children)
			child->onEvent(e);
		GUIContext::get().popPos(x, y);
		onMyEvent(e);
		return;
	}
	//if its mouseevent we need to check if the mouse is over the element,
	//and its not mousemove which is fed to everyone no matter nani
	if (e.isInCategory(Event::EventCategory::Mouse))
	{
		auto& mouseEvent = dynamic_cast<MouseEvent&>(e);
		GUIContext::get().pushPos(x, y);
		for (auto child : getChildren())
		{
			if (child->contains(mouseEvent.getX(), mouseEvent.getY()))
			{
				child->onEvent(e);
				if (e.handled)
					break;
			}
		}
		GUIContext::get().popPos(x, y);

		if (!e.handled)
			onMyEvent(e);
	}
}

void GUIElement::onEventBroadcast(Event& e)
{
	onMyEvent(e);
	GUIContext::get().pushPos(x, y);
	for (auto child : children)
		child->onEventBroadcast(e);
	GUIContext::get().popPos(x, y);
}

void GUIElement::onMyEvent(Event& e)
{
	switch (e.getEventType())
	{
	case Event::EventType::MouseMove:
		checkFocus(static_cast<MouseMoveEvent&>(e));
		break;
	case Event::EventType::MouseFocusGain:
		has_focus = true;
		break;
	case Event::EventType::MouseFocusLost:
		{
			auto& ev = dynamic_cast<MouseEvent&>(e);
			if (ev.getPos() != glm::vec2(this->id, GUIElement_InvalidNumber))//this is an event that we have sent to the other cause we have gained focus
				has_focus = false;
		}
		break;
	case Event::EventType::MousePress:
		{
			auto& ev = dynamic_cast<MousePressEvent&>(e);
			if (ev.getButton() == GLFW_MOUSE_BUTTON_1)
				is_pressed = true;
		}
		break;

	case Event::EventType::MouseRelease:
		{
			auto& ev = dynamic_cast<MouseReleaseEvent&>(e);
			if (ev.getButton() == GLFW_MOUSE_BUTTON_1)
				is_pressed = false;
		}
		break;
	}
	if (!isNotSpacial() && e.getEventType() != Event::EventType::MouseMove)
		e.handled = true;
}
