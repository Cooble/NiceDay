#include "Item.h"
#include "graphics/TextureAtlas.h"

ItemStack::ItemStack(ItemID item, int size)
	: m_item(item), m_size(size), m_nbt(nullptr)
{
}

ItemStack::ItemStack(const ItemStack& s)
	: m_item(s.m_item), m_size(s.m_size), m_nbt(s.m_nbt)
{
	if (s.m_nbt)
		m_nbt = new NBT(*s.m_nbt);
}

ItemStack::~ItemStack()
{
	if (m_nbt)
		delete m_nbt;
}

bool operator==(const ItemStack& a, const ItemStack& b)
{
	if(a.getItemID()==b.getItemID()&&a.getSize()==b.getSize())
	{
		//if (a.getNBT() == nullptr && b.getNBT() == nullptr)
			return true;
		//if (a.getNBT() != nullptr && b.getNBT() != nullptr)
		//	return a.getNBT() == b.getNBT();
		//return false;
	}
	return false;
}

Item::Item(ItemID id, const std::string& name)
:m_maxStackSize(999),m_id(id),m_name(name),m_has_nbt(false),m_is_block(false)
{
}

void Item::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("item/" + toString());
}

int Item::getBlockID() const
{
	return -1;
}

ItemRegistry::~ItemRegistry()
{
	for (auto item : m_items)
		delete item;
}

void ItemRegistry::initTextures(const TextureAtlas& atlas)
{
	for (auto item : m_items)
		item->onTextureLoaded(atlas);
}


void ItemRegistry::registerItem(Item* item)
{
	if (m_items.size() <= item->getID())
		m_items.resize(item->getID() + 1);
	m_items[item->getID()] = item;
	m_itemIDs[item->toString()] = item->getID();
}

const Item& ItemRegistry::getItem(const std::string& id) const
{
	return *m_items[m_itemIDs.at(id)];
}

const Item& ItemRegistry::getItem(ItemID id) const
{
	ASSERT(m_items.size() > id&& id >= 0, "Invalid item id");
	return *m_items[id];
}
