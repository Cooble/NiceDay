#pragma once
#include "core/App.h"

class GUILayer;
class Sandbox:public App
{
private:
	GUILayer* m_guiLayer;
	
public:
	Sandbox();

	inline auto getGUILayer() { return m_guiLayer; }
};
