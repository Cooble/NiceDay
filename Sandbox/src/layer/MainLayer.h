#pragma once
#include "layer/Layer.h"
class MainLayer: public nd::Layer
{
public:
	MainLayer();
	~MainLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onImGuiRender() override;
	virtual void onEvent(nd::Event& e) override;

};

