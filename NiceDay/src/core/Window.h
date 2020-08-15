#pragma once
#include "ndpch.h"
#include "WindowTemplate.h"
#include "Input.h"

struct GLFWwindow;
class FrameBuffer;

class Window:public WindowTemplate
{
private:
	GLFWwindow* m_window;
	FrameBuffer* m_window_fbo;
public:
	Window(int width, int height, const std::string& title,bool fullscreen=false);
	~Window();

	glm::vec2 getPos() override;
	void setSize(int width, int height) override;
	void setFullScreen(bool fullscreen)override;
	void setTitle(const char* title)override;
	void setCursorPolicy(WindowCursor state)override;
	void setCursorPos(glm::vec2 pos)override;
	void setClipboard(const char* c)override;
	void close()override;
	void swapBuffers()override;
	void pollEvents()override;
	bool shouldClose()override;
	
	bool isFocused() const override { return m_data.focused; }
	bool isHovered() const override { return m_data.hovered; }

	void* getWindow() const override { return m_window; }
	FrameBuffer* getFBO() override { return m_window_fbo; }
	const char* getClipboard() const override;
};
class RealInput :public Input
{
private:
	std::vector<int8_t> m_keys;
	int8_t& getKey(int button);
	Window* m_window;
public:
	RealInput(Window* window);
	~RealInput() = default;
	void update() override;
	bool isKeyPressed(KeyCode button) override;
	bool isKeyFreshlyPressed(KeyCode button)override;
	bool isKeyFreshlyReleased(KeyCode button)override;
	bool isMousePressed(MouseCode button)override;
	glm::vec2 getMouseLocation()override;
};

