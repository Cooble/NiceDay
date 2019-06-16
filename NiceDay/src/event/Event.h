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
	virtual int getEventCategories() const = 0;
	virtual const char* getName() const = 0;
	virtual inline std::string toString() const { return getName(); };

	inline bool isInCategory(int category) const { return category & getEventCategories(); }
};

#define EVENT_TYPE_BUILD(type) \
static EventType getEventTypeStatic() {return EventType::##type;};\
virtual EventType getEventType() const override {return getEventTypeStatic();};\
virtual const char* getName() const override {return #type;};

#define EVENT_CATEGORY_BUILD(category) \
virtual int getEventCategories() const override {return category;};

class EventDispatcher {

	template<typename T>
	using EventConsumer = std::function<bool(T&)>;
	
private:
	Event& m_event;
public:
	
	EventDispatcher(Event& event) 
		:m_event(event) {}


	template<typename T>
	bool dispatch(EventConsumer<T> func) {
		if (T::getEventTypeStatic() == m_event.getEventType()) {
			m_event.handled = func(*(T*)&m_event);
			return m_event.handled;
		}
		return false;
	}
};

/*std::ostream& operator<<(std::ostream& Str, Event const& eve) {
	Str << eve.toString();
	return Str;
}*/