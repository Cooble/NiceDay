#include "ndpch.h"
#include "GUIElement.h"
#include "GUIContext.h"
#include "GLFW/glfw3.h"


static GEID getGUIID()
{
	static GEID currentId = 0;
	return currentId++;
}

GUIElement::GUIElement(GETYPE type):
	id(getGUIID()),
	type(type),
	pos({0, 0}),
	dim({0, 0})
{
	setPadding(0);
}

GUIElement::~GUIElement()
{
	for (auto child : m_children)
	{
		delete child;
	}
}

void GUIElement::checkFocus(MouseMoveEvent& e)
{
	if (contains(e.getX(), e.getY()))
	{
		if (!m_has_focus && !e.handled) //if event has been handled we are in a different unfocused window
		{
			m_has_focus = true;
			onMyEvent(MouseFocusGain(e.getX(), e.getY()));
			e.handled = true;
			//GUIContext::get().submitBroadcastEvent(MouseFocusLost(this->id, GUIElement_InvalidNumber));
		}
	}
	else if (m_has_focus)
	{
		m_has_focus = false;
		onMyEvent(MouseFocusLost(e.getX(), e.getY()));
	}
}

void GUIElement::appendChild(GUIElement* element)
{
	GUIE_CHECK_PARENT(element);
	m_children.push_back(element);
	m_children[m_children.size() - 1]->m_parent = this;
	element->onParentAttached();
	onChildChange();
}

void GUIElement::onChildChange()
{
	bool callParent = false;
	if (isAlwaysPacked)
		callParent = packDimensions();
	repositionChildren();

	if (m_parent && callParent)
		m_parent->onChildChange();
	if (callParent)
		if (onDimChange)
			onDimChange(*this);
}

void GUIElement::destroyChild(int index)
{
	ASSERT(index < m_children.size(), "Invalid child id");

	auto child = m_children[index];
	delete child;
	m_children.erase(m_children.begin() + index);

	onChildChange();
}

void GUIElement::destroyChildWithID(GEID id)
{
	for (int i = 0; i < m_children.size(); ++i)
	{
		if(m_children[i]->id==id)
		{
			delete m_children[i];
			m_children.erase(m_children.begin() + i);
		}
	}
	onChildChange();
}

void GUIElement::onParentAttached()
{
	for (auto child : m_children)
	{
		child->onParentAttached();
	}
	adaptToParent();
	if (isAlwaysPacked)
		packDimensions();
	onDimensionChange();
		
}

void GUIElement::onDimensionChange()
{
	adaptToParent();
	for (auto child : m_children)
		child->onParentChanged();

	if (isAlwaysPacked)
		packDimensions();

	repositionChildren();
	if (onDimChange)
		onDimChange(*this);
}

void GUIElement::repositionChildren()
{
	for (auto child : m_children)
	{
		switch (child->m_alignment)
		{
		case GUIAlign::INVALID:
			break;
		case GUIAlign::RIGHT:
			child->x = this->width - child->width - padding[GUI_RIGHT];
			child->y = (this->height - child->height) / 2;
			break;
		case GUIAlign::RIGHT_UP:
			child->x = this->width - child->width - padding[GUI_RIGHT];
			child->y = this->height - child->height - padding[GUI_TOP];
			break;
		case GUIAlign::RIGHT_DOWN:
			child->x = this->width - child->width - padding[GUI_RIGHT];
			child->y = padding[GUI_BOTTOM];
			break;
		case GUIAlign::LEFT:
			child->x = padding[GUI_LEFT];
			child->y = (this->height - child->height) / 2;
			break;
		case GUIAlign::LEFT_UP:
			child->x = padding[GUI_LEFT];
			child->y = this->height - child->height - padding[GUI_TOP];
			break;
		case GUIAlign::LEFT_DOWN:
			child->x = padding[GUI_LEFT];
			child->y = padding[GUI_BOTTOM];
			break;
		case GUIAlign::CENTER:
			child->x = (this->width - child->width) / 2;
			child->y = (this->height - child->height) / 2;
			break;
		case GUIAlign::CENTER_UP:
		case GUIAlign::UP:
			child->x = (this->width - child->width) / 2;
			child->y = this->height - child->height - padding[GUI_TOP];
			break;
		case GUIAlign::CENTER_DOWN:
		case GUIAlign::DOWN:
			child->x = (this->width - child->width) / 2;
			child->y = padding[GUI_BOTTOM];
			break;
		}
	}
}

