#pragma once
#include "GUIElement.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/Sprite.h"

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
	GUIText();
	GUIText(FontMaterial* m);
	bool packDimensions() override;
	virtual void setText(const std::string& val);
	auto& getText() const { return m_text; }
};
class GUIButton :public GUIElement
{
public:
	ActionF onPressed;
	ActionF onFocusGain;
	ActionF onFocusLost;
	float soundVolume = 1;
	std::string soundClick;
	std::string soundFocus;
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
	FontMaterial* fontMaterial;
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
	bool packDimensions() override;

	void onMyEvent(Event& e) override;
	void setGainFocus(bool cond);
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
	void setText(const std::string& text)
	{
		setText(text, text);
	}
	auto& getTrueText() const { return m_textTrue; }
	auto& getFalseText() const { return m_textFalse; }
	virtual void setValue(bool b);
	virtual bool getValue()const { return value; }
	void onMyEvent(Event& e) override;
};

// Positional
class GUIColumn :public GUIElement
{
public:
	GUIAlign child_alignment;
	GUIColumn(GUIAlign childAlignment=GUIAlign::CENTER);

	void repositionChildren() override;
};
class GUIRow :public GUIElement
{
public:
	GUIAlign child_alignment;
	GUIRow(GUIAlign childAlignment = GUIAlign::CENTER);
	void repositionChildren() override;

};
class GUIGrid :public GUIElement
{
public:
	GUIGrid();
	void repositionChildren() override;
	void onChildChange() override;
};

// Split up and down
class GUIHorizontalSplit :public GUIElement
{
protected:
	bool m_is_up_main=true;
	
public:
	GUIHorizontalSplit(GUIElement* eUp, GUIElement* eDown,bool isUpMain=true);
	GUIHorizontalSplit();
	void setPrimaryUp(bool isUpMain);
	void repositionChildren() override;
	void onDimensionChange() override;

	GUIElement* getUp() { return getChildren()[0]; }
	GUIElement* getDown() { return getChildren()[1]; }
	
	void appendChild(GUIElement* element) override;
	void destroyChild(int index) override;
};
class GUIVerticalSplit :public GUIElement
{
protected:
	bool m_is_left_main=true;

public:
	GUIVerticalSplit(bool isLeftMain);
	GUIVerticalSplit();
	
	void setPrimaryLeft(bool left);
	constexpr bool isPrimaryLeft() const { return m_is_left_main; }

	void repositionChildren() override;
	void onDimensionChange() override;

	GUIElement* getLeft() { return getChildren()[0]; }
	GUIElement* getRight() { return getChildren()[1]; }

	void appendChild(GUIElement* element) override;
	void destroyChild(int index) override;
};

// Slider
class GUISlider :public GUIElement
{
	// value between 0 and 1
	float m_normalValue=0;
	// value between minVal and maxVal
	float m_value=0;
	glm::vec2 m_draggedCursor;

public:
	// how big the step is
	// 0 means smooth progress (any value between minV and maxV is possible) 
	float step = 0;
	float minValue=0, maxValue=1;
	// true means up = minValue, down =maxVal
	bool invertedVal=false;
	
	// called when value is changed
	ActionF on_changed;
	GUISlider();
	void onMyEvent(Event& e) override;
	virtual void setValue(float v);
	virtual float getValue() const { return m_value; }
	void prepare(float minValue, float maxValue, float step) { this->minValue = minValue; this->maxValue = maxValue; this->step = step; }
	// enables quantization: minVal = 0, maxValue=(possibleStates-1), step=1
	void setQuantization(int possibleStates);
	// enables quantization: minVal = 0, maxValue=1, step=1/(possibleStates-1)
	void setNormalQuantization(int possibleStates);
	float getGraphicalValue() const { return m_normalValue; }
};
// VSlider
class GUIVSlider :public GUIElement
{
	// value between 0 and 1
	float m_normalValue = 0;
	// value between minVal and maxVal
	float m_value = 0;
	glm::vec2 m_draggedCursor;

	bool m_scroll_focus;
	float m_old_placeOfClick = 0;
public:
	
	float sliderRatio = 0.1f;
	float sliderHeight = 50;
	// how big the step is
	// 0 means smooth progress (any value between minV and maxV is possible) 
	float step = 0;
	float minValue = 0, maxValue = 1;
	// true means up = minValue, down =maxVal
	bool invertedVal = false;

	// called when value is changed
	ActionF on_changed;
	GUIVSlider();
	void onMyEvent(Event& e) override;
	virtual void setValue(float v);
	virtual float getValue() const { return m_value; }
	inline void prepare(float minValue, float maxValue, float step) { this->minValue = minValue; this->maxValue = maxValue; this->step = step; }
	// enables quantization: minVal = 0, maxValue=(possibleStates-1), step=1
	void setQuantization(int possibleStates);
	// enables quantization: minVal = 0, maxValue=1, step=1/(possibleStates-1)
	void setNormalQuantization(int possibleStates);
	float getGraphicalValue() const { return m_normalValue; }
	inline void setHasScrollFocus(bool focus) { m_scroll_focus = focus; }
};
// HSlider
class GUIHSlider :public GUIElement
{
	// value between 0 and 1
	float m_normalValue = 0;
	// value between minVal and maxVal
	float m_value = 0;
	glm::vec2 m_draggedCursor;

	bool m_scroll_focus;
	float m_old_placeOfClick = 0;
public:

	float sliderRatio = 0.1f;
	float sliderWidth = 50;
	// how big the step is
	// 0 means smooth progress (any value between minV and maxV is possible) 
	float step = 0;
	float minValue = 0, maxValue = 1;
	// true means up = minValue, down =maxVal
	bool invertedVal = false;

	// called when value is changed
	ActionF on_changed;
	GUIHSlider();
	void onMyEvent(Event& e) override;
	virtual void setValue(float v);
	virtual float getValue() const { return m_value; }
	inline void prepare(float minValue, float maxValue, float step) { this->minValue = minValue; this->maxValue = maxValue; this->step = step; }
	// enables quantization: minVal = 0, maxValue=(possibleStates-1), step=1
	void setQuantization(int possibleStates);
	// enables quantization: minVal = 0, maxValue=1, step=1/(possibleStates-1)
	void setNormalQuantization(int possibleStates);
	float getGraphicalValue() const { return m_normalValue; }
	inline void setHasScrollFocus(bool focus) { m_scroll_focus = focus; }
};

class GUIView :public GUIElement
{
public:
	GUIView();

	GUIElement* getInside() { return getChildren()[0]; }

	void appendChild(GUIElement* element) override;
	void destroyChild(int index) override;
};

//Special
class GUITextButton :public GUIButton
{
public:
	GUITextButton();
	GUITextButton(const std::string& text, FontMaterial* material);
	void setMaterial(FontMaterial* material);
	GUIText* getTextElement() { return static_cast<GUIText*>(getChildren()[0]); }
};
class GUIImageButton :public GUIButton
{
public:
	GUIImageButton();
	GUIImageButton(Sprite* image);
	void setImage(Sprite* image);
	GUIImage* getImageElement() { return static_cast<GUIImage*>(getChildren()[0]); }
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
	GUISpecialTextButton();
	void update() override;
};

GUIVerticalSplit* createGUISliderView(bool sliderOnLeft=true);



