#include "nd_registry.h"
#include "memory/stack_allocator.h"
#include "block/BlockRegistry.h"
#include "block/Block.h"
#include "inventory/ItemBlock.h"
#include "inventory/items.h"
#include "block/basic_walls.h"
#include "block/basic_blocks.h"
#include "entity/EntityRegistry.h"
#include "entity/entities.h"
#include "entity/EntityPlayer.h"
#include "biome/biomes.h"
#include "biome/BiomeRegistry.h"
#include "biome/BiomeForest.h"
#include "particle/particles.h"
#include "ChunkMesh.h"
#include "graphics/BlockTextureCreator.h"
#include "graphics/TextureAtlas.h"
#include "block/block_datas.h"

using namespace nd;

constexpr int particleAtlasSize = 8;
constexpr int ITEM_ATLAS_SIZE = 16;

void nd_registry::registerEverything(bool loadTextures)
{
	ND_TRACE("Registering: Blocks");
	registerBlocks();
	ND_TRACE("Registering: Items");
	registerItems();
	ND_TRACE("Registering: Entities");
	registerEntities();
	ND_TRACE("Registering: Particles");
	registerParticles();
	ND_TRACE("Registering: Biomes");
	registerBiomes();

	//final loading after everything else is loaded
	BlockRegistry::get().readJSONRegistryAfterEntities();

	ND_TRACE("Registering ===> Done");
	if(loadTextures)
	{
		ND_TRACE("TextureLoading: Blocks");
		loadTexturesBlocks();
		ND_TRACE("TextureLoading: Items");
		loadTexturesItems();
		ND_TRACE("TextureLoading: Entities");
		loadTexturesEntities();
		ND_TRACE("TextureLoading: Particles");
		loadTexturesParticles();
		ND_TRACE("TextureLoading ===> Done");
	}
	else 
		ND_TRACE("TextureLoading ===> Skipping loading textures, -> serverWorld");
}

void nd_registry::registerItemBlocks()
{
	nd::temp_string out = "[Registered ItemBlocks]: ";
	out.reserve(200);
	for (auto& block : BlockRegistry::get().getBlocks())
	{
		bool missing = true;
		if (!block->hasItemVersion())
			continue;

		for (auto& item : ItemRegistry::get().getItems())
		{
			if (item.second->getID() == SID(block->getItemIDFromBlock()))
			{
				missing = false;
				break;
			}
		}
		if (missing)
		{
			ND_REGISTER_ITEM(
				new ItemBlock(SID(block->getStringID()), block->getID(), block->getStringID(), block->hasMetaTexturesInRow() ?
					block->getMaxMetadata() : 0
				));
			out += ", " + block->getStringID();
		}
	}
	ND_TRACE(out);
}

void nd_registry::registerItems()
{
	//items
	ND_REGISTER_ITEM(new ItemPickaxeCopper());
	ND_REGISTER_ITEM(new ItemElPickaxo());
	ND_REGISTER_ITEM(new ItemShotgun());
	ND_REGISTER_ITEM(new ItemTnt());
	ND_REGISTER_ITEM(new ItemIronHelmet());
	ND_REGISTER_ITEM(new ItemWoodHelmet());
	ND_REGISTER_ITEM(new ItemWoodChestplate());
	ND_REGISTER_ITEM(new ItemWoodLeggins());
	ND_REGISTER_ITEM(new ItemWoodBoots());
	ND_REGISTER_ITEM(
		&(new ItemBlock(SID("door"), BlockRegistry::get().getBlockID("door_close"), "door"))->setNoBlockTexture(true));
	registerItemBlocks();
}

