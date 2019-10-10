#include "ndpch.h"
#include "WorldLayer.h"
#include "world/World.h"
#include "world/block/BlockRegistry.h"
#include "world/biome/BiomeRegistry.h"
#include "world/LightCalculator.h"
#include "world/WorldIO.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "App.h"
#include "Stats.h"
#include "layer/imguiplotvar.h"
#include <GLFW/glfw3.h>

#include "world/biome/BiomeForest.h"
#include "world/biome/biomes.h"

#include <imgui.h>
#include "world/block/Block.h"
#include "world/block/basic_blocks.h"
#include "world/block/basic_walls.h"

#include "graphics/BatchRenderer2D.h"
#include "graphics/Sprite.h"
#include "world/entity/EntityRegistry.h"
#include "world/entity/entity_datas.h"
#include "world/entity/entities.h"
#include "graphics/TextureManager.h"
#include "graphics/BlockTextureAtlas.h"
#include "graphics/TextureAtlas.h"
#include "world/particle/ParticleRegistry.h"
#include "world/particle/particles.h"

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

void WorldLayer::registerEverything()
{
	//blocks
	ND_REGISTER_BLOCK(new BlockAir());
	ND_REGISTER_BLOCK(new BlockStone());
	ND_REGISTER_BLOCK(new BlockDirt());
	ND_REGISTER_BLOCK(new BlockGold());
	ND_REGISTER_BLOCK(new BlockAdamantite());
	ND_REGISTER_BLOCK(new BlockPlatform());
	ND_REGISTER_BLOCK(new BlockGrass());
	ND_REGISTER_BLOCK(new BlockGlass());
	ND_REGISTER_BLOCK(new BlockTorch());
	ND_REGISTER_BLOCK(new BlockDoorClose());
	ND_REGISTER_BLOCK(new BlockDoorOpen());
	ND_REGISTER_BLOCK(new BlockPainting());
	ND_REGISTER_BLOCK(new BlockTree());
	ND_REGISTER_BLOCK(new BlockTreeSapling());
	ND_REGISTER_BLOCK(new BlockFlower());
	ND_REGISTER_BLOCK(new BlockGrassPlant());

	//walls
	ND_REGISTER_WALL(new WallAir());
	ND_REGISTER_WALL(new WallDirt());
	ND_REGISTER_WALL(new WallStone());
	ND_REGISTER_WALL(new WallGlass());

	//entities
	ND_REGISTER_ENTITY(ENTITY_TYPE_PLAYER, EntityPlayer);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TNT, EntityTNT);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ZOMBIE, EntityZombie);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ROUND_BULLET, EntityRoundBullet);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_SAPLING, TileEntitySapling);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_TORCH, TileEntityTorch);

	//biomes
	ND_REGISTER_BIOME(new BiomeForest());
	ND_REGISTER_BIOME(new BiomeUnderground());
	ND_REGISTER_BIOME(new BiomeDirt());

	ParticleList::initDefaultParticles(ParticleRegistry::get());
}

constexpr int particleAtlasSize = 8;

