#pragma once
#include "layer/Layer.h"
#include "memory/stack_allocator.h"


struct LuaConsole;
struct lua_State;

class LuaLayer : public Layer
{
private:
	LuaConsole* m_console;
	bool m_show_imgui_console;
	lua_State* m_L;
	void print_error(lua_State* state);

public:
	LuaLayer()=default;
	~LuaLayer()=default;
	
	inline lua_State* getLuaState() { return m_L; }
	inline void openConsole() { m_show_imgui_console = true; }
	inline void closeConsole() { m_show_imgui_console = false; }
	void printToLuaConsole(lua_State* L, const char* c);
	void runScriptInConsole(lua_State* L, const char* c);
	
	void runScriptFromFile(lua_State* L, const nd::temp_string& filePath);
	void onUpdate() override;
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& e) override;
	

};
