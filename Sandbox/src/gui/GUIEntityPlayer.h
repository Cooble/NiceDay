#pragma once
#include "layer/GUILayer.h"
#include "world/entity/EntityManager.h"

class EntityPlayer;

class GUIActionSlots :public GUIElement
{
private:
	int main_slot=-1;
	int old_slot=-1;
	
public:
	GUIActionSlots(PlayerInventory* c,HUD& hud);
	void onMyEvent(Event& e) override;
	void update() override;
	void setMainSlot(int slot);
};

class GUIEntityPlayer:public GUIEntity
{
private:
	EntityID m_player;
	GUIActionSlots* m_gui_action_slots;
	//this is really nasty solution.. nevertheless it works
	EntityPlayer* m_disgusting_player;
public:
	Inventory* getInventory() override;
	GUIEntityPlayer(EntityPlayer* player);
	void update(World& w) override;
	void render(BatchRenderer2D& renderer) override;
	void onAttachedToHUD(HUD& hud) override;
	const std::string& getID() const override;
};
