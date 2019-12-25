#pragma once
#include "gui/GUIElement.h"
#include "inventory/Inventory.h"


//ownerid, slotindex,event
typedef std::function<void(const std::string&,int,Event& e)> ContainerEventConsumer;

class GUIItemContainer :public GUIElement
{
private:
	int m_slotIndex;
	Inventory* m_container=nullptr;
public:
	bool isSlotRendered = true;
	float slotScale = 0;
	ContainerEventConsumer onContainerEventConsumer;
public:
	GUIItemContainer();
	void onMyEvent(Event& e) override;
	void setContainer(Inventory* c, int slot);
	const ItemStack* getItemStack() const;
};
