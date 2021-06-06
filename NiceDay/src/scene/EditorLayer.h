﻿#pragma once
#include "layer/Layer.h"

namespace nd {

class NewScene;

class EditorLayer : public Layer
{
private:
	NewScene* m_scene;
public:
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;
	void onEvent(Event& e) override;

	float getCurrentDepth();
	void onWindowResize(int width, int height) override;
};
}
