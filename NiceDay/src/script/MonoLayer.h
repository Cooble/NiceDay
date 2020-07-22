#pragma once
#include "layer/Layer.h"

class MonoLayer:public Layer
{
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onImGuiRender() override;
};


