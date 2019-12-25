#pragma once
#include "gui/GUIBasic.h"
#include "world/WorldsProvider.h"
#include "event/MessageEvent.h"

class MainWindow:public GUIWindow
{
private:
	GUIImage* m_logo;
	MessageConsumer m_messenger;
public:
	MainWindow(const MessageConsumer& c);
	void update() override;
	
};

class GUIWorldEntry:public GUIBlank
{
	GUIText* m_world_name;
	MessageConsumer* m_messenger;
	
public:
	GUIWorldEntry(MessageConsumer* c);

	void setWorldName(const std::string& name);
	const std::string& getWorldName();
		
};


class PlayWindow :public GUIWindow
{
private:
	std::vector<WorldInfoData> m_worlds;
	GUIElement* m_world_column;
	GUIVSlider* m_world_slider;
	MessageConsumer m_messenger;
public:
	PlayWindow(const MessageConsumer& c);

	void setWorlds(const std::vector<WorldInfoData>& m_worlds);
};

class PauseWindow :public GUIWindow
{
	MessageConsumer m_messenger;
public:
	PauseWindow(const MessageConsumer& c);

};
