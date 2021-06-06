#pragma once
#include "core/App.h"

class GUILayer;
class Sandbox:public nd::App
{
private:
	GUILayer* m_guiLayer;
	
public:
	Sandbox();

	auto getGUILayer() { return m_guiLayer; }
};
