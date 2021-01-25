#pragma once
#include "Item.h"



class Block;
/***
 * Has one bit set to 1
 */
typedef uint64_t ToolType;

/***
 * Represents one or more tooltypes stored as bits
 */
typedef uint64_t ToolTypes;

constexpr ToolType TOOL_TYPE_PICKAXE	=BIT(0);
constexpr ToolType TOOL_TYPE_SHOVEL		=BIT(1);
constexpr ToolType TOOL_TYPE_HAMMER		=BIT(2);
constexpr ToolType TOOL_TYPE_AXE		=BIT(3);

class ItemTool:public Item
{
protected:
	ToolType m_tool_type;
	int m_damage;
	float m_efficiency;
	int m_tier=1;
	//ticks between two digs/swings (of the same block)
	int m_dig_interval = 20;
	static std::vector<glm::vec3> s_dug_blocks;
public:
	ItemTool(ItemID id, const std::string& textName, ToolTypes type);
	ToolTypes getToolType() const {return m_tool_type;}
	bool hasToolType(ToolTypes types) const { return (m_tool_type & types) == types; }

	void* instantiateDataBox() const override;
	void destroyDataBox(void* dataBox) const override;
	void onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const override;
	void onInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;

	// how much mass the tool can dig in one swing
	// 0 means cannot be dig out
	virtual float getEfficiencyOnBlock(const Block& blok, ItemStack* stack) const;

	static auto& getDugBlocks() { return s_dug_blocks; }

	

};
