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
	Window(int width, int height, const std::string& title,bool fullscreen=false);
	~Window();

	void setSize(int width, int height);
	void setFullScreen(bool fullscreen);
	void setTitle(const char* title);
	void close();
	void swapBuffers();
	void pollEvents();
	inline void setEventCallback(const EventCallbackFn& func) { m_data.eventCallback = func; }
	inline glm::vec2 getDimensions() const { return glm::vec2(getWidth(), getHeight()); }
	inline bool shouldClose();


	inline GLFWwindow* getWindow() const { return m_window; }
	inline unsigned int getWidth() const { return m_data.width; }
	inline unsigned int getHeight() const { return m_data.height; }
	inline const char* getTitle() const { return m_data.title.c_str(); }
	inline bool isFullscreen() const { return m_data.fullscreen; }

private:
	//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

