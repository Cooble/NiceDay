#include "SandboxControls.h"
#include "event/ControlMap.h"
#include "event/KeyEvent.h"


ControlType Controls::SPAWN_ENTITY=			(ControlType)KeyCode::E;
ControlType Controls::DROP_ITEM =			(ControlType)KeyCode::Q;
ControlType Controls::OPEN_CONSOLE =		(ControlType)KeyCode::F;
ControlType Controls::OPEN_INVENTORY =		(ControlType)KeyCode::I;
ControlType Controls::GO_UP =				(ControlType)KeyCode::UP;
ControlType Controls::GO_DOWN =				(ControlType)KeyCode::DOWN;
ControlType Controls::GO_LEFT =				(ControlType)KeyCode::LEFT;
ControlType Controls::GO_RIGHT =			(ControlType)KeyCode::RIGHT;
ControlType Controls::SPAWN_TNT =			(ControlType)KeyCode::T;
ControlType Controls::SPAWN_BULLETS =		(ControlType)KeyCode::B;
ControlType Controls::FLY_MODE =			(ControlType)KeyCode::C;

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
