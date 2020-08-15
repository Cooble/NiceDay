#pragma once
#include "scene/NewScene.h"

struct NativeScript
{

	Entity entity;
	NewScene* scene;

	template<typename T>
	auto& getComponent() { return entity.get<T>(); }
	template<typename T>
	bool hasComponent() { return entity.has<T>(); }
	template <typename ComponentType, typename... Args>
	decltype(auto) emplaceOrReplaceComponent(Args&&... args)
	{
		return entity.emplaceOrReplace<ComponentType>(std::forward<Args>(args)...);
	}


	// these methods are completely useless
	// This is just a list of native script methods
	// these methods doesn't need to be declared at all
	void onUpdate(){}
	void onCreate(){}
	void onDestroy(){}
	void onEvent(Event&){}
	
};
