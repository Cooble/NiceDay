#pragma once
#include "ndpch.h"
#include "event/Event.h"
class Layer
{
protected:
	std::string m_name;
public:
	Layer(const std::string& name = "Layer");
	virtual ~Layer();

	virtual void onAttach() {}
	virtual void onDetach() {}
	virtual void onUpdate() {}
	virtual void onImGuiRender() {}
	virtual void onEvent(Event& e) {}

	inline const std::string& getName() const { return m_name; }
};

