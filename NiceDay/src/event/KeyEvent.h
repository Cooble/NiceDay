#pragma once
#include "Event.h"
class KeyPressEvent :public Event
{
private:
	int m_key;
	/*Tells how long it was pressed, if 0 then it was pressed freshly*/
	int m_number;

public:
	KeyPressEvent(int key,int number) :
		m_key(key), m_number(number)
	{
	}
	inline int getKey() const { return m_key; }
	inline int getNumber() const { return m_number; }
	EVENT_TYPE_BUILD(KeyPress)
	EVENT_CATEGORY_BUILD(EventCategoryKey)


};
class KeyReleaseEvent :public Event
{
private:
	int m_key;

public:
	KeyReleaseEvent(int key) :
		m_key(key)
	{
	}
	inline const int getKey() const { return m_key; }
	EVENT_TYPE_BUILD(KeyRelease)
	EVENT_CATEGORY_BUILD(EventCategoryKey)


};
class KeyTypeEvent :public Event
{
private:
	int m_key;

public:
	KeyTypeEvent(int key) :
		m_key(key)
	{
	}
	inline const int getKey() const { return m_key; }
	EVENT_TYPE_BUILD(KeyType)
	EVENT_CATEGORY_BUILD(EventCategoryKey)


};
