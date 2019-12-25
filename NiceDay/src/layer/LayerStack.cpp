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


void LayerStack::pushLayer(Layer* layer)
{
	m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
	m_LayerInsertIndex++;
	layer->onAttach();

}

void LayerStack::pushOverlay(Layer* overlay)
{
	m_Layers.emplace_back(overlay);
	overlay->onAttach();
}

void LayerStack::popLayer(Layer* layer)
{
	auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
	if (it != m_Layers.end())
	{
		m_Layers.erase(it);
		m_LayerInsertIndex--;
	}
	layer->onDetach();
}

void LayerStack::popOverlay(Layer* overlay)
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
			popOverlay(to_pop.l);
		else if(!to_pop.overlay &&!to_pop.add)
			popLayer(to_pop.l);
		else if (to_pop.overlay && to_pop.add)
			pushOverlay(to_pop.l);
		else if (!to_pop.overlay && to_pop.add)
			pushLayer(to_pop.l);
	}
	m_tasks.clear();
}

void LayerStack::popLayerEventually(Layer* layer)
{
	m_tasks.push_back({ false,false,layer });
}

void LayerStack::popOverlayEventually(Layer* overlay)
{
	m_tasks.push_back({ true,false,overlay });
}

void LayerStack::pushLayerEventually(Layer* layer)
{
	m_tasks.push_back({ false,true,layer });
}

void LayerStack::pushOverlayEventually(Layer* overlay)
{
	m_tasks.push_back({ true,true,overlay });
}
