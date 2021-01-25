#include "GUIEntityChest.h"
#include "world/entity/entities.h"

GUISlots::GUISlots(Inventory* inv, HUD& hud, int fromIndex, int toIndex, int rowSize)
	:GUIElement(GETYPE::Other)
{
	isAlwaysPacked = true;
	isVisible = false;
	

	auto column = new GUIColumn(GUIAlign::LEFT);
	column->isAlwaysPacked = true;
	auto row = new GUIRow();
	row->isAlwaysPacked = true;

	for (int i = fromIndex; i < toIndex+1; ++i)
	{
		if(row->getChildren().size()==rowSize)
		{
			column->appendChild(row);
			row = new GUIRow();
			row->isAlwaysPacked = true;
		}
		auto c = new GUIItemContainer();
		m_slots.push_back(c);
		c->setContainer(inv, i);
		c->onContainerEventConsumer = hud.getContainerConsumer();
		row->appendChild(c);
	}
	column->appendChild(row);
	appendChild(column);
}

void GUISlots::updateIndexes(int fromIndex, int toIndex)
{
	int currentIndex = 0;
	for (auto row : getFirstChild()->getChildren())
	{
		for (auto& i : row->getChildren())
		{
			auto c = (GUIItemContainer*)i;
			c->setContainerSlot(fromIndex + currentIndex > toIndex?-1:fromIndex + currentIndex);
			currentIndex++;
		}
	}
}

Inventory* GUIEntityChest::getInventory()
{
	return &m_disgusting_chest->getInventory();
}

GUIEntityChest::GUIEntityChest(TileEntityChest* chest)
	:m_chest(chest->getID()), m_disgusting_chest(chest)
{

}


void GUIEntityChest::onAttachedToHUD(HUD& hud)
{
	m_gui_slots = new GUISlots(&m_disgusting_chest->getInventory(), hud,0,m_disgusting_chest->getInventory().getItemsSize()-1,9);
	m_gui_slots->setAlignment(GUIAlign::LEFT);
	hud.appendChild(m_gui_slots,getID());
}

void GUIEntityChest::onDetached()
{
	m_disgusting_chest->onGUIEntityClosed();
}

const std::string& GUIEntityChest::getID() const
{
	static std::string id = "player_chest";
	return id;
}

