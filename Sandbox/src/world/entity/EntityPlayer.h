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
	PlayerInventory m_inventory;


public:
	EntityPlayer();
	virtual ~EntityPlayer() = default;

	void update(World& w) override;
	EntityType getEntityType() const override;
	bool wantsItem(const ItemStack* stack) const override;
	ItemStack* consumeItem(ItemStack* stack) override;

	void onHit(World& w, WorldEntity* e, float damage) override;

	inline PlayerInventory& getInventory() { return m_inventory; }
	
	TO_ENTITY_STRING(EntityPlayer)
	ND_FACTORY_METH_ENTITY_BUILD(EntityPlayer)

	void save(NBT& src) override;
	void load(NBT& src) override;
};
