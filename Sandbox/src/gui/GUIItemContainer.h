#pragma once
#include "gui/GUIBasic.h"
#include "gui/GUIElement.h"
#include "inventory/Inventory.h"


//ownerid, slotindex, inventory, event
typedef std::function<void(const std::string&,int,Inventory*,Event& e)> ContainerEventConsumer;

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
	int getSlot() const { return m_slotIndex; }
};
class GUIText;
class GUIItemTitle:public GUIBlank
{
private:
	GUIText* m_title;
	GUIText* m_meta;
	
public:
	// rectangle should be drawn under text
	bool isBackgroundRendered = false;
	GUIItemTitle();

	void setTitle(const std::string& s);
	void setMeta(const std::string& s);
};
