﻿#pragma once
#include "gui/GUIBasic.h"
#include "gui/GUIItemContainer.h"
#include "inventory/PlayerInventory.h"

class HUD;
class World;

class GUIEntity
{
public:
	virtual const std::string& getID() const=0;
	virtual ~GUIEntity() = default;
	virtual void onAttachedToHUD(HUD& hud)=0;
	virtual void onDetached() {};
	virtual void render(BatchRenderer2D& renderer)=0;
	virtual void update(World& w)=0;
	// if inventory does not exist return null
	virtual Inventory* getInventory() = 0;
};

class HUD:public GUIWindow
{
	ContainerEventConsumer m_container_consumer;
	struct GUIEntityBundle
	{
		GUIEntity* entity;
		std::vector<GEID> children;
		GUIEntityBundle(GUIEntity* e):entity(e){}
	};
	std::vector<GUIEntityBundle> m_entities;
	PlayerInventory* m_player=nullptr;
	GUIItemContainer* m_hand;
	GUIItemTitle* m_title;
	
	static HUD* s_hud;

	int m_focused_slot;
	std::string m_focused_owner;
public:
	static HUD* get() { return s_hud; }
	HUD();
	~HUD();

	inline const auto& getContainerConsumer() const { return m_container_consumer; }
	void registerGUIEntity(GUIEntity* e);
	bool isRegistered(const std::string& id);
	void unregisterGUIEntity(const std::string& id);
	GUIEntity* getEntity(const std::string& id);
	Inventory* getInventory(const std::string& id);
	void update() override;
	void onMyEvent(Event& e) override;
	void appendChild(GUIElement* element,const std::string& ownerID);
	void appendChild(GUIElement* element) override;
	void consumeContainerEvent(const std::string& id, int slot, Event& e);
	inline const auto& getEntities() { return m_entities; }
};