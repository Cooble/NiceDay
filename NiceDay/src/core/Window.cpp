#include "ndpch.h"
#include "Window.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include "AppGlobals.h"
#include "event/MouseEvent.h"
#include "event/MessageEvent.h"
#include "event/KeyEvent.h"
#include "event/WindowEvent.h"
#include "platform/OpenGL/GLRenderer.h"
#include "graphics/API/FrameBuffer.h"
#include "graphics/Renderer.h"
#include "stb_image.h"
#include "files/FUtil.h"

using namespace nd;

//suggests driver to use nvidia and not integrated
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
}

namespace nd {
static void blankFun(Event& e)
{
}

static bool is_glfw_initialized = false;


Window::Window(int width, int height, const std::string& title, bool fullscreen) :
	m_window(nullptr)
{
	m_data.width = width;
	m_data.height = height;
	m_data.title = title;
	m_data.eventCallback = blankFun;

	if (!is_glfw_initialized)
	{
		is_glfw_initialized = true;
		glfwInit();
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

	m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

	// load icons
	// --------------------
	{
		setIcon(ND_RESLOC("res/engine/images/nd_icon2.png"));
	}

	if (m_window == NULL)
	{
		ND_ERROR("Failed to create GLFW window");
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(0); //vsync off
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ND_ERROR("Failed to initialize GLAD");
		return;
	}

	glfwSetWindowUserPointer(m_window, &m_data);

	//window events
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		GLCall(glViewport(0, 0, width, height));
		d.width = width;
		d.height = height;
		WindowResizeEvent e(width, height);
		d.eventCallback(e);
	});
	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent e;
		d.eventCallback(e);
	});
	glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		d.focused = focused;
	});


	//key events
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			KeyPressEvent e(key, mods, action == GLFW_REPEAT);
			d.eventCallback(e);
		}
		else if (action == GLFW_RELEASE)
		{
			KeyReleaseEvent e(key, mods);
			d.eventCallback(e);
		}
	});
	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int key)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		KeyTypeEvent e(key, 0);
		d.eventCallback(e);
	});
	//mouse events
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (action == GLFW_PRESS)
		{
			MousePressEvent e(x, y, mods, button);
			d.eventCallback(e);
		}
		else if (action == GLFW_RELEASE)
		{
			MouseReleaseEvent e(x, y, mods, button);
			d.eventCallback(e);
		}
	});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		MouseMoveEvent e(x, y);
		d.eventCallback(e);
	});
	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xx, double yy)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		MouseScrollEvent e(x, y, 0, xx, yy);
		d.eventCallback(e);
	});
	glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, int entered)
	{
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		d.hovered = entered;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		MouseEnteredEvent e(x, y, (entered));
		d.eventCallback(e);
	});
	glfwSetDropCallback(m_window, [](GLFWwindow* window, int count, const char** paths)
	{
		//warning paths are valid until this function returns
		WindowData& d = *(WindowData*)glfwGetWindowUserPointer(window);
		DropFilesEvent e(count, paths);
		d.eventCallback(e);
	});


	//GLint i = 0;
	//glad_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	const GLubyte* vendor = glad_glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glad_glGetString(GL_RENDERER); // Returns a hint to the model
	const GLubyte* gl = glad_glGetString(GL_VERSION); // Returns a hint to the model
	const GLubyte* glsl = glad_glGetString(GL_SHADING_LANGUAGE_VERSION); // Returns a hint to the model
	uint64_t maxVertAttrib;
	glad_glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (int*)&maxVertAttrib); // Returns a hint to the model

	ND_TRACE("Graphics card info:");
	ND_TRACE((char*)vendor);
	ND_TRACE((char*)renderer);
	ND_TRACE("GL version: {}", gl);
	ND_TRACE("GLSL version: {}", glsl);
	ND_TRACE("MaxVertexAttribs: {}", maxVertAttrib);


	if (fullscreen)
		setFullScreen(true);

	ND_TRACE("Window created with dimensions: [{}, {}], fullscreen: {}", width, height, fullscreen);
	m_raw_mouse_enabled = glfwRawMouseMotionSupported();
	if (!m_raw_mouse_enabled)
	{
		ND_WARN("Window does not support raw mouse input!");
	}
	//set the default fbo to be this window target
	m_window_fbo = FrameBuffer::create(FrameBufferInfo().windowTarget());
	Renderer::setDefaultFBO(m_window_fbo); //default fbo is in case of opengl "empty" => directly rendered to screen
}


Window::~Window()
{
	if (!m_destroyed)
		glfwDestroyWindow(m_window);
	glfwTerminate();
}

glm::vec2 Window::getPos()
{
	int x, y;
	glfwGetWindowPos(m_window, &x, &y);
	return {x, y};
}

void Window::setSize(int width, int height)
{
	m_data.width = width;
	m_data.height = height;
	glfwSetWindowSize(m_window, width, height);
}

void Window::setFullScreen(bool fullscreen)
{
	if (m_data.fullscreen == fullscreen)
		return;
	if (fullscreen)
	{
		glfwGetWindowPos(m_window, &m_data.lastX, &m_data.lastY);
		glfwGetWindowSize(m_window, &m_data.lastWidth, &m_data.lastHeight);
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, 60);
	}
	else
	{
		glfwSetWindowMonitor(m_window, nullptr, m_data.lastX, m_data.lastY, m_data.lastWidth, m_data.lastHeight, 60);
		m_data.width = m_data.lastWidth;
		m_data.height = m_data.lastHeight;
		m_data.x = m_data.lastX;
		m_data.y = m_data.lastY;
	}
	m_data.fullscreen = fullscreen;
}