void nd_registry::registerBlocks()
{
	//sets default to index 1
	BlockRegistry::get().getConnectGroupIndex("air");
	auto i = BlockRegistry::get().getConnectGroupIndex("default");
	ASSERT(i == 1, "");

	//===loading corners
	//blocks
	BlockRegistry::get().registerCorners("dirt", BLOCK_CORNERS_DIRT, false);
	BlockRegistry::get().registerCorners("glass", BLOCK_CORNERS_GLASS, false);
	//walls
	BlockRegistry::get().registerCorners("dirt", WALL_CORNERS_DIRT, true);
	BlockRegistry::get().registerCorners("glass", WALL_CORNERS_GLASS, true);

	BlockRegistry::get().registerBlockBounds("default", BLOCK_BOUNDS_DEFAULT, 3);
	BlockRegistry::get().registerBlockBounds("door", &BLOCK_BOUNDS_DOOR, 1);
	BlockRegistry::get().registerBlockBounds("platform", &BLOCK_BOUNDS_PLATFORM, 1);

	BlockRegistry::get().readExternalIDList();
	
	//blocks
	ND_REGISTER_BLOCK(new BlockAir());
	ND_REGISTER_BLOCK(new BlockPlatform());
	ND_REGISTER_BLOCK(new BlockGrass());
	ND_REGISTER_BLOCK(new BlockGlass());
	ND_REGISTER_BLOCK(new BlockTorch());
	ND_REGISTER_BLOCK(new BlockDoorClose());
	ND_REGISTER_BLOCK(new BlockDoorOpen());
	ND_REGISTER_BLOCK(new BlockTree());
	ND_REGISTER_BLOCK(new BlockTreeSapling());
	ND_REGISTER_BLOCK(new BlockPumpkin());
	ND_REGISTER_BLOCK(new BlockChest());


	//walls
	ND_REGISTER_WALL(new WallAir());
	ND_REGISTER_WALL(new WallGlass());
	
	BlockRegistry::get().readJSONRegistry();
}

void nd_registry::registerEntities()
{
	//entities
	ND_REGISTER_ENTITY(ENTITY_TYPE_PLAYER, EntityPlayer);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TNT, EntityTNT);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ZOMBIE, EntityZombie);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ROUND_BULLET, EntityRoundBullet);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_SAPLING, TileEntitySapling);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_TORCH, TileEntityTorch);
	ND_REGISTER_ENTITY(ENTITY_TYPE_SNOWMAN, EntitySnowman);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_CHEST, TileEntityChest);
	ND_REGISTER_ENTITY(ENTITY_TYPE_ITEM, EntityItem);
	ND_REGISTER_ENTITY(ENTITY_TYPE_BOMB, EntityBomb);
	ND_REGISTER_ENTITY(ENTITY_TYPE_TILE_RADIO, TileEntityRadio);
}

void nd_registry::registerBiomes()
{
	//biomes
	ND_REGISTER_BIOME(new BiomeForest());
	ND_REGISTER_BIOME(new BiomeUnderground());
	ND_REGISTER_BIOME(new BiomeDirt());
}

void nd_registry::registerParticles()
{
	ParticleList::initDefaultParticles(ParticleRegistry::get());
}

void nd_registry::loadTexturesBlocks()
{
	constexpr int segmentCount = 32;
	constexpr int segmentSize = 8;
	
	BlockTextureAtlas atlas;
	std::string blockAtlasFolder = ND_RESLOC("res/images/blockAtlas/");
	atlas.createAtlas(blockAtlasFolder, segmentCount, segmentSize);
	ND_TRACE("Created BlockAtlas with size of {}*{}",segmentCount, segmentCount);
	ChunkMesh::init();
	BlockRegistry::get().initTextures(atlas);
}

void nd_registry::loadTexturesItems()
{
	BlockTextureCreator t;
	t.createTextures();
	TextureAtlas atlas;
	atlas.createAtlas("res/images/itemAtlas", ITEM_ATLAS_SIZE, 32,TextureAtlasFlags_DontCreateTexture|TextureAtlasFlags_CreateFile);
	ND_TRACE("Created TextureAtlas with size of {}*{}", ITEM_ATLAS_SIZE, ITEM_ATLAS_SIZE);
	ItemRegistry::get().initTextures(atlas);
}

void nd_registry::loadTexturesEntities()
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

void nd_registry::loadTexturesParticles()
{
	//particles
	TextureAtlas particleAtlas;
	particleAtlas.createAtlas("res/images/particleAtlas", particleAtlasSize, 8,TextureAtlasFlags_CreateFile|TextureAtlasFlags_DontCreateTexture);
	ND_TRACE("Created ParticleAtlas with size of {}*{}", particleAtlasSize, particleAtlasSize);
	ParticleRegistry::get().initTextures(particleAtlas);
}
