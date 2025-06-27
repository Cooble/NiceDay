#pragma once
#include "ndpch.h"

#include "layer/Layer.h"
#include "scene/NewScene.h"
#include "types.h"


namespace nd {class EditorLayer;}


struct Ground
{
	std::vector<gfloat> terrain_height;
	std::vector<gfloat> water_height;
	std::vector<gfloat> sediment;
	std::vector<gvec4> flux;
	std::vector<gvec2> velocity;

	int width, height;

	void resize(int size)
	{
		width = size;
		height = size;
		auto sq = width * width;
		terrain_height.resize(sq);
		water_height.resize(sq);
		sediment.resize(sq);
		flux.resize(sq);
		velocity.resize(sq);

		ZeroMemory(terrain_height.data(), terrain_height.size()*sizeof(decltype(terrain_height)::value_type));
		ZeroMemory(water_height.data(), water_height.size() * sizeof(decltype(water_height)::value_type));
		ZeroMemory(sediment.data(), sediment.size() * sizeof(decltype(sediment)::value_type));
		ZeroMemory(flux.data(), flux.size() * sizeof(decltype(flux)::value_type));
		ZeroMemory(velocity.data(), velocity.size() * sizeof(decltype(velocity)::value_type));
	}
};
class TerrainLayer:public nd::Layer
{
private:
	nd::EditorLayer& m_editorLayer;
	nd::Entity m_entity;
	nd::Entity m_water_entity;
	Ground g;

public:
	TerrainLayer(nd::EditorLayer&);
	void onAttach() override;
	void onDetach() override;
	void onImGuiRender() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	void onUpdate() override;
	void createGround();
	void createMaterial();
	void simulate(Ground& g,gfloat delta);
};
