#pragma once
#include "event/Event.h"
#include "event/MouseEvent.h"

constexpr int GUI_LEFT = 0;
constexpr int GUI_TOP = 1;
constexpr int GUI_RIGHT = 2;
constexpr int GUI_BOTTOM = 3;


class GUIElement;
typedef std::function<void(GUIElement&)> ActionF;
typedef int GEID;

enum class GETYPE
{
	Window,
	Label,
	Button,
	Column,
	Row,
	Grid,
	Image,
	CheckBox,
	RadioButton,
	Slider,
	TextBox,
	SplitHorizontal,
	SplitVertical,
	Blank
};

typedef std::string GECLASS;

enum class GUIAlign : int
{
	RIGHT,
	RIGHT_UP,
	RIGHT_DOWN,
	LEFT,
	LEFT_UP,
	LEFT_DOWN,
	CENTER,
	CENTER_UP,
	CENTER_DOWN,
	UP,
	DOWN,
	INVALID
};
enum class GUIDimensionInherit : int
{
	WIDTH,
	HEIGHT,
	WIDTH_HEIGHT,
	INVALID
};

constexpr float GUIElement_InvalidNumber = -100000;
class GUIElement
{
private:
	std::vector<GUIElement*> children;
	GUIAlign m_alignment=GUIAlign::INVALID;
protected:
	GUIElement* parent = nullptr;

	// is mouse over the element
	bool has_focus = false;
	//need onUpdate() to be called
	bool is_updatable = false;
	//if should be rendered (children will be rendered regardless
	bool is_diplayed = true;
	

	//promise that this element wont have any children
	//all events will be consumed by it
	bool is_final_element = true;
	
	//has no dimensions on its own
	//usually some column,row grid or something which is fully embedded in its parent
	bool is_not_spacial = false;
	bool is_pressed = false;


	//checks if element gained or lost focus
	void checkFocus(MouseMoveEvent& e);

public:
	bool is_always_packed = false;
	glm::vec4 color={0,0,0,0};
	float space=0;

	//every element gets unique id in constructor
	const GEID id;
	const GECLASS clas;
	const GETYPE type;

	union
	{
		struct
		{
			float x, y;
		};

		glm::vec2 pos;
	};

	union
	{
		struct
		{
			float width, height;
		};

		glm::vec2 dim;
	};

	float margin[4];
	float padding[4];
	bool is_dirty;
	GUIDimensionInherit dimension_inherit = GUIDimensionInherit::INVALID;

	GUIElement(GETYPE type);
	virtual ~GUIElement();

	inline void setPadding(float paddin)
	{
		for (float& i : padding)
			i = paddin;
	}
	inline bool isUpdateable() const { return is_updatable; }
	inline bool hasFocus() const { return has_focus; }
	inline bool isNotSpacial() const { return is_not_spacial; }
	inline bool isDisplayed() const { return is_diplayed; }
	inline bool isPressed() const { return is_pressed; }
	inline void setParent(GUIElement* parent) { this->parent = parent; }
	
	inline GUIAlign getAlignment() const { return m_alignment; }
	inline void setAlignment(GUIAlign align) { m_alignment = align; }
	

	inline void markDirty() { is_dirty = true; }

	inline GUIElement* getParent() const
	{
		return parent;
	}

	inline auto& getChildren() { return children; }

	//takes ownership
	inline virtual void appendChild(GUIElement* element)
	{
		children.push_back(element);
		children[children.size() - 1]->parent = this;
		element->onParentChanged();
		
		onChildDimensionChange();
	}
	virtual void onChildDimensionChange()
	{
		if (is_always_packed)
			if (packDimensions())
				if (parent)
					parent->onChildDimensionChange();

		repositionChildren();
	}
	
	inline virtual void removeChild(int index)
	{
		ASSERT(index < children.size(), "Invalid child id");

		auto child = children[index];
		delete child;
		children.erase(children.begin() + index);
		
		onChildDimensionChange();
	}
	//called when the dimensions were changed
	virtual void onDimensionChange();

	// changes positions of children to fit current dimensions
	virtual void repositionChildren();

	// adjust dimensions to fit all children and for inheritDimensions
	// NOTE:	it does not reposition children
	// NOTE:	nor does it anything else
	// // return true if dimensions were changed
	virtual bool packDimensions();
	
	//called on parent dimension change
	//used to adapt dimensions to those of the parent
	virtual void onParentChanged();
	
	inline bool contains(float xx, float yy) const;

	//will center element to the center of width and height
	inline void setCenterPosition(float width,float height)
	{
		x = (width - this->width) / 2;
		y = (height - this->height) / 2;
	}
	//will be called each tick if is_updatable
	inline virtual void update(){}

	inline virtual void onEvent(Event& e);

	//will call onMyEvent on element and all its children and all its children and ... you get the idea
	virtual void onEventBroadcast(Event& e);
	//called when broadcasting events
	//called when no child has consumed it
	inline virtual void onMyEvent(Event& e);
};
