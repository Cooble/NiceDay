#include "Item.h"

#include "core/AppGlobals.h"
#include "graphics/TextureAtlas.h"
#include "graphics/font/FontParser.h"
using namespace nd;

ItemStack::ItemStack(ItemID item, int size)
	: m_item(item), m_size(size)
{
}

ItemStack::ItemStack(const ItemStack& s)
	: m_item(s.m_item), m_size(s.m_size),m_metadata(s.m_metadata)
{
	if (getItem().hasNBT())
		m_nbt = s.m_nbt;//copy nbt
}

ItemStack::~ItemStack()
{
}

bool ItemStack::equals(const ItemStack* stack) const
{
	if (stack == nullptr)
		return false;
	return *this == *stack;
}

void ItemStack::serialize(NBT& nbt)
{
	nbt.save("size", m_size);
	nbt.save("meta", m_metadata);
	nbt.save("item", m_item);
	if(m_nbt)
		nbt.save("nbt", m_nbt);
}

bool ItemStack::isFullStack() const
{
	return m_size >= getItem().getMaxStackSize();
}

ItemStack* ItemStack::deserialize(const NBT& nbt)
{
	auto out = ItemStack::create(nbt["item"], nbt["size"]);
	out->setMetadata(nbt["meta"]);
	if (nbt.exists("nbt"))
		out->m_nbt = nbt["nbt"];
	return out;
}

bool operator==(const ItemStack& a, const ItemStack& b)
{
	
	return a.getItemID() == b.getItemID() && a.getMetadata() == b.getMetadata();
}

Item::Item(ItemID id, const std::string& textName)
:m_id(id),m_max_stack_size(999),m_text_name(textName)
{
}

void Item::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getSubImage("item/" + toString());
}

int Item::getTextureOffset(const ItemStack& b) const
{
	return m_texture_pos+(isUseMetaAsTexture()?half_int(b.getMetadata(),0):half_int(0,0));
}

int Item::getBlockID() const
{
	return -1;
}

void Item::onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const
{
	//ND_INFO("Equipped {}", stack.getItem().toString());
}

void Item::onUnequipped(World& world, ItemStack& stack, WorldEntity& owner) const
{
	//ND_INFO("Unequipped {}", stack.getItem().toString());
}

std::string Item::getTitle(ItemStack* stack)const
{
	return "";
	//return Font::colorize(Font::DARK_GREY, Font::AQUA)+toString();
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
	ASSERT(m_items.find(id) != m_items.end(), "Invalid item id: {}",id);
	return *m_items.at(id);
}

Pool<ItemStack>& ItemStack::s_stack_pool()
{
	static Pool<ItemStack> pool(ITEMSTACK_POOL_SIZE);
	return pool;
}

ItemStack* ItemStack::create(ItemID id, int count)
{
	AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();
	return s_stack_pool().allocate(id, count);
}

ItemStack* ItemStack::create(const ItemStack* itemstack)
{
	AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();
	return s_stack_pool().allocate(*itemstack);
}



void ItemStack::destroy(ItemStack* stack)
{
	
	s_stack_pool().deallocate(stack);
	AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();

}
