#include "SandboxControls.h"
#include "GLFW/glfw3.h"
#include "event/ControlMap.h"


ControlType Controls::SPAWN_ENTITY=			GLFW_KEY_E;
ControlType Controls::DROP_ITEM =			GLFW_KEY_Q;
ControlType Controls::OPEN_CONSOLE =		GLFW_KEY_F;
ControlType Controls::OPEN_INVENTORY =		GLFW_KEY_I;
ControlType Controls::GO_UP =				GLFW_KEY_UP;
ControlType Controls::GO_DOWN =				GLFW_KEY_DOWN;
ControlType Controls::GO_LEFT =				GLFW_KEY_LEFT;
ControlType Controls::GO_RIGHT =			GLFW_KEY_RIGHT;
ControlType Controls::SPAWN_TNT=			GLFW_KEY_T;
ControlType Controls::SPAWN_BULLETS =		GLFW_KEY_B;
ControlType Controls::FLY_MODE =			GLFW_KEY_C;

void Controls::init()
{
	using namespace Controls;
	ControlMap::registerControl("SPAWN_ENTITY",		&SPAWN_ENTITY);
	ControlMap::registerControl("DROP_ITEM",		&DROP_ITEM);
	ControlMap::registerControl("OPEN_CONSOLE",		&OPEN_CONSOLE);
	ControlMap::registerControl("OPEN_INVENTORY",	&OPEN_INVENTORY);
	ControlMap::registerControl("GO_UP",			&GO_UP);
	ControlMap::registerControl("GO_DOWN",			&GO_DOWN);
	ControlMap::registerControl("GO_LEFT",			&GO_LEFT);
	ControlMap::registerControl("GO_RIGHT",			&GO_RIGHT);
	ControlMap::registerControl("SPAWN_TNT",		&SPAWN_TNT);
	ControlMap::registerControl("SPAWN_BULLETS",	&SPAWN_BULLETS);
	ControlMap::registerControl("FLY_MODE",	&FLY_MODE);
}
