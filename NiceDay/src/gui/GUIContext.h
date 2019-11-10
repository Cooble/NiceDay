#pragma once
#include "GUIBasic.h"

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
	std::vector<GUIWindow*> m_windows;
	std::vector<Event*> m_event_buffer;
	
	glm::vec2 currentStackPos={0,0};
	
public:
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
		return m_windows[m_windows.size()-1];
	}
	inline auto& getWindows() const { return m_windows; }
	inline void pushPos(float x,float y)
	{
		currentStackPos.x += x;
		currentStackPos.y += y;
	}
	inline void popPos(float x, float y)
	{
		currentStackPos.x -= x;
		currentStackPos.y -= y;
	}
	void onEvent(Event& e);
	void submitBroadcastEvent(Event& e);
	inline const glm::vec2& getStackPos()const { return currentStackPos; }
};


