#pragma once
#include "event/Event.h"
#include "event/MouseEvent.h"
#include <optional>

namespace nd {

struct GUIStyle;

//very simple optional implementation for small types (uses too much copy)
template <typename T>
class nd_optional
{
	bool m_hasValue;
	T m_value;

public:
	nd_optional() : m_hasValue(false)
	{
	}

	nd_optional(T value) : m_hasValue(true), m_value(value)
	{
	}

	nd_optional(nd_optional<T>&& v) = default;
	nd_optional(const nd_optional<T>& v) = default;
	constexpr bool hasValue() const { return m_hasValue; }
	constexpr void clear() { m_hasValue = false; }

	T& value() const
	{
		ASSERT(m_hasValue, "attempt to get value where there is none");
		return m_value;
	}

	nd_optional<T>& operator=(const T& value)
	{
		m_hasValue = true;
		m_value = value;
		return *this;
	}

	nd_optional<T>& operator=(const nd_optional<T>& value)
	{
		m_hasValue = value.m_hasValue;
		m_value = value.m_value;
		return *this;
	}

	//operator T ()() const {return m_value; }
	operator T() const { return m_value; }
};

struct FontMaterial;
constexpr int GUI_LEFT = 0;
constexpr int GUI_TOP = 1;
constexpr int GUI_RIGHT = 2;
constexpr int GUI_BOTTOM = 3;


class GUIElement;
typedef std::function<void(GUIElement&)> ActionF;
typedef std::function<void(Event&, GUIElement&)> EventF;
typedef int GEID;

enum class GETYPE : int
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
	HSlider,
	TextBox,
	TextArea,
	SplitHorizontal,
	SplitVertical,
	View,
	Blank,
	ItemContainer,
	Other,
	//doesn!t have custom render (usually aglomeration of elements)
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
	INVALID,
};

constexpr bool isWidth(GUIDimensionInherit dim)
{
	return dim == GUIDimensionInherit::WIDTH || dim == GUIDimensionInherit::WIDTH_HEIGHT;
}

constexpr bool isHeight(GUIDimensionInherit dim)
{
	return dim == GUIDimensionInherit::HEIGHT || dim == GUIDimensionInherit::WIDTH_HEIGHT;
}

constexpr float GUIElement_InvalidNumber = -100000;

class GUIElement
{
protected:
#ifdef ND_DEBUG
	bool m_has_parent = false;
#endif
	std::vector<GUIElement*> m_children;
	GUIElement* m_parent = nullptr;

	// is mouse over the element
	bool m_has_focus = false;

	//usually some column,row grid or something which is fully embedded in its parent
	bool m_is_pressed = false;

	//checks if element gained or lost focus
	void checkFocus(MouseMoveEvent& e);

#ifdef ND_DEBUG
#define GUIE_CHECK_PARENT(child) ASSERT(!child->m_has_parent,"Adding child which already has a parent!");child->m_has_parent=true;
#else
#define GUIE_CHECK_PARENT()
#endif

public:
	//===FLAGS=========================================
	// if not -> no render and no render of children
	bool isEnabled = true;
	//if should be rendered (children will be rendered regardless)
	bool isVisible = true;
	//has no dimensions on its own
	bool isNotSpatial = false;
	// always adapts dimensions based on the sizes of children
	bool isAlwaysPacked = false;
	bool isDirty;

	//===ALIGNMENT=====================================
	GUIAlign alignment = GUIAlign::INVALID;

	union
	{
		float margin[4];
		glm::vec4 marginVec;
	};

	union
	{
		float padding[4];
		glm::vec4 paddingVec;
	};

	float space = 0;
	float renderAngle = 0;
	GUIDimensionInherit dimInherit = GUIDimensionInherit::INVALID;

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

	//===EVENTS========================================
	ActionF onDimChange;
	EventF onMyEventFunc;


	//===IDS===========================================
	//every element gets unique id in constructor
	const GEID serialID;
	GECLASS clas;
	GETYPE type;

