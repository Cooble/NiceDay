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
	int m_efficiency;
	int m_tier;
public:
	ItemTool(ItemID id, const std::string& textName, ToolTypes type);
	inline ToolTypes getToolType() const {return m_tool_type;}
	inline bool hasToolType(ToolTypes types) const { return (m_tool_type & types) == types; }


};
