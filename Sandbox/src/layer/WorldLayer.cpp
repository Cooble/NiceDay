#include "ndpch.h"
#include "WorldLayer.h"
#include "world/World.h"
#include "world/block/BlockRegistry.h"
#include "world/LightCalculator.h"
#include "world/WorldIO.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "core/Stats.h"
#include <GLFW/glfw3.h>

#include "world/biome/BiomeForest.h"

#include <imgui.h>
#include "world/block/Block.h"

#include "graphics/BatchRenderer2D.h"
#include "graphics/Sprite.h"
#include "world/entity/EntityRegistry.h"
#include "world/entity/entity_datas.h"
#include "world/entity/entities.h"
#include "graphics/TextureManager.h"
#include "world/particle/particles.h"
#include "world/ChunkMesh.h"
#include "core/imgui_utils.h"
#include "inventory/Item.h"
#include "graphics/GContext.h"
#include "core/AppGlobals.h"
#include "event/MessageEvent.h"
#include "CommonMessages.h"
#include "lua/LuaLayer.h"
#include <lua.hpp>
#include "world/entity/EntityPlayer.h"
#include "world/entity/EntityAllocator.h"
#include "inventory/ItemBlock.h"
#include "world/block/block_datas.h"
#include "graphics/BlockTextureCreator.h"
#include "event/SandboxControls.h"
#include "audio/player.h"
#include "world/nd_registry.h"
#include "gui/GUIContext.h"

const char* WORLD_FILE_PATH;
int CHUNKS_LOADED;
int CHUNKS_DRAWN;
int WORLD_CHUNK_WIDTH;
int WORLD_CHUNK_HEIGHT;
std::string CURRENT_BLOCK_ID;
const BlockStruct* CURRENT_BLOCK;
float CURSOR_X;
float CURSOR_Y;

float CAM_POS_X;
float CAM_POS_Y;

static int BLOCK_PALLETE_SELECTED = 0;
static int ENTITY_PALLETE_SELECTED = 0;
static bool BLOCK_OR_WALL_SELECTED = true;
static void* playerBuff;
static EntityID playerID;
static SpriteSheetResource* res;

static bool m_is_world_ready = false;
static bool s_no_save = false;

constexpr int particleAtlasSize = 8;
constexpr int ITEM_ATLAS_SIZE = 16;

WorldLayer::WorldLayer()
	: Layer("WorldLayer")
{
	loadResources();
}

void WorldLayer::loadResources()
{
	ND_PROFILE_METHOD();

	nd_registry::registerEverything(true);


	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/borderBox.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 1, 1);

	Stats::bound_sprite = new Sprite(&res);
	Stats::bound_sprite->setSpriteIndex(0, 0);
	Stats::bound_sprite->setPosition(glm::vec3(0, 0, 0));
	Stats::bound_sprite->setSize(glm::vec2(1, 1));
}

void WorldLayer::loadWorld(nd::temp_string& worldname, bool regen)
{
	m_has_world = true;

	m_cam = new Camera();
	m_cam->setChunkRadius({6, 6});


	m_batch_renderer = new BatchRenderer2D();
	m_particle_renderer = new ParticleRenderer();

	//world===================================================
	WorldInfo info;

	strcpy_s(info.name, worldname.c_str());
	info.chunk_width = 50;
	info.chunk_height = 10;
	info.seed = 0;
	info.terrain_level = (info.chunk_height - 4) * WORLD_CHUNK_SIZE;
	info.time = TICKS_PER_MINUTE * 60 * 7;//start day at 7 on clock
	m_world = new World("worlds/" + std::string(info.name) + ".world", info);

	bool worldAlreadyLoaded = true;

	if (regen)
	{
		m_world->genWorld();
		ND_INFO("Generated world: {}", m_world->getFilePath());
	}
	else
	{
		auto world = m_world;
		auto job = m_world->loadWorld();
		if (job == nullptr)
		{
			ND_INFO("World is missing, generating new one: {}", m_world->getFilePath());
			m_world->genWorld();
		}
		else
		{
			worldAlreadyLoaded = false;
			ND_SCHED.callWhenDone([this, job, world, info]()
				{
					if (job->m_variable != JobAssignment::JOB_SUCCESS)
					{
						ND_INFO("World is corrupted: {}.world, generating new one", std::string(info.name));
						world->genWorld();
					}
					else
						ND_INFO("World loaded: {}.world", world->getFilePath());
					onWorldLoaded();
				}, job);
		}
	}
	if (worldAlreadyLoaded)
		onWorldLoaded();
}

EntityPlayer& WorldLayer::getPlayer()
{
	auto p = dynamic_cast<EntityPlayer*>(m_world->getEntityManager().entity(playerID));
	ASSERT(p, "Player is not loaded");
	return *p;
}

