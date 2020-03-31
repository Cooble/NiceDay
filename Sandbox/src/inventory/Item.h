#pragma once
#include "ndpch.h"
#include "memory/Pool.h"
#include "core/sids.h"
#include "core/NBT.h"

#define ND_REGISTER_ITEM(item)\
	ItemRegistry::get().registerItem(item);


class WorldEntity;
class World;
class ItemRegistry;
class TextureAtlas;
class ItemStack;
typedef uint64_t ItemID;

class Block;
struct BlockStruct;
class Item
{
protected:
	const ItemID m_id;
	int m_maxStackSize;
	const std::string m_text_name;
	half_int m_texture_pos;
	bool m_has_nbt;
	bool m_is_block;
	// number of textures that will corespond to the item based on its metadata
	uint64_t m_max_texture_metadata = 0;

	
public:
	virtual ~Item() = default;
	Item(ItemID id,const std::string& textName);

	inline Item& setMaxStackSize(int size) { m_maxStackSize = size; return *this; }
	inline uint64_t getMaxTextureMeta()const { return m_max_texture_metadata; }

	virtual void onTextureLoaded(const TextureAtlas& atlas);
	virtual int getTextureOffset(const ItemStack& b) const;

	inline int getMaxStackSize() const { return m_maxStackSize; }
	inline ItemID getID() const { return m_id; }
	inline const std::string& toString() const { return m_text_name; }
	inline bool isBlock() const { return m_is_block; }
	inline bool hasNBT() const { return m_has_nbt; }
	virtual int getBlockID() const;

	// how fast the blok can be dig out
	// 0 means cannot be dig out
	virtual float getEfficiencyOnBlock(const Block& blok, ItemStack* stack) const { return 0; }

	//=============================EVENTS=============================

	// called when entity chooses this item to be held in hand
	virtual void onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const;

	// called when entity switches from this item to be held in hand
	virtual void onUnequipped(World& world, ItemStack& stack, WorldEntity& owner) const;
	
	// called when entity wants to use this item
	virtual bool onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const { return false; }

	// called when player uses item on block
	// return true if event was consumed
	virtual bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const { return onRightClick(world, stack, owner,x,y); }

	// called when player uses item on entity
	virtual bool onRightClickOnEntity(World& world, ItemStack& stack, WorldEntity& owner, int x,int y, WorldEntity& target) const { return onRightClick(world, stack, owner, x, y); }
	
	// called before the item is thrown away
	virtual void onDisposed(World& world, ItemStack& stack, WorldEntity& owner) const {}

	// return true if entity was damaged
	virtual bool hitEntity(World& w, ItemStack* stack, WorldEntity& owner, WorldEntity& target)const { return false; }

	// called each tick the item is used to dig some block
	virtual void onBlockBeingDigged(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const {}
	
	// Called each tick when item is held in hand = active slot
	virtual void onItemHeldInHand(World& w, WorldEntity& owner, ItemStack& stack) const {}

	virtual std::string getTitle(ItemStack* stack)const;
};


class ItemRegistry
{
private:
	std::unordered_map<ItemID, Item*> m_items;
	ItemRegistry() = default;
public:
	ItemRegistry(ItemRegistry const&) = delete;
	void operator=(ItemRegistry const&) = delete;
	~ItemRegistry();

	inline const auto& getItems() { return m_items; }

	void initTextures(const TextureAtlas& atlas);

	static inline ItemRegistry& get() {
		static ItemRegistry s_instance;
		return s_instance;
	}
public:
	//takes ownership
	void registerItem(Item* item);
	const Item& getItem(ItemID id) const;
};

class ItemStack
{
private:
	static Pool<ItemStack>& s_stack_pool();
public:
	static ItemStack* create(ItemID id, int  count = 1);
	static ItemStack* create(const ItemStack* itemstack);
	static ItemStack* deserialize(const NBT& nbt);
	static void destroy(ItemStack* stack);
private:
	ItemID m_item;
	uint64_t m_metadata=0;
	int m_size;
	NBT m_nbt;
public:
	ItemStack(ItemID item, int size = 1);
	ItemStack(const ItemStack& s);
	~ItemStack();

	inline uint64_t getMetadata() const { return m_metadata; }
	inline void setMetadata(uint64_t meta) { m_metadata = meta; }
	inline void setSize(int size) { m_size = size; }
	inline int size()const { return m_size; }
	inline bool isEmpty() const { return m_size <= 0; }
	inline ItemID getItemID() const { return m_item; }
	inline const Item& getItem() const { return ItemRegistry::get().getItem(m_item); }
	inline const NBT& getNBT() const { return m_nbt; }
	inline NBT& getNBT() { return m_nbt; }
	inline void destroy() { ItemStack::destroy(this); }
	inline ItemStack* copy() const { return ItemStack::create(this); }
	bool equals(const ItemStack* stack) const;
	inline void addSize(int count) { ASSERT(m_size + count <= getItem().getMaxStackSize(), "Too big itemstack"); m_size += count; }
	void serialize(NBT& nbt);
	bool isFullStack() const;
};
bool operator==(const ItemStack& a, const ItemStack& b);

