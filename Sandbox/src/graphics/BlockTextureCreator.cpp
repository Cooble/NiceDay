﻿#include "BlockTextureCreator.h"
#include "world/block/BlockRegistry.h"
#include "graphics/API/Texture.h"
#include "graphics/Effect.h"
#include "graphics/GContext.h"
#include "world/ChunkMesh.h"
#include "platform/OpenGL/GLRenderer.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include <filesystem>

using namespace nd;
static int itemBlockSize = 32;

void BlockTextureCreator::createTextures()
{
	stbi_flip_vertically_on_write(true);

	ND_TRACE("[BlockTextureCreator]: making textures...");
	m_fbo.replaceTexture(nd::Texture::create(TextureInfo().size(itemBlockSize)));

	m_vbo = VertexBuffer::create(nullptr, sizeof(ChunkMesh::PosVertexData) * 6 * 9, BufferUsage::DYNAMIC_DRAW);
	m_vbo->setLayout(ChunkMesh::getLayout());
	m_vao = VertexArray::create();
	m_vao->addBuffer(*m_vbo);

	auto& blocks = BlockRegistry::get().getBlocks();
	for (auto& block : blocks)
		if (block->hasItemVersion())
			createTexture(*block);
	auto& walls = BlockRegistry::get().getWalls();
	for (auto& wall : walls)
		if (wall->hasItemVersion())
			createTexture(*wall);

	ND_TRACE("[BlockTextureCreator]: making textures... {} done", blocks.size()+walls.size());

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	delete m_vbo;
	delete m_vao;
}


static void drawQuad(ChunkMesh::PosVertexData* point, float x, float y, half_int t_offset, half_int t_corner_offset,
                     float co, float co_corner,float size=1)
{
	//0,0
	{
		point->pos = glm::vec2(x, y);
		//check if t_offset == -1 -> discard
		point->uv_0 = glm::vec2(t_offset.x, t_offset.y) * co;
		point->uv_1 = glm::vec2(t_corner_offset.x, t_corner_offset.y) * co_corner;
	}
	++point;
	//1,0
	{
		point->pos = glm::vec2(x + size, y);
		//check if t_offset == -1 -> discard
		point->uv_0 = glm::vec2(t_offset.x + 1, t_offset.y) * co;
		point->uv_1 = glm::vec2(t_corner_offset.x + 1, t_corner_offset.y) * co_corner;
	}
	++point;
	//1,1
	{
		point->pos = glm::vec2(x + size, y + size);
		//check if t_offset == -1 -> discard
		point->uv_0 = glm::vec2(t_offset.x + 1, t_offset.y + 1) * co;
		point->uv_1 = glm::vec2(t_corner_offset.x + 1, t_corner_offset.y + 1) * co_corner;
	}
	++point;
	//0,0
	{
		auto pp = point - 3;
		point->pos = pp->pos;
		point->uv_0 = pp->uv_0;
		point->uv_1 = pp->uv_1;
	}
	++point;
	//1,1
	{
		auto pp = point - 2;
		point->pos = pp->pos;
		point->uv_0 = pp->uv_0;
		point->uv_1 = pp->uv_1;
	}
	++point;
	//0,1
	{
		point->pos = glm::vec2(x, y + size);
		//check if t_offset == -1 -> discard
		point->uv_0 = glm::vec2(t_offset.x, t_offset.y + 1) * co;
		point->uv_1 = glm::vec2(t_corner_offset.x, t_corner_offset.y + 1) * co_corner;
	}
}