WorldLayer::~WorldLayer()
{
	//this does ondetach
	/*delete m_chunk_loader;
	delete m_world;
	delete m_batch_renderer;
	delete m_particle_renderer;
	delete m_render_manager;*/
}

void WorldLayer::pause(bool pause)
{
	m_paused = pause;
}

bool WorldLayer::isPaused() const
{
	return m_paused;
}

void WorldLayer::onAttach()
{
	m_paused = false;

	//prepare lua world interface
	LuaLayer& lua = *App::get().getLua();
	auto L = lua.getLuaState();
}


//called after world was gen or loaded
void WorldLayer::onWorldLoaded()
{
	Stats::world = m_world;
	m_chunk_loader = new ChunkLoader(m_world);

	m_render_manager = new WorldRenderManager(m_cam, m_world);
	Texture* particleAtlasT = Texture::create(
		TextureInfo("res/images/particleAtlas/atlas.png")
		.filterMode(TextureFilterMode::LINEAR)
		.format(TextureFormat::RGBA));
	Texture* blockAtlas = Texture::create(
		TextureInfo("res/images/blockAtlas/atlas.png")
		.filterMode(TextureFilterMode::NEAREST)
		.format(TextureFormat::RGBA));
	*m_world->particleManager() = new ParticleManager(5000, particleAtlasT, particleAtlasSize, blockAtlas,
	                                                  BLOCK_TEXTURE_ATLAS_SIZE);

	//load entity manager
	if (m_world->getWorldNBT().exists("playerID"))
	{
		m_world->getWorldNBT().load("playerID",playerID);
		ChunkID chunk = m_world->getWorldNBT()["player_chunkID"];
		m_world->loadChunk( //load chunk where player is
			half_int::X(chunk),
			half_int::Y(chunk));
		/*if(m_world->getLoadedEntity(playerID)==nullptr)
		{
			ND_ERROR("corrupted world file:(");
			ND_WAIT_FOR_INPUT;
		exit(1);
		}*/
		int timeout = App::get().getTPS() * 1; //wait n seconds to load entity
		ND_SCHED.runTaskTimer([this, timeout]() mutable
			{
				if (m_world->getLoadedEntity(playerID))
				{
					afterPlayerLoaded();
					return true;
				}
				else
				{
					if (timeout-- == 0)
					{
						ND_ERROR("World save was corrupted, cannot load player");
						return true;
					}
				}
				return false;
			});
	}
	else
	{
		ND_INFO("No player found, creating new one");
		auto buff = (EntityItem*)EntityAllocator::createEntity(ENTITY_TYPE_PLAYER);

		playerID = m_world->spawnEntity((WorldEntity*)buff);
		getPlayer().getPosition() = {
			m_world->getInfo().chunk_width * WORLD_CHUNK_SIZE / 2, m_world->getInfo().terrain_level
		};
		afterPlayerLoaded();
	}
}

static World* s_lua_world_ref = nullptr;

static World& luaGetWorldRef()
{
	ASSERT(s_lua_world_ref, "World is nullptr");
	return *s_lua_world_ref;
}

static WorldEntity* s_lua_player_ref = nullptr;

static WorldEntity& luaGetPlayerRef()
{
	ASSERT(s_lua_player_ref, "Player is nullptr");
	return *s_lua_player_ref;
}

void WorldLayer::loadLuaWorldLibs()
{
	auto L = App::get().getLua()->getLuaState();


	/*luabridge::getGlobalNamespace(L)
		.beginNamespace("Particle")
		.addVariable("torch_fire", &ParticleList::torch_fire, false)
		.addVariable("bulletShatter", &ParticleList::bulletShatter, false)
		.addVariable("dot", &ParticleList::dot, false)
		.addVariable("line", &ParticleList::line, false)
		.addVariable("torch_smoke", &ParticleList::torch_smoke, false)
		.addVariable("note", &ParticleList::note, false)
		.endNamespace();

	//world wrapper
	luabridge::getGlobalNamespace(L)
		.beginClass<World>("worldClass")
		.addFunction("getName", &World::getName)
		.addFunction("spawnParticle", &World::spawnParticle)
		.endClass();


	s_lua_player_ref = &getPlayer();
	//player wrapper
	luabridge::getGlobalNamespace(L)
		.beginClass<WorldEntity>("playerClass")
		.addFunction("getPosition", (glm::vec2& (WorldEntity::*)())&WorldEntity::getPosition)
		.endClass();


	s_lua_world_ref = m_world;
	luabridge::getGlobalNamespace(L)
		.addFunction("World", &luaGetWorldRef)
		.addFunction("Player", &luaGetPlayerRef);


	App::get().getLua()->runScriptInConsole(L, "world = World()");
	App::get().getLua()->runScriptInConsole(L, "player = Player()");
	App::get().getLua()->runScriptInConsole(L,
	                                        "function playerPos() "
	                                        "	local po = player:getPosition() "
	                                        "	return VEC2(po.x,po.y)"
	                                        "end");
											
	ND_TRACE("Loaded world lua bindings");*/
}

