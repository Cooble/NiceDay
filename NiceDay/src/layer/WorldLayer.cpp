#include "ndpch.h"
#include "WorldLayer.h"
#include "world/BlockRegistry.h"
#include "world/WorldIO.h"
#include "Core.h"
#include "event/KeyEvent.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

const char* WORLD_FILE_PATH;
int CHUNKS_LOADED;
int WORLD_CHUNK_WIDTH;
int WORLD_CHUNK_HEIGHT;

float CAM_POS_X;
float CAM_POS_Y;

class ChunkLoadingCam: public IChunkLoaderEntity
{


public:
	glm::vec2 pos;
	ChunkLoadingCam(){}
	~ChunkLoadingCam(){}
	
	virtual const glm::vec2& getPosition() const override
	{
		return pos;
	}
	void setPos(glm::vec2& vec)
	{
		pos = vec;
	}


	/*Return 0 if no chunk should be updated*/
	virtual int getChunkRadius() const override
	{
		return 4;
	}


};
WorldLayer::WorldLayer()
	:Layer("WorldLayer") 
{
	Block stone(0);
	BlockRegistry::get().registerBlock(stone);
	WorldIOInfo info;
	info.world_name = "NiceWorld";
	info.chunk_width = 5;
	info.chunk_height = 5;
	info.seed = 0;
	//m_world = WorldIO::loadWorld(info.world_name);
	auto stream = WorldIO::Session(info.world_name+".world", true,true);

	m_world = stream.genWorld(info);
	stream.close();

	m_world->genWorld(info.seed);

	m_chunk_loader = new ChunkLoader(m_world);
	m_cam = new ChunkLoadingCam();
	m_cam->pos = { 2,1 };
	m_chunk_loader->registerEntity(m_cam);
	m_chunk_loader->onUpdate();
	m_cam->pos = { 128,2 };
	m_chunk_loader->onUpdate();

	ChunkMesh::init();
	m_mesh = new ChunkMeshInstance();
	m_mesh->createVBOFromChunk(*m_world, m_world->getChunk(0,0));



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
	auto s = WorldIO::Session(m_world->getFilePath(), true);
	s.saveWorld(m_world);
	s.close();
}
void WorldLayer::onUpdate()
{
	CAM_POS_X = (int)m_cam->pos.x>>WORLD_CHUNK_BIT_SIZE;
	CAM_POS_Y = (int)m_cam->pos.y >> WORLD_CHUNK_BIT_SIZE;
	CHUNKS_LOADED = m_world->getNumberOfLoadedChunks();
	WORLD_FILE_PATH = m_world->getFilePath().c_str();
	WORLD_CHUNK_WIDTH = m_world->getInfo().chunk_width;
	WORLD_CHUNK_HEIGHT = m_world->getInfo().chunk_height;

	m_chunk_loader->onUpdate();
	m_world->onUpdate();
}
void WorldLayer::onRender()
{
	m_world->onRender();
	m_mesh->render();
}
void WorldLayer::onImGuiRender()
{
	static bool show = true;

	if (!ImGui::Begin("WorldInfo", &show))
	{
		ImGui::End();
		return;
	}
	ImGui::Text("World filepath: %s", WORLD_FILE_PATH);
	ImGui::Text("Chunks Size: (%d/%d)", WORLD_CHUNK_WIDTH, WORLD_CHUNK_HEIGHT);
	ImGui::Text("Chunks loaded: %d", CHUNKS_LOADED);
	ImGui::Text("Cam X: %.2f", CAM_POS_X);
	ImGui::Text("Cam Y: %.2f", CAM_POS_Y);
	ImGui::Separator();

	ImGui::Text("Active Chunks:");
	ImGui::Columns(2, "mycolumns3", false);  // 3-ways, no border
	ImGui::Separator();
	auto& map = m_world->getMap();
	int ind = 0;
	for (auto& iterator = map.begin(); iterator != map.end(); ++iterator) {
		char label[32];
		int x, y;
		Chunk::getChunkPosFromID(iterator->first,x,y);
		sprintf(label, "%d, %d", x,y);
		ImGui::Text(label);
		//if (ImGui::Selectable(label)) {}
		
		ind++;
		if (ind == map.size() / 2)
			ImGui::NextColumn();
	}

	

	//ImGui::Columns(1);
	//ImGui::Separator();
	ImGui::End();
}
void WorldLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::KeyPress) {
		auto event = dynamic_cast<KeyPressEvent*>(&e);
		//if (event->getNumber() == 0) {
			if (event->getKey() == GLFW_KEY_RIGHT) 
				m_cam->pos.x += 64;
			if (event->getKey() == GLFW_KEY_LEFT)
				m_cam->pos.x -= 64;
			if (event->getKey() == GLFW_KEY_UP)
				m_cam->pos.y += 64;
			if (event->getKey() == GLFW_KEY_DOWN)
				m_cam->pos.y -= 64;
		//}
	}

}