bool GUIElement::packDimensions()
{
	if (dimInherit == GUIDimensionInherit::WIDTH_HEIGHT) 
		return false;
	
	float lineW[3] = {0, 0, 0};
	float lineH[3] = {0, 0, 0};


	float maxW = 0, maxH = 0;
	for (auto child : m_children)
	{
		switch (child->m_alignment)
		{
		case GUIAlign::INVALID:
			maxW = std::max(maxW, child->x + child->width);
			maxH = std::max(maxH, child->y + child->height);

			break;
		case GUIAlign::RIGHT_UP:
		case GUIAlign::UP:
		case GUIAlign::LEFT_UP:
			lineH[0] = std::max(lineH[0], child->height);
			break;
		case GUIAlign::RIGHT:
		case GUIAlign::CENTER:
		case GUIAlign::LEFT:
			lineH[1] = std::max(lineH[1], child->height);
			break;
		case GUIAlign::RIGHT_DOWN:
		case GUIAlign::DOWN:
		case GUIAlign::LEFT_DOWN:
			lineH[2] = std::max(lineH[2], child->height);
			break;
		}

		switch (child->m_alignment)
		{
		case GUIAlign::INVALID:
			maxW = std::max(maxW, child->x + child->width);
			maxH = std::max(maxH, child->y + child->height);
			break;
		case GUIAlign::RIGHT_UP:
		case GUIAlign::RIGHT:
		case GUIAlign::RIGHT_DOWN:
			lineW[2] = std::max(lineW[2], child->width);
			break;
		case GUIAlign::UP:
		case GUIAlign::CENTER:
		case GUIAlign::DOWN:
			lineW[1] = std::max(lineW[1], child->width);

			break;
		case GUIAlign::LEFT_UP:
		case GUIAlign::LEFT:
		case GUIAlign::LEFT_DOWN:
			lineW[0] = std::max(lineW[0], child->width);
			break;
		}
	}


	maxW = std::max(maxW, lineW[0] + space + lineW[1] + space + lineW[2] + widthPadding());
	maxH = std::max(maxH, lineH[0] + space + lineH[1] + space + lineH[2] + heightPadding());

	bool change;
	switch (dimInherit)
	{
	case GUIDimensionInherit::WIDTH:
		change = height != maxH;
		height = maxH;
		return change;
	case GUIDimensionInherit::HEIGHT:
		change = width != maxW;
		width = maxW;
		return change;
	case GUIDimensionInherit::INVALID:
		change = width != maxW || height != maxH;
		height = maxH;
		width = maxW;
		return change;
	}
	return false;
}

bool GUIElement::adaptToParent()
{
	if (getParent() == nullptr)
		return false;
	auto oldw = width;
	auto oldh = height;
	if (dimInherit != GUIDimensionInherit::INVALID)
	{
		switch (dimInherit)
		{
		case GUIDimensionInherit::WIDTH:
			width = getParent()->width - getParent()->widthPadding();
			break;
		case GUIDimensionInherit::HEIGHT:
			height = getParent()->height - getParent()->heightPadding();
			break;
		case GUIDimensionInherit::WIDTH_HEIGHT:
			width = getParent()->width - getParent()->widthPadding();
			height = getParent()->height - getParent()->heightPadding();
			break;
		}
	}
	return oldw != width || oldh != height;
}

void GUIElement::onParentChanged()
{
	if (adaptToParent())
		onDimensionChange();
}

bool GUIElement::contains(float xx, float yy) const
{
	if (isNotSpaci()) //lives inside whole parent
		return true;
	auto& stack = GUIContext::get().getStackPos();
	xx -= stack.x;
	yy -= stack.y;
	return xx >= x && xx < x + width && yy >= y && yy < y + height;
}

void GUIElement::update()
{
	for (auto child : m_children)
		child->update();
}

void GUIElement::onEvent(Event& e)
{
	//get all broadcast types
	switch (e.getEventType())
	{
	case Event::EventType::MouseMove:
	case Event::EventType::MouseScroll:
	case Event::EventType::MouseDrag:
	case Event::EventType::MouseRelease:
	case Event::EventType::KeyPress:
	case Event::EventType::KeyRelease:
	case Event::EventType::KeyType:
		GUIContext::get().pushPos(x, y);
		for (auto child : m_children)
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
			if (child->contains(mouseEvent.getX(), mouseEvent.getY()))
			{
				child->onEvent(e);
				if (e.handled)
					break;
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
	for (auto child : m_children)
		child->onEventBroadcast(e);
	GUIContext::get().popPos(x, y);
}

void GUIElement::onMyEvent(Event& e)
{
	if (onMyEventFunc)
		onMyEventFunc(e, *this);
	switch (e.getEventType())
	{
	case Event::EventType::MouseMove:
		checkFocus(static_cast<MouseMoveEvent&>(e));
		break;
	case Event::EventType::MouseFocusGain:
		m_has_focus = true;
		break;
	case Event::EventType::MouseFocusLost:
		{
			auto& ev = dynamic_cast<MouseEvent&>(e);
			if (ev.getPos() != glm::vec2(this->id, GUIElement_InvalidNumber))
				//this is an event that we have sent to the other cause we have gained focus
				m_has_focus = false;
		}
		break;
	case Event::EventType::MousePress:
		{
			auto& ev = dynamic_cast<MousePressEvent&>(e);
			if (ev.getButton() == GLFW_MOUSE_BUTTON_1)
				m_is_pressed = true;
		}
		break;

	case Event::EventType::MouseRelease:
		{
			auto& ev = dynamic_cast<MouseReleaseEvent&>(e);
			if (ev.getButton() == GLFW_MOUSE_BUTTON_1)
				m_is_pressed = false;
		}
		break;
	}
	if (!isNotSpaci() && e.getEventType() != Event::EventType::MouseMove&&e.getEventType()==Event::EventType::MousePress)
		e.handled = true;
}

void GUIElement::clearChildren()
{
	for (int i = 0; i < m_children.size(); ++i)
	{
		delete m_children[i];
	}
	m_children.clear();
	onChildChange();
}

bool GUIElement::hasChild(GEID id)
{
	for (auto child : m_children)
	{
		if (child->id == id)
			return true;
	}
	return false;
}

GUIElement* GUIElement::takeChild(GEID id)
{
	for (int i =0;i<m_children.size();i++)
	{
		auto child = m_children[i];
		if (child->id == id)
		{
			m_children.erase(m_children.begin() + i);
			onChildChange();
			child->m_parent = nullptr;
#ifdef ND_DEBUG
			child->m_has_parent= false;
#endif
			return child;
		}
	}
	return nullptr;
}