//called after chunk with player was loaded
void WorldLayer::afterPlayerLoaded()
{
	LightCalculator& c = m_world->getLightCalculator();
	c.run();

	//add camera
	m_cam->setPosition(getPlayer().getPosition());
	c.registerLight(dynamic_cast<LightSource*>(m_cam));

	m_chunk_loader->registerEntity(dynamic_cast<IChunkLoaderEntity*>(m_cam));
	m_is_world_ready = true;

	loadLuaWorldLibs();
}

void WorldLayer::onDetach()
{
	if (m_world == nullptr)
		return;
	m_world->getLightCalculator().stop();
	if (s_no_save)
		return;
	auto pos = getPlayer().getPosition();
	m_world->getWorldNBT().save("player_chunkID", Chunk::getChunkIDFromWorldPos(pos.x, pos.y));
	m_world->getWorldNBT().save("playerID", playerID);

	m_chunk_loader->clearEntities();
	m_chunk_loader->onUpdate(); //this will unload all chunks

	auto job = m_world->saveWorld();
	while (!job->isDone())
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	if (job->isFailure())
		ND_INFO("Cannot save world");
	else
		ND_INFO("World saved");
	ND_SCHED.deallocateJob(job);

	ND_SCHED.update(); //this is really nasty and bad and disgusting
	ND_SCHED.update(); //this is really nasty and bad and disgusting
	ND_INFO("Waiting for save 2 seconds");
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	delete m_chunk_loader;
	delete m_world;
	delete m_batch_renderer;
	delete m_particle_renderer;
	delete m_render_manager;
	m_world = nullptr;
	m_is_world_ready = false;
	m_paused = false;
}

static int fpsCount;

//mouse was pressed and is held rightnow
static bool isDragging = false;


void WorldLayer::onUpdate()
{
	if (m_paused)
		return;
	if (!m_is_world_ready)
		return;

	auto& play = getPlayer();

	m_world->onUpdate();
	m_cam->setPosition(play.getPosition());
	Sounder::get().updateSpatialData(Sounder::PLAYER_ID, { play.getPosition(),{0,0} });
	m_chunk_loader->onUpdate();

	m_cam->m_light_intensity = Stats::player_light_intensity;
	fpsCount++;
	static int lightCalcDelay = 0;
	if (lightCalcDelay)
	{
		lightCalcDelay--;
	}
	else
	{
		m_world->getLightCalculator().snapshot();
		lightCalcDelay = 1;
	}
	auto pair = App::get().getInput().getMouseLocation();
	CURSOR_X = pair.x - App::get().getWindow()->getWidth() / 2;
	CURSOR_Y = -pair.y + App::get().getWindow()->getHeight() / 2;

	CURSOR_X = CURSOR_X / BLOCK_PIXEL_SIZE + m_cam->getPosition().x;
	CURSOR_Y = CURSOR_Y / BLOCK_PIXEL_SIZE + m_cam->getPosition().y;

	getPlayer().setFacingDirection(glm::normalize(glm::vec2(CURSOR_X, CURSOR_Y) - getPlayer().getPosition()));

	if (m_world->isBlockValid(CURSOR_X, CURSOR_Y))
	{
		CURRENT_BLOCK = m_world->getBlockOrAir(CURSOR_X, CURSOR_Y);
		CURRENT_BLOCK_ID = BlockRegistry::get().getBlock(CURRENT_BLOCK->block_id).getStringID();
	}

	bool istsunderBlock = !m_world->isAir(m_cam->getPosition().x, m_cam->getPosition().y - 1);

	if (getPlayer().hasCreative())
		onCreativeUpdate();
	else onSurvivalUpdate();


	//camera movement===================================================================
	glm::vec2 accel = glm::vec2(0, 0);
	accel.y = -9.0f / 60;
	glm::vec2& velocity = play.getVelocity();
	float acc = 0.3f;
	float moveThroughBlockSpeed = 6;

	if (!GUIContext::isAnyItemActive())
	{
		if (Stats::move_through_blocks_enable)
		{
			velocity = glm::vec2(0, 0);
			if (App::get().getInput().isKeyPressed(Controls::GO_RIGHT))
				velocity.x = moveThroughBlockSpeed;
			if (App::get().getInput().isKeyPressed(Controls::GO_LEFT))
				velocity.x = -moveThroughBlockSpeed;


			if (App::get().getInput().isKeyPressed(Controls::GO_UP))
				velocity.y = moveThroughBlockSpeed;
			if (App::get().getInput().isKeyPressed(Controls::GO_DOWN))
				velocity.y = -moveThroughBlockSpeed;
		}

		else
		{
			if (App::get().getInput().isKeyPressed(Controls::GO_RIGHT))
				accel.x = acc;
			if (App::get().getInput().isKeyPressed(Controls::GO_LEFT))
				accel.x = -acc;
			if (App::get().getInput().isKeyPressed(Controls::GO_UP))
			{
				if (istsunderBlock || Stats::fly_enable)
					velocity.y = 10;
			}
			getPlayer().getAcceleration() = accel;
		}
	}
	CAM_POS_X = (m_cam->getPosition().x);
	CAM_POS_Y = (m_cam->getPosition().y);
	CHUNKS_LOADED = m_world->getMap().size();
	CHUNKS_DRAWN = m_render_manager->getMap().size();
	WORLD_FILE_PATH = m_world->getFilePath().c_str();
	WORLD_CHUNK_WIDTH = m_world->getInfo().chunk_width;
	WORLD_CHUNK_HEIGHT = m_world->getInfo().chunk_height;
}

