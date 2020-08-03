#pragma once
#include "layer/Layer.h"

class TestEnntLayer :public Layer
{
private:

public:
	TestEnntLayer() = default;

	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;

	
};