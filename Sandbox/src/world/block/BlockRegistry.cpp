#include "ndpch.h"
#include "BlockRegistry.h"
#include "world/entity/EntityRegistry.h"
#include "basic_blocks.h"

static std::vector<std::string> allConnectGroups;
void BlockRegistry::initTextures(const BlockTextureAtlas& atlas)
{
	for (Block* block : m_blocks)
		block->onTextureLoaded(atlas);
	for (Wall* wall : m_walls)
		wall->onTextureLoaded(atlas);
}

void BlockRegistry::readAllConnectGroupsJSON()
{
	allConnectGroups.clear();
	NBT t;
	NBT::loadFromFile("res/registry/blocks/connect_groups.json", t);
	if(t.isArray())
	{
		allConnectGroups.reserve(t.size());
		for (auto& item : t.arrays())
			allConnectGroups.push_back(item.string());
	}
}



void BlockRegistry::checkIfConnectGroupRegistered(const std::string& s,const std::string& elementID)
{
	for (auto& all_connect_group : allConnectGroups)
		if(all_connect_group==s)
			return;
	ND_WARN("Unregistered connectGroup detected: {} in: {}", s,elementID);
}

void BlockRegistry::readExternalIDList()
{
	{
		auto t = std::ifstream(ND_RESLOC("res/registry/blocks/blocks.ids"));
		std::string line;
		while (std::getline(t, line))
		{
			std::istringstream iss(line);
			if (line.find(' ') == std::string::npos)
				continue;
			int index;
			std::string name;
			if (!(iss >> index >> name)) { continue; } // error
			m_blockIDs[name] = index;
		}
	}
	{
		auto t = std::ifstream(ND_RESLOC("res/registry/blocks/walls.ids"));
		std::string line;
		while (std::getline(t, line))
		{
			std::istringstream iss(line);
			if (line.find(' ') == std::string::npos)
				continue;
			int index;
			std::string name;
			if (!(iss >> index >> name)) { continue; } // error
			m_wallIDs[name] = index;
		}
	}

	//prepare arrays, insert nullptrs
	m_blocks.resize(m_blockIDs.size());
	ZeroMemory(m_blocks.data(), m_blockIDs.size() * sizeof(Block*));
	m_walls.resize(m_wallIDs.size());
	ZeroMemory(m_walls.data(), m_wallIDs.size() * sizeof(Wall*));

	readAllConnectGroupsJSON();

	//first we register SpecialFlags
	m_block_flags.clear();
	m_block_flags["hasItemVersion"] = 0;
	m_block_flags["metaTexturesInRow"] = 1;
	m_block_flags["needsWall"] = 2;
	m_block_flags["cannotFloat"] = 3;
	m_block_flags["hasBigTexture"] = 4;
	m_block_flags["solid"] = 5;
	
	{
		auto t = std::ifstream(ND_RESLOC("res/registry/blocks/flags.ids"));
		std::string line;
		while (std::getline(t, line))
		{
			std::istringstream iss(line);
			if (line.find(' ') == std::string::npos)
				continue;
			int index;
			std::string name;
			if (!(iss >> index >> name)) { continue; } // error
			m_block_flags[name]=index;
		}
	}
	
}

static NBT blocksNBT;
void BlockRegistry::readJSONRegistry()
{

	if (NBT::loadFromFile(ND_RESLOC("res/registry/blocks/blocks.json"), blocksNBT))
		for (auto& map : blocksNBT.maps())
		{
			auto it = m_blockIDs.find(map.first);
			if (it == m_blockIDs.end())
				createBlockFromJSON(map.first, map.second);
			else if(m_blocks[it->second]==nullptr)
				createBlockFromJSON(map.first, map.second);
			updateBlockFromJSON(map.first, map.second);
		}
	NBT base_nbt;
	if (NBT::loadFromFile(ND_RESLOC("res/registry/blocks/walls.json"), base_nbt))
		for (auto& map : base_nbt.maps())
		{
			auto it = m_wallIDs.find(map.first);
			if (it == m_wallIDs.end())
				createWallFromJSON(map.first, map.second);
			else if (m_walls[it->second] == nullptr)
				createWallFromJSON(map.first, map.second);
			updateWallFromJSON(map.first, map.second);
		}
}

void BlockRegistry::readJSONRegistryAfterEntities()
{
	for (auto& map : blocksNBT.maps())
		updateBlockFromJSONAfterEntities(map.first, map.second);
	blocksNBT = NBT();
}

