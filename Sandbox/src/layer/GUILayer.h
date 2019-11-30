#pragma once
#include "layer/Layer.h"
#include "gui/GUIContext.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/TextureAtlas.h"
#include "gui/GUICustomRenderer.h"

class GUILayer : public Layer
{
private:
	GUIContextID m_gui_context;
	GUICustomRenderer m_gui_renderer;
	BatchRenderer2D m_renderer;
	TextureAtlas m_gui_atlas;
	Texture* m_background;

public:
	GUILayer();
	~GUILayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onEvent(Event& e) override;

};
