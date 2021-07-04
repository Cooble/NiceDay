#pragma once
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
	extern ControlType AUTO_BLOCK_PICKER;
	
	extern void init();

}