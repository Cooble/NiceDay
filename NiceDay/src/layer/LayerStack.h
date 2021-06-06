#pragma once
#include "Layer.h"

namespace nd {

class LayerStack
{
public:
	LayerStack();
	~LayerStack();

	void pushLayer(Layer* layer);
	void pushOverlay(Layer* overlay);
	void popLayer(Layer* layer);
	void popOverlay(Layer* overlay);

	void popPending();
	void popLayerEventually(Layer* layer);
	void popOverlayEventually(Layer* overlay);
	void pushLayerEventually(Layer* layer);
	void pushOverlayEventually(Layer* overlay);

	std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
	std::vector<Layer*>::iterator end() { return m_Layers.end(); }
private:
	std::vector<Layer*> m_Layers;

	struct Task
	{
		bool overlay;
		bool add;
		Layer* l;
	};

	std::vector<Task> m_tasks;
	unsigned int m_LayerInsertIndex = 0;
};
}
