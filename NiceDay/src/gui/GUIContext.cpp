﻿#include "ndpch.h"
#include "GUIContext.h"
#include "event/WindowEvent.h"

void GUIContext::onUpdate()
{
	currentStackPos = { 0,0 };
	for (int i = m_windows.size() - 1; i >= 0; --i)
		m_windows[i]->update();
	closePendingWins();
}

void GUIContext::onEvent(Event& e)
{
	if(e.getEventType()==Event::EventType::WindowResize)
	{
		auto m = static_cast<WindowResizeEvent&>(e);
		for (auto window : m_windows)
		{
			if(window->dimInherit==GUIDimensionInherit::WIDTH_HEIGHT)
			{
				window->height = m.getHeight();
				window->width = m.getWidth();
				window->onDimensionChange();
			}
		}
		return;
	}
	if (m_focused_element)
	{
		switch (e.getEventType())
		{
		case Event::EventType::MouseMove:
		case Event::EventType::MouseDrag:
		case Event::EventType::MouseRelease:
		case Event::EventType::MouseScroll:
			for (int i = m_windows.size() - 1; i >= 0; --i)
			{
				m_windows[i]->onEvent(e);
				auto& m = dynamic_cast<MouseEvent&>(e);
				if (m_windows[i]->contains(m.getX(), m.getY()))
					e.handled = true;
			}
			break;
		default:
			glm::vec2 pos = { 0,0 };
			GUIElement* el = m_focused_element->getParent();
			while (el)
			{
				pos += el->pos;
				el = el->getParent();
			}
			this->pushPos(pos.x,pos.y);
			m_focused_element->onEvent(e);
			this->popPos(pos.x,pos.y);
			break;
		}
	}

	//element has removed itself from m_focused_element
	if(m_focused_element==nullptr)
	{
		int focusIndex = -1;
		for (int i = m_windows.size() - 1; i >= 0; --i)
		{
			switch (e.getEventType())
			{
			case Event::EventType::MouseMove:
			case Event::EventType::MouseDrag:
			case Event::EventType::MouseRelease:
			case Event::EventType::MouseScroll:
				m_windows[i]->onEvent(e);
				auto& m = dynamic_cast<MouseEvent&>(e);
				if (m_windows[i]->contains(m.getX(), m.getY()))
					e.handled = true;
				continue;
			}

			if (e.isInCategory(Event::EventCategory::Mouse))
			{
				auto& m = dynamic_cast<MouseEvent&>(e);
				if (m_windows[i]->contains(m.getX(), m.getY()))
				{
					m_windows[i]->onEvent(e);
					if (e.handled)
					{
						focusIndex = i;
						break;
					}
				}
			}
		}
		if (focusIndex != -1 && focusIndex != m_windows.size() - 1)
		{
			auto win = m_windows[focusIndex];
			m_windows.erase(m_windows.begin() + focusIndex);
			m_windows.push_back(win);
			for (int i = 0; i < m_windows.size() - 1; ++i) //anotate all other windows -> they had lost focus
				m_windows[i]->onMyEvent(MouseFocusLost(-10000, -10000));
		}

		if (!m_event_buffer.empty())
		{
			for (auto& event : m_event_buffer)
			{
				for (auto window : m_windows)
				{
					window->onEventBroadcast(*event);
				}
				delete event;
			}
			m_event_buffer.clear();
		}
	}

	closePendingWins();
}

void GUIContext::submitBroadcastEvent(Event& e)
{
	m_event_buffer.push_back(e.allocateCopy());
	closePendingWins();
}