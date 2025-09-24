#pragma once
#include "DropletSim.h"
#include "ndpch.h"

#include "EulerSim.h"

#include "layer/Layer.h"
#include "scene/NewScene.h"
#include "types.h"


namespace nd {class EditorLayer;}



class TerrainLayer:public nd::Layer
{
private:
	nd::EditorLayer& m_editorLayer;
	nd::Entity m_entity;
	nd::Entity m_water_entity;
	nd::Entity m_sphere;
	Droplet m_droplet;
	Euler m_euler;
	EulerGround g;

public:
	TerrainLayer(nd::EditorLayer&);
	void onAttach() override;
	void onImGuiRender() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	void onUpdate() override;

private:
	void recreateTerrain(EulerGround& g);
	void calculateStatistics(EulerGround& g);
	void createGround();
	void createMaterial();
	void simulate(EulerGround& g);
	void runDropBenchmark(int passes);
	void onImGuiRenderSimulator();
	void onImGuiRenderGenerator();

};
