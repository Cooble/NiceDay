#pragma once
#include "ndpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Window
{
private:
	GLFWwindow* m_window;
	
	struct WindowData
	{
		unsigned int width, height;
		std::string title;
		void(*eventCallback)();
	};
	WindowData m_data;


public:
	Window(int width, int height, const char* title);
	~Window();

	void setSize(int width, int height);
	void setTitle(const char* title);
	void setEventCallBack();
	inline bool shouldClose() { return m_window != nullptr && glfwWindowShouldClose(m_window); }

	inline int getWidth() const { return m_data.width; }
	inline int getHeight() const { return m_data.height; }
	inline const char& getTitle() const { return *m_data.title.c_str(); }
private:
	//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