void BlockRegistry::createBlockFromJSON(const std::string& id, NBT& nbt)
{
	//ND_TRACE("Creating block which has not src code: {}", id);
	if (nbt.exists("multiBlock"))
		registerBlock(new MultiBlock(id));
	else if (nbt.exists("plant") && nbt["plant"].isBool() && (bool)nbt["plant"])
		registerBlock(new BlockPlant(id));
	else
		registerBlock(new Block(id));
}

void BlockRegistry::updateBlockFromJSON(const std::string& id, NBT& nbt)
{
	Block* block = m_blocks[m_blockIDs[id]];
	nbt.load("hardness", block->m_hardness);
	//nbt.load("maxStackSize", block->m_ma);
	if(nbt.exists("opacity"))
	{
		auto& t = nbt["opacity"];
		if (t.isString())
		{
			if (t.string() == "air")
				block->m_opacity = OPACITY_AIR;
			else if (t.string() == "solid")
				block->m_opacity = OPACITY_SOLID;
		}
		else block->m_opacity = t;
	}
	nbt.load("lightSrc", block->m_light_src);
	nbt.load("maxMeta", block->m_max_metadata);

	auto& hasBigTexture = nbt["hasBigTexture"];
	if (hasBigTexture.isBool())
		block->setFlag(BLOCK_FLAG_HAS_BIG_TEXTURE,hasBigTexture);
	auto& hasItemVersion = nbt["hasItemVersion"];
	if (hasItemVersion.isBool())
		block->setFlag(BLOCK_FLAG_HAS_ITEM_VERSION, hasItemVersion);
	auto& needsWall = nbt["needsWall"];
	if (needsWall.isBool())
		block->setFlag(BLOCK_FLAG_NEEDS_WALL, needsWall);
	auto& cannotFloat = nbt["cannotFloat"];
	if (cannotFloat.isBool())
		block->setFlag(BLOCK_FLAG_CANNOT_FLOAT, cannotFloat);
	auto& metaTexInRow = nbt["metaTexturesInRow"];
	if (metaTexInRow.isBool())
		block->setFlag(BLOCK_FLAG_HAS_METATEXTURES_IN_ROW, metaTexInRow);
	auto& isSolid = nbt["solid"];
	if (isSolid.isBool())
		block->setFlag(BLOCK_FLAG_SOLID, isSolid);

	//other custom flags
	auto& otherFlags = nbt["flags"];
	if (otherFlags.isMap())
		for (auto& pair : otherFlags.maps())
			block->setFlag(getFlagIndex(pair.first),pair.second);
	else if(otherFlags.isArray())
		for (auto& item : otherFlags.arrays())
			block->setFlag(getFlagIndex(item.string()), true);
	
	//multiblock
	if (!dynamic_cast<MultiBlock*>(block) && nbt.exists("multiBlock"))
		ND_WARN("[{}] MultiBlock is specified in json but in source code it is not! so its ignored", id);
	else if (nbt.exists("multiBlock"))
	{
		auto& mltNBT = nbt["multiBlock"];
		MultiBlock* multi = dynamic_cast<MultiBlock*>(block);
		mltNBT.load("width", multi->m_width);
		mltNBT.load("height", multi->m_height);
	}
	//group index
	auto& groups = block->m_block_connect_group;
	if (nbt.exists("connectGroups"))
	{
		groups = 0;
		auto& ii = nbt["connectGroups"];
		if (ii.isArray())
			for (auto& item : ii.arrays()) {
				checkIfConnectGroupRegistered(item.string(), id);
				groups |= 1 << getConnectGroupIndex(item.string());
			}
		else {
			checkIfConnectGroupRegistered(ii.string(), id);
			groups |= 1 << getConnectGroupIndex(ii.string());
		}
	}
	else groups = 1 << getConnectGroupIndex("default");

	//corners it can id of corners or 'false' or '0' to not use anything
	if (nbt.exists("corners") && nbt["corners"].isString())
		block->m_corner_translate_array = getCorners(nbt["corners"].string(), false);

	if (nbt.exists("collision")) {
		auto& item = nbt["collision"];
		if (item.isString()) {
			int size;
			block->m_collision_box = getBlockBounds(item.string(), size);
			block->m_collision_box_size = size;
		}
		else if (item.isArray()) {
			std::string name;
			Phys::Polygon* gon;
			int numberOfTypes = 0;
			if (item[0].isArray()) {
				for (auto& arra : item.arrays())
				{
					if (arra.isArray())
						numberOfTypes++;
					else name = arra.string();
				}
				gon = new Phys::Polygon[numberOfTypes];
				for (int i = 0; i < numberOfTypes; ++i)
				{
					auto& ray = item[i];
					gon[i] = Phys::toPolygon(Phys::Rectangle::createFromDimensions(ray[0], ray[1], ray[2], ray[3]));
				}
			}
			else
			{
				gon = new Phys::Polygon();
				gon[0] = Phys::toPolygon(Phys::Rectangle::createFromDimensions(item[0], item[1], item[2], item[3]));
				if (item.size() == 5)
					name = item[4];
			}
			registerBlockBounds(name.empty() ? id + "_collision" : name, gon, numberOfTypes);
			block->m_collision_box = gon;
			block->m_collision_box_size = numberOfTypes;
		}
		else if(item.isBool()&&!((bool)item))
		{
			block->m_collision_box = nullptr;
			block->m_collision_box_size = 0;
		}
	}

}

