#include "ndpch.h"
#include "LuaExampel.h"

void LuaExampel::killMe()
{
	ND_INFO("I have died");
}

void LuaExampel::call(std::string smth)
{
	ND_INFO("calling {}", smth);
}

int LuaExampel::getID3() const
{
	return id3;
}

void LuaExampel::callStatic(std::string smth)
{
	ND_INFO("calling static {}", smth);
}

void callMeMaybe(std::string smth)
{
	ND_INFO("calling static {}", smth);

}

void callMeMaybe()
{
	ND_INFO("Calling maybe");
}
