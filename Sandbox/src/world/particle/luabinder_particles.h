#pragma once
#include <sol/forward.hpp>
class World;
class EntityPlayer;
namespace lua_binder_particles
{
	void bindEverything(sol::state& state,  World* w, EntityPlayer* p);

}