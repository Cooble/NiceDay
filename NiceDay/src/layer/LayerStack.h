#pragma once
#include "Layer.h"

class LayerStack
{
public:
	LayerStack();
	~LayerStack();

	void PushLayer(Layer* layer);
	void PushOverlay(Layer* overlay);
	void PopLayer(Layer* layer);
	void PopOverlay(Layer* overlay);

	void popPending();
	void PopLayerEventually(Layer* layer);
	void PopOverlayEventually(Layer* overlay);
	void PushLayerEventually(Layer* layer);
	void PushOverlayEventually(Layer* overlay);

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

