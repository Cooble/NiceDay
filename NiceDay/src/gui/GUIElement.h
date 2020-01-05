#pragma once
#include "event/Event.h"
#include "event/MouseEvent.h"

constexpr int GUI_LEFT = 0;
constexpr int GUI_TOP = 1;
constexpr int GUI_RIGHT = 2;
constexpr int GUI_BOTTOM = 3;


class GUIElement;
typedef std::function<void(GUIElement&)> ActionF;
typedef std::function<void(Event&,GUIElement&)> EventF;
typedef int GEID;

enum class GETYPE
{
	Window,
	Text,
	Button,
	Column,
	Row,
	Grid,
	Image,
	CheckBox,
	RadioButton,
	Slider,
	VSlider,
	TextBox,
	TextArea,
	SplitHorizontal,
	SplitVertical,
	View,
	Blank,
	ItemContainer,
	Other, //doesn!t have custom render (usually aglomeration of elements)
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
	GUIAlign m_alignment=GUIAlign::INVALID;
protected:
	std::vector<GUIElement*> m_children;
	GUIElement* m_parent = nullptr;

	// is mouse over the element
	bool m_has_focus = false;
	
	//if should be rendered (children will be rendered regardless)
	
	//has no dimensions on its own
	//usually some column,row grid or something which is fully embedded in its parent
	bool m_is_pressed = false;


	//checks if element gained or lost focus
	void checkFocus(MouseMoveEvent& e);

public:
	/**
	 * if not -> no render and no render of children
	 */
	bool isEnabled = true;
	float renderAngle=0;
	bool isVisible = true;
	bool isNotSpacial = false;
	ActionF onDimChange;
	EventF onMyEventFunc;

	// always adapts dimensions based on the sizes of children
	bool isAlwaysPacked = false;
	glm::vec4 color={0,0,0,0};
	float space=0;

	//every element gets unique id in constructor
	const GEID id;
	const GECLASS clas;
	GETYPE type;

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
	GUIDimensionInherit dimInherit = GUIDimensionInherit::INVALID;

	GUIElement(GETYPE type);
	virtual ~GUIElement();

	inline void setPadding(float paddin)
	{
		for (float& i : padding)
			i = paddin;
	}
	inline bool hasFocus() const { return m_has_focus; }
	inline bool isNotSpaci() const { return isNotSpacial; }
	inline bool isPressed() const { return m_is_pressed; }
	inline void setParent(GUIElement* parent) { this->m_parent = parent; }
	
	inline GUIAlign getAlignment() const { return m_alignment; }
	inline void setAlignment(GUIAlign align) { m_alignment = align; }


	inline void markDirty() { is_dirty = true; }

	inline GUIElement* getParent() const
	{
		return m_parent;
	}

	inline auto& getChildren() { return m_children; }
	inline GUIElement* getFirstChild()
	{
		ASSERT(!m_children.empty(), "no children");
		return m_children[0];
	}

	//takes ownership
	inline virtual void appendChild(GUIElement* element);

	inline virtual void removeChild(int index);

	virtual void removeChildWithID(GEID id);

	// called when this instance was given as a child to some parent element
	// used to update all static positions and stuff
	virtual void onParentAttached();
	
	virtual void onChildChange();

	//called when the dimensions were changed
	virtual void onDimensionChange();

	// changes positions of children to fit current dimensions
	virtual void repositionChildren();

	// adjust dimensions to fit all children and for inheritDimensions
	// NOTE:	it does not reposition children
	// NOTE:	nor does it anything else
	// // return true if dimensions were changed
	virtual bool packDimensions();
	
	// used to adapt dimensions to those of the parent
	// @return true if change in dimensions
	virtual bool adaptToParent();

	//called on parent dimension change
	virtual void onParentChanged();
	
	bool contains(float xx, float yy) const;

	//will center element to the center of width and height
	inline void setCenterPosition(float width,float height)
	{
		x = (width - this->width) / 2;
		y = (height - this->height) / 2;
	}
	//will be called each tick if is_updatable
	virtual void update();
	
	inline float widthPadding()const { return padding[GUI_LEFT] + padding[GUI_RIGHT]; }
	inline float heightPadding()const { return padding[GUI_BOTTOM] + padding[GUI_TOP]; }

	inline virtual void onEvent(Event& e);

	//will call onMyEvent on element and all its children and all its children and ... you get the idea
	virtual void onEventBroadcast(Event& e);
	//called when broadcasting events
	//called when no child has consumed it
	inline virtual void onMyEvent(Event& e);
	void clearChildren();

	bool hasChild(GEID id);
	//removes child but doesnt call destructor
	GUIElement* takeChild(GEID id);

};
