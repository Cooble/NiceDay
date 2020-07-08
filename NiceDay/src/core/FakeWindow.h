#pragma once
#include "WindowTemplate.h"
#include "Input.h"

class FakeWindow:public WindowTemplate
{
private:
	//offset of fake window in screen space
	glm::vec2 m_real_window_offset;
	//offset of fake window in physical window space
	glm::vec2 m_relative_offset;
	glm::vec2 m_dim;
	bool m_dirty_dim=false;
	bool m_is_focused;
	bool m_is_hovered;
	
	WindowTemplate* m_window;
	FrameBuffer* m_fbo;
public:
	FakeWindow(WindowTemplate* realWindow,int width, int height, const std::string& title, bool fullscreen = false);
	~FakeWindow();


	glm::vec2 getPos() override;
	
	//offset of fake window in physical window space
	glm::vec2 getOffset() const { return m_relative_offset; }
	void setSize(int width, int height) override;
	void setFullScreen(bool fullscreen)override;
	void setTitle(const char* title)override;
	void setCursorPolicy(WindowCursor state)override;
	void setCursorPos(glm::vec2 pos)override;
	void setClipboard(const char* c)override;
	void close()override;


	FrameBuffer* getFBO() override;


	bool isFocused() const override { return m_is_focused; }
	bool isHovered() const override { return m_is_focused; }
	// called when everything has been already rendered
	// changes fbos

	void swapBuffers()override;
	
	//update its state from imgui window and fire some events
	void pollEvents()override;
	bool shouldClose()override;

	//render fbo to imgui window
	void renderView();

	//will convert physical window event to this window event or mark it as consumed if event should be ignored
	void convertEvent(Event& e);

	void* getWindow() const override;
	const char* getClipboard() const override;

};
class FakeInput :public Input
{
private:
	FakeWindow* m_window;
	Input* m_input;
public:
	FakeInput(FakeWindow* window, Input* realInput);
	bool isKeyPressed(int button) override;
	bool isKeyFreshlyPressed(int button)override;
	bool isKeyFreshlyReleased(int button)override;
	bool isMousePressed(int button)override;
	glm::vec2 getMouseLocation()override;
};
