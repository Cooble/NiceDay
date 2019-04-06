#pragma once
#include "ndpch.h"
#include <GLFW/glfw3.h>
#include "Core.h"
class Event {

public:
	enum class EventType {
		WindowClose,WindowResize,
		MousePress, MouseRelease, MouseScroll,MouseMove,
		KeyPress,KeyRelease,KeyType,
	};
	enum EventCategory {
		EventCategoryNone = 0,
		EventCategoryMouse = BIT(0),
		EventCategoryMouseKey = BIT(1),
		EventCategoryKey = BIT(2),
		EventCategoryWindow = BIT(3),
	};

	bool handled = false;

	virtual EventType getEventType() const = 0;
	virtual EventCategory getEventCategory() const = 0;
	virtual const char* getName() const = 0;
	virtual inline std::string toString() const { return getName(); };

	inline bool isInCategory(int category) const { return category & getEventCategory(); }
};

#define EVENT_TYPE_BUILD(type) \
static EventType getEventTypeStatic() {return EventType::##type;};\
virtual EventType getEventType() const override {return getEventTypeStatic();};\
virtual const char* getName() const override {return #type;};

#define EVENT_CATEGORY_BUILD(type) \
static EventCategory getEventCategoryStatic() {return type;};\
virtual EventCategory getEventCategory() const override {return getEventCategoryStatic();};

std::ostream& operator<<(std::ostream& Str, Event const& eve) {
	Str << eve.toString();
	return Str;
}