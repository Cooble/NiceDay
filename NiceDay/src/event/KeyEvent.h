#pragma once
#include "Event.h"

namespace nd {

#define EVENT_KEY_BUILD_STATI(eventType)\
	static KeyCode getKeyNumber(Event& e)\
	{\
	if (e.getEventType()!=EventType::eventType)\
		return KeyCode::UNKNOWN;\
	return static_cast<KeyEvent*>(&e)->getKey();\
	}

enum KeyCode :int
{
	UNKNOWN = -1,

	/* Printable keys */
	SPACE = 32,
	APOSTROPHE = 39,
	/* ' */
	COMMA = 44,
	/* , */
	MINUS = 45,
	/* - */
	PERIOD = 46,
	/* . */
	SLASH = 47,
	/* / */
	NUM_0 = 48,
	NUM_1 = 49,
	NUM_2 = 50,
	NUM_3 = 51,
	NUM_4 = 52,
	NUM_5 = 53,
	NUM_6 = 54,
	NUM_7 = 55,
	NUM_8 = 56,
	NUM_9 = 57,
	SEMICOLON = 59,
	/* ; */
	EQUAL = 61,
	/* = */
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	LEFT_BRACKET = 91,
	/* [ */
	BACKSLASH = 92,
	/* \ */
	RIGHT_BRACKET = 93,
	/* ] */
	GRAVE_ACCENT = 96,
	/* ` */
	WORLD_1 = 161,
	/* non-US #1 */
	WORLD_2 = 162,
	/* non-US #2 */

	/* Function keys */
	ESCAPE = 256,
	ENTER = 257,
	TAB = 258,
	BACKSPACE = 259,
	INSERT = 260,
	DELETE_KEY = 261,
	RIGHT = 262,
	LEFT = 263,
	DOWN = 264,
	UP = 265,
	PAGE_UP = 266,
	PAGE_DOWN = 267,
	HOME = 268,
	END = 269,
	CAPS_LOCK = 280,
	SCROLL_LOCK = 281,
	NUM_LOCK = 282,
	PRINT_SCREEN = 283,
	PAUSE = 284,
	F1 = 290,
	F2 = 291,
	F3 = 292,
	F4 = 293,
	F5 = 294,
	F6 = 295,
	F7 = 296,
	F8 = 297,
	F9 = 298,
	F10 = 299,
	F11 = 300,
	F12 = 301,
	F13 = 302,
	F14 = 303,
	F15 = 304,
	F16 = 305,
	F17 = 306,
	F18 = 307,
	F19 = 308,
	F20 = 309,
	F21 = 310,
	F22 = 311,
	F23 = 312,
	F24 = 313,
	F25 = 314,
	KP_0 = 320,
	KP_1 = 321,
	KP_2 = 322,
	KP_3 = 323,
	KP_4 = 324,
	KP_5 = 325,
	KP_6 = 326,
	KP_7 = 327,
	KP_8 = 328,
	KP_9 = 329,
	KP_DECIMAL = 330,
	KP_DIVIDE = 331,
	KP_MULTIPLY = 332,
	KP_SUBTRACT = 333,
	KP_ADD = 334,
	KP_ENTER = 335,
	KP_EQUAL = 336,
	LEFT_SHIFT = 340,
	LEFT_CONTROL = 341,
	LEFT_ALT = 342,
	LEFT_SUPER = 343,
	RIGHT_SHIFT = 344,
	RIGHT_CONTROL = 345,
	RIGHT_ALT = 346,
	RIGHT_SUPER = 347,
	MENU = 348
};

class KeyEvent : public Event
{
private:
	int m_key;
	/*Tells how long it was pressed, if 0 then it was pressed freshly*/
	int m_mods;
	bool m_repeat;

public:
	KeyEvent(int key, int mods, bool repeat = false) :
		m_key(key), m_mods(mods), m_repeat(repeat)
	{
	}

	KeyCode getKey() const { return (KeyCode)m_key; }
	KeyCode getFreshKey() const { return (KeyCode)(isRepeating() ? -1 : m_key); }
	int getMods() const { return m_mods; }
	int isRepeating() const { return m_repeat; }

	bool isAltPressed() const { return m_mods & Event::KeyMods::Alt; }
	bool isControlPressed() const { return m_mods & Event::KeyMods::Control; }
	bool isShiftPressed() const { return m_mods & Event::KeyMods::Shift; }

	EVENT_CATEGORY_BUILD(CatKey);

	static KeyCode getKeyNumber(Event& e)
	{
		if (!e.isInCategory(EventCategory::CatKey))
			return KeyCode::UNKNOWN;
		return static_cast<KeyEvent*>(&e)->getKey();
	}
};

class KeyPressEvent : public KeyEvent
{
public:
	KeyPressEvent(int key, int mods, bool repeat = false) : KeyEvent(key, mods, repeat)
	{
	}

	EVENT_TYPE_BUILD(KeyPress);
	EVENT_COPY(KeyPressEvent);
	EVENT_KEY_BUILD_STATI(KeyPress);

	// return keycode of key that was pressed once right now
	// return invalid if event is of type repeat
	static KeyCode getFreshKeyNumber(Event& e)
	{
		if (e.getEventType() != EventType::KeyPress)
			return KeyCode::UNKNOWN;
		return static_cast<KeyEvent*>(&e)->getFreshKey();
	}
};

class KeyReleaseEvent : public KeyEvent
{
public:
	KeyReleaseEvent(int key, int mods) : KeyEvent(key, mods, false)
	{
	}

	EVENT_TYPE_BUILD(KeyRelease);
	EVENT_COPY(KeyReleaseEvent);
	EVENT_KEY_BUILD_STATI(KeyRelease);
};

class KeyTypeEvent : public KeyEvent
{
public:
	KeyTypeEvent(int key, int mods) : KeyEvent(key, mods, false)
	{
	}

	EVENT_TYPE_BUILD(KeyType);
	EVENT_COPY(KeyTypeEvent);
	EVENT_KEY_BUILD_STATI(KeyType);
};
}
