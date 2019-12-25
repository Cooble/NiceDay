#pragma once
#include "ndpch.h"
#include "memory/Pool.h"
#include "core/sids.h"

#define ND_REGISTER_ITEM(item)\
	ItemRegistry::get().registerItem(item);

class WorldEntity;
class World;
class ItemRegistry;
class TextureAtlas;
class ItemStack;
typedef uint64_t ItemID;

class Item
{
protected:
	const ItemID m_id;
	int m_maxStackSize;
	const std::string m_text_name;
	half_int m_texture_pos;
	bool m_has_nbt;
	bool m_is_block;
	
public:
	virtual ~Item() = default;
	Item(ItemID id,const std::string& textName);

	inline Item& setMaxStackSize(int size) { m_maxStackSize = size; return *this; }

	virtual void onTextureLoaded(const TextureAtlas& atlas);
	virtual int getTextureOffset(const ItemStack& b) const;

	inline int getMaxStackSize() const { return m_maxStackSize; }
	inline ItemID getID() const { return m_id; }
	inline const std::string& toString() const { return m_text_name; }
	inline bool isBlock() const { return m_is_block; }
	virtual int getBlockID() const;

	//=============================EVENTS=============================

	//called when entity chooses this item to be held in hand
	inline virtual void onEquipped(World& world, WorldEntity& entity,ItemStack& stack){}
	//called when entity wants to use this item
	inline virtual void onUse(World& world, WorldEntity& entity, ItemStack& stack){}
	//called when item thrown away
	inline virtual void onDisposed(World& world, WorldEntity& entity, ItemStack& stack){}
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
	uint64_t m_metadata;
	int m_size;
	NBT* m_nbt;
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
	inline const NBT& getNBT() const { return *m_nbt; }
	inline NBT& getNBT() { return *m_nbt; }
	inline void destroy() { ItemStack::destroy(this); }
	inline ItemStack* copy() const { return ItemStack::create(this); }
	bool equals(const ItemStack* stack) const;
	inline void addSize(int count) { ASSERT(m_size + count <= getItem().getMaxStackSize(), "Too big itemstack"); m_size += count; }
	void serialize(NBT& nbt);
	bool isFullStack() const;
};
bool operator==(const ItemStack& a, const ItemStack& b);

