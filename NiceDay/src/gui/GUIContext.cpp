#include "ndpch.h"
#include "GUIContext.h"

void GUIContext::onEvent(Event& e)
{
	int focusIndex = -1;
	for (int i = m_windows.size() - 1; i >= 0; --i)
	{
		switch (e.getEventType())
		{
		case Event::EventType::MouseMove:
		case Event::EventType::MouseDrag:
		case Event::EventType::MouseRelease:
			m_windows[i]->onEvent(e);
			auto& m = dynamic_cast<MouseEvent&>(e);
			if (m_windows[i]->contains(m.getX(), m.getY()))
				e.handled = true;
			continue;
		}
		
		if (e.isInCategory(Event::EventCategory::Mouse)) {
			auto& m = dynamic_cast<MouseEvent&>(e);
			if (m_windows[i]->contains(m.getX(),m.getY()))
			{
				m_windows[i]->onEvent(e);
				if (e.handled) {
					focusIndex = i;
					break;
				}
			}
		}
	}
	if(focusIndex!=-1&&focusIndex!=m_windows.size()-1)
	{
		auto win = m_windows[focusIndex];
		m_windows.erase(m_windows.begin()+focusIndex);
		m_windows.push_back(win);
		for (int i = 0; i < m_windows.size() - 1; ++i)//anotate all other windows -> they had lost focus
			m_windows[i]->onMyEvent(MouseFocusLost(-10000,-10000));
	}

	if(!m_event_buffer.empty())
	{
		for (auto& event : m_event_buffer) {
			for (auto window : m_windows) {
				window->onEventBroadcast(*event);
			}
			delete event;
		}
		m_event_buffer.clear();
	}
}

void GUIContext::submitBroadcastEvent(Event& e)
{
	m_event_buffer.push_back(e.allocateCopy());
}
