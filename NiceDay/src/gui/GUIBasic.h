#pragma once
#include "GUIElement.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/Sprite.h"

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
class GUITextBox :public GUIElement
{
	std::string m_value;
	bool m_has_total_focus=false;
public:
	TextMesh m_text_mesh;
public:
	GUITextBox();

	virtual bool hasTotalFocus() const { return m_has_total_focus; }
	virtual void setValue(const std::string& val);
	inline auto& getValue() const { return m_value; }

	void onMyEvent(Event& e) override;
};
class GUIButton :public GUIElement
{
	std::string m_text;
public:
	ActionF on_pressed;
	TextMesh m_text_mesh;
public:
	GUIButton();
	virtual void setText(const std::string& val);
	inline auto& getText() const { return m_text; }
	void onMyEvent(Event& e) override;
};
class GUICheckBox :public GUIElement
{
	std::string m_textTrue;
	std::string m_textFalse;
	bool value;
public:
	ActionF on_pressed;
	TextMesh m_text_mesh;
	Sprite* spriteTrue;
	Sprite* spriteFalse;
public:
	GUICheckBox();
	virtual void setText(const std::string& trueText, const std::string& falseText);
	inline void setText(const std::string& text)
	{
		setText(text, text);
	}
	inline auto& getTrueText() const { return m_textTrue; }
	inline auto& getFalseText() const { return m_textFalse; }
	virtual void setValue(bool b);
	virtual bool getValue()const { return value; }
	void onMyEvent(Event& e) override;
};

class GUIImage :public GUIElement
{
public:
	Sprite* src;
	GUIImage();
	void setValue(Sprite* sprite);
	Sprite* getValue();
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

class GUISlider :public GUIElement
{
	float value=0;
	glm::vec2 m_draggedCursor;

public:
	int dividor=0;
	ActionF on_changed;
	GUISlider();
	void onMyEvent(Event& e) override;
	virtual void setValue(float v);
	virtual float getValue()const { return value; }
};