	//===OTHER=========================================
	glm::vec4 color = {0, 0, 0, 0};

	// unique identificator of element
	std::string id;


	// every guiElement must have at least non parametric constructor
	GUIElement(GETYPE type);
	virtual ~GUIElement();


	constexpr bool hasFocus() const { return m_has_focus; }
	constexpr bool isPressed() const { return m_is_pressed; }
	float widthPadding() const { return padding[GUI_LEFT] + padding[GUI_RIGHT]; }
	float heightPadding() const { return padding[GUI_BOTTOM] + padding[GUI_TOP]; }


	void setAlignment(GUIAlign align) { alignment = align; }

	void setPadding(float p)
	{
		for (float& i : padding)
			i = p;
	}

	//will center element to the center of width and height
	void setCenterPosition(float width, float height)
	{
		x = (width - this->width) / 2;
		y = (height - this->height) / 2;
	}

	void markDirty() { isDirty = true; }

	void setParent(GUIElement* parent) { this->m_parent = parent; }

	GUIElement* getParent() const
	{
		return m_parent;
	}

	auto& getChildren() { return m_children; }

	GUIElement* getFirstChild()
	{
		ASSERT(!m_children.empty(), "no children");
		return m_children[0];
	}

	//takes ownership
	inline virtual void appendChild(GUIElement* element);

	inline virtual void destroyChild(int index);

	virtual void destroyChildWithID(GEID id);

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


	//will be called each tick if is_updatable
	virtual void update();


	inline virtual void onEvent(Event& e);

	//will call onMyEvent on element and all its children and all its children and ... you get the idea
	virtual void onEventBroadcast(Event& e);
	//called when broadcasting events
	//called when no child has consumed it
	inline virtual void onMyEvent(Event& e);
	void clearChildren();
	//searches through all grand..children and returns child with the id or nullptr
	GUIElement* getChildWithID(const std::string& id)
	{
		for (auto child : m_children)
		{
			if (child->id == id)
				return child;
			auto grand = child->getChildWithID(id);
			if (grand)
				return grand;
		}
		return nullptr;
	}

	bool hasChild(GEID id);
	//removes child but doesnt call destructor
	GUIElement* takeChild(GEID id);

	virtual void applyStyle(GUIStyle& style);

	template <typename ElementType>
	ElementType* get(const std::string& id) { return (ElementType*)this->getChildWithID(id); }
};


#define ND_GUI_STYLE_BUILD(att) if (!target.att.hasValue()) target.att = att;

struct GUIStyle
{
	nd_optional<FontMaterial*> textMaterial;
	nd_optional<GUIDimensionInherit> dimInherit;
	nd_optional<bool> isEnabled;
	nd_optional<float> renderAngle;
	nd_optional<bool> isVisible;
	nd_optional<bool> isNotSpatial;
	nd_optional<bool> isAlwaysPacked;
	nd_optional<glm::vec4> color;
	nd_optional<float> space;
	nd_optional<float> width;
	nd_optional<float> height;
	nd_optional<GUIAlign> alignment;
	nd_optional<glm::vec4> margin;
	nd_optional<glm::vec4> padding;

	//fill missing attribs of target with this
	void apply(GUIStyle& target) const
	{
		ND_GUI_STYLE_BUILD(textMaterial);
		ND_GUI_STYLE_BUILD(dimInherit);
		ND_GUI_STYLE_BUILD(isEnabled);
		ND_GUI_STYLE_BUILD(renderAngle);
		ND_GUI_STYLE_BUILD(isVisible);
		ND_GUI_STYLE_BUILD(isNotSpatial);
		ND_GUI_STYLE_BUILD(isAlwaysPacked);
		ND_GUI_STYLE_BUILD(color);
		ND_GUI_STYLE_BUILD(space);
		ND_GUI_STYLE_BUILD(width);
		ND_GUI_STYLE_BUILD(height);
		ND_GUI_STYLE_BUILD(alignment);
		ND_GUI_STYLE_BUILD(margin);
		ND_GUI_STYLE_BUILD(padding);
	}
};
}