void WorldLayer::onCreativeUpdate()
{
	bool tntenable = true;
	constexpr int maxDeltaBum = 2;
	static int deltaBum = 0;
	if (!GUIContext::isAnyItemActive())
	{
		if (App::get().getInput().isKeyPressed(Controls::SPAWN_TNT))
		{
			if (tntenable)
			{
				if (deltaBum-- == 0)
				{
					deltaBum = maxDeltaBum;
					auto bullet = (EntityRoundBullet*)EntityAllocator::createEntity(ENTITY_TYPE_TNT);
					bullet->getPosition() = getPlayer().getPosition() + glm::vec2(0, 1.5);
					bullet->fire({CURSOR_X, CURSOR_Y}, 60.f / 60);
					bullet->setOwner(getPlayer().getID());
					m_world->spawnEntity(bullet);
				}
			}
		}
		if (App::get().getInput().isKeyFreshlyPressed(Controls::FLY_MODE))
		{
			Stats::fly_enable = !Stats::fly_enable;
			ND_INFO("Fly mode: {}", Stats::fly_enable);
		}
		if (App::get().getInput().isKeyFreshlyPressed(Controls::SPAWN_ENTITY))
		{
			auto entityBuff = malloc(EntityRegistry::get().getBucket(ENTITY_PALLETE_SELECTED).byte_size);
			EntityRegistry::get().createInstance(ENTITY_PALLETE_SELECTED, entityBuff);

			((WorldEntity*)entityBuff)->getPosition() = m_cam->getPosition();
			((WorldEntity*)entityBuff)->getPosition().y += 10;
			m_world->spawnEntity((WorldEntity*)entityBuff);
			ND_TRACE("bum");
		}

		if (App::get().getInput().isKeyPressed(Controls::SPAWN_BULLETS))
		{
			constexpr int BULLET_CADENCE_DELAY = 8;
			static int counter = 0;
			if (counter++ == BULLET_CADENCE_DELAY)
			{
				counter = 0;
				const int BULLET_COUNT = 16;
				for (int i = 0; i < BULLET_COUNT; ++i)
				{
					auto bullet = (EntityRoundBullet*)EntityAllocator::createEntity(ENTITY_TYPE_ROUND_BULLET);
					bullet->getPosition() = m_cam->getPosition() + glm::vec2(0, 10.f);
					bullet->fire(3.14159f * 2 / BULLET_COUNT * i, 50.f / 60);
					m_world->spawnEntity(bullet);
				}
			}
		}

		if (isDragging)
		{
			if (Stats::gun_enable)
			{
				constexpr int BULLET_CADENCE_DELAY = 3;
				static int counter = 0;
				if (counter++ == BULLET_CADENCE_DELAY)
				{
					counter = 0;
					auto bullet = (EntityRoundBullet*)EntityAllocator::createEntity(ENTITY_TYPE_ROUND_BULLET);
					bullet->getPosition() = m_cam->getPosition() + glm::vec2(0, 1.f);
					bullet->fire({CURSOR_X, CURSOR_Y}, 50.f / 60);
					bullet->setOwner(getPlayer().getID());
					m_world->spawnEntity(bullet);
				}
			}
			else if (BLOCK_OR_WALL_SELECTED)
			{
				auto& str = *m_world->getBlockOrAir(CURSOR_X, CURSOR_Y);
				if (str.block_id != BLOCK_PALLETE_SELECTED && BlockRegistry::get()
				                                              .getBlock(BLOCK_PALLETE_SELECTED).canBePlaced(
					                                              *m_world, CURSOR_X, CURSOR_Y))
					m_world->setBlockWithNotify(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
			}

			else
			{
				if (m_world->getBlock(CURSOR_X, CURSOR_Y)->isWallOccupied())
				{
					if (m_world->getBlock(CURSOR_X, CURSOR_Y)->wallID() != BLOCK_PALLETE_SELECTED)
						m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
				}
				else
					m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
			}
		}
	}
}

void WorldLayer::onSurvivalUpdate()
{
	if (isDragging)
	{
		auto& inHand = getPlayer().getInventory().itemInHand();
		getPlayer().setItemSwinging(false);

		if (inHand)
		{
			auto& item = inHand->getItem();
			if (item.isBlock())
			{
				auto& itemBlock = dynamic_cast<const ItemBlock&>(item);
				auto blok = m_world->getBlockM(CURSOR_X, CURSOR_Y);
				if (blok == nullptr)
					return;

				auto blokid = item.getBlockID();
				if (blokid >= 0)
				{
					if (blok->block_id == item.getBlockID())
						return;
					if (!BlockRegistry::get().getBlock(blokid).canBePlaced(*m_world, CURSOR_X, CURSOR_Y))
						return;
					BlockStruct stru = {};
					stru.block_id = itemBlock.getBlockID();
					stru.block_metadata = itemBlock.getBlockMetadata(inHand);
					m_world->setBlockWithNotify(CURSOR_X, CURSOR_Y, stru);

					inHand->addSize(-1);
					if (inHand->size() == 0)
					{
						inHand->destroy();
						inHand = nullptr;
					}
					m_world->getBlockM(CURSOR_X, CURSOR_Y)->block_metadata = stru.block_metadata;
					return;
				}
			}


			item.onBlockBeingDigged(*m_world, *inHand, getPlayer(), CURSOR_X, CURSOR_Y);
			getPlayer().setItemSwinging(true);
			getPlayer().setFacingDir(getPlayer().getFacingDirection().x < 0);
			auto structInWorld = m_world->getBlockM(CURSOR_X, CURSOR_Y);

			if (structInWorld->block_id == BLOCK_AIR)
				return;
			auto& blok = BlockRegistry::get().getBlock(structInWorld->block_id);

			auto efficiency = item.getEfficiencyOnBlock(blok, inHand);
			if (efficiency == 0)
				return;
			auto t = blok.createItemStackFromBlock(*structInWorld);
			if (t == nullptr)
			{
				ASSERT(false, "Cannot spawn item from block: {}", blok.getStringID());
				return;
			}
			auto itemEntity = (EntityItem*)EntityAllocator::createEntity(ENTITY_TYPE_ITEM);
			itemEntity->setItemStack(t);
			itemEntity->getPosition() = glm::vec2((int)CURSOR_X + 0.5f, (int)CURSOR_Y + 0.5f);
			itemEntity->getVelocity() = {0, 5 / 60.f};
			m_world->spawnBlockBreakParticles(CURSOR_X, CURSOR_Y);
			m_world->setBlockWithNotify(CURSOR_X, CURSOR_Y, 0);
			m_world->spawnEntity(itemEntity);
			return;


			auto& entities = m_world->getLoadedEntities();
			bool foundEntity = false;
			for (auto e : entities)
			{
				auto entity = dynamic_cast<PhysEntity*>(m_world->getEntityManager().entity(e));
				if (entity)
					if (Phys::contains(entity->getCollisionBox(), {
						                   CURSOR_X - entity->getPosition().x, CURSOR_Y - entity->getPosition().y
					                   }))
					{
						item.hitEntity(*m_world, inHand, getPlayer(), *entity);
						foundEntity = true;
						break;
					}
			}
		}
		getPlayer().setItemSwinging(false);
	}
}

void WorldLayer::onRender()
{
	ND_PROFILE_METHOD();
	if (m_world == nullptr)
		return;
	Gcon.enableDepthTest(false);
	if (!m_is_world_ready)
		return;
	{
		ND_PROFILE_SCOPE("rendeerupdate");
		m_render_manager->update();
	}
	//world
	m_render_manager->render(*m_batch_renderer, Renderer::getDefaultFBO());
	
	//entities
	auto entityFbo = m_render_manager->getEntityFBO();
	entityFbo->bind();
	entityFbo->clear(BuffBit::COLOR, { 0,0,0,0 });
	
	Gcon.enableBlend();
	Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-m_cam->getPosition().x, -m_cam->getPosition().y, 0));

	m_batch_renderer->begin(m_render_manager->getEntityFBO());
	m_batch_renderer->push(m_render_manager->getProjMatrix() * worldMatrix);

	for (auto it = m_world->beginEntities(); it != m_world->endEntities(); ++it)
	{
		WorldEntity* entity = m_world->getLoadedEntity(*it);
		auto ren = dynamic_cast<IBatchRenderable2D*>(entity);
		if (ren)
			ren->render(*m_batch_renderer);
	}

	m_batch_renderer->pop();
	m_batch_renderer->flush();
	m_render_manager->applyLightMap(m_render_manager->getLightTextureBlur(),entityFbo);
	entityFbo->unbind();

	Gcon.enableBlend();
	glDisable(GL_DEPTH_TEST);
	Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	Effect::render(m_render_manager->getEntityFBO()->getAttachment(),Renderer::getDefaultFBO());

	{
		ND_PROFILE_SCOPE("particle render");
		//particles
		//Gcon.enableDepthTest(true);
		m_particle_renderer->begin(Renderer::getDefaultFBO());
		//m_particle_renderer->submit({ -1,-1,0 }, { 2,2 }, UVQuad::elementary(), UVQuad::elementary(), m_render_manager->getLightTextureSmooth(),0);
		
		m_particle_renderer->push(m_render_manager->getProjMatrix() * worldMatrix);
		(*m_world->particleManager())->render(*m_particle_renderer);
		m_particle_renderer->pop();
		m_particle_renderer->flush();
		
		//Gcon.enableDepthTest(false);
	}
}

