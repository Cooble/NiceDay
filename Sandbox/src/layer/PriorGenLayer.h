#pragma once
#include "layer/Layer.h"
#include "world/gen/PriorGen.h"

class PriorGenLayer : public Layer
{
private:
	PriorGen m_gen;
	Texture* m_tex;

public:
	PriorGenLayer();


	void onAttach() override;
	void runInner();
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;
};
