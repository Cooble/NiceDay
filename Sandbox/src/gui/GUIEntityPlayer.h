﻿#pragma once
#include "layer/GUILayer.h"
#include "world/entity/EntityManager.h"
#include "GUIEntityChest.h"

class PlayerInventory;
class EntityPlayer;
class GUISlots;

class GUIActionSlots :public nd::GUIElement
{
private:
	bool m_show_title = false;
	int main_slot=-1;
	int old_slot=-1;
	PlayerInventory* m_inventory;
	nd::GUIText* m_title;
	nd::GUIColumn* m_col;
	void showTitleInternal(bool show);
public:
	GUIActionSlots(PlayerInventory* c,HUD& hud);
	void onMyEvent(nd::Event& e) override;
	void update() override;
	void setMainSlot(int slot);
	void showTitle(bool show);
};

class GUIEntityPlayer:public GUIEntity
{
private:
	EntityID m_player;
	GUIActionSlots* m_gui_action_slots;
	GUISlots* m_gui_slots=nullptr;
	nd::GUIColumn* m_col;
	//this is really nasty solution.. nevertheless it works
	EntityPlayer* m_disgusting_player;
	bool m_is_inventory_open=false;
public:
	Inventory* getInventory() override;
	inline bool isOpenedInventory() { return m_is_inventory_open; }
	GUIEntityPlayer(EntityPlayer* player);
	void update(World& w) override;
	void render(nd::BatchRenderer2D& renderer) override;
	void onAttachedToHUD(HUD& hud) override;
	const std::string& getID() const override;
	void openInventory(bool open);
	inline auto getPlayerEntity() { return m_disgusting_player; }
};

class GUIEntityConsole :public GUIEntity
{
private:
	std::vector<std::string> m_messages;
	std::vector<nd::GUIText*> m_lines;
	std::string m_currentMessage;
	nd::GUIColumn* m_col;
	int m_max_lines = 4;
	bool m_opened=false;
public:
	GUIEntityConsole();
	void clearChat();
	void addMessage(const std::string& message);
	void updateChat();
	void update(World& w) override;
	void onEvent(nd::Event& e) override;
	void render(nd::BatchRenderer2D& renderer) override;
	void onAttachedToHUD(HUD& hud) override;
	const std::string& getID() const override;
	void open(bool open);
};

