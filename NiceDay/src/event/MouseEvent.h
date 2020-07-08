#pragma once
#include "Event.h"

class MouseEvent : public Event
{
private:
	union {
		struct {
			float m_x;
			float m_y;
		};
		glm::vec2 m_pos;
	};

public:
	MouseEvent(float x, float y) :
		m_x(x), m_y(y)
	{
	}
	inline void flipY(float screenHeight)
	{
		m_y = screenHeight - 1 - m_y;
	}
	inline float getX() const { return m_x; }
	inline float getY() const { return m_y; }
	inline const glm::vec2& getPos() const { return m_pos; }
	//for internal engine purposes (do not use)
	void setPos(float x, float y) { m_x = x; m_y = y; }
};

class MouseScrollEvent : public MouseEvent
{
private:

	float m_scrollX;
	float m_scrollY;

public:
	MouseScrollEvent(float x, float y, float scrollX, float scrollY):
		MouseEvent(x, y), m_scrollX(scrollX), m_scrollY(scrollY)
	{
	}

	inline float getScrollX() const { return m_scrollX; }
	inline float getScrollY() const { return m_scrollY; }
	EVENT_TYPE_BUILD(MouseScroll)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseScrollEvent)
};

class MouseFocusGain : public MouseEvent
{
public:
	MouseFocusGain(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseFocusGain)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseFocusGain)
};

class MouseFocusLost : public MouseEvent
{
public:
	MouseFocusLost(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseFocusLost)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseFocusLost)

};

class MouseDrag : public MouseEvent
{
public:
	MouseDrag(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseDrag)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseDrag)
};

class MouseMoveEvent : public MouseEvent
{
public:
	MouseMoveEvent(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseMove)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseMoveEvent)
};

class MousePressEvent : public MouseEvent
{
private:
	int m_button;
public:
	MousePressEvent(float x, float y, int button) :
		MouseEvent(x, y), m_button(button)
	{
	}

	inline int getButton() const { return m_button; }
	EVENT_TYPE_BUILD(MousePress)
	EVENT_CATEGORY_BUILD(Mouse | MouseKey)
	EVENT_COPY(MousePressEvent)
};

class MouseReleaseEvent : public MouseEvent
{
private:
	int m_button;
public:
	MouseReleaseEvent(float x, float y, int button) :
		MouseEvent(x, y), m_button(button)
	{
	}

	inline int getButton() const { return m_button; }
	EVENT_TYPE_BUILD(MouseRelease)
	EVENT_CATEGORY_BUILD(Mouse | MouseKey)
	EVENT_COPY(MouseReleaseEvent)
};
class MouseEnteredEvent : public MouseEvent
{
private:
	bool m_entered;
	int m_window_id;
public:
	MouseEnteredEvent(float x, float y, bool entered,int windowID = 0)
	:MouseEvent(x,y),m_entered(entered),m_window_id(windowID){}
	
	constexpr bool hasEntered()const { return m_entered; }
	constexpr int windowID()const { return m_window_id; }

	EVENT_TYPE_BUILD(MouseEnter)
	EVENT_CATEGORY_BUILD(Mouse)
	EVENT_COPY(MouseEnteredEvent)
};