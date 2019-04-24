#pragma once
#include "Layer.h"
#include "world/World.h"
class WorldLayer : public Layer
{
private:
	World* m_world;
public:
	WorldLayer();
	~WorldLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& e) override;

};

