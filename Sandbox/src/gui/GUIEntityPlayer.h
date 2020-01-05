#pragma once
#include "layer/GUILayer.h"
#include "world/entity/EntityManager.h"
#include "GUIEntityChest.h"

class EntityPlayer;
class GUISlots;

class GUIActionSlots :public GUIElement
{
private:
	bool m_show_title = false;
	int main_slot=-1;
	int old_slot=-1;
	PlayerInventory* m_inventory;
	GUIText* m_title;
	GUIColumn* m_col;
	void showTitleInternal(bool show);
public:
	GUIActionSlots(PlayerInventory* c,HUD& hud);
	void onMyEvent(Event& e) override;
	void update() override;
	void setMainSlot(int slot);
	void showTitle(bool show);
};

class GUIEntityPlayer:public GUIEntity
{
private:
	EntityID m_player;
	GUIActionSlots* m_gui_action_slots;
	GUISlots* m_gui_slots;
	GUIColumn* m_col;
	//this is really nasty solution.. nevertheless it works
	EntityPlayer* m_disgusting_player;
	bool m_is_inventory_open=false;
public:
	Inventory* getInventory() override;
	inline bool isOpenedInventory() { return m_is_inventory_open; }
	GUIEntityPlayer(EntityPlayer* player);
	void update(World& w) override;
	void render(BatchRenderer2D& renderer) override;
	void onAttachedToHUD(HUD& hud) override;
	const std::string& getID() const override;
	void openInventory(bool open);
	inline auto getPlayerEntity() { return m_disgusting_player; }
};
