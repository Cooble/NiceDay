﻿#include "FakeWindow.h"
#include "imgui.h"
#include "event/MouseEvent.h"
#include "graphics/Renderer.h"
#include "graphics/API/Texture.h"
#include "graphics/API/FrameBuffer.h"
#include "event/WindowEvent.h"
#include "App.h"

static Texture* templateTex = nullptr;
FakeWindow::FakeWindow(WindowTemplate* realWindow,int width, int height, const std::string& title, bool fullscreen)
:
	m_window(realWindow){
	m_data.width = width;
	m_data.height = height;
	m_data.title = title;
	m_data.fullscreen = fullscreen;

	//just to be sure
	if (height == 0)
		height = 100;
	if (width == 0)
		width = 100;
	
	m_fbo = FrameBuffer::create(FrameBufferInfo(width,height,TextureFormat::RGBA).special(FBAttachment::DEPTH_STENCIL));
	m_fbo->clear(BuffBit::COLOR|BuffBit::DEPTH, { 1,0,0,1 });
	Renderer::setDefaultFBO(m_fbo);

	templateTex = Texture::create(TextureInfo("res/images/gui_back.png"));
}

FakeWindow::~FakeWindow()
{
	//is it neccessary?
	//delete m_fbo;
}

glm::vec2 FakeWindow::getPos()
{
	return m_real_window_offset;
}

void FakeWindow::setSize(int width, int height)
{
	m_data.width = width;
	m_data.height = height;
}

void FakeWindow::setFullScreen(bool fullscreen)
{
	m_data.fullscreen = fullscreen;
}

void FakeWindow::setTitle(const char* title)
{
	m_data.title = title;
}

void FakeWindow::setCursorPolicy(WindowCursor state)
{
	m_window->setCursorPolicy(state);
}


void FakeWindow::setCursorPos(glm::vec2 pos)
{
	m_window->setCursorPos(pos + m_real_window_offset);
	
}

void FakeWindow::setClipboard(const char* c)
{
	m_window->setClipboard(c);
}

void FakeWindow::close()
{
}

FrameBuffer* FakeWindow::getFBO()
{
	return m_fbo;
}

void FakeWindow::swapBuffers()
{
	if (m_dirty_dim)
	{
		m_dirty_dim = false;
		m_fbo->resize(m_dim.x, m_dim.y);
		auto e = WindowResizeEvent(m_data.width, m_data.height);
		App::get().fireEvent(e);

		
	}
}

void FakeWindow::renderView()
{
	static bool opened = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	//ImGui::SetNextWindowDockID(dock_left_id, ImGuiCond_Once);
	ImGui::Begin("FakeWindow", &opened, ImGuiWindowFlags_NoDecoration);
	m_is_hovered = ImGui::IsWindowHovered();
	m_is_focused = ImGui::IsWindowFocused();
	
	
	ImGui::PopStyleVar(2);
	auto off = ImGui::GetWindowPos();
	
	m_real_window_offset = { off.x,off.y };
	m_relative_offset = m_real_window_offset - m_window->getPos();
	auto lastDim = m_dim;
	m_dim = { ImGui::GetWindowWidth(),ImGui::GetWindowHeight() };
	m_data.width = m_dim.x;
	m_data.height = m_dim.y;
	m_dirty_dim |= lastDim != m_dim;

	ImGui::Image((void*)m_fbo->getAttachmentID(0), { m_dim.x,m_dim.y }, { 0,1 }, { 1,0 });
	ImGui::End();
}

void FakeWindow::pollEvents()
{
	
}

bool FakeWindow::shouldClose()
{
	return false;
}


void FakeWindow::convertEvent(Event& e)
{
	if(e.getEventCategories() & Event::EventCategory::Mouse)
	{
		auto& ee = dynamic_cast<MouseEvent&>(e);
		auto pos = ee.getPos() - m_relative_offset;

		//check if mouse cursor is in the window and is normal
		if((pos.x<0||pos.y<0||pos.x>m_dim.x||pos.y>m_dim.y)&&m_window->getCursorPolicy()==CURSOR_ENABLED)
		{
			e.handled = true;
			return;
		}
		ee.setPos(pos.x,pos.y);
	}
	//ignore change of physical window
	if (e.getEventType() == Event::EventType::WindowResize)
		e.handled = true;
}

void* FakeWindow::getWindow() const
{
	return nullptr;
}

const char* FakeWindow::getClipboard() const
{
	return m_window->getClipboard();
}

//=========================INPUT====================================================

FakeInput::FakeInput(FakeWindow* window, Input* realInput)
	:m_window(window), m_input(realInput) {
}

bool FakeInput::isKeyPressed(int button)
{
	return m_input->isKeyPressed(button);
}

bool FakeInput::isKeyFreshlyPressed(int button)
{
	return m_input->isKeyFreshlyPressed(button);
}

bool FakeInput::isKeyFreshlyReleased(int button)
{
	return m_input->isKeyFreshlyReleased(button);
}

bool FakeInput::isMousePressed(int button)
{
	return m_input->isMousePressed(button);
}

glm::vec2 FakeInput::getMouseLocation()
{
	return m_input->getMouseLocation() - m_window->getOffset();
}
