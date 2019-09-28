#pragma once
#include "ndpch.h"
#include "event/Event.h"

struct GLFWwindow;

class Window
{
private:
	bool m_destroyed = false;
	GLFWwindow* m_window;
	using EventCallbackFn = std::function<void(Event&)>;

	struct WindowData
	{
		int width, height;
		int lastWidth, lastHeight;
		int x, y;
		int lastX, lastY;
		bool fullscreen=false;
		std::string title;
		EventCallbackFn eventCallback;
	};
	WindowData m_data;


public:
	Window(int width, int height, const std::string& title);
	~Window();

	void setSize(int width, int height);
	void setFullScreen(bool fullscreen);
	void setTitle(const char* title);
	void close();
	void update();
	inline void setEventCallback(const EventCallbackFn& func) { m_data.eventCallback = func; };
	inline bool shouldClose();


	inline GLFWwindow* getWindow() const { return m_window; }
	inline unsigned int getWidth() const { return m_data.width; }
	inline unsigned int getHeight() const { return m_data.height; }
	inline const char* getTitle() const { return m_data.title.c_str(); }
	inline bool isFullscreen() const { return m_data.fullscreen; }

private:
	//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

