#pragma once
#include "layer/Layer.h"
#include "world/World.h"
#include "world/ChunkLoader.h"
#include "world/WorldRenderManager.h"
#include "world/Camera.h"
#include "graphics/Sprite.h"
#include "world/entity/entities.h"
#include "graphics/ParticleRenderer.h"
#include "graphics/TextureAtlas.h"
#include "graphics/BlockTextureAtlas.h"


class BatchRenderer2D;
class EntityPlayer;
class WorldLayer : public Layer
{
private:
	std::vector<Sprite*> m_sprites;
	BatchRenderer2D* m_batch_renderer;
	ParticleRenderer* m_particle_renderer;
	World* m_world=nullptr;
	ChunkLoader* m_chunk_loader;
	TextureAtlas m_item_atlas;
	BlockTextureAtlas m_block_atlas;
	//ChunkMeshInstance* m_mesh;
	WorldRenderManager* m_render_manager;
	Camera* m_cam;

	bool m_paused;
	bool m_has_world = false;
	void registerEverything();
	void registerItems();
	void registerBlocks();
	void registerEntities();
	void registerBiomes();
	void registerParticles();

public:
	WorldLayer();
	void loadResources();
	void loadWorld(nd::temp_string& worldname,bool regen);
	EntityPlayer& getPlayer();
	~WorldLayer();

	inline World* getWorld() { return m_world; }
	inline const TextureAtlas& getItemAtlas()const { return m_item_atlas; }
	void pause(bool pause);
	bool isPaused() const;
	virtual void onAttach() override;
	void onWorldLoaded();
	void loadLuaWorldLibs();
	void afterPlayerLoaded();
	void onDetach() override;
	void onUpdate() override;
	void onCreativeUpdate();
	void onSurvivalUpdate();
	void onRender() override;
	void onImGuiRender() override;
	void onImGuiRenderTelemetrics();
	void onImGuiRenderWorld();
	void onImGuiRenderChunks();
	virtual void onEvent(Event& e) override;
	virtual void onCreativeEvent(Event& e);
	virtual void onSurvivalEvent(Event& e);
	

};