static bool showTelem = false;
static bool showChunks = false;

void WorldLayer::onImGuiRender()
{
	if (m_world == nullptr/*||isPaused()*/)
		return;
	onImGuiRenderWorld();

	if (showTelem)
	{
		onImGuiRenderTelemetrics();
	}
	if (showChunks)
	{
		onImGuiRenderChunks();
	}
}

void WorldLayer::onImGuiRenderTelemetrics()
{
	static bool showTelemetrics = false;
	if (!ImGui::Begin("Telemetrics", &showTelemetrics, ImGuiWindowFlags_NoNav))
	{
		ImGui::End();
		return;
	}
	if (Stats::light_enable)
	{
		ImGui::PlotVar("Light millis", Stats::light_millis, true);
	}
	ImGui::End();
}

void WorldLayer::onImGuiRenderWorld()
{
	if (m_world == nullptr)
		return;
	static bool showBase = true;

	if (!ImGui::Begin("WorldInfo", &showBase))
	{
		ImGui::End();
		return;
	}
	ImGui::Checkbox("Show Telemetrics", &showTelem);
	ImGui::Checkbox("Show Chunks", &showChunks);
	ImGui::Checkbox("Show CollisionBox", &Stats::show_collisionBox);
	/*BiomeDistances d = Stats::biome_distances;
	ImGui::Text("Biomes:");
	for (int i = 0; i < 4; ++i)
	{
		ImGui::Text("* %d, %.2f", d.biomes[i], d.intensities[i]);

	}*/
	bool l = Stats::light_enable;
	ImGui::Checkbox(Stats::light_enable ? "Lighting Enabled" : "Lighting Disabled", &Stats::light_enable);
	if (l != Stats::light_enable)
	{
		//if (!l) m_world->getLightCalculator().run();
		//else m_world->getLightCalculator().stop();
	}

	if (m_is_world_ready)
	{
		bool ll = getPlayer().hasCreative();
		ImGui::Checkbox(ll ? "Creative" : "Survival", &ll);
		if (ll != getPlayer().hasCreative())
		{
			getPlayer().setCreative(ll);
		}
	}

	ImGui::Checkbox(Stats::move_through_blocks_enable ? "Move through Blocks Enabled" : "Move through Blocks Disabled",
	                &Stats::move_through_blocks_enable);
	l = BLOCK_OR_WALL_SELECTED;
	ImGui::Checkbox(BLOCK_OR_WALL_SELECTED ? "BLOCK mode" : "WALL mode", &BLOCK_OR_WALL_SELECTED);
	if (l != BLOCK_OR_WALL_SELECTED)
		BLOCK_PALLETE_SELECTED = 0;
	ImGui::Checkbox(Stats::gun_enable ? "Gun Enabled" : "Gun Disabled", &Stats::gun_enable);
	ImGui::InputFloat("Player speed: ", &Stats::player_speed);
	ImGui::InputFloat("Light Intensity: ", &Stats::player_light_intensity);

	ImGui::InputFloat("Debug X: ", &Stats::edge_scale);
	ImGui::SliderFloat("slider float", &Stats::edge_scale, 0.0f, 1.0f, "ratio = %.2f");


	ImGui::Text("World filepath: %s", m_world->getFilePath());
	ImGui::Text("WorldTime: (%d) %d:%d", m_world->getWorldTime().m_ticks, (int)m_world->getWorldTime().hour(),
	            (int)m_world->getWorldTime().minute());
	ImGui::SliderFloat("Time speed", &m_world->m_time_speed, 0.f, 25.0f, "ratio = %.2f");
	ImGui::Text("Cam X: %.2f \t(%d)", CAM_POS_X, (int)CAM_POS_X >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cam Y: %.2f \t(%d)", CAM_POS_Y, (int)CAM_POS_Y >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor X: %.2f \t(%d)", CURSOR_X, (int)CURSOR_X >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor y: %.2f \t(%d)", CURSOR_Y, (int)CURSOR_Y >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Separator();
	ImGui::Text("Loaded entities: %d", m_world->getLoadedEntities().size());
	ImGui::Text("particles: %d", Stats::particle_count);
	ImGui::Text("Chunks Size: (%d/%d)", WORLD_CHUNK_WIDTH, WORLD_CHUNK_HEIGHT);
	ImGui::Text("Chunks loaded: %d", CHUNKS_LOADED);
	ImGui::Text("Chunks drawn: %d", CHUNKS_DRAWN);
	ImGui::Separator();
	ImGui::Text("Dynamic segments: %d", m_world->getNBTSaver().getSegmentCount());
	ImGui::Text("Dynamic free segments: %d", m_world->getNBTSaver().getFreeSegmentCount());

	if (auto current = m_world->getBlock(CURSOR_X, CURSOR_Y))
	{
		CURRENT_BLOCK = current;
		quarter_int metaSections = CURRENT_BLOCK->block_metadata;
		ImGui::Text("Current Block: %s (id: %d, corner: %d, meta: %d ->[%d,%d,%d,%d])", CURRENT_BLOCK_ID.c_str(),
		            CURRENT_BLOCK->block_id,
		            CURRENT_BLOCK->block_corner, CURRENT_BLOCK->block_metadata, metaSections.x, metaSections.y,
		            metaSections.z, metaSections.w);
		int wallid = CURRENT_BLOCK->wall_id[0];
		if (!CURRENT_BLOCK->isWallOccupied())
			wallid = -1;
		ImGui::Text("Current Wall: %s (id: %d)",
		            BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[0]).getStringID().c_str(), wallid);

		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
			{
				const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
				ImGui::Text(" -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j],
				            w.getStringID().c_str());
			}
		int ccx = CURSOR_X;
		int ccy = CURSOR_Y;
		int j = CURSOR_X - ccx < 0.5f ? 0 : 1;
		int i = CURSOR_Y - ccy < 0.5f ? 0 : 1;
		const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
		ImGui::Text("Selected Wall -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j],
		            w.getStringID().c_str());
	}

	ImGui::Separator();

	ImGui::Columns(2, "mycolumns3", false); // 3-ways, no border
	ImGui::Separator();
	if (BLOCK_OR_WALL_SELECTED)
	{
		if (ImGui::TreeNode("Block Selection"))
		{
			auto& blocks = BlockRegistry::get().getBlocks();
			for (int n = 0; n < blocks.size(); n++)
			{
				char buf[32];
				sprintf(buf, "%s", blocks[n]->getStringID().c_str());
				if (ImGui::Selectable(buf, BLOCK_PALLETE_SELECTED == n))
					BLOCK_PALLETE_SELECTED = n;
			}
			ImGui::TreePop();
		}
	}
	else
	{
		if (ImGui::TreeNode("Wall Selection"))
		{
			auto& walls = BlockRegistry::get().getWalls();
			for (int n = 0; n < walls.size(); n++)
			{
				char buf[32];
				sprintf(buf, "%s", walls[n]->getStringID().c_str());
				if (ImGui::Selectable(buf, BLOCK_PALLETE_SELECTED == n))
					BLOCK_PALLETE_SELECTED = n;
			}
			ImGui::TreePop();
		}
	}
	if (ImGui::TreeNode("Entity Selection"))
	{
		auto& blocks = EntityRegistry::get().getData();

		for (int n = 0; n < blocks.size(); n++)
		{
			char buf[32];
			sprintf(buf, "%s", blocks[n].name.c_str());
			if (ImGui::Selectable(buf, ENTITY_PALLETE_SELECTED == blocks[n].entity_type))
				ENTITY_PALLETE_SELECTED = blocks[n].entity_type;
		}
		ImGui::TreePop();
	}
	ImGui::Separator();

	/*ImGui::Text("Drawn Chunks:");
	auto& map = m_render_manager->getMap();
	int ind = 0;
	for (auto& iterator = map.begin(); iterator != map.end(); ++iterator) {
		char label[32];
		int x, y;
		Chunk::getChunkPosFromID(iterator->first, x, y);
		sprintf(label, "%d, %d", x, y);
		ImGui::Text(label);
		ind++;
		if (ind == map.size() / 2)
			ImGui::NextColumn();
	}
	ImGui::Separator();*/

	ImGui::End();
	//static bool op = false;
	//ImGui::ShowDemoWindow(&op);
}

