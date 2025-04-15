#pragma once
#include "layer/Layer.h"
#include "scene/NewScene.h"


namespace nd {class EditorLayer;}


class TerrainLayer:public nd::Layer
{
private:
	nd::EditorLayer& m_editorLayer;
	nd::Entity m_entity;
public:
	TerrainLayer(nd::EditorLayer&);
	void onAttach() override;
	void onDetach() override;
	void onImGuiRender() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	void onUpdate() override;
};
