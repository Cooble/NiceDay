#include "ndpch.h"
#include "LayerStack.h"


LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
	for (Layer* layer : m_Layers) 
		delete layer;
	
}


void LayerStack::PushLayer(Layer* layer)
{
	m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
	m_LayerInsertIndex++;
	layer->onAttach();

}

void LayerStack::PushOverlay(Layer* overlay)
{
	m_Layers.emplace_back(overlay);
	overlay->onAttach();
}

void LayerStack::PopLayer(Layer* layer)
{
	auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
	if (it != m_Layers.end())
	{
		m_Layers.erase(it);
		m_LayerInsertIndex--;
	}
	layer->onDetach();
}

void LayerStack::PopOverlay(Layer* overlay)
{
	auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
	if (it != m_Layers.end())
		m_Layers.erase(it);
	overlay->onDetach();

}

void LayerStack::popPending()
{
	for (auto to_pop : m_tasks)
	{
		if (to_pop.overlay&&!to_pop.add)
			PopOverlay(to_pop.l);
		else if(!to_pop.overlay &&!to_pop.add)
			PopLayer(to_pop.l);
		else if (to_pop.overlay && to_pop.add)
			PushOverlay(to_pop.l);
		else if (!to_pop.overlay && to_pop.add)
			PushLayer(to_pop.l);
	}
	m_tasks.clear();
}

void LayerStack::PopLayerEventually(Layer* layer)
{
	m_tasks.push_back({ false,false,layer });
}

void LayerStack::PopOverlayEventually(Layer* overlay)
{
	m_tasks.push_back({ true,false,overlay });
}

void LayerStack::PushLayerEventually(Layer* layer)
{
	m_tasks.push_back({ false,true,layer });
}

void LayerStack::PushOverlayEventually(Layer* overlay)
{
	m_tasks.push_back({ true,true,overlay });
}
