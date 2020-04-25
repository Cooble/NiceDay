#pragma once
#include "layer/Layer.h"
#include "graphics/API/Texture.h"


class PriorGenLayer : public Layer
{
private:
	Texture* m_tex;

public:
	PriorGenLayer();


	void onAttach() override;
	void onDetach() override;
	void runInner();
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;
	void onEvent (Event& e) override;
};