static std::string toString(World::ChunkState s)
{
	switch (s)
	{
	case World::BEING_LOADED: return "BE_LOADED";
	case World::BEING_GENERATED: return "BE_GENERA";
	case World::GENERATED: return "GENERATED";
	case World::BEING_UNLOADED: return "BE_UNLOAD";
	case World::UNLOADED: return "UNLOADED ";
	default: return "INVAL";
	}
}

void WorldLayer::onImGuiRenderChunks()
{
	static bool showBase = true;

	if (!ImGui::Begin("Chunks", &showBase))
	{
		ImGui::End();
		return;
	}
	ImGui::Value("Tasksize ", App::get().getScheduler().size());
	auto& t = m_world->getHeaders();
	int offset = 0;
	for (auto& header : t)
	{
		if (offset < 10)
		{
			ImGui::Text("0%d: (%d/%d), %s, ac:%d, ID:%d",
			            offset++,
			            (int)header.getJobConst().m_worker,
			            (int)header.getJobConst().m_main,
			            toString(header.getState()).c_str(),
			            header.isAccessible(),
			            header.getChunkID()

			);
		}
		else
		{
			ImGui::Text("%d: (%d/%d), %s, ac:%d, ID:%d",
			            offset++,
			            (int)header.getJobConst().m_worker,
			            (int)header.getJobConst().m_main,
			            toString(header.getState()).c_str(),
			            header.isAccessible(),
			            header.getChunkID()

			);
		}
	}

	ImGui::End();
}

