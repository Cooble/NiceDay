#include "items.h"

#include "audio/player.h"
#include "core/App.h"
#include "core/AppGlobals.h"
#include "world/block/block_datas.h"
#include "world/entity/EntityPlayer.h"

struct ItemToolDataBox;
using namespace nd;

ItemPickaxeCopper::ItemPickaxeCopper()
	: ItemTool(SID("pickaxe"), "pickaxe", TOOL_TYPE_PICKAXE)
{
	m_tier = 1;
	m_efficiency = 0.5f;
}

ItemElPickaxo::ItemElPickaxo()
	: ItemTool(SID("el_pickaxo"), "el_pickaxo", TOOL_TYPE_PICKAXE)
{
	m_tier = 10000;
	m_efficiency = 100.f;
	m_dig_interval = 1;
}

ItemMagicWand::ItemMagicWand()
	: Item(SID("magic_wand"), "magic_wand") {}

//iterate over possible block corner states
bool ItemMagicWand::onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y,
                                        BlockStruct& block) const
{
	if (block.isAir())
		return false;

	int mask = block.block_corner >> 4; //get only lower 4 bits
	mask++;
	if (mask > 0b100)
		mask = 0;
	block.block_corner &= 0x0F;
	block.block_corner |= mask << 4;

	world.getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE)->markDirty(); //update mesh
	return true;
}

ItemHammer::ItemHammer() : ItemTool(SID("hammer"), "hammer", TOOL_TYPE_HAMMER) {}


void ItemHammer::onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y,
                                   Interaction interaction, int ticksPressed) const
{
	glm::ivec2 newPos;
	if (App::get().getSettings()["auto_block_picker"])
	{
		newPos = EntityPlayer::pickBlockToDig(w, owner.getNPos(), glm::vec2(x, y), 7,false);
	}
	else
	{
		newPos = glm::ivec2(x, y);
		AppGlobals::get().nbt["diggingPos"] = NBT(); //remove gui selector
	}
	if (newPos.x != -1)
		AppGlobals::get().nbt["diggingPos"] = newPos;
	else
		AppGlobals::get().nbt["diggingPos"] = NBT();


	//for (auto pos : EntityPlayer::pickBlocksToDig(w, owner.getNPos(), glm::vec2(x, y), 6))
	//	AppGlobals::get().nbt["diggingPos"].push_back(pos);

	auto& player = dynamic_cast<EntityPlayer&>(owner);
	auto data = (ItemToolDataBox*)dataBox;

	if (ticksPressed == 0)
	{
		player.setItemSwinging(true);
		//data->blockX = x;
		//data->blockY = y;
		data->ticksForNextSwing = 0;
	}
	else if (interaction == Interaction::RELEASED)
	{
		data->ticksForNextSwing = 0;
		player.setItemSwinging(false);
		AppGlobals::get().nbt["diggingPos"] = NBT();
		return;
	}
	//decrease counter for next swing
	data->ticksForNextSwing--;
	if (data->ticksForNextSwing < 0)
		data->ticksForNextSwing = 0;

	player.setFacingDir(player.getFacingDirection().x < 0);

	if (newPos.x != -1)
	{
		x = newPos.x;
		y = newPos.y;
	}
	else
	{
		data->ticksForNextSwing = 0;
		//player.setItemSwinging(false);
		return;
	}

	auto structInWorld = w.getBlockM(x, y);

	if (structInWorld->isWallFree())
		return;

	auto& wall = BlockRegistry::get().getWall(structInWorld->wallID());
	auto efficiency = 20; //ticks to dig

	constexpr int hardness = 10;

	if (data->ticksForNextSwing == 0)
	{
		//we haven't started digging yet
		data->ticksForNextSwing = m_dig_interval;

		{
			//spawn particles and update block cracks
			w.spawnWallBreakParticles(x, y, 4);
		}
		{
			auto audioPath = ND_RESLOC("res/audio/dig_stone/dig_") + std::to_string(std::rand() % 4) + ".ogg";
			//play dig sound
			Sounder::get().playSound(audioPath, 0.5f);
		}


		auto& state = getDiggingBlock(x, y);
		if (state.z == -1.f)
			state.z = hardness;

		state.z -= efficiency;
		if (state.z <= 0.f) //block is finished
		{
			if (hardness == 0.f) //insta dig means no waiting for another swing
				data->ticksForNextSwing = 2;

			ItemTool::getDugBlocks().erase(ItemTool::getDugBlocks().begin() + getDiggingIndex(x, y));
			auto t = wall.createItemStackFromWall(*structInWorld);
			if (t == nullptr)
			{
				ASSERT(false, "Cannot spawn item from wall: {}", wall.getStringID());
				return;
			}
			auto itemEntity = (EntityItem*)EntityAllocator::createEntity(ENTITY_TYPE_ITEM);
			itemEntity->setItemStack(t);
			itemEntity->getPosition() = glm::vec2((int)x + 0.5f, (int)y + 0.5f);
			itemEntity->getVelocity() = {0, 5 / 60.f};
			w.spawnWallBreakParticles(x, y);
			w.setWallWithNotify(x, y, 0);
			w.spawnEntity(itemEntity);
		}
	}
}


