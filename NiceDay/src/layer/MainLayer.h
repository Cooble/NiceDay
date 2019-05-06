#pragma once
#include "Layer.h"
class MainLayer: public Layer
{
public:
	MainLayer();
	~MainLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& e) override;

};