void Window::setTitle(const char* title)
{
	m_data.title = title;
	glfwSetWindowTitle(m_window, title);
}

void Window::setCursorPolicy(WindowCursor state)
{
	auto s = GLFW_CURSOR_NORMAL;
	if (state == CURSOR_DISABLED)
		s = GLFW_CURSOR_DISABLED;
	else if (state == CURSOR_HIDDEN)
		s = GLFW_CURSOR_HIDDEN;
	glfwSetInputMode(m_window, GLFW_CURSOR, s);
	if (m_raw_mouse_enabled)
		glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, s == GLFW_CURSOR_DISABLED);
	m_cursor_policy = state;
}

void Window::setCursorPos(glm::vec2 pos)
{
	glfwSetCursorPos(m_window, pos.x, pos.y);
}

void Window::setClipboard(const char* c)
{
	glfwSetClipboardString(NULL, c);
}

void Window::close()
{
	m_destroyed = true;
	glfwDestroyWindow(m_window);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(m_window);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::pollEvents()
{
	glfwPollEvents();
	m_iconified = (bool)glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
}

bool Window::shouldClose()
{
	return m_window != nullptr && glfwWindowShouldClose(m_window);
}

void Window::setIcon(std::string_view image_path)
{
	stbi_set_flip_vertically_on_load(false);

	int w, h, mbbpp;
	GLFWimage icons[1];
	if (FUtil::exists(image_path))
	{
		icons->pixels = stbi_load(std::string(image_path).c_str(), &w, &h, &mbbpp, 4);
		icons->width = w;
		icons->height = h;

		glfwSetWindowIcon(m_window, 1, icons);
		stbi_image_free(icons[0].pixels);
	}
}

void Window::setClipboard(const wchar_t* c)
{
	ASSERT(false, "cliboard from wchar not implemented yet");
}

const char* Window::getClipboard() const
{
	return glfwGetClipboardString(NULL);
}

//==============================INPUT==============================================
int8_t& RealInput::getKey(int button)
{
	if (button >= m_keys.size())
	{
		auto lastSize = m_keys.size();
		m_keys.resize(button + 1);
		for (int i = 0; i < (button + 1) - lastSize; ++i) //clear all new keys
			m_keys[i + lastSize] = 0;
		m_keys[button] = 0;
	}
	return m_keys[button];
}

int8_t& RealInput::getMouseKey(int button)
{
	if (button >= m_mouse_keys.size())
	{
		auto lastSize = m_mouse_keys.size();
		m_mouse_keys.resize(button + 1);
		for (int i = 0; i < (button + 1) - lastSize; ++i) //clear all new keys
			m_mouse_keys[i + lastSize] = 0;
		m_mouse_keys[button] = 0;
	}
	return m_mouse_keys[button];
}

RealInput::RealInput(Window* window)
	: m_window(window)
{
}

void RealInput::update()
{
	for (int i = 0; i < m_keys.size(); ++i)
	{
		auto& k = getKey(i);
		if (isKeyPressed((KeyCode)i))
		{
			if (k < 127)
				++k;
		}
		else if (k > 0)
			k = -1;
		else k = 0;
	}
	for (int i = 0; i < m_mouse_keys.size(); ++i)
	{
		auto& k = getMouseKey(i);
		if (isMousePressed((MouseCode)i))
		{
			if (k < 127)
				++k;
		}
		else if (k > 0)
			k = -1;
		else k = 0;
	}
	if (isMouseFreshlyPressed(MouseCode::LEFT) || isMouseFreshlyPressed(MouseCode::MIDDLE) || isMouseFreshlyPressed(
		MouseCode::RIGHT))
		m_drag_offset = getMouseLocation();
}

bool RealInput::isKeyPressed(KeyCode button)
{
	getKey(button); //save this button to vector
	auto w = (GLFWwindow*)m_window->getWindow();
	auto state = glfwGetKey(w, button);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool RealInput::isKeyFreshlyPressed(KeyCode button)
{
	return getKey(button) == 1;
}

bool RealInput::isKeyFreshlyReleased(KeyCode button)
{
	return getKey(button) == -1;
}

bool RealInput::isMousePressed(MouseCode button)
{
	getMouseKey((int)button); //save this button to vector

	auto w = (GLFWwindow*)m_window->getWindow();
	auto state = glfwGetMouseButton(w, (int)button);
	return state == GLFW_PRESS;
}

bool RealInput::isMouseFreshlyPressed(MouseCode button)
{
	return getMouseKey((int)button) == 1;
}

bool RealInput::isMouseFreshlyReleased(MouseCode button)
{
	return getMouseKey((int)button) == -1;
}

glm::vec2 RealInput::getDragging()
{
	return getMouseLocation() - m_drag_offset;
}

glm::vec2 RealInput::getMouseLocation()
{
	auto w = (GLFWwindow*)m_window->getWindow();

	double x, y;
	glfwGetCursorPos(w, &x, &y);
	return {x, y};
}
}
