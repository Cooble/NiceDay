#pragma once
#include "ndpch.h"
#include "core/Core.h"

namespace nd {
class Event
{
public:
	//MAPPED FROM GLFW_MOD
	enum KeyMods :int
	{
		Shift = BIT(0),
		Control = BIT(1),
		Alt = BIT(2),
		Super = BIT(3),
		CapsLock = BIT(4),
		NumLock = BIT(5),
	};

	enum EventType
	{
		WindowClose,
		WindowResize,
		MousePress,
		MouseRelease,
		MouseScroll,
		MouseMove,
		MouseFocusGain,
		MouseFocusLost,
		MouseDrag,
		MouseEnter,
		KeyPress,
		KeyRelease,
		KeyType,
		Message,
		Drop
	};

	enum EventCategory :int
	{
		CatNone = 0,
		CatMouse = BIT(0),
		CatMouseKey = BIT(1),
		CatKey = BIT(2),
		CatWindow = BIT(3),
		CatMessage = BIT(4),
	};

	bool handled = false;

	virtual EventType getEventType() const = 0;
	virtual int getEventCategories() const = 0;
	virtual const char* getName() const = 0;
	virtual std::string toString() const { return getName(); };
	virtual Event* allocateCopy() const = 0;

	bool isInCategory(int category) const { return category & getEventCategories(); }
};

#define EVENT_TYPE_BUILD(type) \
static EventType getEventTypeStatic() {return EventType::##type;};\
virtual EventType getEventType() const override {return getEventTypeStatic();};\
virtual const char* getName() const override {return #type;};

#define EVENT_CATEGORY_BUILD(category) \
virtual int getEventCategories() const override {return (int)category;};

#define EVENT_COPY(classe) \
virtual Event* allocateCopy() const{return new classe(*this);}

template <typename T>
using EventConsumer = std::function<bool(T&)>;

class EventDispatcher
{
private:
	Event& m_event;
public:
	EventDispatcher(Event& event)
		: m_event(event)
	{
	}


	template <typename T>
	bool dispatch(EventConsumer<T> func)
	{
		if (T::getEventTypeStatic() == m_event.getEventType())
		{
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
}
