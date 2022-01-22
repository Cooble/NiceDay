#pragma once
#include "ndpch.h"
#include "core/NBT.h"
#include "Item.h"


#define ND_REGISTER_ITEM(item)\
	ItemRegistry::get().registerItem(item);

class ItemTool;
class ItemArmor;


class ItemRegistry
{
private:
	nd::NBT m_templates;
	std::unordered_map<ItemID, Item*> m_items;
	ItemRegistry() = default;
public:
	ItemRegistry(ItemRegistry const&) = delete;
	void operator=(ItemRegistry const&) = delete;
	~ItemRegistry();

	inline const auto& getItems() { return m_items; }

	void initTextures(const nd::TextureAtlas& atlas);

	static inline ItemRegistry& get() {
		static ItemRegistry s_instance;
		return s_instance;
	}
public:
	void registerFromJSON();
	//takes ownership
	void registerItem(const std::string& id, nd::NBT& nbt);
	void registerItem(Item* item);
	const Item& getItem(ItemID id) const;
private:
	void parseTemplatesFromJSON();
	void fillFromTemplateTool(ItemTool* item, const nd::NBT& nbt);
	void fillFromTemplateArmor(ItemArmor* item, const nd::NBT& nbt);
	void fillFromTemplate(Item* item, const nd::NBT& nbt);
};
