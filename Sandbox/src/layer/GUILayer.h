#pragma once
#include "layer/Layer.h"
#include "gui/GUIContext.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/TextureAtlas.h"
#include "gui/GUICustomRenderer.h"
#include "event/MessageEvent.h"
#include "gui/MainWindow.h"
#include "gui/HUD.h"

class MessageEvent;
class WorldLayer;
class GUIEntityPlayer;
class GUIEntityConsole;
class GUIEntityCreativeTab;
class GUILayer : public Layer
{
private:
	enum GameScreen:int{
		GUI,World,Pause
	}m_game_screen;
	
	GUIContextID m_gui_context;
	GUICustomRenderer m_gui_renderer;
	BatchRenderer2D m_renderer;
	TextureAtlas m_gui_atlas;
	Texture* m_background;
	bool m_background_enable = true;
	std::vector<MessageEvent> m_window_event_buffer;
	MessageConsumer m_bound_func;

	
	//shows list of worlds to play
	PlayWindow* m_play_window=nullptr;
	//play, settings, quit
	MainWindow* m_main_window=nullptr;
	//world is paused
	PauseWindow* m_pause_window=nullptr;
	ControlsWindow* m_controls_window = nullptr;
	
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

	inline HUD& getHUD() { return *m_hud; }
	void updateWorldList();
	void proccessWindowEvent(const MessageEvent& e);
	void consumeWindowEvent(const MessageEvent& e);
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onEvent(Event& e) override;

};
