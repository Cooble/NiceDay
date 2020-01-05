#include "ItemTool.h"

ItemTool::ItemTool(ItemID id, const std::string& textName, ToolType type)
	:Item(id,textName),m_tool_type(type)
{
	m_maxStackSize = 1;
}
