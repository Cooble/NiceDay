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
	void setContainerSlot(int slot);
	void setContainer(Inventory* c, int slot);
	const ItemStack* getItemStack() const;
};
class GUIText;
class GUIItemTitle:public GUIElement
{
private:
	GUIText* m_title;
	GUIText* m_meta;
	
public:
	GUIItemTitle();

	void setTitle(const std::string& s);
	void setMeta(const std::string& s);
};
