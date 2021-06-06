﻿#pragma once
#include "layer/Layer.h"
#include "gui/GUIContext.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/TextureAtlas.h"
#include "gui/GUICustomRenderer.h"
#include "event/MessageEvent.h"
#include "gui/MainWindow.h"
#include "gui/HUD.h"

enum WindowMess : int;
class MessageEvent;
class WorldLayer;
class GUIEntityPlayer;
class GUIEntityConsole;
class GUIEntityCreativeTab;
class GUILayer : public nd::Layer
{
private:
	enum GameScreen:int{
		GUI,World,Pause
	}m_game_screen;

	nd::GUIContextID m_gui_context;
	GUICustomRenderer m_gui_renderer;
	nd::BatchRenderer2D m_renderer;
	nd::TextureAtlas m_gui_atlas;
	nd::Texture* m_background;
	bool m_background_enable = true;
	std::vector<MessageEvent> m_window_event_buffer;
	MessageConsumer m_bound_func;

	nd::GUIWindow* m_currentWindow=nullptr;
	
	//all ingame gui: inventory, health bar ...
	HUD* m_hud;
	
	WorldLayer* m_world;
	GUIEntityPlayer* m_gui_player;
	GUIEntityConsole* m_gui_console;
	GUIEntityCreativeTab* m_gui_creative=nullptr;
public:
	GUILayer();
	~GUILayer();

	void setWorldLayer(WorldLayer* l);

	HUD& getHUD() { return *m_hud; }
	void updateWorldList();
	void openWindow(WindowMess mess);
	void proccessWindowEvent(const MessageEvent& e);
	void consumeWindowEvent(const MessageEvent& e);
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	void onImGuiRender() override;

};
