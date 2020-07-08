#pragma once
#include "layer/Layer.h"

class Scene;
class SceneLayer:public Layer
{
private:
	Scene* m_scene;
public:
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;
	void onEvent(Event& e) override;
};
