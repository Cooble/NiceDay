#include "ndpch.h"
#include "WorldLayer.h"
#include "world/World.h"
#include "world/block/BlockRegistry.h"
#include "world/biome/BiomeRegistry.h"
#include "world/LightCalculator.h"
#include "world/WorldIO.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "Game.h"
#include "Stats.h"
#include "imguiplotvar.h"
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


WorldLayer::WorldLayer()
	: Layer("WorldLayer")
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

	//walls
	ND_REGISTER_WALL(new WallAir());
	ND_REGISTER_WALL(new WallDirt());
	ND_REGISTER_WALL(new WallStone());
	ND_REGISTER_WALL(new WallGlass());

	//biomes
	ND_REGISTER_BIOME(new BiomeForest());
	ND_REGISTER_BIOME(new BiomeUnderground());
	ND_REGISTER_BIOME(new BiomeDirt());

	//entities
	ND_REGISTER_ENTITY(ENTITY_TYPE_PLAYER, EntityPlayer);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TNT, EntityTNT);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ZOMBIE, EntityZombie);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ROUND_BULLET, EntityRoundBullet);


	WorldInfo info;
	strcpy_s(info.name, "NiceWorld");
	info.chunk_width = 200;
	info.chunk_height = 10;
	info.seed = 0;
	info.terrain_level = (info.chunk_height - 2) * WORLD_CHUNK_SIZE;

	m_world = new World(std::string(info.name) + ".world", info);


	bool genW = false;


	if (genW)
		m_world->genWorld();
	else
	{
		if (!m_world->loadWorld())
		{
			ND_WARN("Cannot load world: {}\nGenerating new one", m_world->getFilePath());
			m_world->genWorld();
		}
	}

	m_cam = new Camera();

	m_chunk_loader = new ChunkLoader(m_world);

	ChunkMesh::init();
	m_render_manager = new WorldRenderManager(m_cam, m_world);
	/*Texture* t = Texture::create(TextureInfo("res/images/bg/dirt.png"));
	Texture* t2 = Texture::create(TextureInfo("res/images/icon.png"));
	for (int i = 0; i < 4; ++i)
	{
		auto sprite = new Sprite(t2);
		sprite->setRectangle(glm::vec3(0, 10, 0), glm::vec2(10, 10));
		m_sprites.push_back(sprite);
	}
	chunkOfSprites = (Sprite*)malloc(sizeof(Sprite) * 1000);
	for (int i = 0; i < 1000; ++i)
	{
		auto sprite = new(chunkOfSprites+i) Sprite(t);
		sprite->setRectangle(glm::vec3(i/100.0f, 0, 0), glm::vec2(6, 6));
		*/

	m_batch_renderer = new BatchRenderer2D();
}

EntityPlayer& WorldLayer::getPlayer()
{
	return *dynamic_cast<EntityPlayer*>(m_world->getEntityManager().entity(playerID));
}

WorldLayer::~WorldLayer()
{
	delete m_chunk_loader;
	delete m_world;
	delete m_batch_renderer;
	delete m_render_manager;
}

void WorldLayer::onAttach()
{
	//load entity manager
	if (m_world->getWorldNBT().exists<EntityID>("playerID"))
	{
		playerID = m_world->getWorldNBT().get<EntityID>("playerID");
		ChunkID chunk = m_world->getWorldNBT().get<ChunkID>("player_chunkID");
		m_world->loadChunk( //load chunk where player is
			half_int::getX(chunk),
			half_int::getY(chunk));
	}
	else
	{
		auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_PLAYER).byte_size);
		EntityRegistry::get().createInstance(ENTITY_TYPE_PLAYER, buff);

		playerID = m_world->spawnEntity((WorldEntity*)buff);
		getPlayer().getPosition() = {
			m_world->getInfo().chunk_width * WORLD_CHUNK_SIZE / 2, m_world->getInfo().terrain_level
		};
	}

	LightCalculator& c = m_world->getLightCalculator();

	//add camera
	m_cam->setPosition(getPlayer().getPosition().asGLM());
	m_cam->setChunkRadius({4, 3});
	c.registerLight(dynamic_cast<LightSource*>(m_cam));

	m_chunk_loader->registerEntity(dynamic_cast<IChunkLoaderEntity*>(m_cam));
	c.run();
}

