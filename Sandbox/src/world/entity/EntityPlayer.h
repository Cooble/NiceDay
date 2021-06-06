#pragma once
#include "entities.h"
#include "inventory/PlayerInventory.h"

class EntityPlayer : public Creature
{
private:
	std::string m_name = "Karel";
	float m_pose = 0;
	float m_last_pose = 0;
	int m_animation_var = 0;
	bool m_has_creative;
	PlayerInventory m_inventory;

	//in hand item
	float m_item_angle= 0;
	bool m_is_swinging = false;
	bool m_is_last_swing = false;
	bool m_is_facing_left;
	
	
	

public:
	EntityPlayer();
	virtual ~EntityPlayer() = default;

	void render(nd::BatchRenderer2D& renderer) override;
	void setItemSwinging(bool swing);
	void setFacingDir(bool left);
	void setCreative(bool cre){m_has_creative = cre;}
	bool hasCreative() const { return m_has_creative; }
	void update(World& w) override;
	EntityType getEntityType() const override;
	bool wantsItem(const ItemStack* stack) const override;
	ItemStack* consumeItem(ItemStack* stack) override;

	void onHit(World& w, WorldEntity* e, float damage) override;

	PlayerInventory& getInventory() { return m_inventory; }

	static glm::ivec2 pickBlockToDig(World& w, glm::vec2 pos, glm::vec2 cursorPos, float radius);
	static std::vector<glm::ivec2> pickBlocksToDig(World& w, glm::vec2 pos, glm::vec2 cursorPos, float radius);
	
	TO_ENTITY_STRING(EntityPlayer)
	ND_FACTORY_METH_ENTITY_BUILD(EntityPlayer)

	void save(nd::NBT& src) override;
	void load(nd::NBT& src) override;
};
enum MouseState
{
	PRESS,RELEASE,HOLD,NONE
};
enum MouseButton
{
	LEFT,RIGHT
};
class PlayerInteractor
{
	EntityPlayer* m_player;
public:
	PlayerInteractor(EntityPlayer* player):m_player(player){}
	
	void click(World* w, glm::vec2 pos, MouseButton leftRight, MouseState state);

	void update(World* w);
};
