#include "ndpch.h"
#include "Window.h"

#include "event/MouseEvent.h"
#include "event/KeyEvent.h"
#include "event/WindowEvent.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/OpenGLRenderer.h"

static void blankFun(Event& e) {}
static bool is_glfw_initialized = false;

Window::Window(int width, int height, const char* title) :
	m_window(nullptr)
{
	m_data.width = width;
	m_data.height = height;
	m_data.title = title;
	m_data.eventCallback = blankFun;

	if (!is_glfw_initialized) {
		is_glfw_initialized = true;
		glfwInit();
	}

	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
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
	glfwSwapInterval(1); // Enable vsync
	glfwMakeContextCurrent(m_window);
	// glad: load all OpenGL function pointers
		// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ND_ERROR("Failed to initialize GLAD");
		return;
	}
	
	glfwSetWindowUserPointer(m_window, &m_data);

	//window events
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width, int height) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		GLCall(glViewport(0, 0, width, height));
		d.width = width;
		d.height = height;
		WindowResizeEvent e(width, height);
		d.eventCallback(e);
	});
	glfwSetWindowCloseCallback(m_window, [](GLFWwindow * window) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent e;
		d.eventCallback(e);
	});


	//key events
	glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scancode, int action, int mods) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS) {
			KeyPressEvent e(key, mods);
			d.eventCallback(e);
		}
		else if (action == GLFW_RELEASE) {
			KeyReleaseEvent e(key);
			d.eventCallback(e);
		}
	});
	glfwSetCharCallback(m_window, [](GLFWwindow * window, unsigned int key) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		KeyTypeEvent e(key);
		d.eventCallback(e);
	});
	//mouse events
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow * window, int button, int action, int mods) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (action == GLFW_PRESS) {
			MousePressEvent e(x, y, button);
			d.eventCallback(e);
		}
		else if (action == GLFW_RELEASE) {
			MouseReleaseEvent e(x, y, button);
			d.eventCallback(e);
		}

	});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow * window, double x, double y) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		MouseMoveEvent e(x, y);
		d.eventCallback(e);
	});
	glfwSetScrollCallback(m_window, [](GLFWwindow * window, double xx, double yy) {
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		MouseScrollEvent e(x, y, xx, yy);
		d.eventCallback(e);
	});

	//GLint i = 0;
	//glad_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	const GLubyte* vendor = glad_glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glad_glGetString(GL_RENDERER); // Returns a hint to the model
	const GLubyte* gl = glad_glGetString(GL_VERSION); // Returns a hint to the model
	const GLubyte* glsl = glad_glGetString(GL_SHADING_LANGUAGE_VERSION); // Returns a hint to the model
	ND_INFO("Graphics card info:");
	ND_INFO((char*)vendor);
	ND_INFO((char*)renderer);
	ND_INFO("GL version: {}", gl);
	ND_INFO("GLSL version: {}", glsl);
}


Window::~Window()
{
	if (!m_destroyed)
		glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::setSize(int width, int height)
{
	m_data.width = width;
	m_data.height = height;
	glfwSetWindowSize(m_window, width, height);
}

void Window::setTitle(const char * title)
{
	m_data.title = title;
	glfwSetWindowTitle(m_window, title);
}

void Window::close()
{
	m_destroyed = true;
	glfwDestroyWindow(m_window);
	
}

void Window::update()
{
	glfwPollEvents();
	glfwSwapBuffers(m_window);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