void WorldLayer::onDetach()
{
	m_world->getLightCalculator().stop();

	auto pos = getPlayer().getPosition();
	m_world->getWorldNBT().set("player_chunkID", Chunk::getChunkIDFromWorldPos(pos.x, pos.y));
	m_world->getWorldNBT().set("playerID", playerID);

	m_chunk_loader->clearEntities();
	m_chunk_loader->onUpdate(); //this will unload all chunks

	m_world->saveWorld();
}

static int fpsCount;

void WorldLayer::onUpdate()
{
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
	auto pair = Game::get().getInput().getMouseLocation();
	CURSOR_X = pair.first - Game::get().getWindow()->getWidth() / 2;
	CURSOR_Y = -pair.second + Game::get().getWindow()->getHeight() / 2;

	CURSOR_X = CURSOR_X / BLOCK_PIXEL_SIZE + m_cam->getPosition().x;
	CURSOR_Y = CURSOR_Y / BLOCK_PIXEL_SIZE + m_cam->getPosition().y;

	if (m_world->isBlockValid(CURSOR_X, CURSOR_Y))
	{
		CURRENT_BLOCK = &m_world->getBlock(CURSOR_X, CURSOR_Y);
#ifdef ND_DEBUG
		CURRENT_BLOCK_ID = BlockRegistry::get().getBlock(m_world->getBlock(CURSOR_X, CURSOR_Y).block_id).toString();
#endif
	}

	bool istsunderBlock = !m_world->isAir(m_cam->getPosition().x, m_cam->getPosition().y - 1);


	bool tntenable = true;
	constexpr int maxDeltaBum = 2;
	static int deltaBum = 0;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_T))
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
	if (Game::get().getInput().isKeyFreshlyPressed(GLFW_KEY_C))
	{
		Stats::fly_enable = !Stats::fly_enable;
		ND_INFO("Fly mode: {}", Stats::fly_enable);
	}
	if (Game::get().getInput().isKeyFreshlyPressed(GLFW_KEY_E))
	{
		auto t = Phys::asVect(m_cam->getPosition()).copy();

		auto entityBuff = malloc(EntityRegistry::get().getBucket(ENTITY_PALLETE_SELECTED).byte_size);
		EntityRegistry::get().createInstance(ENTITY_PALLETE_SELECTED, entityBuff);

		((WorldEntity*)entityBuff)->getPosition() = m_cam->getPosition();
		((WorldEntity*)entityBuff)->getPosition().y += 10;
		m_world->spawnEntity((WorldEntity*)entityBuff);
	}

	if (Game::get().getInput().isKeyPressed(GLFW_KEY_B))
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

	if (Game::get().getInput().isMousePressed(GLFW_MOUSE_BUTTON_1))
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
				bullet->getPosition() = m_cam->getPosition() + Phys::Vect(0, 10.f).asGLM();
				bullet->fire(Phys::Vect(CURSOR_X, CURSOR_Y), 50.f / 60);
				m_world->spawnEntity(bullet);
			}
		}
		else if (BLOCK_OR_WALL_SELECTED)
		{
			if (m_world->getBlock(CURSOR_X, CURSOR_Y).block_id != BLOCK_PALLETE_SELECTED)
				m_world->setBlock(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
		}
		else
		{
			if (m_world->getBlock(CURSOR_X, CURSOR_Y).isWallOccupied())
			{
				if (m_world->getBlock(CURSOR_X, CURSOR_Y).wallID() != BLOCK_PALLETE_SELECTED)
					m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
			}
			else
				m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
		}
	}

	glm::vec2 accel = glm::vec2(0, 0);
	accel.y = -9.0f / 60;
	glm::vec2& velocity = getPlayer().getVelocity().asGLM();
	float acc = 0.3f;


	if (Game::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
		accel.x = acc;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
		accel.x = -acc;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_UP))
	{
		if (istsunderBlock || Stats::fly_enable)
			velocity.y = 10;
	}

	getPlayer().getAcceleration() = accel;
	//std::cout << "Acceleration: " << Phys::asVect(accel) << "\n";
	m_cam->setPosition(getPlayer().getPosition().asGLM());


	CAM_POS_X = (m_cam->getPosition().x);
	CAM_POS_Y = (m_cam->getPosition().y);
	CHUNKS_LOADED = m_world->getMap().size();
	CHUNKS_DRAWN = m_render_manager->getMap().size();
	WORLD_FILE_PATH = m_world->getFilePath().c_str();
	WORLD_CHUNK_WIDTH = m_world->getInfo().chunk_width;
	WORLD_CHUNK_HEIGHT = m_world->getInfo().chunk_height;

	m_world->onUpdate();
	m_chunk_loader->onUpdate();
	m_render_manager->onUpdate();


	auto chunkP = m_world->getLoadedChunkPointer((int)getPlayer().getPosition().x >> WORLD_CHUNK_BIT_SIZE,
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

				for (int i = 0; i < std::rand()%5+1; ++i)
				{
					int x = std::rand() % (4 * WORLD_CHUNK_SIZE);
					x -= 2 * WORLD_CHUNK_SIZE;


					auto buff = malloc(EntityRegistry::get().getBucket(ENTITY_TYPE_ROUND_BULLET).byte_size);
					EntityRegistry::get().createInstance(ENTITY_TYPE_ROUND_BULLET, buff);
					auto bullet = (EntityRoundBullet*)buff;
					bullet->getPosition() = m_cam->getPosition() + Phys::Vect(x, 2 * WORLD_CHUNK_SIZE).asGLM();
					bullet->fire(2 * 3.14159f * 3.0f / 4 + (((std::rand() % 10) - 5) / 5.f*0.3f), ((std::rand() % 20) + 40.f) / 60);
					m_world->spawnEntity(bullet);
				}
				
			}
		}
	}

	//m_physics_system->proccess(m_physics_manager);
}

