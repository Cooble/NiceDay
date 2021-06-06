﻿#pragma once
#include "GUIBasic.h"

namespace nd {

typedef int GUIContextID;

class GUIContext

{
private:
	inline static std::vector<GUIContext> s_contexts;

	inline static GUIContextID s_currentlyBoundInstance;
public:
	static GUIContextID create()
	{
		s_contexts.emplace_back();
		setContext(s_contexts.size() - 1);
		return s_contexts.size() - 1;
	}

	static void destroy(GUIContextID id)
	{
		ASSERT(id < s_contexts.size(), "Invalid id");
		s_contexts[id].destroy();
	}

	static void setContext(GUIContextID id)
	{
		ASSERT(id < s_contexts.size(), "Invalid id");
		ASSERT(!s_contexts[id].isDestroyed(), "Context has already been destroyed");

		s_currentlyBoundInstance = id;
	}

	static GUIContext& get()
	{
		return s_contexts[s_currentlyBoundInstance];
	}

private:
	bool m_destroyed = false;
	GUIElement* m_focused_element;
	std::vector<GUIWindow*> m_windows;
	std::vector<Event*> m_event_buffer;
	std::vector<GEID> m_toclose_wins;


	glm::vec2 currentStackPos = {0, 0};

public:
	static bool& isAnyItemActive()
	{
		static bool hehe = false;
		return hehe;
	}

	// when element is submitted here, all events (except broadcasts) will go directly to that element onEvent(),
	// no other elements like windows are called (except broadcasts), the element has full control over whole context
	// to exit this mode, one needs to setFocusedElement(nullptr)
	// when element calls setFocusElement(nullptr) in some event, the event will be delivered to all elements later on
	// you can check if focus on focused element was lost and if positive -> set nullptr to allow other elements to react
	void setFocusedElement(GUIElement* e);
	inline GUIElement* getFocusedElement() { return m_focused_element; }

	inline void destroy()
	{
		m_destroyed = true;
		m_windows.clear();
	}

	inline bool isDestroyed() const { return m_destroyed; }
	inline auto& getWindows() { return m_windows; }

	inline GUIWindow* getFocusedWindow()
	{
		if (m_windows.empty())
			return nullptr;
		return m_windows[m_windows.size() - 1];
	}

	inline auto& getWindows() const { return m_windows; }

	inline void pushPos(float x, float y)
	{
		currentStackPos.x += x;
		currentStackPos.y += y;
	}

	inline void popPos(float x, float y)
	{
		currentStackPos.x -= x;
		currentStackPos.y -= y;
	}

	/***
	 * Removes all windows from context
	 * calls destructors
	 */
	inline void destroyWindows()
	{
		for (auto win : m_windows)
			delete win;
		m_windows.clear();
	}

	/***
	 * Removes all windows from context
	 * Doesn't call destructors
	 */
	inline void closeWindows()
	{
		m_windows.clear();
	}

	/***
	 * Removes window from context
	 * calls destructors
	 */
	inline void destroyWindow(GEID winid)
	{
		for (int i = 0; i < m_windows.size(); ++i)
		{
			auto win = m_windows[i];
			if (win->serialID == winid)
			{
				m_windows.erase(m_windows.begin() + i);
				delete win;
				return;
			}
		}
	}

	/***
	 * Removes window from context
	 * Doesn't call destructors
	 */
	inline void closeWindow(GEID winid)
	{
		for (int i = 0; i < m_windows.size(); ++i)
		{
			auto win = m_windows[i];
			if (win->serialID == winid)
			{
				m_windows.erase(m_windows.begin() + i);
				return;
			}
		}
	}

	inline void destroyWindowEventually(GEID win) { m_toclose_wins.push_back(win); }

	inline void destroyPendingWins()
	{
		for (auto toclose_win : m_toclose_wins)
			destroyWindow(toclose_win);
		m_toclose_wins.clear();
	}

	void openWindow(GUIWindow* win);

	void onUpdate();
	void onEvent(Event& e);
	void submitBroadcastEvent(Event& e);
	inline const glm::vec2& getStackPos() const { return currentStackPos; }
};
}
