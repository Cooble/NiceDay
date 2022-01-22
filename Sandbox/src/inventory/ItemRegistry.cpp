#include "ItemRegistry.h"

#include "items.h"
#include "ItemTool.h"

typedef nd::NBT NBT;

ItemRegistry::~ItemRegistry()
{
	for (auto item : m_items)
		delete item.second;
}

void ItemRegistry::initTextures(const nd::TextureAtlas& atlas)
{
	for (auto item : m_items)
		item.second->onTextureLoaded(atlas);
}


void ItemRegistry::parseTemplatesFromJSON()
{
	NBT::loadFromFile(ND_RESLOC("res/registry/items/templates.json"), m_templates);
}

void ItemRegistry::registerFromJSON()
{
	parseTemplatesFromJSON();

	NBT items;
	if (NBT::loadFromFile(ND_RESLOC("res/registry/items/items.json"), items))
		for (auto& map : items.maps())
			registerItem(map.first, map.second);
}


static void toFlags(nd::NBT& nbt)
{
	nd::NBT out;
	if (nbt.isArray())
	{
		for (auto& flag : nbt.arrays())
			out[flag.string()] = true;
		nbt = std::move(out);
	}
}
static ToolType getToolType(const nd::NBT& nbt)
{
	if (!nbt.isString())
		throw std::string("Invalid tooltype ") + nbt.c_str();

	auto s = nbt.string();
	nd::SUtil::toUpper(s);

	if (s == "PICKAXE")
		return TOOL_TYPE_PICKAXE;
	if (s == "SHOVEL")
		return TOOL_TYPE_SHOVEL;
	if (s == "AXE")
		return TOOL_TYPE_AXE;
	if (s == "HAMMER")
		return TOOL_TYPE_HAMMER;
	throw std::string("Invalid tooltype ") + s;
}
static int getItemFlag(const std::string& ss)
{
	auto s = ss;
	nd::SUtil::toUpper(s);

	if (s == "IS_BLOCK")
		return ITEM_FLAG_IS_BLOCK;
	if (s == "HAS_NBT")
		return ITEM_FLAG_HAS_NBT;
	if (s == "USE_META_AS_TEXTURE")
		return ITEM_FLAG_USE_META_AS_TEXTURE;
	if (s == "ARMOR_HEAD")
		return ITEM_FLAG_ARMOR_HEAD;
	if (s == "ARMOR_CHEST")
		return ITEM_FLAG_ARMOR_CHEST;
	if (s == "ARMOR_LEGGINS")
		return ITEM_FLAG_ARMOR_LEGGINS;
	if (s == "ARMOR_BOOTS")
		return ITEM_FLAG_ARMOR_BOOTS;
	if (s == "AMMO")
		return ITEM_FLAG_AMMO;


	throw std::string("unknown item flag ") + s;
}

static Item* createItem(const std::string& id, const nd::NBT& nbt)
{

	if (nbt.exists("tool")) {
		if (nbt["tool"]["type"].string() == "hammer")
			return new ItemHammer(SID(id), id);
		return new ItemTool(SID(id), id, getToolType(nbt["tool"]["type"]));
	}
	if (nbt.exists("armor"))
		return new ItemArmor(SID(id), id);

	return new Item(SID(id), id);
}

void ItemRegistry::fillFromTemplateTool(ItemTool* item, const nd::NBT& nbt)
{
	auto& tool = nbt["tool"];
	item->m_tool_type = getToolType(tool["type"]);
	item->m_damage= tool["damage"];
	item->m_efficiency= tool["efficiency"];
	item->m_tier= tool["tier"];
	item->m_dig_time= tool["digTime"];
}
void ItemRegistry::fillFromTemplateArmor(ItemArmor* item, const nd::NBT& nbt)
{
	auto& tool = nbt["armor"];
	item->m_defense = tool["defense"];

}
void ItemRegistry::fillFromTemplate(Item* item, const nd::NBT& nbt)
{
	item->setMaxStackSize(nbt["maxStackSize"]);

	nd::NBT flags = nbt["flags"];
	toFlags(flags);

	for (auto& flag : flags.maps()) 
		item->setFlag(getItemFlag(flag.first), flag.second);
}

void ItemRegistry::registerItem(const std::string& id, nd::NBT& nbt)
{
	const std::string GLOBAL_PREFIX = "global.";

	for (auto& sub : nbt.maps())
	{
		if (m_templates.exists(sub.first))
		{
			nd::NBT cp = m_templates[sub.first];
			// add overriden attribs to template
			for (auto& specials : sub.second.maps())
				cp[specials.first] = specials.second;

			// add global variables from template to global nbt
			for (auto& alls : cp.maps())
				if (nd::SUtil::startsWith(alls.first, GLOBAL_PREFIX))
					nbt[alls.first.substr(GLOBAL_PREFIX.size())] = alls.second;

			nbt[sub.first] = cp;
		}
	}
	auto sid = SID(id);

	if (m_items.find(sid) == m_items.end())
		m_items[sid] = createItem(id, nbt);

	auto& item = m_items[sid];
	fillFromTemplate(item, nbt);

   if(nbt.exists("tool"))
	   fillFromTemplateTool(dynamic_cast<ItemTool*>(item), nbt);
    if(nbt.exists("armor"))
	   fillFromTemplateArmor(dynamic_cast<ItemArmor*>(item), nbt);
}

void ItemRegistry::registerItem(Item* item)
{
	m_items[item->getID()] = item;
}

const Item& ItemRegistry::getItem(ItemID id) const
{
	ASSERT(m_items.find(id) != m_items.end(), "Invalid item id: {}", id);
	return *m_items.at(id);
}