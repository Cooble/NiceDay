#pragma once
#include "gui/HUD.h"
#include "world/entity/EntityManager.h"


class TileEntityChest;

class GUISlots :public GUIElement
{
public:
	GUISlots(Inventory* inv, HUD& hud,int fromIndex,int toIndex,int rowSize);
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
	void update(World& w) override;
	void render(BatchRenderer2D& renderer) override;
	void onAttachedToHUD(HUD& hud) override;
	void onDetached() override;
	const std::string& getID() const override;
};