void BlockRegistry::updateBlockFromJSONAfterEntities(const std::string& id, NBT& nbt)
{
	Block* block = m_blocks[m_blockIDs[id]];
	if (nbt.exists("tileEntity"))
		block->m_tile_entity = EntityRegistry::get().getEntityType(nbt["tileEntity"].string());
	
}


void BlockRegistry::createWallFromJSON(const std::string& id, NBT& nbt)
{
	registerWall(new Wall(id));
}

void BlockRegistry::updateWallFromJSON(const std::string& id, NBT& nbt)
{
	Wall* wall = m_walls[m_wallIDs[id]];
	nbt.load("isTransparent", wall->m_transparent);

	//corners it can id of corners or 'false' or '0' to not use anything
	if (nbt.exists("corners") && nbt["corners"].isString())
		wall->m_corner_translate_array = getCorners(nbt["corners"].string(), true);
}

void BlockRegistry::registerBlock(Block* block)
{
	auto it = m_blockIDs.find(block->getStringID());
	ASSERT(it != m_blockIDs.end(), "Unregistered block name, write entry in blocks.ids");
	m_blocks[it->second] = block;
	block->m_id = it->second;
}

void BlockRegistry::registerWall(Wall* wall)
{
	auto it = m_wallIDs.find(wall->getStringID());
	ASSERT(it != m_wallIDs.end(), "Unregistered wall name, write entry in walls.ids");
	m_walls[it->second] = wall;
	wall->m_id = it->second;
}

const Block& BlockRegistry::getBlock(BlockID block_id)
{
	ASSERT(m_blocks.size() > block_id&&block_id>=0, "Invalid block id");
	return *m_blocks[block_id];
}

const Wall& BlockRegistry::getWall(int wall_id)
{
	ASSERT(m_walls.size() > wall_id&&wall_id >= 0, "Invalid wall id");
	return *m_walls[wall_id];
}

const Block& BlockRegistry::getBlock(const std::string& block_id) const
{
	ASSERT(m_blockIDs.find(block_id)!=m_blockIDs.end(), "Invalid block id");
	return *m_blocks[m_blockIDs.at(block_id)];
}

const Wall& BlockRegistry::getWall(const std::string& wall_id) const
{
	return *m_walls[m_wallIDs.at(wall_id)];
}

void BlockRegistry::registerCorners(const std::string& id, const half_int* data, bool isWall)
{
	if (!isWall)
		m_block_corners[id] = data;
	else m_wall_corners[id] = data;
}

void BlockRegistry::registerBlockBounds(const std::string& id, const Phys::Polygon* data, int size)
{
	m_block_bounds[id] = std::make_pair(data,size);
}

int BlockRegistry::getConnectGroupIndex(const std::string& id)
{
	auto& it = m_connect_groups.find(id);
	if (it == m_connect_groups.end())
		m_connect_groups[id] = m_currentConnectGroup++;
	return m_connect_groups[id];
}

int BlockRegistry::getFlagIndex(const std::string& id)
{
	auto& it = m_block_flags.find(id);
	ASSERT(it != m_block_flags.end(), "Unregistered block flag {}",id);
	return m_block_flags[id];
}

const half_int* BlockRegistry::getCorners(const std::string& id, bool isWall) const
{
	if (!isWall)
	{
		auto& it = m_block_corners.find(id);
		ASSERT(it != m_block_corners.end(), "use of unregistered corners");
		return it->second;
	}
	auto& it = m_wall_corners.find(id);
	ASSERT(it != m_wall_corners.end(), "use of unregistered corners");
	return it->second;
}

const Phys::Polygon* BlockRegistry::getBlockBounds(const std::string& id,int& size) const
{
	size = 0;
	auto& it = m_block_bounds.find(id);
	if (it == m_block_bounds.end())
		return nullptr;
	size = it->second.second;
	return it->second.first;
}

BlockRegistry::~BlockRegistry()
{
	for (auto b : m_blocks)
		delete b;
	for (auto b : m_walls)
		delete b;
}
