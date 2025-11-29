#pragma once
#include "DropletSim.h"
#include "ndpch.h"
#include "EulerSim.h"
#include "layer/Layer.h"
#include "scene/NewScene.h"
#include "types.h"

namespace nd { class EditorLayer; }

enum class TerrainLayerType
{
	FLAT,
	SINE,
	PERLIN,
	BASIN
};

// Forward declaration
struct TerrainMesh;

// Encapsulates a single terrain instance with its own ground and mesh
struct TerrainInstance
{
	EulerGround ground;
	std::unique_ptr<TerrainMesh> mesh;
	nd::MaterialPtr m_terrain_material;
	nd::MaterialPtr m_water_material;
	nd::Entity terrain_entity;
	nd::Entity water_entity;
	Euler euler_sim;

	void init(int size, Euler::EulerSettings* s);
	void recreateTerrain(TerrainLayerType type);
	void refreshMeshData();
};

class TerrainLayer : public nd::Layer
{
private:
	nd::EditorLayer& m_editor_layer;
	nd::Entity m_sphere;
	Droplet m_droplet;

	Euler::EulerSettings m_euler_settings;

	// Two terrain instances for side-by-side comparison
	TerrainInstance m_terrain_vanilla;
	TerrainInstance m_terrain_simd;

	// UI/Simulation state
	bool m_enable_simd = false;
	bool m_enable_vanilla = true; 
	int m_ground_size = 514;
	TerrainLayerType m_terrain_type = TerrainLayerType::FLAT;

public:
	TerrainLayer(nd::EditorLayer&);
	void onAttach() override;
	void onImGuiRender() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	void onUpdate() override;

private:
	void createMaterials();
	void createTerrainInstances();
	void recreateBothTerrains();
	void calculateStatistics(EulerGround& g);
	void simulate();
	void runDropBenchmark(int passes);
	void onImGuiRenderSimulator();
	void onImGuiRenderGenerator();

	// Helper to get the currently active terrain for mouse interaction
	TerrainInstance& getActiveTerrainInstance();
};