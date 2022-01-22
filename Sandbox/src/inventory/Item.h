#pragma once
#include "ndpch.h"
#include "core/sids.h"

namespace nd { class TextureAtlas; }

constexpr int ITEMSTACK_POOL_SIZE = 1000;

class WorldEntity;
class World;
class ItemStack;
typedef uint64_t ItemID;

class Block;
struct BlockStruct;


constexpr int ITEM_FLAG_IS_BLOCK = 0;
constexpr int ITEM_FLAG_HAS_NBT = 1;

// if the texture metadata is determined by metadata
constexpr int ITEM_FLAG_USE_META_AS_TEXTURE = 2;

constexpr int ITEM_FLAG_ARMOR_HEAD = 3;
constexpr int ITEM_FLAG_ARMOR_CHEST = 4;
constexpr int ITEM_FLAG_ARMOR_LEGGINS = 5;
constexpr int ITEM_FLAG_ARMOR_BOOTS = 6;
constexpr int ITEM_FLAG_AMMO = 7;

class ItemRegistry;


class Item 
{
protected:
	friend ItemRegistry;
	const ItemID m_id;
	int m_max_stack_size;
	const std::string m_text_name;
	half_int m_texture_pos;
	int m_flags = 0;

	// how many types of the same exists (tells creative tab to list all possibilities)
	int m_max_metadata = 0;

	constexpr void setFlag(int flag, bool value = true) {
		if (value)
			m_flags |= (1 << flag);
		else
			m_flags &= ~(1 << flag);
	}
public:

	enum Interaction
	{
		PRESSED, RELEASED
	};
	inline const static int INFINITE_SIZE = -1;
	virtual ~Item() = default;
	Item(ItemID id, std::string textName);


	Item& setMaxStackSize(int size) { m_max_stack_size = size; return *this; }

	int getMaxMeta()const { return m_max_metadata; }

	virtual void onTextureLoaded(const nd::TextureAtlas& atlas);
	virtual int getTextureOffset(const ItemStack& b) const;

	int getMaxStackSize() const { return m_max_stack_size; }
	ItemID getID() const { return m_id; }
	const std::string& toString() const { return m_text_name; }

	// flags
	constexpr bool hasFlag(int flag) const {
		return m_flags & (1 << flag);
	}
	constexpr int isUseMetaAsTexture() const { return hasFlag(ITEM_FLAG_USE_META_AS_TEXTURE); }
	constexpr int hasNBT() const { return hasFlag(ITEM_FLAG_HAS_NBT); }
	constexpr int isBlock() const { return hasFlag(ITEM_FLAG_IS_BLOCK); }

	virtual int getBlockID() const;



	//=============================EVENTS=============================

	// called when entity chooses this item to be held in hand
	virtual void onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const;

	// called when entity switches from this item to be held in hand
	virtual void onUnequipped(World& world, ItemStack& stack, WorldEntity& owner) const;

	// called when entity wants to use this item
	virtual bool onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const { return false; }

	// called when player uses item on block
	// return true if event was consumed
	virtual bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const { return onRightClick(world, stack, owner, x, y); }

	// called when player uses item on entity
	virtual bool onRightClickOnEntity(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, WorldEntity& target) const { return onRightClick(world, stack, owner, x, y); }

	// called before the item is thrown away
	virtual void onDisposed(World& world, ItemStack& stack, WorldEntity& owner) const {}

	// return true if entity was damaged
	virtual bool hitEntity(World& w, ItemStack* stack, WorldEntity& owner, WorldEntity& target)const { return false; }

	// called each tick the item is used to dig some block
	virtual void onBlockBeingDigged(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const {}

	// Called each tick when item is held in hand = active slot
	virtual void onItemHeldInHand(World& w, WorldEntity& owner, ItemStack& stack) const {}

	// returns pointer to new custom structure that will hold temporary data of item or nullptr
	// is called before onEquipped
	virtual void* instantiateDataBox() const { return nullptr; }

	// destroys created dataBox returned by instantiateDataBox()
	virtual void destroyDataBox(void* dataBox) const {}

	// called when clicked with held item somewhere at world location x,y
	// Params:
	//		ticksPressed number of ticks this item is pressed. When clicked first ticksPressed is 0
	//		stack held item, if stack.size() is zero stack will be automatically destroyed. -> don't destroy the item!
	//		dataBox custom data structure pointer returned by instantiateDataBox() call (will be reset each onEquipped())
	virtual void onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const {}

	virtual std::string getTitle(ItemStack* stack)const;
};




