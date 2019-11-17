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
	int cursorBlink = 0;
	int textClipOffset=0;
	TextMesh m_text_mesh;
	TextMesh m_cursorMesh;
	CursorProp prop;
	int cursorPos=0;
	char cursorChar = '|';
public:
	GUITextBox();

	virtual bool hasTotalFocus() const { return m_has_total_focus; }
	virtual void setValue(const std::string& val);
	inline auto& getValue() const { return m_value; }
	virtual void moveCursor(int delta);
	

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
	inline void setTextWidth(float width) { this->width = width + padding[GUI_LEFT] + padding[GUI_RIGHT]; }
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

	enum
	{
		invalid,
		up, down, left, right,
		left_up, left_down, right_up, right_down
	}m_resizeMode = invalid;
	float m_resize_x, m_resize_y,m_resize_w,m_resize_h;
public:
	bool moveable = true;
	bool resizable = true;
	GUIWindow();
	void onMyEvent(Event& e) override;
};

class GUIColumn :public GUIElement
{
public:
	GUIAlign child_alignment;
	GUIColumn(GUIAlign childAlignment= GUIAlign::CENTER);

	void repositionChildren() override;
};


class GUIRow :public GUIElement
{
public:
	GUIAlign child_alignment;
	GUIRow(GUIAlign childAlignment = GUIAlign::UP);
	void repositionChildren() override;

};

class GUIGrid :public GUIElement
{
public:
	GUIGrid();
	void repositionChildren() override;
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

class GUIBlank :public GUIElement
{

public:
	GUIBlank();
};
class GUIHorizontalSplit :public GUIElement
{
protected:
	bool m_is_up_main;
	
public:
	GUIHorizontalSplit(GUIElement* eUp, GUIElement* eDown,bool isUpMain=true);
	GUIHorizontalSplit(bool isUpMain=true);

	void repositionChildren() override;
	void onDimensionChange() override;

	inline GUIElement* getUpChild() { return getChildren()[0]; }
	inline GUIElement* getDownChild() { return getChildren()[1]; }
	
	void appendChild(GUIElement* element) override;
	void removeChild(int index) override;
};

class GUIVerticalSplit :public GUIElement
{
protected:
	bool m_is_left_main;

public:
	GUIVerticalSplit(GUIElement* eUp, GUIElement* eDown, bool isleftMain = true);
	GUIVerticalSplit(bool isLeftMain = true);

	void repositionChildren() override;
	void onDimensionChange() override;

	inline GUIElement* getRightChild() { return getChildren()[0]; }
	inline GUIElement* getLeftChild() { return getChildren()[1]; }

	void appendChild(GUIElement* element) override;
	void removeChild(int index) override;
};


