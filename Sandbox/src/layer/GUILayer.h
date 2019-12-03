#pragma once
#include "layer/Layer.h"
#include "gui/GUIContext.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/TextureAtlas.h"
#include "gui/GUICustomRenderer.h"
#include "event/MessageEvent.h"
#include "gui/MainWindow.h"

class MessageEvent;
class WorldLayer;
class GUILayer : public Layer
{
private:
	GUIContextID m_gui_context;
	GUICustomRenderer m_gui_renderer;
	BatchRenderer2D m_renderer;
	TextureAtlas m_gui_atlas;
	Texture* m_background;
	bool m_background_enable = true;
	std::vector<MessageEvent> m_window_event_buffer;
	MessageConsumer m_bound_func;

	PlayWindow* m_play_window=nullptr;
	MainWindow* m_main_window=nullptr;
	WorldLayer* m_world;

public:
	GUILayer();
	~GUILayer();

	inline void setWorldLayer(WorldLayer* l) { m_world = l; }
	void updateWorldList();
	void proccessWindowEvent(const MessageEvent& e);
	void consumeWindowEvent(const MessageEvent& e);
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onEvent(Event& e) override;

};
