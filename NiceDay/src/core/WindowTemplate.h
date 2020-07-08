#pragma once
#include "ndpch.h"
#include "event/Event.h"

class FrameBuffer;
class WindowTemplate
{
public:
	using EventCallbackFn = std::function<void(Event&)>;
protected:
	bool m_raw_mouse_enabled;
	bool m_destroyed = false;
	bool m_iconified = false;
	struct WindowData
	{
		int width, height;
		int lastWidth, lastHeight;
		int x, y;
		int lastX, lastY;
		bool fullscreen = false;
		std::string title;
		EventCallbackFn eventCallback;
		bool focused;
		bool hovered;
	} m_data;

public:
	enum WindowCursor
	{
		CURSOR_DISABLED = 0,
		CURSOR_ENABLED = 1,
		CURSOR_HIDDEN = 2
	};
protected:
	WindowCursor m_cursor_policy=CURSOR_ENABLED;
public:
	virtual ~WindowTemplate() = default;

	//position of window on screen
	virtual glm::vec2 getPos() = 0;
	virtual void setSize(int width, int height)=0;
	virtual void setFullScreen(bool fullscreen)=0;
	void toggleFullscreen() { setFullScreen(!m_data.fullscreen); }
	virtual void setTitle(const char* title) = 0;
	virtual void setCursorPolicy(WindowCursor state) = 0;
	virtual void setCursorPos(glm::vec2 pos) = 0;
	virtual void setClipboard(const char* c) = 0;
	virtual void close() = 0;
	virtual void swapBuffers() {}
	virtual void pollEvents() {}
	virtual void setEventCallback(const EventCallbackFn& func) { m_data.eventCallback = func; }
	glm::vec2 getDimensions() const { return glm::vec2(m_data.width,m_data.height); }
	virtual bool shouldClose() = 0;
	virtual FrameBuffer* getFBO()=0;

	virtual bool isFocused() const = 0;
	virtual bool isHovered() const = 0;
 


	virtual void* getWindow() const { return nullptr; }
	unsigned int getWidth() const { return m_data.width; }
	unsigned int getHeight() const { return m_data.height; }
	const char* getTitle() const { return m_data.title.c_str(); }
	bool isFullscreen() const { return m_data.fullscreen; }
	//is window minimized?
	bool isIconified() const { return m_iconified; }

	virtual const char* getClipboard() const = 0;
	WindowCursor getCursorPolicy() const { return m_cursor_policy; }
};

