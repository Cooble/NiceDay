#pragma once
#include "gui/GUIBasic.h"

class MainWindow:public GUIWindow
{
private:
	GUIImage* m_logo;
public:
	MainWindow();
	void update() override;
	
};

class GUIWorldEntry:public GUIBlank
{
	GUIText* m_world_name;
public:
	GUIWorldEntry();

	void setWorldName(const std::string& name);
	const std::string& getWorldName();
		
};
class PlayWindow :public GUIWindow
{
private:
public:
	PlayWindow();
	

};