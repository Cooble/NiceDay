#pragma once
#include "Event.h"
class KeyPressEvent :public Event
{
private:
	int m_key;
	/*Tells how long it was pressed, if 0 then it was pressed freshly*/
	int m_mods;
	bool m_repeat;

public:
	KeyPressEvent(int key,int mods,bool repeat=false) :
		m_key(key), m_mods(mods)
	{
	}
	inline int getKey() const { return m_key; }
	inline int getMods() const { return m_mods; }
	inline int isRepeating() const { return m_repeat; }
	EVENT_TYPE_BUILD(KeyPress)
	EVENT_CATEGORY_BUILD(Key)
	EVENT_COPY(KeyPressEvent)


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
	EVENT_CATEGORY_BUILD(Key)
	EVENT_COPY(KeyReleaseEvent)


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
	EVENT_CATEGORY_BUILD(Key)
	EVENT_COPY(KeyTypeEvent)


};
