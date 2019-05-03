#pragma once
#include "Layer.h"
#include "world/World.h"
#include "world/ChunkLoader.h"
#include "ChunkMeshInstance.h"
class ChunkLoadingCam;
class WorldLayer : public Layer
{
private:
	World* m_world;
	ChunkLoader* m_chunk_loader;
	ChunkLoadingCam* m_cam;
	ChunkMeshInstance* m_mesh;
public:
	WorldLayer();
	~WorldLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& e) override;

};

