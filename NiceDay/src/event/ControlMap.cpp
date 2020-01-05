﻿#include "ndpch.h"
#include "ControlMap.h"
#include "GLFW/glfw3.h"

std::unordered_map<std::string, ControlButton> ControlMap::s_buttons;
std::unordered_map<uint64_t, std::string> ControlMap::s_button_names;

void ControlMap::registerControl(const std::string& id, uint64_t* pointer, const std::string& family)
{
	ControlButton b;
	b.id = id;
	b.pointer = pointer;
	b.family = family;
	s_buttons[id] = b;
}

void ControlMap::registerControl(const std::string& id, uint64_t* pointer)
{
	ControlButton b;
	b.id = id;
	b.pointer = pointer;
	b.family = "";
	s_buttons[id] = b;
}

const ControlButton* ControlMap::getButtonData(const std::string& id)
{
	auto it = s_buttons.find(id);
	if (it == s_buttons.end())
		return nullptr;
	return &(it->second);
}

void ControlMap::serialize(NBT& t)
{
	int idx = 0;
	for (auto& pair : s_buttons)
	{
		NBT tt;
		tt.set("name", pair.second.id);
		tt.set("value", *pair.second.pointer);
		t.set(nd::to_string(idx++), tt);
	}
}

void ControlMap::deserialize(NBT& t)
{
	int idx = 0;
	while (t.exists<NBT>(nd::to_string(idx)))
	{
		auto& tt = t.get<NBT>(nd::to_string(idx++));
		*s_buttons[tt.get<std::string>("name")].pointer = tt.get<uint64_t>("value");
	}
}

