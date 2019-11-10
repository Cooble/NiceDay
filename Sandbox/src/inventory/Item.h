#pragma once
#include "ndpch.h"

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
	int m_maxStackSize;
	const ItemID m_id;
	const std::string m_name;
	half_int m_texture_pos;
	bool m_has_nbt;
	bool m_is_block;
	
public:
	virtual ~Item() = default;
	Item(ItemID id,const std::string& name);

	virtual void onTextureLoaded(const TextureAtlas& atlas);
	
	inline int getMaxStackSize() const { return m_maxStackSize; }
	inline ItemID getID() const { return m_id; }
	inline const std::string& toString() const { return m_name; }
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
	std::unordered_map<std::string, int> m_itemIDs;
	std::vector<Item*> m_items;
	ItemRegistry() = default;
public:
	ItemRegistry(ItemRegistry const&) = delete;
	void operator=(ItemRegistry const&) = delete;
	~ItemRegistry();

	inline const std::vector<Item*>& getItems() { return m_items; }

	void initTextures(const TextureAtlas& atlas);

	static inline ItemRegistry& get() {
		static ItemRegistry s_instance;
		return s_instance;
	}
public:
	//takes ownership
	void registerItem(Item* item);
	
	const Item& getItem(const std::string& id) const;
	const Item& getItem(ItemID id) const;

	inline int getItemID(const std::string& itemId) const { return m_itemIDs.at(itemId); }
};

class ItemStack
{
private:
	ItemID m_item;
	uint64_t m_metadata;
	NBT* m_nbt;
	int m_size;
public:
	ItemStack(ItemID item, int size = 1);
	ItemStack(const ItemStack& s);
	~ItemStack();

	inline uint64_t getMetadata() const { return m_metadata; }
	inline void setMetadata(uint64_t meta) { m_metadata = meta; }
	inline void setSize(int size) { m_size = size; }
	inline int getSize()const { return m_size; }
	inline ItemID getItemID() const { return m_item; }
	inline const Item& getItem() const { return ItemRegistry::get().getItem(m_item); }
	inline NBT* getNBT() const { return m_nbt; }

};
bool operator==(const ItemStack& a, const ItemStack& b);

