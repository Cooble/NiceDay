#include "ndpch.h"
#include "WorldLayer.h"
#include "world/BlockRegistry.h"
#include "world/LightCalculator.h"
#include "world/WorldIO.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"
#include "Game.h"
#include "Stats.h"
#include "imguiplotvar.h"
#include <GLFW/glfw3.h>


#include <imgui.h>
#include "graphics/Renderer.h"
#include "world/block/Block.h"
#include "world/block/basic_blocks.h"

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

static int BLOCK_PALLETE_SELECTED=0;
WorldLayer::WorldLayer()
	:Layer("WorldLayer") 
{
	BlockRegistry::get().registerBlock(new BlockAir());
	BlockRegistry::get().registerBlock(new BlockStone());
	BlockRegistry::get().registerBlock(new BlockDirt());
	BlockRegistry::get().registerBlock(new BlockGold());
	BlockRegistry::get().registerBlock(new BlockAdamantite());
	BlockRegistry::get().registerBlock(new BlockPlatform());
	BlockRegistry::get().registerBlock(new BlockGrass());

	WorldIOInfo info;
	info.world_name = "NiceWorld";
	info.chunk_width = 5;
	info.chunk_height = 5;
	info.seed = 0;
	info.terrain_level = (info.chunk_height - 2)*WORLD_CHUNK_SIZE;


	bool genW = true;


	if (genW) {
		auto stream = WorldIO::Session(info.world_name + ".world", true,true);
		m_world = stream.genWorldFile(info);
		stream.close();
		m_world->genWorld(info.seed);
	}else
	{
		auto stream = WorldIO::Session(info.world_name + ".world", false);
		m_world = stream.loadWorld();
		stream.close();
	}

	if (!m_world) {
		ND_ERROR("cannot load world file corrputed");
	}


	m_chunk_loader = new ChunkLoader(m_world);
	m_cam = new Camera();
	glm::vec2 po = { m_world->getInfo().chunk_width*WORLD_CHUNK_SIZE/2,m_world->getInfo().terrain_level };
	m_cam->setPosition(po);
	m_cam->setChunkRadius(4);

	m_chunk_loader->registerEntity(dynamic_cast<IChunkLoaderEntity*>(m_cam));
	m_chunk_loader->onUpdate();

	ChunkMesh::init();
	LightCalculator& c = m_world->getLightCalculator();
	c.registerLight(dynamic_cast<LightSource*>(m_cam));
	m_render_manager = new WorldRenderManager(m_cam,m_world);


}

WorldLayer::~WorldLayer() {
	delete m_chunk_loader;
	delete m_world;

}

void WorldLayer::onAttach()
{

}
void WorldLayer::onDetach()
{
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
	if(lightCalcDelay)
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

	CURSOR_X = CURSOR_X / BLOCK_PIXEL_SIZE*2+m_cam->getPosition().x;
	CURSOR_Y = CURSOR_Y / BLOCK_PIXEL_SIZE*2 + m_cam->getPosition().y;

	if(m_world->isValidBlock(CURSOR_X,CURSOR_Y))
	{
		
	}
	CURRENT_BLOCK = &m_world->getBlock(CURSOR_X, CURSOR_Y);
	CURRENT_BLOCK_ID = BlockRegistry::get().getBlock(m_world->getBlock(CURSOR_X, CURSOR_Y).id).toString();

	if (Game::get().getInput().isMousePressed(GLFW_MOUSE_BUTTON_1))
		if (m_world->getBlock(CURSOR_X, CURSOR_Y).id != BLOCK_PALLETE_SELECTED)
			m_world->setBlock(CURSOR_X, CURSOR_Y, BLOCK_PALLETE_SELECTED);

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
}
void WorldLayer::onRender()
{
	m_render_manager->render();

}
void WorldLayer::onImGuiRender()
{
	float fps = FLT_MAX;
	if(fpsCount>Game::get().getTargetTPS())
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

	ImGui::PlotVar("FPS", fps);
	bool l = Stats::light_enable;
	ImGui::Checkbox(Stats::light_enable?"Lighting Enabled":"Lighting Disabled", &Stats::light_enable);
	if(l!=Stats::light_enable)
	{
		if(!l) m_world->getLightCalculator().run();
		else m_world->getLightCalculator().stop();
	}
	ImGui::Checkbox(Stats::move_through_blocks_enable?"Move through Blocks Enabled":"Move through Blocks Disabled", &Stats::move_through_blocks_enable);
	ImGui::InputFloat("Player speed: ",&Stats::player_speed);
	ImGui::InputFloat("Light Intensity: ",&Stats::player_light_intensity);

	ImGui::Text("World filepath: %s", WORLD_FILE_PATH);
	ImGui::Text("Chunks Size: (%d/%d)", WORLD_CHUNK_WIDTH, WORLD_CHUNK_HEIGHT);
	ImGui::Text("Chunks loaded: %d", CHUNKS_LOADED);
	ImGui::Text("Chunks drawn: %d", CHUNKS_DRAWN);
	ImGui::Text("Cam X: %.2f", CAM_POS_X);
	ImGui::Text("Cam Y: %.2f", CAM_POS_Y);
	ImGui::Text("Cursor X: %.2f", CURSOR_X);
	ImGui::Text("Cursor y: %.2f", CURSOR_Y);
	if(CURRENT_BLOCK)
		ImGui::Text("Current Block: %s (id: %d, corner: %d)", CURRENT_BLOCK_ID.c_str(),CURRENT_BLOCK->id,CURRENT_BLOCK->corner);
	ImGui::Separator();

	ImGui::Columns(2, "mycolumns3", false);  // 3-ways, no border
	ImGui::Separator();
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
	ImGui::Separator();

	ImGui::Text("Drawn Chunks:");
	auto& map = m_render_manager->getMap();
	int ind = 0;
	for (auto& iterator = map.begin(); iterator != map.end(); ++iterator) {
		char label[32];
		int x, y;
		Chunk::getChunkPosFromID(iterator->first,x,y);
		sprintf(label, "%d, %d", x,y);
		ImGui::Text(label);		
		ind++;
		if (ind == map.size() / 2)
			ImGui::NextColumn();
	}
	ImGui::Separator();
	
	ImGui::End();
}
void WorldLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::KeyPress) {
		auto event = dynamic_cast<KeyPressEvent*>(&e);

		if (event->getKey() == GLFW_KEY_ESCAPE) {
			Game::get().stop();
			event->handled = true;
		}
	}

	if (e.getEventType() == Event::EventType::WindowResize) {
		auto event = dynamic_cast<WindowResizeEvent*>(&e);
		m_render_manager->onScreenResize();
	}

	if (e.getEventType() == Event::EventType::MousePress) {
		auto event = dynamic_cast<MousePressEvent*>(&e);
		if(event->getButton()==GLFW_MOUSE_BUTTON_2)
		{
			BLOCK_PALLETE_SELECTED = m_world->getBlock(CURSOR_X, CURSOR_Y).id;
			event->handled = true;
		}
	}

}
