#include "ndpch.h"
#include "WorldLayer.h"
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
#include "graphics/Renderer.h"
#include "world/block/Block.h"
#include "world/block/basic_blocks.h"
#include "world/block/basic_walls.h"

#include "graphics/BatchRenderer2D.h"
#include "graphics/Sprite.h"

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
static bool BLOCK_OR_WALL_SELECTED = true;

//static Entity m_player_handle;
WorldLayer::WorldLayer()
	: Layer("WorldLayer"), chunkOfSprites(nullptr)
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

	WorldIOInfo info;
	info.world_name = "NiceWorld";
	info.chunk_width = 200;
	info.chunk_height = 10;
	info.seed = 0;
	info.terrain_level = (info.chunk_height - 2) * WORLD_CHUNK_SIZE;


	bool genW = true;


	if (genW)
	{
		auto stream = WorldIO::Session(info.world_name + ".world", true, true);
		m_world = stream.genWorldFile(info);
		stream.close();
	}
	else
	{
		auto stream = WorldIO::Session(info.world_name + ".world", false);
		m_world = stream.loadWorld();
		stream.close();
	}

	if (!m_world)
	{
		ND_ERROR("cannot load world file corrputed");
	}


	m_chunk_loader = new ChunkLoader(m_world);
	m_cam = new Camera();
	glm::vec2 po = {m_world->getInfo().chunk_width * WORLD_CHUNK_SIZE / 2, m_world->getInfo().terrain_level};
	m_cam->setPosition(po);
	m_cam->setChunkRadius({4, 3});

	m_chunk_loader->registerEntity(dynamic_cast<IChunkLoaderEntity*>(m_cam));
	m_chunk_loader->onUpdate();

	ChunkMesh::init();
	LightCalculator& c = m_world->getLightCalculator();
	c.registerLight(dynamic_cast<LightSource*>(m_cam));
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

WorldLayer::~WorldLayer() {
	delete m_chunk_loader;
	delete m_world;
	delete m_batch_renderer;
	delete m_render_manager;
	/*for (auto s:m_sprites)
	{
		delete s;
	}*/
	//free(chunkOfSprites);
}

