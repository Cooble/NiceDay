#pragma once
#include "Event.h"
class MouseEvent :public Event
{
private:
	float m_x;
	float m_y;

public:
	MouseEvent(float x,float y) :
		m_x(x), m_y(y)
	{
	}
	inline const float getX() const { return m_x; }
	inline const float getY() const { return m_y; }
};
class MouseScrollEvent :public MouseEvent
{
private:
	float m_scrollX;
	float m_scrollY;

public:
	MouseScrollEvent(float x,float y,float scrollX,float scrollY):
		MouseEvent(x,y),m_scrollX(scrollX),m_scrollY(scrollY)
	{
	}
	inline const float getScrollX() const { return m_scrollX; }
	inline const float getScrollY() const { return m_scrollY; }
	EVENT_TYPE_BUILD(MouseScroll)
	EVENT_CATEGORY_BUILD(EventCategoryMouse)


	
};
class MouseMoveEvent :public MouseEvent
{
public:
	MouseMoveEvent(float x, float y) :
		MouseEvent(x, y)
	{
	}
	EVENT_TYPE_BUILD(MouseMove)
	EVENT_CATEGORY_BUILD(EventCategoryMouse)


};
class MousePressEvent :public MouseEvent
{
private:
	int m_button;
public:
	MousePressEvent(float x, float y,int button) :
		MouseEvent(x, y), m_button(button)
	{
	}
	inline const int getButton() const { return m_button; }
	EVENT_TYPE_BUILD(MousePress)
	EVENT_CATEGORY_BUILD(EventCategoryMouse | EventCategoryMouseKey)

};
class MouseReleaseEvent :public MouseEvent
{
private:
	int m_button;
public:
	MouseReleaseEvent(float x, float y, int button) :
		MouseEvent(x, y), m_button(button)
	{
	}
	inline const int getButton() const { return m_button; }
	EVENT_TYPE_BUILD(MouseRelease)
	EVENT_CATEGORY_BUILD(EventCategoryMouse | EventCategoryMouseKey)

};