//what next corner to iterate through using hammer
static int cornerWheel[16] //Wheel, Of, Fortuuune! what did I win?... A sad feeling.
{
	BLOCK_STATE_FULL, //00 BLOCK_STATE_FULL
	BLOCK_STATE_LINE_DOWN, //01 BLOCK_STATE_LINE_UP
	BLOCK_STATE_CORNER_UP_LEFT, //02 BLOCK_STATE_LINE_LEFT
	BLOCK_STATE_CORNER_UP_RIGHT, //03 BLOCK_STATE_CORNER_UP_LEFT
	BLOCK_STATE_LINE_HORIZONTAL, //04 BLOCK_STATE_LINE_DOWN
	BLOCK_STATE_LINE_VERTICAL, //05 BLOCK_STATE_LINE_HORIZONTAL
	BLOCK_STATE_CORNER_DOWN_RIGHT, //06 BLOCK_STATE_CORNER_DOWN_LEFT
	BLOCK_STATE_CORNER_UP_LEFT, //07 BLOCK_STATE_LINE_END_LEFT
	BLOCK_STATE_CORNER_UP_LEFT, //08 BLOCK_STATE_LINE_RIGHT
	BLOCK_STATE_CORNER_DOWN_LEFT, //09 BLOCK_STATE_CORNER_UP_RIGHT
	BLOCK_STATE_CORNER_UP_LEFT, //10 BLOCK_STATE_LINE_VERTICAL
	BLOCK_STATE_CORNER_UP_LEFT, //11 BLOCK_STATE_LINE_END_UP
	BLOCK_STATE_LINE_UP, //12 BLOCK_STATE_CORNER_DOWN_RIGHT
	BLOCK_STATE_CORNER_UP_LEFT, //13 BLOCK_STATE_LINE_END_RIGHT
	BLOCK_STATE_CORNER_UP_LEFT, //14 BLOCK_STATE_LINE_END_DOWN
	BLOCK_STATE_CORNER_UP_LEFT, //15 BLOCK_STATE_BIT
};

bool ItemHammer::onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y,
                                     BlockStruct& block) const
{
	if (block.isAir())
		return false;
	bool hammerFresh = !(block.block_corner & BLOCK_STATE_HAMMER);
	block.block_corner |= BLOCK_STATE_HAMMER;
	auto big_bits = block.block_corner >> 4;

	if (hammerFresh)
	{
		//nothing more, just set hammer bit
	}
	else
		block.block_corner = cornerWheel[block.block_corner & BLOCK_STATE_PURE_MASK];

	//reverse back big bits
	block.block_corner |= big_bits << 4;

	world.getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE)->markDirty();

	return true;
}

ItemWoodHelmet::ItemWoodHelmet()
	: Item(SID("wood_helmet"), "wood_helmet")
{
	setFlag(ITEM_FLAG_ARMOR_HEAD);
	m_max_stack_size = 1;
}

