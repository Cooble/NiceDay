#pragma once
#include "GUIElement.h"
#include "graphics/font/TextBuilder.h"

class GUILabel :public GUIElement
{
	std::string m_value;
public:
	TextMesh m_text_mesh;
	
public:
	GUILabel();

	virtual void setValue(const std::string& val);
	inline auto& getValue() const { return m_value; }
};
class GUIButton :public GUIElement
{
	std::string m_value;
public:
	ActionF on_pressed;
	TextMesh m_text_mesh;
public:
	GUIButton();
	virtual void setValue(const std::string& val);
	inline auto& getValue() const { return m_value; }
	void onMyEvent(Event& e) override;
};
class GUIWindow:public GUIElement
{
private:
	glm::vec2 m_draggedCursor;
public:
	GUIWindow();
	void onMyEvent(Event& e) override;
};
class GUIColumn :public GUIElement
{
public:
	GUIAlign alignment=GUIAlign::CENTER;
	float space = 5;
	GUIColumn();
	void appendChild(GUIElement* element) override;
};

class GUIRow :public GUIElement
{
public:
	GUIAlign alignment = GUIAlign::LEFT;
	float space = 5;
	GUIRow();
	void appendChild(GUIElement* element) override;

};
class GUIGrid :public GUIElement
{
	float getLowestY();
public:
	GUIAlign alignment = GUIAlign::LEFT;
	float space = 5;
	GUIGrid();
	void appendChild(GUIElement* element) override;

};
