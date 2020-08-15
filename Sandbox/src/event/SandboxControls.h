#pragma once
#include "event/KeyEvent.h"
typedef unsigned long long ControlType;

namespace Controls {
	extern ControlType SPAWN_ENTITY;
	extern ControlType DROP_ITEM;
	extern ControlType OPEN_CONSOLE;
	extern ControlType OPEN_INVENTORY;

	extern ControlType GO_UP;
	extern ControlType GO_DOWN;
	extern ControlType GO_LEFT;
	extern ControlType GO_RIGHT;

	extern ControlType SPAWN_TNT;
	extern ControlType SPAWN_BULLETS;

	extern ControlType FLY_MODE;
	
	extern void init();

}
inline bool operator==(const KeyCode& lhs, const ControlType& rhs) { return ((ControlType)lhs) == rhs; }
inline bool operator!=(const KeyCode& lhs, const ControlType& rhs) { return ((ControlType)lhs) != rhs; }