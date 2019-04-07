#pragma once
#include "ndpch.h"
#include "event/Event.h"
class Layer
{
protected:
	std::string m_name;
public:
	Layer(std::string name = "Layer");
	virtual ~Layer();

	virtual void onAttach() = 0;
	virtual void onDetach() = 0;
	virtual void onUpdate() = 0;
	virtual void onImGuiRender() = 0;
	virtual void onEvent(Event& e) = 0;

	inline const std::string& getName() { return m_name; }
};

