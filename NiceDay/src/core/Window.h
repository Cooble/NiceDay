#pragma once
#include "ndpch.h"
#include "WindowTemplate.h"
#include "Input.h"

struct GLFWwindow;

namespace nd {
class FrameBuffer;

class Window : public WindowTemplate
{
private:
	GLFWwindow* m_window;
	FrameBuffer* m_window_fbo;
public:
	Window(int width, int height, const std::string& title, bool fullscreen = false);
	~Window() override;

	glm::vec2 getPos() override;
	void setSize(int width, int height) override;
	void setFullScreen(bool fullscreen) override;
	void setTitle(const char* title) override;
	void setCursorPolicy(WindowCursor state) override;
	void setCursorPos(glm::vec2 pos) override;
	void setClipboard(const char* c) override;
	void close() override;
	void swapBuffers() override;
	void pollEvents() override;
	bool shouldClose() override;
	void setIcon(std::string_view image_path) override;
	void setClipboard(const wchar_t* c) override;

	bool isFocused() const override { return m_data.focused; }
	bool isHovered() const override { return m_data.hovered; }

	void* getWindow() const override { return m_window; }
	FrameBuffer* getFBO() override { return m_window_fbo; }
	const char* getClipboard() const override;
};

class RealInput : public Input
{
private:
	std::vector<int8_t> m_keys;
	std::vector<int8_t> m_mouse_keys;
	int8_t& getKey(int button);
	int8_t& getMouseKey(int button);
	Window* m_window;
	glm::vec2 m_drag_offset = glm::vec2(0.f);
public:
	RealInput(Window* window);
	~RealInput() = default;
	void update() override;
	bool isKeyPressed(KeyCode button) override;
	bool isKeyFreshlyPressed(KeyCode button) override;
	bool isKeyFreshlyReleased(KeyCode button) override;
	bool isMousePressed(MouseCode button) override;
	bool isMouseFreshlyPressed(MouseCode button) override;
	bool isMouseFreshlyReleased(MouseCode button) override;
	glm::vec2 getDragging() override;

	glm::vec2 getMouseLocation() override;
};
}
