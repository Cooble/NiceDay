#pragma once
#include "ndpch.h"
#include "Core.h"
class Event {

public:
	enum class EventType {
		WindowClose,WindowResize,
		MousePress, MouseRelease, MouseScroll,MouseMove, MouseFocusGain, MouseFocusLost, MouseDrag,
		KeyPress,KeyRelease,KeyType,
		
	};
	enum EventCategory {
		None = 0,
		Mouse = BIT(0),
		MouseKey = BIT(1),
		Key = BIT(2),
		Window = BIT(3),
	};

	bool handled = false;

	virtual EventType getEventType() const = 0;
	virtual int getEventCategories() const = 0;
	virtual const char* getName() const = 0;
	virtual inline std::string toString() const { return getName(); };
	virtual Event* allocateCopy() const = 0;

	inline bool isInCategory(int category) const { return category & getEventCategories(); }
};

#define EVENT_TYPE_BUILD(type) \
static EventType getEventTypeStatic() {return EventType::##type;};\
virtual EventType getEventType() const override {return getEventTypeStatic();};\
virtual const char* getName() const override {return #type;};

#define EVENT_CATEGORY_BUILD(category) \
virtual int getEventCategories() const override {return category;};

#define EVENT_COPY(classe) \
virtual Event* allocateCopy() const{return new classe(*this);}

template<typename T>
using EventConsumer = std::function<bool(T&)>;

class EventDispatcher {
	
	
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