WorldLayer::WorldLayer()
	: Layer("WorldLayer")
{
	std::string blockAtlasFolder = "D:/Dev/C++/NiceDay/Sandbox/res/images/blockAtlas/";

	BlockTextureAtlas blockAtlas;
	blockAtlas.createAtlas(blockAtlasFolder, 32, 8);

	std::string particleAtlasFolder = "D:/Dev/C++/NiceDay/Sandbox/res/images/particleAtlas/";

	TextureAtlas particleAtlas;
	particleAtlas.createAtlas(particleAtlasFolder, particleAtlasSize, 8);

	registerEverything();

	ParticleRegistry::get().initTextures(particleAtlas);
	BlockRegistry::get().initTextures(blockAtlas);

	{
		//call all constructors of entities to load static data on main thread
		size_t max = 0;
		for (auto& bucket : EntityRegistry::get().getData())
			max = std::max(bucket.byte_size, max);
		auto entityBuff = malloc(max);
		for (auto& bucket : EntityRegistry::get().getData())
			EntityRegistry::get().createInstance(bucket.entity_type, entityBuff);
		free(entityBuff);
	}

	ChunkMesh::init();


	m_cam = new Camera();

	m_batch_renderer = new BatchRenderer2D();
	m_particle_renderer = new ParticleRenderer();

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/borderBox.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 1, 1);

	Stats::bound_sprite = new Sprite(&res);
	Stats::bound_sprite->setSpriteIndex(0, 0);
	Stats::bound_sprite->setPosition(glm::vec3(0, 0, 0));
	Stats::bound_sprite->setSize(glm::vec2(1, 1));

	//world===================================================
	WorldInfo info;
	strcpy_s(info.name, "NiceWorld");
	info.chunk_width = 50;
	info.chunk_height = 50;
	info.seed = 0;
	info.terrain_level = (info.chunk_height - 4) * WORLD_CHUNK_SIZE;

	m_world = new World(std::string(info.name) + ".world", info);


	bool genW = false;
	bool worldAlreadyLoaded = true;

	if (genW)
	{
		m_world->genWorld();
		ND_INFO("New world generated.");
	}
	else
	{
		auto world = m_world;
		auto job = m_world->loadWorld();
		if (job == nullptr)
		{
			ND_INFO("World is missing: {}, generating new one", std::string(info.name) + ".world");
			m_world->genWorld();
		}
		else
		{
			worldAlreadyLoaded = false;
			ND_SCHED.callWhenDone([this,job, world,info]()
				{
					if (job->m_variable != JobAssignment::JOB_SUCCESS)
					{
						ND_INFO("World is corrupted: {}, generating new one", std::string(info.name) + ".world");
						world->genWorld();
					}
					else
						ND_INFO("World loaded: {}", std::string(info.name) + ".world");
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
	delete m_chunk_loader;
	delete m_world;
	delete m_batch_renderer;
	delete m_particle_renderer;
	delete m_render_manager;
}

void WorldLayer::onAttach()
{
}

//called after world was gen or loaded
void WorldLayer::onWorldLoaded()
{
	Stats::world = m_world;
	m_chunk_loader = new ChunkLoader(m_world);

	m_render_manager = new WorldRenderManager(m_cam, m_world);
	Texture* particleAtlasT = Texture::create(
		TextureInfo("res/images/particleAtlas/atlas.png")
		.filterMode(TextureFilterMode::NEAREST)
		.format(TextureFormat::RGBA));
	*m_world->particleManager() = new ParticleManager(5000, particleAtlasT, particleAtlasSize);

	//load entity manager
	if (m_world->getWorldNBT().exists<EntityID>("playerID"))
	{
		playerID = m_world->getWorldNBT().get<EntityID>("playerID");
		ChunkID chunk = m_world->getWorldNBT().get<ChunkID>("player_chunkID");
		m_world->loadChunk( //load chunk where player is
			half_int::X(chunk),
			half_int::Y(chunk));
		/*if(m_world->getLoadedEntity(playerID)==nullptr)
		{
			ND_ERROR("corrupted world file:(");
			ND_WAIT_FOR_INPUT;
		exit(1);
		}*/
		int timeout = App::get().getTPS()*1;//wait n seconds to load entity
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
		auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_PLAYER).byte_size);
		EntityRegistry::get().createInstance(ENTITY_TYPE_PLAYER, buff);

		playerID = m_world->spawnEntity((WorldEntity*)buff);
		getPlayer().getPosition() = {
			m_world->getInfo().chunk_width * WORLD_CHUNK_SIZE / 2, m_world->getInfo().terrain_level
		};
		afterPlayerLoaded();
	}
}

//called after chunk with player was loaded
void WorldLayer::afterPlayerLoaded()
{
	LightCalculator& c = m_world->getLightCalculator();
	c.run();

	//add camera
	m_cam->setPosition(getPlayer().getPosition().asGLM());
	m_cam->setChunkRadius({4, 3});
	c.registerLight(dynamic_cast<LightSource*>(m_cam));

	m_chunk_loader->registerEntity(dynamic_cast<IChunkLoaderEntity*>(m_cam));
	m_is_world_ready = true;
}

void WorldLayer::onDetach()
{
	m_world->getLightCalculator().stop();

	auto pos = getPlayer().getPosition();
	m_world->getWorldNBT().set("player_chunkID", Chunk::getChunkIDFromWorldPos(pos.x, pos.y));
	m_world->getWorldNBT().set("playerID", playerID);

	m_chunk_loader->clearEntities();
	m_chunk_loader->onUpdate(); //this will unload all chunks

	auto job = m_world->saveWorld();
	while (!job->isDone())
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	if (job->m_variable != JobAssignment::JOB_SUCCESS)
		ND_INFO("Cannot save world");
	else
		ND_INFO("World saved");
	ND_SCHED.deallocateJob(job);
}

static int fpsCount;

void WorldLayer::onUpdate()
{
	if (!m_is_world_ready)
		return;

	auto& play = getPlayer();

	m_world->onUpdate();
	m_cam->setPosition(play.getPosition().asGLM());
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
	CURSOR_X = pair.first - App::get().getWindow()->getWidth() / 2;
	CURSOR_Y = -pair.second + App::get().getWindow()->getHeight() / 2;

	CURSOR_X = CURSOR_X / BLOCK_PIXEL_SIZE + m_cam->getPosition().x;
	CURSOR_Y = CURSOR_Y / BLOCK_PIXEL_SIZE + m_cam->getPosition().y;

	if (m_world->isBlockValid(CURSOR_X, CURSOR_Y))
	{
		CURRENT_BLOCK = m_world->getBlockOrAir(CURSOR_X, CURSOR_Y);
#ifdef ND_DEBUG
		CURRENT_BLOCK_ID = BlockRegistry::get().getBlock(CURRENT_BLOCK->block_id).toString();
#endif
	}

	bool istsunderBlock = !m_world->isAir(m_cam->getPosition().x, m_cam->getPosition().y - 1);


	bool tntenable = true;
	constexpr int maxDeltaBum = 2;
	static int deltaBum = 0;
	if (App::get().getInput().isKeyPressed(GLFW_KEY_T))
	{
		if (tntenable)
		{
			if (deltaBum-- == 0)
			{
				deltaBum = maxDeltaBum;
				auto t = Phys::asVect(m_cam->getPosition()).copy();
				t.plus({0, 3});
				Phys::Vect throwVect = Phys::Vect(CURSOR_X, CURSOR_Y) - t;
				throwVect.normalize();
				throwVect *= 10.f;
				throwVect += getPlayer().getVelocity();
				auto tnt = (EntityTNT*)malloc(sizeof(EntityTNT));
				EntityRegistry::get().createInstance(ENTITY_TYPE_TNT, tnt);
				tnt->getPosition() = m_cam->getPosition();
				tnt->getPosition().y += 3;
				tnt->getVelocity() = throwVect.asGLM();
				m_world->spawnEntity(tnt);
				//tnt->getAcceleration() = glm::vec2(0,-9.8)
			}
		}
	}
	if (App::get().getInput().isKeyFreshlyPressed(GLFW_KEY_C))
	{
		Stats::fly_enable = !Stats::fly_enable;
		ND_INFO("Fly mode: {}", Stats::fly_enable);
	}
	if (App::get().getInput().isKeyFreshlyPressed(GLFW_KEY_E))
	{
		auto t = Phys::asVect(m_cam->getPosition()).copy();

		auto entityBuff = malloc(EntityRegistry::get().getBucket(ENTITY_PALLETE_SELECTED).byte_size);
		EntityRegistry::get().createInstance(ENTITY_PALLETE_SELECTED, entityBuff);

		((WorldEntity*)entityBuff)->getPosition() = m_cam->getPosition();
		((WorldEntity*)entityBuff)->getPosition().y += 10;
		m_world->spawnEntity((WorldEntity*)entityBuff);
	}

	if (App::get().getInput().isKeyPressed(GLFW_KEY_B))
	{
		constexpr int BULLET_CADENCE_DELAY = 8;
		static int counter = 0;
		if (counter++ == BULLET_CADENCE_DELAY)
		{
			counter = 0;
			const int BULLET_COUNT = 16;
			for (int i = 0; i < BULLET_COUNT; ++i)
			{
				auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_ROUND_BULLET).byte_size);
				EntityRegistry::get().createInstance(ENTITY_TYPE_ROUND_BULLET, buff);
				auto bullet = (EntityRoundBullet*)buff;
				bullet->getPosition() = m_cam->getPosition() + Phys::Vect(0, 10.f).asGLM();
				bullet->fire(3.14159f * 2 / BULLET_COUNT * i, 50.f / 60);
				m_world->spawnEntity(bullet);
			}
		}
	}

	if (!ImGui::IsMouseHoveringAnyWindow())
		if (App::get().getInput().isMousePressed(GLFW_MOUSE_BUTTON_1))
		{
			if (Stats::gun_enable)
			{
				constexpr int BULLET_CADENCE_DELAY = 3;
				static int counter = 0;
				if (counter++ == BULLET_CADENCE_DELAY)
				{
					counter = 0;
					auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_ROUND_BULLET).byte_size);
					EntityRegistry::get().createInstance(ENTITY_TYPE_ROUND_BULLET, buff);
					auto bullet = (EntityRoundBullet*)buff;
					bullet->getPosition() = m_cam->getPosition() + Phys::Vect(0, 4.f).asGLM();
					bullet->fire(Phys::Vect(CURSOR_X, CURSOR_Y), 50.f / 60);
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


	//camera movement===================================================================
	glm::vec2 accel = glm::vec2(0, 0);
	accel.y = -9.0f / 60;
	glm::vec2& velocity = play.getVelocity().asGLM();
	float acc = 0.3f;
	float moveThroughBlockSpeed = 6;

	if (Stats::move_through_blocks_enable)
	{
		velocity = glm::vec2(0, 0);
		if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
			velocity.x = moveThroughBlockSpeed;
		if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
			velocity.x = -moveThroughBlockSpeed;


		if (App::get().getInput().isKeyPressed(GLFW_KEY_UP))
			velocity.y = moveThroughBlockSpeed;
		if (App::get().getInput().isKeyPressed(GLFW_KEY_DOWN))
			velocity.y = -moveThroughBlockSpeed;
	}

	else
	{
		if (App::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
			accel.x = acc;
		if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
			accel.x = -acc;
		if (App::get().getInput().isKeyPressed(GLFW_KEY_UP))
		{
			if (istsunderBlock || Stats::fly_enable)
				velocity.y = 10;
		}
		getPlayer().getAcceleration() = accel;
	}

	//std::cout << "Acceleration: " << Phys::asVect(accel) << "\n";

	// Particle Sh't
	/*
	static float angle=0;
	static int tickToChangeAngle = 1;
	tickToChangeAngle--;
	static float angleee = 0;
	static float speeeed = 1;
	if(tickToChangeAngle<=0)
	{
		tickToChangeAngle = rand() % 10 + 5;
		angleee = randFloat()*3.14159f*2;
		speeeed = randFloat() * 2 + 0.5;
	}
	m_world->spawnParticle(ParticleList::line, getPlayer().getPosition(), Phys::vectFromAngle(angleee)*speeeed, { 0 }, (60 + rand() % 10) * 5);



	for (int i = 0; i < 10; ++i)
	{
		angle += 0.03f;
		auto speed = Phys::vectFromAngle(angle + randDispersedFloat() + 3.14159f / 2);
		auto speed2 = speed *randFloat();
		m_world->spawnParticle(ParticleList::dot, getPlayer().getPosition(), speed2, speed*(-0.005f+ randDispersedFloat()*0.2f), (60 + rand() % 10) * 5);
	}
	*/

	CAM_POS_X = (m_cam->getPosition().x);
	CAM_POS_Y = (m_cam->getPosition().y);
	CHUNKS_LOADED = m_world->getMap().size();
	CHUNKS_DRAWN = m_render_manager->getMap().size();
	WORLD_FILE_PATH = m_world->getFilePath().c_str();
	WORLD_CHUNK_WIDTH = m_world->getInfo().chunk_width;
	WORLD_CHUNK_HEIGHT = m_world->getInfo().chunk_height;


	return;
	//rain
	auto chunkP = m_world->getChunk((int)getPlayer().getPosition().x >> WORLD_CHUNK_BIT_SIZE,
	                                (int)getPlayer().getPosition().y >> WORLD_CHUNK_BIT_SIZE);
	if (chunkP)
	{
		auto& biome = BiomeRegistry::get().getBiome(chunkP->getBiome());
		if (biome.getID() == BIOME_FOREST)
		{
			static int counter = 0;
			static int rainDropDelay = 8;
			if (counter++ >= rainDropDelay)
			{
				counter = 0;
				rainDropDelay = std::rand() % 5 + 5;

				for (int i = 0; i < std::rand() % 5 + 1; ++i)
				{
					int x = std::rand() % (4 * WORLD_CHUNK_SIZE);
					x -= 2 * WORLD_CHUNK_SIZE;


					auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_ROUND_BULLET).byte_size);
					EntityRegistry::get().createInstance(ENTITY_TYPE_ROUND_BULLET, buff);
					auto bullet = (EntityRoundBullet*)buff;
					bullet->getPosition() = m_cam->getPosition() + Phys::Vect(x, 2 * WORLD_CHUNK_SIZE).asGLM();
					bullet->fire(2 * 3.14159f * 3.0f / 4 + (((std::rand() % 10) - 5) / 5.f * 0.3f),
					             ((std::rand() % 20) + 40.f) / 60);
					m_world->spawnEntity(bullet);
				}
			}
		}
	}
}

