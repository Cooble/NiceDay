#pragma once
#include "GUIElement.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/Sprite.h"
#include "graphics/FontMaterial.h"

class GUIAnimation
{
private:
	std::vector<GUIElement> m_poses;
	GUIElement* m_element;
	float m_time;
	
	
public:
	//GUIAnimation(GUIElement* e);
	
};
// Basic
class GUIBlank :public GUIElement
{
public:
	GUIBlank();
};
class GUIText :public GUIElement
{
	std::string m_text;
public:
	FontMaterial* fontMaterial;
	TextMesh textMesh;
	float textScale = 1;
public:
	GUIText(FontMaterial* mat);
	bool packDimensions() override;
	virtual void setText(const std::string& val);
	inline auto& getText() const { return m_text; }
};
class GUIButton :public GUIElement
{
public:
	ActionF onPressed;
	ActionF onFocusGain;
	ActionF onFocusLost;
public:
	GUIButton();
	void onMyEvent(Event& e) override;
};
class GUIImage :public GUIElement
{
public:
	float scale=1;
	Sprite* image;
	GUIImage();
	bool packDimensions() override;
	void setImage(Sprite* sprite);

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
	bool isMoveable = true;
	bool isResizable = true;
	GUIWindow();
	void onMyEvent(Event& e) override;
};
class GUITextBox :public GUIElement
{
protected:
	std::string m_text;
	bool m_has_total_focus=false;
public:
	int cursorBlink = 0;
	int textClipOffset=0;
	TextMesh textMesh;
	TextMesh cursorMesh;
	FontMaterial* font;
	CursorProp prop;
	int cursorPos=0;
	char cursorChar = '|';
	ActionF onValueEntered;
public:
	GUITextBox();

	virtual bool hasTotalFocus() const { return m_has_total_focus; }
	virtual void setValue(const std::string& val);
	virtual void onValueModified();
	inline auto& getValue() const { return m_text; }
	virtual void moveCursor(int delta);

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

// Positional
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
	void onChildChange() override;
};

// Split
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
	GUIVerticalSplit(bool isLeftMain = true);

	void repositionChildren() override;
	void onDimensionChange() override;

	inline GUIElement* getRightChild() { return getChildren()[0]; }
	inline GUIElement* getLeftChild() { return getChildren()[1]; }

	void appendChild(GUIElement* element) override;
	void removeChild(int index) override;
};

// Slider
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
class GUIVSlider :public GUIElement
{
	float value = 0;
	glm::vec2 m_draggedCursor;
	float m_oldVal;

public:
	float sliderRatio=0.1f;
	float sliderHeight = 50;
	int dividor = 0;
	ActionF on_changed;
	GUIVSlider();
	void onMyEvent(Event& e) override;
	virtual void setValue(float v);
	virtual float getValue()const { return value; }
};
class GUIView :public GUIElement
{
public:
	GUIView();

	inline GUIElement* getInside() { return getChildren()[0]; }

	void appendChild(GUIElement* element) override;
	void removeChild(int index) override;
};

//Special
class GUITextButton :public GUIButton
{
public:
	GUITextButton(const std::string& text, FontMaterial* material);
	inline GUIText* getTextElement() { return static_cast<GUIText*>(getChildren()[0]); }
};
class GUIImageButton :public GUIButton
{
public:
	GUIImageButton(Sprite* image);
	inline GUIImage* getImageElement() { return static_cast<GUIImage*>(getChildren()[0]); }
};

class GUISpecialTextButton :public GUITextButton
{
public:
	float currentScale=1;
	float minScale=1;
	float maxScale=1.5;
	float animationSpeed=0.1f;
public:
	GUISpecialTextButton(const std::string& text, FontMaterial* material);
	void update() override;
};

GUIVerticalSplit* createGUISliderView(bool sliderOnLeft=true);



