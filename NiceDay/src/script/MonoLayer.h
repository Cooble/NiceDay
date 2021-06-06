#pragma once
#include "layer/Layer.h"
#include "mono/metadata/object-forward.h"

namespace nd {

class MonoLayer : public Layer
{
private:
	MonoObject* callEntryMethod(const char* methodName, void* obj = nullptr, void** params = nullptr,
	                            MonoObject** ex = nullptr);
public:
	bool hotSwapEnable = true;
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onImGuiRender() override;

	void reloadAssembly();
	bool isMonoLoaded();
};
}