void WorldLayer::onRender()
{
	if (!m_is_world_ready)
		return;

	m_render_manager->onUpdate();

	//world
	m_render_manager->render();

	//entities
	auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-m_cam->getPosition().x, -m_cam->getPosition().y, 0));

	m_batch_renderer->begin();
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


	//particles
	m_particle_renderer->begin();
	m_particle_renderer->push(m_render_manager->getProjMatrix() * worldMatrix);
	(*m_world->particleManager())->render(*m_particle_renderer);
	m_particle_renderer->pop();
	m_particle_renderer->flush();
}

static bool showTelem = false;
static bool showChunks = false;

void WorldLayer::onImGuiRender()
{
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
	if (!ImGui::Begin("Telemetrics", &showTelemetrics))
	{
		ImGui::End();
		return;
	}

	static int maxMillis = 0;
	static int maxMillisLight = 0;
	static int maxupdatesPerFrame = 0;
	static int maxmillisrender = 0;
	constexpr int MAX_maxmillisUpdateInterval = 60 * 2; //evry two sec will reset the max millis tick duiration checkr
	static int maxMillisUpdateInterval = MAX_maxmillisUpdateInterval;
	if (maxMillisUpdateInterval-- == 0)
	{
		maxMillisUpdateInterval = MAX_maxmillisUpdateInterval;
		maxMillis = 0;
		maxMillisLight = 0;
		maxupdatesPerFrame = 0;
		maxmillisrender = 0;
	}
	maxMillis = max(maxMillis, App::get().getTickMillis());
	maxMillisLight = max(maxMillisLight, Stats::light_millis);
	maxupdatesPerFrame = max(Stats::updates_per_frame, maxupdatesPerFrame);
	maxmillisrender = max(App::get().getRenderMillis(), maxmillisrender);

	ImGui::PlotVar("Tick Millis", App::get().getTickMillis());
	ImGui::Text("Max tick   millis: %d", maxMillis);
	ImGui::PlotVar("Render Millis", App::get().getRenderMillis());
	ImGui::Text("Max render millis: %d", maxmillisrender);


	ImGui::PlotVar("Updates per Frame", Stats::updates_per_frame);
	ImGui::Text("Max Updates: %d", maxupdatesPerFrame);

	ImGui::PlotVar("FPS", App::get().getFPS());
	if (Stats::light_enable)
	{
		ImGui::PlotVar("Light millis", Stats::light_millis);
		ImGui::Text("Max millis: %d", maxMillisLight);
	}
	ImGui::End();
}