void BlockTextureCreator::createTexture(const Block& block)
{
	FrameBufferTexturePair tempFBO;

	float co = 1.0f / BLOCK_TEXTURE_ATLAS_SIZE;
	float co_corner = 1.0f / BLOCK_CORNER_ATLAS_SIZE;

	auto possibleLoc = std::string(ND_RESLOC("res/images/itemAtlas/item/" + block.getItemIDFromBlock() + ".png"));
	auto destLoc = std::string(ND_RESLOC("res/images/itemAtlas/item_block/" + block.getItemIDFromBlock() + ".png"));

	std::filesystem::create_directories(ND_RESLOC("res/images/itemAtlas/item_block"));
	//overriding default block image generation
	if (std::filesystem::is_regular_file(possibleLoc))
	{
		if (std::filesystem::exists(destLoc))
			std::filesystem::remove(destLoc);
		std::filesystem::copy(possibleLoc, destLoc.c_str());
		/*
		int width = 0;
		int height = 0;
		int BPP = 0;
		void* currentImage = stbi_load(possibleLoc.c_str(), &width, &height, &BPP, 4);
		ASSERT(currentImage, "invalid image");
		stbi_write_png(std::string(ND_RESLOC("res/images/itemAtlas/item_block/" + block.toString() + ".png")).c_str(), 
			width, height, STBI_rgb_alpha, currentImage, itemBlockSize * 4 * width);
		stbi_image_free(currentImage);*/
		return;
	}

	if (block.getMaxMetadata() == 0)
	{
		BlockStruct templ = {};
		templ.block_id = block.getID();
		templ.block_metadata = 0;
		templ.block_corner = BLOCK_STATE_BIT;

		half_int t_offset = block.getTextureOffset(0, 0, templ);
		half_int t_corner_offset = block.getCornerOffset(0, 0, templ);

		m_vbo->bind();
		auto point = (ChunkMesh::PosVertexData*)m_vbo->mapPointer();

		float x = -0.5f;
		float y = -0.5f;
		drawQuad(point, x, y, t_offset, t_corner_offset, co, co_corner);
		m_vbo->unMapPointer();

		m_fbo.bind();
		Gcon.setClearColor(0, 0, 0, 0);
		Gcon.clear(BuffBit::COLOR);
		m_vao->bind();
		ChunkMesh::getProgram()->bind();
		std::static_pointer_cast<internal::GLShader>(ChunkMesh::getProgram())->setUniformMat4(
			"u_transform", glm::mat4(1.f));
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);

		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		m_fbo.unbind();

		GLCall(glBindTexture(GL_TEXTURE_2D, m_fbo.getTexture()->getID()));
		auto deformArray = new GLubyte[itemBlockSize * itemBlockSize * 4];

		GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, deformArray));

		stbi_write_png(
			std::string(ND_RESLOC("res/images/itemAtlas/item_block/" + block.getItemIDFromBlock() + ".png")).c_str(),
			itemBlockSize, itemBlockSize, STBI_rgb_alpha, deformArray, itemBlockSize * 4);

		delete[] deformArray;
	}
	else if (block.hasMetaTexturesInRow())
	{
		int maxMetaSize = block.getMaxMetadata();
		tempFBO.replaceTexture(nd::Texture::create(TextureInfo().size(itemBlockSize * maxMetaSize, itemBlockSize)));
		tempFBO.bind();
		Gcon.setClearColor(0, 0, 0, 0);
		Gcon.clear(BuffBit::COLOR);
		m_vao->bind();
		ChunkMesh::getProgram()->bind();
		auto m = glm::scale(glm::mat4(1.f), {1.f / maxMetaSize, 1, 1});
		std::static_pointer_cast<nd::internal::GLShader>(ChunkMesh::getProgram())->setUniformMat4("u_transform", m);
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);

		for (int currentMeta = 0; currentMeta < maxMetaSize; ++currentMeta)
		{
			BlockStruct templ = {};
			templ.block_id = block.getID();
			templ.block_metadata = currentMeta;
			templ.block_corner = BLOCK_STATE_BIT;

			half_int t_offset = block.getTextureOffset(0, 0, templ);
			half_int t_corner_offset = block.getCornerOffset(0, 0, templ);

			m_vbo->bind();
			auto point = (ChunkMesh::PosVertexData*)m_vbo->mapPointer();


			float x = -maxMetaSize + 0.5f + currentMeta * 2;
			float y = -0.5f;
			drawQuad(point, x, y, t_offset, t_corner_offset, co, co_corner);
			m_vbo->unMapPointer();
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		}
		tempFBO.unbind();

		GLCall(glBindTexture(GL_TEXTURE_2D, tempFBO.getTexture()->getID()));
		auto deformArray = new GLubyte[itemBlockSize * maxMetaSize * itemBlockSize * 4];

		GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, deformArray));

		stbi_write_png(
			std::string(ND_RESLOC("res/images/itemAtlas/item_block/" + block.getItemIDFromBlock() + ".png")).c_str(),
			itemBlockSize * maxMetaSize, itemBlockSize, STBI_rgb_alpha, deformArray, itemBlockSize * 4 * maxMetaSize);

		delete[] deformArray;
	}
}

