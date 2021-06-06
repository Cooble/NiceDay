#pragma once
#include "layer/Layer.h"
#include "world/World.h"
#include "world/ChunkLoader.h"
#include "world/WorldRenderManager.h"
#include "world/Camera.h"
#include "graphics/Sprite.h"
#include "world/entity/entities.h"
#include "graphics/ParticleRenderer.h"


class EntityPlayer;
class WorldLayer : public nd::Layer
{
private:
	std::vector<nd::Sprite*> m_sprites;
	nd::BatchRenderer2D* m_batch_renderer;
	nd::ParticleRenderer* m_particle_renderer;
	World* m_world=nullptr;
	ChunkLoader* m_chunk_loader;
	
	//ChunkMeshInstance* m_mesh;
	WorldRenderManager* m_render_manager;
	Camera* m_cam;

	bool m_paused;
	bool m_has_world = false;

public:
	WorldLayer();
	void loadResources();
	void loadWorld(nd::temp_string& worldname,bool regen);
	EntityPlayer& getPlayer();
	~WorldLayer();

	inline World* getWorld() { return m_world; }
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
	void onImGuiRenderWorld();
	void onImGuiRenderChunks();
	virtual void onEvent(nd::Event& e) override;
	virtual void onCreativeEvent(nd::Event& e);
	virtual void onSurvivalEvent(nd::Event& e);
	

};

