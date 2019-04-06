#pragma once
#include "Event.h"
class WindowCloseEvent :public Event
{
public:
	EVENT_TYPE_BUILD(WindowClose)
	EVENT_CATEGORY_BUILD(EventCategoryWindow)


};
class WindowResizeEvent :public Event
{
private:
	int m_neww, m_newh;

public:
	WindowResizeEvent(int neww,int newh):
		m_neww(neww), m_newh(newh)
	{}
	inline const int getWidth() const { return m_neww; }
	inline const int getHeight() const { return m_newh; }

	EVENT_TYPE_BUILD(WindowResize)
	EVENT_CATEGORY_BUILD(EventCategoryWindow)


};

