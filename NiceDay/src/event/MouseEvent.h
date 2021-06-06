#pragma once
#include "Event.h"

namespace nd {
enum class MouseCode :int
{
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2,
	NONE
};

class MouseEvent : public Event
{
protected:
	union
	{
		struct
		{
			float m_x;
			float m_y;
		};

		glm::vec2 m_pos;
	};

	int m_mods;
	//might not be used
	MouseCode m_button = MouseCode::NONE;

public:
	MouseEvent(float x, float y, int mods = 0) :
		m_x(x), m_y(y), m_mods(mods)
	{
	}

	void flipY(float screenHeight)
	{
		m_y = screenHeight - 1 - m_y;
	}

	float getX() const { return m_x; }
	float getY() const { return m_y; }
	const glm::vec2& getPos() const { return m_pos; }
	//for internal engine purposes (do not use)
	void setPos(float x, float y)
	{
		m_x = x;
		m_y = y;
	}

	MouseCode getButton() const { return m_button; }

	bool isAltPressed() const { return m_mods & Event::KeyMods::Alt; }
	bool isControlPressed() const { return m_mods & Event::KeyMods::Control; }
	bool isShiftPressed() const { return m_mods & Event::KeyMods::Shift; }
};

class MouseScrollEvent : public MouseEvent
{
private:
	float m_scrollX;
	float m_scrollY;

public:
	MouseScrollEvent(float x, float y, int mods, float scrollX, float scrollY) :
		MouseEvent(x, y, mods), m_scrollX(scrollX), m_scrollY(scrollY)
	{
	}

	float getScrollX() const { return m_scrollX; }
	float getScrollY() const { return m_scrollY; }
	EVENT_TYPE_BUILD(MouseScroll);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseScrollEvent);
};

class MouseFocusGain : public MouseEvent
{
public:
	MouseFocusGain(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseFocusGain);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseFocusGain);
};

class MouseFocusLost : public MouseEvent
{
public:
	MouseFocusLost(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseFocusLost);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseFocusLost);
};

class MouseDrag : public MouseEvent
{
public:
	MouseDrag(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseDrag);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseDrag);
};

class MouseMoveEvent : public MouseEvent
{
public:
	MouseMoveEvent(float x, float y) :
		MouseEvent(x, y)
	{
	}

	EVENT_TYPE_BUILD(MouseMove);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseMoveEvent);
};

class MousePressEvent : public MouseEvent
{
public:
	MousePressEvent(float x, float y, int mods, int button) :
		MouseEvent(x, y, mods)
	{
		m_button = static_cast<MouseCode>(button);
	}

	EVENT_TYPE_BUILD(MousePress);
	EVENT_CATEGORY_BUILD(CatMouse | CatMouseKey);
	EVENT_COPY(MousePressEvent);
};

class MouseReleaseEvent : public MouseEvent
{
public:
	MouseReleaseEvent(float x, float y, int mods, int button) :
		MouseEvent(x, y, mods)
	{
		m_button = static_cast<MouseCode>(button);
	}

	EVENT_TYPE_BUILD(MouseRelease);
	EVENT_CATEGORY_BUILD(CatMouse | CatMouseKey);
	EVENT_COPY(MouseReleaseEvent);
};

class MouseEnteredEvent : public MouseEvent
{
private:
	bool m_entered;
	int m_window_id;
public:
	MouseEnteredEvent(float x, float y, bool entered, int windowID = 0)
		: MouseEvent(x, y), m_entered(entered), m_window_id(windowID)
	{
	}

	constexpr bool hasEntered() const { return m_entered; }
	constexpr int windowID() const { return m_window_id; }

	EVENT_TYPE_BUILD(MouseEnter);
	EVENT_CATEGORY_BUILD(CatMouse);
	EVENT_COPY(MouseEnteredEvent);
};
}
