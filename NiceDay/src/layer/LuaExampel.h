#pragma once
#include "ndpch.h"

class LuaExampel
{
private:
	int id1;
	uint32_t id2;
public:
	int id3;
	int id4;
	std::string text;
public:
	LuaExampel() = default;

	void killMe();
	void call(std::string smth);
	int getID3()const;

public:
	static void callStatic(std::string smth);
	
};

void callMeMaybe(std::string smth);
void callMeMaybe();