ItemIronHelmet::ItemIronHelmet()
	: Item(SID("iron_helmet"), "iron_helmet")
{
	setFlag(ITEM_FLAG_ARMOR_HEAD);
	m_max_stack_size = 1;
}

ItemWoodChestplate::ItemWoodChestplate()
	: Item(SID("wood_chestplate"), "wood_chestplate")
{
	setFlag(ITEM_FLAG_ARMOR_CHEST);
	m_max_stack_size = 1;
}

ItemWoodLeggins::ItemWoodLeggins()
	: Item(SID("wood_leggins"), "wood_leggins")
{
	setFlag(ITEM_FLAG_ARMOR_LEGGINS);
	m_max_stack_size = 1;
}

ItemWoodBoots::ItemWoodBoots()
	: Item(SID("wood_boots"), "wood_boots")
{
	setFlag(ITEM_FLAG_ARMOR_BOOTS);
	m_max_stack_size = 1;
}

ItemShotgun::ItemShotgun()
	: Item(SID("shotgun"), "shotgun")
{
	//m_has_nbt = true;
	m_max_stack_size = 1;
}

bool ItemShotgun::onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const
{
	float angle = Phys::angleRad(dynamic_cast<Creature&>(owner).getFacingDirection());
	const int BULLET_COUNT = 10;
	constexpr float spreadAngle = 0.35;
	constexpr float speed = 1.2;
	//ND_INFO("NAgle {}", glm::vec2(dynamic_cast<Creature&>(owner).getFacingDirection()).angleDegrees());

	for (int i = 0; i < BULLET_COUNT; ++i)
	{
		float ratio = i / (float)(BULLET_COUNT - 1);

		auto bullet = (EntityRoundBullet*)EntityAllocator::createEntity(ENTITY_TYPE_ROUND_BULLET);
		bullet->getPosition() = owner.getPosition() + glm::vec2(0, 1.5);
		bullet->fire(angle + ((ratio - 0.5f) * spreadAngle), speed);
		bullet->setOwner(owner.getID());
		world.spawnEntity(bullet);
	}

	return true;
}

std::string ItemShotgun::getTitle(ItemStack* stack) const
{
	return Font::colorize(Font::BLACK, Font::DARK_GREY) + "SuperShotgun" + Font::BLACK + "XXX";
}

ItemTnt::ItemTnt()
	: Item(SID("tnt"), "tnt")
{
	m_max_stack_size = 111;
	setFlag(ITEM_FLAG_USE_META_AS_TEXTURE);
	m_max_metadata = 3;
}

bool ItemTnt::onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const
{
	stack.addSize(-1);
	auto tnt = (EntityBomb*)EntityAllocator::createEntity(ENTITY_TYPE_BOMB);
	tnt->getPosition() = owner.getPosition() + glm::vec2(0, 1.5);
	tnt->setBombType(stack.getMetadata() + 1 * 3, stack.getMetadata() == 2, stack.getMetadata());
	tnt->getVelocity() = glm::normalize(glm::vec2(x, y) - owner.getPosition()) * 0.9f;
	world.spawnEntity(tnt);
	return true;
}

static std::array<const char*, 2> vinylPlays = {"Neon", "Tower Clock"};
static std::array<const char*, 2> vinylPlaysPath = {"neon.ogg", "tower_clock.ogg"};

ItemVinyl::ItemVinyl()
	: Item(SID("vinyl"), "vinyl")
{
	m_max_stack_size = 1;
	setFlag(ITEM_FLAG_HAS_NBT);
	m_max_metadata = 2;
}

std::string ItemVinyl::getTitle(ItemStack* stack) const
{
	return Font::colorize(Font::BLACK, Font::YELLOW) + "Vinyl: " + vinylPlays[stack->getMetadata()] + Font::BLACK +
		"XXX";
}

bool ItemVinyl::onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y,
                                    BlockStruct& block) const
{
	if (world.getBlock(x, y)->block_id == BLOCK_RADIO) { }
	return true;
}
