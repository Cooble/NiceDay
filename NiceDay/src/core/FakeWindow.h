#pragma once
#include "WindowTemplate.h"
#include "Input.h"

namespace nd {
struct NavigationBar
{
	bool moveActive = false;
	bool lockActive = false;
	bool rotationActive = false;
	bool scrollActive = false;
	bool freshPress = false;
	bool freshRelease = false;
	glm::vec2 drag;

	constexpr bool isAnyActive() { return lockActive | rotationActive | moveActive | scrollActive; }
};

class FakeWindow : public WindowTemplate
{
private:
	//offset of fake window in screen space
	glm::vec2 m_real_window_offset;
	//offset of fake window in physical window space
	glm::vec2 m_relative_offset;
	glm::vec2 m_dim;
	bool m_dirty_dim = false;
	bool m_is_focused;
	bool m_is_hovered;
	WindowTemplate* m_window;
	FrameBuffer* m_fbo;
	NavigationBar m_nav_bar;

	void drawNavBar();
public:
	bool m_enableNavigationBar = true;
	FakeWindow(WindowTemplate* realWindow, int width, int height, const std::string& title, bool fullscreen = false);
	~FakeWindow() override;


	glm::vec2 getPos() override;

	//offset of fake window in physical window space
	glm::vec2 getOffset() const { return m_relative_offset; }
	void setSize(int width, int height) override;
	void setFullScreen(bool fullscreen) override;
	void setTitle(const char* title) override;
	void setCursorPolicy(WindowCursor state) override;
	void setCursorPos(glm::vec2 pos) override;
	void setClipboard(const char* c) override;
	void setClipboard(const wchar_t* c) override;
	void close() override;
	void setIcon(std::string_view image_path) override;


	FrameBuffer* getFBO() override;


	bool isFocused() const override { return m_is_focused; }
	bool isHovered() const override { return m_is_focused; }
	// called when everything has been already rendered
	// changes fbos

	void swapBuffers() override;

	//update its state from imgui window and fire some events
	void pollEvents() override;
	bool shouldClose() override;

	//render fbo to imgui window
	void renderView();

	const NavigationBar& getNavigationBar() const { return m_nav_bar; }

	//will convert physical window event to this window event or mark it as consumed if event should be ignored
	void convertEvent(Event& e);

	void* getWindow() const override;
	const char* getClipboard() const override;
};

class FakeInput : public Input
{
private:
	FakeWindow* m_window;
	Input* m_input;
public:
	FakeInput(FakeWindow* window, Input* realInput);
	bool isKeyPressed(KeyCode button) override;
	bool isKeyFreshlyPressed(KeyCode button) override;
	bool isKeyFreshlyReleased(KeyCode button) override;
	bool isMousePressed(MouseCode button) override;
	glm::vec2 getMouseLocation() override;
	bool isMouseFreshlyReleased(MouseCode button) override;
	bool isMouseFreshlyPressed(MouseCode button) override;
	glm::vec2 getDragging() override;
};
}