void WorldLayer::onAttach()
{

}
void WorldLayer::onDetach()
{
	m_world->getLightCalculator().stop();
	m_chunk_loader->clearEntities();
	m_chunk_loader->onUpdate();//this will unload all chunks

	auto s = WorldIO::Session(m_world->getFilePath(), true);
	s.saveWorld(m_world);
	ND_INFO("Saving world {}", m_world->getFilePath());
	s.close();
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
	else {
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

	if (Game::get().getInput().isMousePressed(GLFW_MOUSE_BUTTON_1))
		if (BLOCK_OR_WALL_SELECTED) {
			if (m_world->getBlock(CURSOR_X, CURSOR_Y).block_id != BLOCK_PALLETE_SELECTED)
				m_world->setBlock(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
		}
		else
		{
			if (m_world->getBlock(CURSOR_X, CURSOR_Y).isWallOccupied()) {
				if (m_world->getBlock(CURSOR_X, CURSOR_Y).wallID() != BLOCK_PALLETE_SELECTED)
					m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);

			}
			else
				m_world->setWall(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);
		}

	auto pos = m_cam->getPosition();
	float speed = Stats::player_speed;

	if (Game::get().getInput().isKeyPressed(GLFW_KEY_RIGHT))
		pos.x += speed;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_LEFT))
		pos.x -= speed;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_UP))
		pos.y += speed;
	if (Game::get().getInput().isKeyPressed(GLFW_KEY_DOWN))
		pos.y -= speed;

	if (pos.x < 0)
		pos.x = 0;
	if (pos.y < 0)
		pos.y = 0;
	if (pos.x > m_world->getInfo().chunk_width*WORLD_CHUNK_SIZE - 1)
		pos.x = m_world->getInfo().chunk_width*WORLD_CHUNK_SIZE - 1;
	if (pos.y > m_world->getInfo().chunk_height*WORLD_CHUNK_SIZE - 1)
		pos.y = m_world->getInfo().chunk_height*WORLD_CHUNK_SIZE - 1;
	if (!Stats::move_through_blocks_enable) {
		if (m_world->isAir((int)pos.x, (int)pos.y))
			m_cam->setPosition(pos);
	}
	else m_cam->setPosition(pos);

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


	//m_physics_system->proccess(m_physics_manager);
}
void WorldLayer::onRender()
{
	/*m_render_manager->render();
	m_mesh_system->proccess(m_mesh_manager, m_physics_manager);
*/
/*
	m_batch_renderer->begin();
	m_batch_renderer->push(m_render_manager->getProjMatrix());
	static float rot = 0;
	rot += 0.01f;
	static float rot2 = 0;
	rot2 += 0.02f;
	auto m = glm::rotate(glm::mat4(1.0f), rot,glm::vec3(0, 0, 1));
	auto m2 = glm::translate(glm::mat4(1.0f), glm::vec3(15, 16, 0));
	 m2 = glm::rotate(m2, rot2,glm::vec3(0, 0, 1));
	m_batch_renderer->push(m);
	for (int i = 0; i < m_sprites.size(); ++i)
	{
		m_batch_renderer->submit(*m_sprites[i]);
	}
	m_batch_renderer->push(m2);
	m_batch_renderer->submit(*m_sprites[m_sprites.size()/7]);
	m_batch_renderer->pop();
	m_batch_renderer->pop();
	m_batch_renderer->pop();
	m_batch_renderer->flush();*/

}
void WorldLayer::onImGuiRender()
{
	float fps = FLT_MAX;
	if (fpsCount > Game::get().getTargetTPS())
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
	ImGui::PlotVar("Tick Millis", Game::get().getTickMillis());
	ImGui::PlotVar("FPS", fps);
	bool l = Stats::light_enable;
	ImGui::Checkbox(Stats::light_enable ? "Lighting Enabled" : "Lighting Disabled", &Stats::light_enable);
	if (l != Stats::light_enable)
	{
		if (!l) m_world->getLightCalculator().run();
		else m_world->getLightCalculator().stop();
	}
	if(Stats::light_enable)
		ImGui::PlotVar("Light millis", Stats::light_millis);
	ImGui::Checkbox(Stats::move_through_blocks_enable ? "Move through Blocks Enabled" : "Move through Blocks Disabled", &Stats::move_through_blocks_enable);
	l = BLOCK_OR_WALL_SELECTED;
	ImGui::Checkbox(BLOCK_OR_WALL_SELECTED ? "BLOCK mode" : "WALL mode", &BLOCK_OR_WALL_SELECTED);
	if (l != BLOCK_OR_WALL_SELECTED)
		BLOCK_PALLETE_SELECTED = 0;
	ImGui::InputFloat("Player speed: ", &Stats::player_speed);
	ImGui::InputFloat("Light Intensity: ", &Stats::player_light_intensity);
	ImGui::InputFloat("Debug X: ", &Stats::debug_x);
	
	ImGui::SliderFloat("slider float", &Stats::debug_x, -1.0f, 1.0f, "ratio = %.3f");


	ImGui::Text("World filepath: %s", WORLD_FILE_PATH);
	ImGui::Text("Chunks Size: (%d/%d)", WORLD_CHUNK_WIDTH, WORLD_CHUNK_HEIGHT);
	ImGui::Text("Chunks loaded: %d", CHUNKS_LOADED);
	ImGui::Text("Chunks drawn: %d", CHUNKS_DRAWN);
	ImGui::Text("Cam X: %.2f (%d)", CAM_POS_X,(int)CAM_POS_X>>WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cam Y: %.2f (%d)", CAM_POS_Y, (int)CAM_POS_Y >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor X: %.2f (%d)", CURSOR_X, (int)CURSOR_X >> WORLD_CHUNK_BIT_SIZE);
	ImGui::Text("Cursor y: %.2f (%d)", CURSOR_Y, (int)CURSOR_Y >> WORLD_CHUNK_BIT_SIZE);
#ifdef ND_DEBUG

	if (auto current = m_world->getLoadedBlockPointer(CURSOR_X, CURSOR_Y)) {
		CURRENT_BLOCK = current;
		ImGui::Text("Current Block: %s (id: %d, corner: %d)", CURRENT_BLOCK_ID.c_str(), CURRENT_BLOCK->block_id, CURRENT_BLOCK->block_corner);
		int wallid = CURRENT_BLOCK->wall_id[0];
		if (!CURRENT_BLOCK->isWallOccupied())
			wallid = -1;
		ImGui::Text("Current Wall: %s (id: %d)", BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[0]).toString().c_str(), wallid);

		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
			{
				const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
				ImGui::Text(" -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j], w.toString().c_str());
			}
		int ccx = CURSOR_X;
		int ccy = CURSOR_Y;
		int j = CURSOR_X - ccx < 0.5f?0:1;
		int i = CURSOR_Y - ccy < 0.5f?0:1;
		const Wall& w = BlockRegistry::get().getWall(CURRENT_BLOCK->wall_id[i * 2 + j]);
		ImGui::Text("Selected Wall -> (%d, %d): corner:%d, %s ", j, i, CURRENT_BLOCK->wall_corner[i * 2 + j], w.toString().c_str());

	}

	ImGui::Separator();

	ImGui::Columns(2, "mycolumns3", false);  // 3-ways, no border
	ImGui::Separator();
	if (BLOCK_OR_WALL_SELECTED) {
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
	static bool op = false;
	ImGui::ShowDemoWindow(&op);
}
void WorldLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::KeyPress) {
		auto event = dynamic_cast<KeyPressEvent*>(&e);

		if (event->getKey() == GLFW_KEY_ESCAPE) {
			auto e = WindowCloseEvent();
			Game::get().fireEvent(e);
			event->handled = true;
		}
	}

	if (e.getEventType() == Event::EventType::WindowResize) {
		auto event = dynamic_cast<WindowResizeEvent*>(&e);
		m_render_manager->onScreenResize();
	}

	if (e.getEventType() == Event::EventType::MousePress) {
		auto event = dynamic_cast<MousePressEvent*>(&e);
		if (event->getButton() == GLFW_MOUSE_BUTTON_2)
		{
			if (BLOCK_OR_WALL_SELECTED) {
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
