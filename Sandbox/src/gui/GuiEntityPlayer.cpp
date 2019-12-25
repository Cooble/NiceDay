#include "GUIEntityPlayer.h"
#include "world/entity/EntityPlayer.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"

GUIActionSlots::GUIActionSlots(PlayerInventory* player, HUD& hud)
	:GUIElement(GETYPE::Other)
{
	isAlwaysPacked = true;
	isVisible = false;

	constexpr int slotCount = 8;

	auto row = new GUIRow();
	row->isAlwaysPacked = true;

	for (int i = 0; i < slotCount; ++i)
	{
		auto c = new GUIItemContainer();
		c->dim = { 64,64 };
		c->setContainer(player,InventorySlot::INVENTORY_SLOT_FIRST+i);
		c->onContainerEventConsumer = hud.getContainerConsumer();
		row->appendChild(c);
	}

	appendChild(row);
}

void GUIActionSlots::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);
	if(e.getEventType()==Event::EventType::KeyType)
	{
		auto m = static_cast<KeyTypeEvent&>(e);
		if(m.getKey()>=GLFW_KEY_1&&m.getKey()<=GLFW_KEY_8)
		{
			int slot = m.getKey() - GLFW_KEY_1;
			if(main_slot!=slot)
				setMainSlot(slot);
			e.handled = true;
		}
	}
}

void GUIActionSlots::update()
{
	constexpr float closeSpeed = 1;
	constexpr float maxScale = 6;
	if (old_slot != -1)
	{
		auto& sca = ((GUIItemContainer*)getFirstChild()->getChildren()[old_slot])->slotScale;
		sca -= closeSpeed;
		if(sca<0)
		{
			sca = 0;
			old_slot = -1;
		}
	}
	if (main_slot != -1)
	{
		auto& sca = ((GUIItemContainer*)getFirstChild()->getChildren()[main_slot])->slotScale;
		sca += closeSpeed;
		if (sca > maxScale)
		{
			sca = maxScale;
		}
	}
}

void GUIActionSlots::setMainSlot(int slot)
{
	if (slot<0 || slot>getFirstChild()->getChildren().size())
		return;//invalid slot
	
	if(old_slot!=-1)
	{
		((GUIItemContainer*)getFirstChild()->getChildren()[old_slot])->slotScale = 0;
	}
	old_slot = main_slot;
	main_slot = slot;
}

Inventory* GUIEntityPlayer::getInventory()
{
	return &m_disgusting_player->getInventory();
}

GUIEntityPlayer::GUIEntityPlayer(EntityPlayer* player)
	:m_player(player->getID()), m_disgusting_player(player)
{
	
}

void GUIEntityPlayer::update(World& w)
{
}

void GUIEntityPlayer::render(BatchRenderer2D& renderer)
{
}

void GUIEntityPlayer::onAttachedToHUD(HUD& hud)
{
	m_gui_action_slots = new GUIActionSlots(&m_disgusting_player->getInventory(),hud);
	m_gui_action_slots->setAlignment(GUIAlign::LEFT_UP);
	hud.appendChild(m_gui_action_slots,getID());
}

const std::string& GUIEntityPlayer::getID() const
{
	static std::string id = "player";
	return id;
}
