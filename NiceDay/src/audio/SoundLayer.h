#pragma once
#include "layer/Layer.h"


class SoundLayer : public Layer
{
private:
	void* m_stream=nullptr;
public:
	SoundLayer();

	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onImGuiRender() override;
	void onEvent(Event& e) override;
	
};
