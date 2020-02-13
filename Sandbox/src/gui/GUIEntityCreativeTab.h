#pragma once
#include "gui/GUIElement.h"
#include "GUIEntityChest.h"


class HUD;
class Inventory;
class CreativeInventory;
class GUICreativeTab :public GUIElement
{
protected:
	Inventory* m_inv;
	int m_item_index_offset = 0;
	int m_item_line_count;
	int m_item_lines_count;
	GUISlots* m_slots;
public:
	
	GUICreativeTab(HUD& hud);
	inline auto getInventory() { return m_inv; }
};

class GUIEntityCreativeTab :public GUIEntity
{
private:
	GUICreativeTab* m_element;
public:
	GUIEntityCreativeTab() = default;
	Inventory* getInventory() override;
	void onAttachedToHUD(HUD& hud) override;
	void onDetached() override;
	const std::string& getID() const override;
};