void WorldLayer::onRender()
{
	m_render_manager->render();

	m_batch_renderer->begin();
	m_batch_renderer->push(m_render_manager->getProjMatrix());
	auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-m_cam->getPosition().x, -m_cam->getPosition().y, 0));
	m_batch_renderer->push(worldMatrix);
	for (auto it = m_world->beginEntities(); it != m_world->endEntities(); ++it)
	{
		WorldEntity* entity = m_world->getLoadedEntity(*it);
		auto ren = dynamic_cast<IBatchRenderable2D*>(entity);
		if (ren)
			ren->render(*m_batch_renderer);
	}
	m_batch_renderer->pop(); //world matrix
	m_batch_renderer->pop(); //proj matrix
	m_batch_renderer->flush();
}

void WorldLayer::onImGuiRender()
{
	float fps = FLT_MAX;
	if (fpsCount > Game::get().getTargetTPS() / 10)
	{
		fpsCount = 0;
		fps = Game::get().getFPS();
	}
	static bool show = true;

	if (!ImGui::Begin("WorldInfo", &show))
	{
		ImGui::End();
		return;
	}
	/*BiomeDistances d = Stats::biome_distances;
	ImGui::Text("Biomes:");
	for (int i = 0; i < 4; ++i)
	{
		ImGui::Text("* %d, %.2f", d.biomes[i], d.intensities[i]);

	}*/
	static int maxMillis = 0;
	static int maxMillisLight = 0;
	constexpr int MAX_maxmillisUpdateInterval = 60 * 2; //evry two sec will reset the max millis tick duiration checkr
	static int maxMillisUpdateInterval = MAX_maxmillisUpdateInterval;
	if (maxMillisUpdateInterval-- == 0)
	{
		maxMillisUpdateInterval = MAX_maxmillisUpdateInterval;
		maxMillis = 0;
		maxMillisLight = 0;
	}
	maxMillis = max(maxMillis, Game::get().getTickMillis());
	maxMillisLight = max(maxMillisLight, Stats::light_millis);

	ImGui::PlotVar("Tick Millis", Game::get().getTickMillis());
	ImGui::Text("Max millis: %d", maxMillis);
	ImGui::PlotVar("FPS", fps);
	bool l = Stats::light_enable;
	ImGui::Checkbox(Stats::light_enable ? "Lighting Enabled" : "Lighting Disabled", &Stats::light_enable);
	if (l != Stats::light_enable)
	{
		if (!l) m_world->getLightCalculator().run();
		else m_world->getLightCalculator().stop();
	}
	if (Stats::light_enable)
	{
		ImGui::PlotVar("Light millis", Stats::light_millis);
		ImGui::Text("Max millis: %d", maxMillisLight);
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
	ImGui::Text("WorldTime: %d:%d", (int)m_world->getWorldTime().hours(), (int)m_world->getWorldTime().minutes());
	ImGui::Text("Chunks Size: (%d/%d)", WORLD_CHUNK_WIDTH, WORLD_CHUNK_HEIGHT);
	ImGui::Text("Chunks loaded: %d", CHUNKS_LOADED);
	ImGui::Text("Chunks drawn: %d", CHUNKS_DRAWN);
	ImGui::Text("Dynamic segments: %d", m_world->getNBTSaver().getSegmentCount());
	ImGui::Text("Dynamic free segments: %d", m_world->getNBTSaver().getFreeSegmentCount());
	ImGui::Text("Cam X: %.2f (%d)", CAM_POS_X, (int)CAM_POS_X >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cam Y: %.2f (%d)", CAM_POS_Y, (int)CAM_POS_Y >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor X: %.2f (%d)", CURSOR_X, (int)CURSOR_X >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor y: %.2f (%d)", CURSOR_Y, (int)CURSOR_Y >> WORLD_CHUNK_BIT_SIZE);
#ifdef ND_DEBUG

	if (auto current = m_world->getLoadedBlockPointer(CURSOR_X, CURSOR_Y))
	{
		CURRENT_BLOCK = current;
		ImGui::Text("Current Block: %s (id: %d, corner: %d)", CURRENT_BLOCK_ID.c_str(), CURRENT_BLOCK->block_id,
		            CURRENT_BLOCK->block_corner);
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

void WorldLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto event = dynamic_cast<KeyPressEvent*>(&e);

		if (event->getKey() == GLFW_KEY_ESCAPE)
		{
			auto e = WindowCloseEvent();
			Game::get().fireEvent(e);
			event->handled = true;
		}
	}

	if (e.getEventType() == Event::EventType::WindowResize)
	{
		auto event = dynamic_cast<WindowResizeEvent*>(&e);
		m_render_manager->onScreenResize();
	}

	if (e.getEventType() == Event::EventType::MousePress)
	{
		auto event = dynamic_cast<MousePressEvent*>(&e);
		if (event->getButton() == GLFW_MOUSE_BUTTON_2)
		{
			if (BLOCK_OR_WALL_SELECTED)
			{
				BLOCK_PALLETE_SELECTED = m_world->getBlock(CURSOR_X, CURSOR_Y).block_id;
			}
			else
			{
				BLOCK_PALLETE_SELECTED = m_world->getBlock(CURSOR_X, CURSOR_Y).wallID();
			}
			event->handled = true;
		}
	}
}