void WorldLayer::onImGuiRenderWorld()
{
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
		if (!l) m_world->getLightCalculator().run();
		else m_world->getLightCalculator().stop();
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


	ImGui::Text("World filepath: %s", WORLD_FILE_PATH);
	ImGui::Text("WorldTime: (%d) %d:%d", m_world->getWorldTime().m_ticks, (int)m_world->getWorldTime().hour(),
	            (int)m_world->getWorldTime().minute());
	ImGui::SliderFloat("Time speed", &m_world->m_time_speed, 0.f, 20.0f, "ratio = %.2f");
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
#ifdef ND_DEBUG

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
		            BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[0]).toString().c_str(), wallid);

		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
			{
				const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
				ImGui::Text(" -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j],
				            w.toString().c_str());
			}
		int ccx = CURSOR_X;
		int ccy = CURSOR_Y;
		int j = CURSOR_X - ccx < 0.5f ? 0 : 1;
		int i = CURSOR_Y - ccy < 0.5f ? 0 : 1;
		const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
		ImGui::Text("Selected Wall -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j],
		            w.toString().c_str());
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
				sprintf(buf, "%s", blocks[n]->toString().c_str());
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
				sprintf(buf, "%s", walls[n]->toString().c_str());
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
			if (ImGui::Selectable(buf, ENTITY_PALLETE_SELECTED == n))
				ENTITY_PALLETE_SELECTED = n;
		}
		ImGui::TreePop();
	}
