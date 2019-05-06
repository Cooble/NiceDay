#pragma once
#include "Layer.h"
#include "world/World.h"
#include "world/ChunkLoader.h"
#include "world/ChunkMeshInstance.h"
#include "world/WorldRenderManager.h"
#include "entity/Camera.h"

class WorldLayer : public Layer
{
private:
	World* m_world;
	ChunkLoader* m_chunk_loader;
	//ChunkMeshInstance* m_mesh;
	WorldRenderManager* m_render_manager;
	Camera* m_cam;
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

