#include "ndpch.h"
#include "Window.h"
Window::Window(int width,int height,const char* title):
	m_window(nullptr)
{
	m_data.width = width;
	m_data.height = height;
	m_data.title = title;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (m_window == NULL)
	{
		ND_ERROR("Failed to create GLFW window");
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, &m_data);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width, int height) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		//throw eventos
		//todo refresh buffer size when resized
	});
	

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ND_ERROR("Failed to initialize GLAD");
		return;
	}

}


Window::~Window()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::setSize(int width, int height)
{
	m_data.width = width;
	m_data.height = height;
	glfwSetWindowSize(m_window, width,height);
}

void Window::setTitle(const char * title)
{
	m_data.title = title;
	glfwSetWindowTitle(m_window,title);
}
