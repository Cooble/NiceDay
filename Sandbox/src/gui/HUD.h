#pragma once
#include "gui/GUIBasic.h"
#include "gui/GUIItemContainer.h"

class HUD;
class World;

class GUIEntity
{
public:
	virtual const std::string& getID() const=0;
	virtual ~GUIEntity() = default;
	virtual void onAttachedToHUD(HUD& hud)=0;
	virtual void onDetached() {};
	virtual void render(nd::BatchRenderer2D& renderer){};
	virtual void update(World& w){};
	virtual void onEvent(nd::Event& e){};
	// if inventory does not exist return null
	virtual Inventory* getInventory() { return nullptr; };
};

class HUD :public nd::GUIWindow
{
	ContainerEventConsumer m_container_consumer;
	struct GUIEntityBundle
	{
		GUIEntity* entity;
		std::vector<nd::GEID> children;
		GUIEntityBundle(GUIEntity* e) :entity(e) {}
	};
	std::vector<GUIEntityBundle> m_entities;
	Inventory* m_hand_inventory = nullptr;
	GUIItemContainer* m_hand;
	GUIItemTitle* m_title;

	static HUD* s_hud;

	// mouse position when hovering over some itemcontainer stopped
	glm::vec2 m_mouse_pos_when_exit = glm::vec2(std::numeric_limits<float>().max());
	int m_focused_slot;
	std::string m_focused_owner;
public:
	static HUD* get() { return s_hud; }
	HUD();
	~HUD();

	void setHandSlot(Inventory* inv, int slot);

	const auto& getContainerConsumer() const { return m_container_consumer; }
	void registerGUIEntity(GUIEntity* e);
	bool isRegistered(const std::string& id);
	void unregisterGUIEntity(const std::string& id);
	GUIEntity* getEntity(const std::string& id);
	Inventory* getInventory(const std::string& id);
	void update() override;
	void onMyEvent(nd::Event& e) override;
	void appendChild(GUIElement* element,const std::string& ownerID);
	void appendChild(GUIElement* element) override;
	void consumeContainerEvent(const std::string& id, int slot, Inventory* inv, nd::Event& e);
	inline const auto& getEntities() { return m_entities; }
};
