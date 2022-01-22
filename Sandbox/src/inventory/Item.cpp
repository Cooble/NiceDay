#include "Item.h"

#include <utility>

#include "core/AppGlobals.h"
#include "ItemStack.h"
#include "graphics/TextureAtlas.h"


using namespace nd;


Item::Item(ItemID id, std::string textName)
:m_id(id),m_max_stack_size(999),m_text_name(std::move(textName))
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

