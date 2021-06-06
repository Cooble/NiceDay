#pragma once
#include "gui/GUIBasic.h"
#include "world/WorldsProvider.h"
#include "event/MessageEvent.h"

class GameFonts {
public:
	static nd::FontMaterial* bigFont;
	static nd::FontMaterial* smallFont;
};

class MainWindow:public nd::GUIWindow
{
private:
	nd::GUIImage* m_logo;
	MessageConsumer m_messenger;
public:
	MainWindow(const MessageConsumer& c);
	void update() override;
	
};

class GUIWorldEntry:public nd::GUIBlank
{
	nd::GUIText* m_world_name;
	MessageConsumer* m_messenger;
	
public:
	GUIWorldEntry(MessageConsumer* c);

	void setWorldName(const std::string& name);
	const std::string& getWorldName();
		
};


class SelectWorldWindow :public nd::GUIWindow
{
private:
	std::vector<WorldInfoData> m_worlds;
	GUIElement* m_world_column;
	nd::GUIVSlider* m_world_slider;
	MessageConsumer m_messenger;
public:
	SelectWorldWindow(const MessageConsumer& c);

	void setWorlds(const std::vector<WorldInfoData>& m_worlds);
};

class PauseWindow :public nd::GUIWindow
{
	MessageConsumer m_messenger;
public:
	PauseWindow(const MessageConsumer& c);

};
class ControlsWindow :public nd::GUIWindow
{
	MessageConsumer m_messenger;
public:
	ControlsWindow(const MessageConsumer& c);

};
class SettingsWindow :public nd::GUIWindow
{
	MessageConsumer m_messenger;
public:
	SettingsWindow(const MessageConsumer& c);

};
class LanguageWindow :public nd::GUIWindow
{
	MessageConsumer m_messenger;
public:
	LanguageWindow(const MessageConsumer& c);

};
class SkinWindow :public nd::GUIWindow
{
	MessageConsumer m_messenger;
public:
	SkinWindow(const MessageConsumer& c);
	void onMyEvent(nd::Event& e) override;

};
