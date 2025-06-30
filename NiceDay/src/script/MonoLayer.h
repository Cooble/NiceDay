#pragma once
#include "layer/Layer.h"

namespace nd {

class MonoLayer : public Layer
{
private:
	bool is_mono_loaded = false;
public:
	bool hotSwapEnable = true;
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	//void onImGuiRender() override;

	//void reloadAssembly();
	bool isMonoLoaded() const;
};
}