#endif
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
	if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto event = dynamic_cast<KeyPressEvent*>(&e);

		if (event->getKey() == GLFW_KEY_ESCAPE)
		{
			auto e = WindowCloseEvent();
			App::get().fireEvent(e);
			event->handled = true;

			if (App::get().getInput().isKeyPressed(GLFW_KEY_DELETE))
			{
				std::remove(m_world->getFilePath().c_str());
				ND_INFO("Erasing world");
			}
		}
	}

	if (e.getEventType() == Event::EventType::WindowResize)
	{
		auto event = dynamic_cast<WindowResizeEvent*>(&e);
		m_render_manager->onScreenResize();
	}
	if (!ImGui::IsMouseHoveringAnyWindow())
		if (e.getEventType() == Event::EventType::MousePress)
		{
			auto event = dynamic_cast<MousePressEvent*>(&e);
			if (event->getButton() == GLFW_MOUSE_BUTTON_2)
			{
				if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT_CONTROL))
				{
					auto& stru = *m_world->getBlockM(CURSOR_X, CURSOR_Y);
					BlockRegistry::get().getBlock(stru.block_id).onBlockClicked(
						*m_world, &getPlayer(), CURSOR_X, CURSOR_Y, stru);
				}
				else
				{
					if (BLOCK_OR_WALL_SELECTED)
					{
						BLOCK_PALLETE_SELECTED = m_world->getBlockM(CURSOR_X, CURSOR_Y)->block_id;
					}
					else
					{
						BLOCK_PALLETE_SELECTED = m_world->getBlockM(CURSOR_X, CURSOR_Y)->wallID();
					}
				}
				event->handled = true;
			}
		}
}
