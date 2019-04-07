#pragma once
#include "Layer.h"
class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override {};
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& e) override {};

	void begin();
	void end();
};

