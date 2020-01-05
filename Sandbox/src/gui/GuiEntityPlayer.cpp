#include "GUIEntityPlayer.h"
#include "world/entity/EntityPlayer.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"
#include "core/Stats.h"



GUIActionSlots::GUIActionSlots(PlayerInventory* player, HUD& hud)
	:GUIElement(GETYPE::Other),m_inventory(player)
{
	isAlwaysPacked = true;
	isVisible = false;

	constexpr int slotCount = 10;

	m_col = new GUIColumn(GUIAlign::LEFT);
	m_col->isAlwaysPacked = true;
	auto row = new GUIRow();
	row->isAlwaysPacked = true;

	for (int i = 0; i < slotCount; ++i)
	{
		auto c = new GUIItemContainer();
		c->setContainer(player,InventorySlot::INVENTORY_SLOT_ACTION_FIRST+i);
		c->onContainerEventConsumer = hud.getContainerConsumer();
		c->onContainerEventConsumer = [this](const std::string& s, int i, Event& e)
		{
			HUD::get()->consumeContainerEvent(s, i, e);
			if(e.getEventType()!=Event::EventType::MouseMove)
				showTitleInternal(this->m_show_title);
		};
		row->appendChild(c);
	}
	m_col->appendChild(row);

	m_title = new GUIText(FontMatLib::getMaterial("res/fonts/andrew.fnt"));
	m_title->dim = { 0,15 };
	m_title->isAlwaysPacked = false;
	m_col->appendChild(m_title);
	appendChild(m_col);
	setMainSlot(m_inventory->getHandIndex());
	showTitleInternal(true);
	showTitleInternal(false);

}

void GUIActionSlots::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);
	if(e.getEventType()==Event::EventType::KeyType)
	{
		auto m = static_cast<KeyTypeEvent&>(e);
		if(m.getKey()>=GLFW_KEY_0&&m.getKey()<=GLFW_KEY_9)
		{
			int slot = m.getKey() - GLFW_KEY_1;
			if (slot == -1)
				slot = 9;

			if(main_slot!=slot)
				setMainSlot(slot);
			e.handled = true;
		}
	}
	else if (e.getEventType() == Event::EventType::MouseScroll)
	{
		auto m = static_cast<MouseScrollEvent&>(e);
		auto slot = main_slot + (m.getScrollY()>0?-1:1);
		if (slot < 0)
			slot = 9;
		else if (slot > 9)
			slot = 0;
		if (main_slot != slot)
			setMainSlot(slot);
		e.handled = true;
		
	}
}

void GUIActionSlots::update()
{
	constexpr float closeSpeed = 1;
	constexpr float maxScale = 6;
	if (old_slot != -1)
	{
		auto& sca = ((GUIItemContainer*)getFirstChild()->getFirstChild()->getChildren()[old_slot])->slotScale;
		sca -= closeSpeed;
		if(sca<0)
		{
			sca = 0;
			old_slot = -1;
		}
	}
	if (main_slot != -1)
	{
		auto& sca = ((GUIItemContainer*)getFirstChild()->getFirstChild()->getChildren()[main_slot])->slotScale;
		sca += closeSpeed;
		if (sca > maxScale)
		{
			sca = maxScale;
		}
	}
}

void GUIActionSlots::setMainSlot(int slot)
{
	if (slot>getFirstChild()->getFirstChild()->getChildren().size())
		return;//invalid slot
	m_inventory->setHandIndex(InventorySlot::INVENTORY_SLOT_ACTION_FIRST + slot);
	if(old_slot!=-1)
	{
		((GUIItemContainer*)getFirstChild()->getFirstChild()->getChildren()[old_slot])->slotScale = 0;
	}
	old_slot = main_slot;
	main_slot = slot;
	showTitleInternal(m_show_title);
}

void GUIActionSlots::showTitleInternal(bool show)
{
	if (m_col->hasChild(m_title->id) && !show)
		m_col->takeChild(m_title->id);
	else if (!m_col->hasChild(m_title->id) && show)
		m_col->appendChild(m_title);
	if (show)
	{
		if (main_slot == -1) {
			m_title->setText("");
		}
		else {
			auto stack = m_inventory->getItemStack(InventorySlot::INVENTORY_SLOT_ACTION_FIRST + main_slot);
			m_title->setText(stack ? stack->getItem().getTitle(stack) : "");
		}
	}
}

void GUIActionSlots::showTitle(bool show)
{
	m_show_title = show;
	showTitleInternal(m_show_title);
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

	m_col = new GUIColumn();
	m_col->isAlwaysPacked = true;
	m_col->setAlignment(GUIAlign::LEFT_UP);
	
	m_gui_action_slots = new GUIActionSlots(&m_disgusting_player->getInventory(),hud);
	m_col->appendChild(m_gui_action_slots);
	


	hud.appendChild(m_col, getID());

	openInventory(false);
}

const std::string& GUIEntityPlayer::getID() const
{
	static std::string id = "player";
	return id;
}

void GUIEntityPlayer::openInventory(bool open)
{
	m_is_inventory_open = open;
	
	while(m_col->getChildren().size()>1)
		m_col->removeChild(m_col->getChildren().size()-1);

	if(m_is_inventory_open)
	{
		auto filler = new GUIBlank();
		filler->dim = { 0,15 };
		filler->isAlwaysPacked = false;
		m_col->appendChild(filler);
		
		m_gui_slots = new GUISlots(&m_disgusting_player->getInventory(), *HUD::get(), 
			InventorySlot::INVENTORY_SLOT_RANDOM_FIRST, InventorySlot::INVENTORY_SLOT_RANDOM_FIRST + 39, 10);
		m_col->appendChild(m_gui_slots);
		m_gui_action_slots->showTitle(false);
	}
	else{
		auto& inHand = m_disgusting_player->getInventory().handSlot();
		if(inHand!=nullptr)
		{
			m_disgusting_player->throwItem(*Stats::world, inHand);
			inHand = nullptr;
		}
		auto enties = HUD::get()->getEntities();
		std::vector<std::string> ids;
		for (auto& enty : enties)
		{
			if (enty.entity->getID() != this->getID())
				ids.push_back(enty.entity->getID());
		}
		for (auto& cs : ids)
		{
			HUD::get()->unregisterGUIEntity(cs);

			//remove all except our entity
		}
		m_gui_action_slots->showTitle(true);
	}
}