void BlockTextureCreator::createTexture(const Wall& wall)
{
	FrameBufferTexturePair tempFBO;

	float co = 1.0f / WALL_TEXTURE_ATLAS_SIZE;
	float co_corner = 1.0f / WALL_CORNER_ATLAS_SIZE;

	auto possibleLoc = std::string(ND_RESLOC("res/images/itemAtlas/item/" + wall.getItemIDFromWall() + ".png"));
	auto destLoc = std::string(ND_RESLOC("res/images/itemAtlas/item_wall/" + wall.getItemIDFromWall()+ ".png"));

	std::filesystem::create_directories(ND_RESLOC("res/images/itemAtlas/item_wall"));
	//overriding default block image generation
	if (std::filesystem::is_regular_file(possibleLoc))
	{
		if (std::filesystem::exists(destLoc))
			std::filesystem::remove(destLoc);
		std::filesystem::copy(possibleLoc, destLoc.c_str());
		/*
		int width = 0;
		int height = 0;
		int BPP = 0;
		void* currentImage = stbi_load(possibleLoc.c_str(), &width, &height, &BPP, 4);
		ASSERT(currentImage, "invalid image");
		stbi_write_png(std::string(ND_RESLOC("res/images/itemAtlas/item_block/" + block.toString() + ".png")).c_str(), 
			width, height, STBI_rgb_alpha, currentImage, itemBlockSize * 4 * width);
		stbi_image_free(currentImage);*/
		return;
	}


	BlockStruct templ = {};
	templ.setWall(wall.getID());

	m_vbo->bind();
	auto point = (ChunkMesh::PosVertexData*)m_vbo->mapPointer();

	char corners[9]
	{

		BLOCK_STATE_CORNER_DOWN_LEFT,
		BLOCK_STATE_LINE_DOWN,
		BLOCK_STATE_CORNER_DOWN_RIGHT,
		
		BLOCK_STATE_LINE_LEFT,
		BLOCK_STATE_FULL,
		BLOCK_STATE_LINE_RIGHT,

		BLOCK_STATE_CORNER_UP_LEFT,
		BLOCK_STATE_LINE_UP,
		BLOCK_STATE_CORNER_UP_RIGHT,
	};
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
		{
			half_int t_offset = wall.getTextureOffset(x, y, templ);
			templ.wall_corner[0] = corners[y * 3 + x];
			half_int t_corner_offset = wall.getCornerOffset(0, 0, templ);
			drawQuad(point, -0.75f+x*0.5f,-0.75f+y*0.5f, t_offset, t_corner_offset, co, co_corner,0.5f);
			point += 6;
		}

	m_vbo->unMapPointer();

	m_fbo.bind();
	Gcon.setClearColor(0, 0, 0, 0);
	Gcon.clear(BuffBit::COLOR);
	m_vao->bind();
	ChunkMesh::getProgram()->bind();
	std::static_pointer_cast<internal::GLShader>(ChunkMesh::getProgram())->setUniformMat4("u_transform", glm::mat4(1.f));
	ChunkMesh::getAtlas()->bind(0);
	ChunkMesh::getCornerAtlas()->bind(1);

	GLCall(glDrawArrays(GL_TRIANGLES, 0, 6*9));
	m_fbo.unbind();

	GLCall(glBindTexture(GL_TEXTURE_2D, m_fbo.getTexture()->getID()));
	auto deformArray = new GLubyte[itemBlockSize * itemBlockSize * 4];

	GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, deformArray));

	stbi_write_png(
		std::string(ND_RESLOC("res/images/itemAtlas/item_wall/" + wall.getItemIDFromWall() + ".png")).c_str(),
		itemBlockSize, itemBlockSize, STBI_rgb_alpha, deformArray, itemBlockSize * 4);

	delete[] deformArray;
}
