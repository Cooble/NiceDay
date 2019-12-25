#include "Item.h"
#include "graphics/TextureAtlas.h"

ItemStack::ItemStack(ItemID item, int size)
	: m_item(item), m_size(size), m_nbt(nullptr)
{
}

ItemStack::ItemStack(const ItemStack& s)
	: m_item(s.m_item), m_size(s.m_size), m_nbt(nullptr)
{
	if (s.m_nbt)
		m_nbt = NBT::create(*s.m_nbt);
}

ItemStack::~ItemStack()
{
	if (m_nbt)
		m_nbt->destroy();
}

bool ItemStack::equals(const ItemStack* stack) const
{
	if (stack == nullptr)
		return false;
	return *this == *stack;
}

void ItemStack::serialize(NBT& nbt)
{
	nbt.set("size", m_size);
	nbt.set("meta", m_metadata);
	nbt.set("item", m_item);
	if(m_nbt)
		nbt.set("nbt", *m_nbt);
}

bool ItemStack::isFullStack() const
{
	return m_size >= getItem().getMaxStackSize();
}

ItemStack* ItemStack::deserialize(const NBT& nbt)
{
	auto out = ItemStack::create(nbt.get<ItemID>("item"), nbt.get<int>("size"));

	if (nbt.exists<NBT>("nbt"))
		out->m_nbt = nbt.get<NBT>("nbt").copy();
	return out;
}

bool operator==(const ItemStack& a, const ItemStack& b)
{
	
	return a.getItemID() == b.getItemID() && a.getMetadata() == b.getMetadata();
}

Item::Item(ItemID id, const std::string& textName)
:m_id(id),m_maxStackSize(999),m_text_name(textName),m_has_nbt(false),m_is_block(false)
{
}

void Item::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("item/" + toString());
}

int Item::getTextureOffset(const ItemStack& b) const
{
	return m_texture_pos;
}

int Item::getBlockID() const
{
	return -1;
}

ItemRegistry::~ItemRegistry()
{
	for (auto item : m_items)
		delete item.second;
}

void ItemRegistry::initTextures(const TextureAtlas& atlas)
{
	for (auto item : m_items)
		item.second->onTextureLoaded(atlas);
}


void ItemRegistry::registerItem(Item* item)
{
	m_items[item->getID()] = item;
}

const Item& ItemRegistry::getItem(ItemID id) const
{
	ASSERT(m_items.find(id) != m_items.end(), "Invalid item id");
	return *m_items.at(id);
}

Pool<ItemStack>& ItemStack::s_stack_pool()
{
	static Pool<ItemStack> pool(1000);
	return pool;
}

ItemStack* ItemStack::create(ItemID id, int count)
{
	return s_stack_pool().allocate(id, count);
}

ItemStack* ItemStack::create(const ItemStack* itemstack)
{
	return s_stack_pool().allocate(*itemstack);
}



void ItemStack::destroy(ItemStack* stack)
{
	s_stack_pool().deallocate(stack);
}
