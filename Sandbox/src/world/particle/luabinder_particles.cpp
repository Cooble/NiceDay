#pragma once
#include "luabinder_particles.h"
#ifndef NOO_SOOL
#include <sol/sol.hpp>
#endif
#include "world/particle/particles.h"
#include "world/World.h"
#include "world/entity/EntityPlayer.h"
#include "layer/WorldLayer.h"
#include "lua/LuaLayer.h"
#include "core/App.h"

using namespace nd;

static EntityPlayer* player;

void lua_binder_particles::bindEverything(sol::state& state,World* w, EntityPlayer* p)
{
#ifndef NOO_SOOL
	auto particles = state["Particle"].get_or_create<sol::table>();

	int ind = 0;
	for (auto& templ : ParticleRegistry::get().getList())
		particles.set(templ.name, ind++);

	state.new_usertype<World>("WorldClass",
		"",sol::no_constructor,
		"spawnParticle", &World::spawnParticle);
	
	state.set_function("World", [w]() {return w; });
	state.set_function("PlayerPos", [p](){return p->getPosition(); });

	App::get().getLua()->runScriptInConsole(state.lua_state(), "world = World()");
	App::get().getLua()->runScriptInConsole(state.lua_state(), "playerPos = PlayerPos()");
#endif

	
/*	state.new_usertype<SoundHandle>("Sound",
		"play", sol::overload(&SoundHandle::play, [](SoundHandle& h) { h.play(); }),
		"stop", sol::overload(&SoundHandle::stop, [](SoundHandle& h) { h.stop(); }),
		"setPitch", sol::overload(&SoundHandle::setPitch, [](SoundHandle& h, float p)
			{
				h.setPitch(p);
			}),
		"setVolume",
				sol::overload(&SoundHandle::setVolume, [](SoundHandle& h, float p)
					{
						h.setVolume(p);
					}),
				"pause", &SoundHandle::pause,
						"open", &SoundHandle::open,
						"setLoop", &SoundHandle::setLoop,
						"isPlaying", &SoundHandle::isPlaying
						);*/
	
	/*luabridge::getGlobalNamespace(L)
	.beginNamespace("Particle")
	.addVariable("torch_fire", &ParticleList::torch_fire, false)
	.addVariable("bulletShatter", &ParticleList::bulletShatter, false)
	.addVariable("dot", &ParticleList::dot, false)
	.addVariable("line", &ParticleList::line, false)
	.addVariable("torch_smoke", &ParticleList::torch_smoke, false)
	.addVariable("note", &ParticleList::note, false)
	.endNamespace();

//world wrapper
luabridge::getGlobalNamespace(L)
	.beginClass<World>("worldClass")
	.addFunction("getName", &World::getName)
	.addFunction("spawnParticle", &World::spawnParticle)
	.endClass();


s_lua_player_ref = &getPlayer();
//player wrapper
luabridge::getGlobalNamespace(L)
	.beginClass<WorldEntity>("playerClass")
	.addFunction("getPosition", (glm::vec2& (WorldEntity::*)())&WorldEntity::getPosition)
	.endClass();


s_lua_world_ref = m_world;
luabridge::getGlobalNamespace(L)
	.addFunction("World", &luaGetWorldRef)
	.addFunction("Player", &luaGetPlayerRef);



App::get().getLua()->runScriptInConsole(L,
										"function playerPos() "
										"	local po = player:getPosition() "
										"	return VEC2(po.x,po.y)"
										"end");
										*/
ND_TRACE("Loaded world lua bindings");
	
}
