#pragma once
#include "gui/HUD.h"
#include "world/entity/EntityManager.h"


class TileEntityChest;

class GUISlots :public nd::GUIElement
{
	std::vector<GUIItemContainer*> m_slots;
public:
	GUISlots(Inventory* inv, HUD& hud,int fromIndex,int toIndex,int rowSize);

	//changes containers to point at different slots
	//if toIndex-fromIndex < container size  -> the rest of containers will be invalidated
	void updateIndexes(int fromIndex, int toIndex);
	auto& getSlots() { return m_slots; }
};

class GUIEntityChest :public GUIEntity
{
private:
	EntityID m_chest;
	GUISlots* m_gui_slots;
	//this is really nasty solution.. nevertheless it works
	TileEntityChest* m_disgusting_chest;
public:
	Inventory* getInventory() override;
	GUIEntityChest(TileEntityChest* chest);
	void onAttachedToHUD(HUD& hud) override;
	void onDetached() override;
	const std::string& getID() const override;
};