void WorldLayer::onEvent(Event& e)
{
	if (isPaused())
		return;

	if (e.getEventType() == Event::EventType::Message)
	{
		auto m = dynamic_cast<MessageEvent*>(&e);
		if (strcmp(m->getTitle(), CommonMessages::WorldMessage::NAME) == 0)
		{
			//todo maybe make each message as a class with inheritance and allocate it on stack
		}
	}
	if (!m_has_world)
		return;

	if (e.getEventType() == Event::EventType::WindowResize)
	{
		auto event = dynamic_cast<WindowResizeEvent*>(&e);
		m_render_manager->onScreenResize();
	}
	//if (!ImGui::IsMouseHoveringAnyWindow())

	if (e.getEventType() == Event::EventType::MouseRelease)
		isDragging = false;

	if (e.getEventType() == Event::EventType::MousePress)
	{
		auto event = dynamic_cast<MousePressEvent*>(&e);

		if (event->getButton() == GLFW_MOUSE_BUTTON_1)
		{
			isDragging = true;
			event->handled = true;
		}

		if (event->getButton() == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			if (BLOCK_OR_WALL_SELECTED)
			{
				BLOCK_PALLETE_SELECTED = m_world->getBlockM(CURSOR_X, CURSOR_Y)->block_id;
			}
			else
			{
				BLOCK_PALLETE_SELECTED = m_world->getBlockM(CURSOR_X, CURSOR_Y)->wallID();
			}

			event->handled = true;
		}
		if (event->getButton() == GLFW_MOUSE_BUTTON_RIGHT)
		{
			auto inHand = getPlayer().getInventory().itemInHand();
			if (inHand)
			{
				auto t = m_world->getEntitiesAtLocation({CURSOR_X, CURSOR_Y});
				if (!t.empty())
				{
					inHand->getItem().onRightClickOnEntity(*m_world, *inHand, getPlayer(), CURSOR_X, CURSOR_Y, *t[0]);
				}
				else
				{
					auto& stru = *m_world->getBlockM(CURSOR_X, CURSOR_Y);
					if (!inHand->getItem().onRightClickOnBlock(*m_world, *inHand, getPlayer(), CURSOR_X, CURSOR_Y, stru)
					)
						BlockRegistry::get().getBlock(stru.block_id).onBlockClicked(
							*m_world, &getPlayer(), CURSOR_X, CURSOR_Y, stru);
				}
				if(inHand->isEmpty())
				{
					//destroy inHandItem
					getPlayer().getInventory().takeFromIndex(getPlayer().getInventory().itemInHandSlot(),-1)->destroy();
				}
			}
			else
			{
				auto& stru = *m_world->getBlockM(CURSOR_X, CURSOR_Y);
				BlockRegistry::get().getBlock(stru.block_id).onBlockClicked(
					*m_world, &getPlayer(), CURSOR_X, CURSOR_Y, stru);
			}
			event->handled = true;
		}
		if (getPlayer().hasCreative())
			onCreativeEvent(e);
		else onSurvivalEvent(e);
	}
}

void WorldLayer::onCreativeEvent(Event& e)
{
	
}

void WorldLayer::onSurvivalEvent(Event& e)
{
}