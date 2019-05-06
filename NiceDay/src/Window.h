#pragma once
#include "ndpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "event/Event.h"


class Window
{
private:
	bool m_destroyed = false;
	GLFWwindow* m_window;
	using EventCallbackFn = std::function<void(Event&)>;

	struct WindowData
	{
		unsigned int width, height;
		std::string title;
		EventCallbackFn eventCallback;
	};
	WindowData m_data;


public:
	Window(int width, int height, const char* title);
	~Window();

	void setSize(int width, int height);
	void setTitle(const char* title);
	void close();
	void update();
	inline void setEventCallback(const EventCallbackFn& func) { m_data.eventCallback = func; };
	inline bool shouldClose() { return m_window != nullptr && glfwWindowShouldClose(m_window); }
	

	inline GLFWwindow* getWindow() const { return m_window; }
	inline unsigned int getWidth() const { return m_data.width; }
	inline unsigned int getHeight() const { return m_data.height; }
	inline const char* getTitle() const { return m_data.title.c_str(); }

private:
	//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

