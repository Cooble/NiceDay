#include "items.h"
#include "world/block/block_datas.h"

ItemPickaxe::ItemPickaxe()
	:Item(SID("pickaxe"),"pickaxe")
{
	m_maxStackSize = 1;
}