#define CM_ADD_KEY(x) s_button_names[x] = std::string(#x).substr(9)
void ControlMap::init()
{
	CM_ADD_KEY(GLFW_KEY_UNKNOWN);
	/* Printable keys */
	CM_ADD_KEY(GLFW_KEY_SPACE);
	CM_ADD_KEY(GLFW_KEY_APOSTROPHE);
	CM_ADD_KEY(GLFW_KEY_COMMA);
	CM_ADD_KEY(GLFW_KEY_MINUS);
	CM_ADD_KEY(GLFW_KEY_PERIOD);
	CM_ADD_KEY(GLFW_KEY_SLASH);
	CM_ADD_KEY(GLFW_KEY_0);
	CM_ADD_KEY(GLFW_KEY_1);
	CM_ADD_KEY(GLFW_KEY_2);
	CM_ADD_KEY(GLFW_KEY_3);
	CM_ADD_KEY(GLFW_KEY_4);
	CM_ADD_KEY(GLFW_KEY_5);
	CM_ADD_KEY(GLFW_KEY_6);
	CM_ADD_KEY(GLFW_KEY_7);
	CM_ADD_KEY(GLFW_KEY_8);
	CM_ADD_KEY(GLFW_KEY_9);
	CM_ADD_KEY(GLFW_KEY_SEMICOLON);
	CM_ADD_KEY(GLFW_KEY_EQUAL);
	CM_ADD_KEY(GLFW_KEY_A);
	CM_ADD_KEY(GLFW_KEY_B);
	CM_ADD_KEY(GLFW_KEY_C);
	CM_ADD_KEY(GLFW_KEY_D);
	CM_ADD_KEY(GLFW_KEY_E);
	CM_ADD_KEY(GLFW_KEY_F);
	CM_ADD_KEY(GLFW_KEY_G);
	CM_ADD_KEY(GLFW_KEY_H);
	CM_ADD_KEY(GLFW_KEY_I);
	CM_ADD_KEY(GLFW_KEY_J);
	CM_ADD_KEY(GLFW_KEY_K);
	CM_ADD_KEY(GLFW_KEY_L);
	CM_ADD_KEY(GLFW_KEY_M);
	CM_ADD_KEY(GLFW_KEY_N);
	CM_ADD_KEY(GLFW_KEY_O);
	CM_ADD_KEY(GLFW_KEY_P);
	CM_ADD_KEY(GLFW_KEY_Q);
	CM_ADD_KEY(GLFW_KEY_R);
	CM_ADD_KEY(GLFW_KEY_S);
	CM_ADD_KEY(GLFW_KEY_T);
	CM_ADD_KEY(GLFW_KEY_U);
	CM_ADD_KEY(GLFW_KEY_V);
	CM_ADD_KEY(GLFW_KEY_W);
	CM_ADD_KEY(GLFW_KEY_X);
	CM_ADD_KEY(GLFW_KEY_Y);
	CM_ADD_KEY(GLFW_KEY_Z);
	CM_ADD_KEY(GLFW_KEY_LEFT_BRACKET);
	CM_ADD_KEY(GLFW_KEY_BACKSLASH);
	CM_ADD_KEY(GLFW_KEY_RIGHT_BRACKET);
	CM_ADD_KEY(GLFW_KEY_GRAVE_ACCENT);
	CM_ADD_KEY(GLFW_KEY_WORLD_1);
	CM_ADD_KEY(GLFW_KEY_WORLD_2);
	CM_ADD_KEY(GLFW_KEY_ESCAPE);
	CM_ADD_KEY(GLFW_KEY_ENTER);
	CM_ADD_KEY(GLFW_KEY_TAB);
	CM_ADD_KEY(GLFW_KEY_BACKSPACE);
	CM_ADD_KEY(GLFW_KEY_INSERT);
	CM_ADD_KEY(GLFW_KEY_DELETE);
	CM_ADD_KEY(GLFW_KEY_RIGHT);
	CM_ADD_KEY(GLFW_KEY_LEFT);
	CM_ADD_KEY(GLFW_KEY_DOWN);
	CM_ADD_KEY(GLFW_KEY_UP);
	CM_ADD_KEY(GLFW_KEY_PAGE_UP);
	CM_ADD_KEY(GLFW_KEY_PAGE_DOWN);
	CM_ADD_KEY(GLFW_KEY_HOME);
	CM_ADD_KEY(GLFW_KEY_END);
	CM_ADD_KEY(GLFW_KEY_CAPS_LOCK);
	CM_ADD_KEY(GLFW_KEY_SCROLL_LOCK);
	CM_ADD_KEY(GLFW_KEY_NUM_LOCK);
	CM_ADD_KEY(GLFW_KEY_PRINT_SCREEN);
	CM_ADD_KEY(GLFW_KEY_PAUSE);
	CM_ADD_KEY(GLFW_KEY_F1);
	CM_ADD_KEY(GLFW_KEY_F2);
	CM_ADD_KEY(GLFW_KEY_F3);
	CM_ADD_KEY(GLFW_KEY_F4);
	CM_ADD_KEY(GLFW_KEY_F5);
	CM_ADD_KEY(GLFW_KEY_F6);
	CM_ADD_KEY(GLFW_KEY_F7);
	CM_ADD_KEY(GLFW_KEY_F8);
	CM_ADD_KEY(GLFW_KEY_F9);
	CM_ADD_KEY(GLFW_KEY_F10);
	CM_ADD_KEY(GLFW_KEY_F11);
	CM_ADD_KEY(GLFW_KEY_F12);
	CM_ADD_KEY(GLFW_KEY_F13);
	CM_ADD_KEY(GLFW_KEY_F14);
	CM_ADD_KEY(GLFW_KEY_F15);
	CM_ADD_KEY(GLFW_KEY_F16);
	CM_ADD_KEY(GLFW_KEY_F17);
	CM_ADD_KEY(GLFW_KEY_F18);
	CM_ADD_KEY(GLFW_KEY_F19);
	CM_ADD_KEY(GLFW_KEY_F20);
	CM_ADD_KEY(GLFW_KEY_F21);
	CM_ADD_KEY(GLFW_KEY_F22);
	CM_ADD_KEY(GLFW_KEY_F23);
	CM_ADD_KEY(GLFW_KEY_F24);
	CM_ADD_KEY(GLFW_KEY_F25);
	CM_ADD_KEY(GLFW_KEY_KP_0);
	CM_ADD_KEY(GLFW_KEY_KP_1);
	CM_ADD_KEY(GLFW_KEY_KP_2);
	CM_ADD_KEY(GLFW_KEY_KP_3);
	CM_ADD_KEY(GLFW_KEY_KP_4);
	CM_ADD_KEY(GLFW_KEY_KP_5);
	CM_ADD_KEY(GLFW_KEY_KP_6);
	CM_ADD_KEY(GLFW_KEY_KP_7);
	CM_ADD_KEY(GLFW_KEY_KP_8);
	CM_ADD_KEY(GLFW_KEY_KP_9);
	CM_ADD_KEY(GLFW_KEY_KP_DECIMAL);
	CM_ADD_KEY(GLFW_KEY_KP_DIVIDE);
	CM_ADD_KEY(GLFW_KEY_KP_MULTIPLY);
	CM_ADD_KEY(GLFW_KEY_KP_SUBTRACT);
	CM_ADD_KEY(GLFW_KEY_KP_ADD);
	CM_ADD_KEY(GLFW_KEY_KP_ENTER);
	CM_ADD_KEY(GLFW_KEY_KP_EQUAL);
	CM_ADD_KEY(GLFW_KEY_LEFT_SHIFT);
	CM_ADD_KEY(GLFW_KEY_LEFT_CONTROL);
	CM_ADD_KEY(GLFW_KEY_LEFT_ALT);
	CM_ADD_KEY(GLFW_KEY_LEFT_SUPER);
	CM_ADD_KEY(GLFW_KEY_RIGHT_SHIFT);
	CM_ADD_KEY(GLFW_KEY_RIGHT_CONTROL);
	CM_ADD_KEY(GLFW_KEY_RIGHT_ALT);
	CM_ADD_KEY(GLFW_KEY_RIGHT_SUPER);
	CM_ADD_KEY(GLFW_KEY_MENU);

	/*
	GLFW_MOUSE_BUTTON_LAST
	GLFW_MOUSE_BUTTON_LEFT
	GLFW_MOUSE_BUTTON_RIGHT
	GLFW_MOUSE_BUTTON_MIDDLE
	*/
}

const std::string& ControlMap::getKeyName(uint64_t key)
{
	return s_button_names[key];
}
