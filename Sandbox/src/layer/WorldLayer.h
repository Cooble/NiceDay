#pragma once
#include "layer/Layer.h"
#include "world/World.h"
#include "world/ChunkLoader.h"
#include "world/WorldRenderManager.h"
#include "world/Camera.h"
#include "graphics/Sprite.h"
#include "world/entity/entities.h"
#include "graphics/ParticleRenderer.h"


class BatchRenderer2D;
class WorldLayer : public Layer
{
private:
	std::vector<Sprite*> m_sprites;
	BatchRenderer2D* m_batch_renderer;
	ParticleRenderer* m_particle_renderer;
	World* m_world;
	ChunkLoader* m_chunk_loader;
	//ChunkMeshInstance* m_mesh;
	WorldRenderManager* m_render_manager;
	Camera* m_cam;

	void registerEverything();

public:
	WorldLayer();
	EntityPlayer& getPlayer();
	~WorldLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate() override;
	virtual void onRender() override;
	virtual void onImGuiRender() override;
	void onImGuiRenderTelemetrics();
	void onImGuiRenderWorld();
	void onImGuiRenderChunks();
	virtual void onEvent(Event& e) override;
	

};

