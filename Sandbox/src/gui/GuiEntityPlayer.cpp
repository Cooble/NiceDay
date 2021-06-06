#include "GUIEntityPlayer.h"
#include "Translator.h"
#include "world/entity/EntityPlayer.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"
#include "core/Stats.h"
#include "event/SandboxControls.h"

using namespace nd;

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
		c->setContainer(player,InventorySlot::ACTION_FIRST+i);
		c->onContainerEventConsumer = hud.getContainerConsumer();
		c->onContainerEventConsumer = [this](const std::string& s, int i,Inventory* inv, Event& e)
		{
			HUD::get()->consumeContainerEvent(s, i,inv, e);
			if(e.getEventType()!=Event::EventType::MouseMove)
				showTitleInternal(this->m_show_title);
		};
		row->appendChild(c);
	}
	m_col->appendChild(row);

	m_title = new GUIText(GameFonts::smallFont);
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
		if(m.getKey()>=KeyCode::NUM_0&&m.getKey()<=KeyCode::NUM_9)
		{
			int slot = m.getKey() - KeyCode::NUM_1;
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
	if (slot > getFirstChild()->getFirstChild()->getChildren().size())
		return;//invalid slot
	m_inventory->setHandIndex(InventorySlot::ACTION_FIRST + slot);
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
	if (m_col->hasChild(m_title->serialID) && !show)
		m_col->takeChild(m_title->serialID);
	else if (!m_col->hasChild(m_title->serialID) && show)
		m_col->appendChild(m_title);
	if (show)
	{
		if (main_slot == -1) {
			m_title->setText("");
		}
		else {
			auto stack = m_inventory->getItemStack(InventorySlot::ACTION_FIRST + main_slot);
			auto title = stack ? stack->getItem().getTitle(stack) : "";
			if (title.empty() && stack)
				title = ND_TRANSLATE("item.", stack->getItem().toString());
			m_title->setText(title);
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
	m_col->child_alignment = GUIAlign::LEFT;
	
	m_gui_action_slots = new GUIActionSlots(&m_disgusting_player->getInventory(),hud);

	m_col->appendChild(m_gui_action_slots);

	hud.appendChild(m_col, getID());
	hud.setHandSlot(&m_disgusting_player->getInventory(), InventorySlot::HAND);
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
		m_col->destroyChild(m_col->getChildren().size()-1);

	if(m_is_inventory_open)
	{
		auto filler = new GUIBlank();
		filler->dim = { 0,15 };
		filler->isAlwaysPacked = false;
		m_col->appendChild(filler);
		auto horizont = new GUIRow();
		horizont->isAlwaysPacked = true;
		m_col->appendChild(horizont);
		m_gui_slots = new GUISlots(&m_disgusting_player->getInventory(), *HUD::get(), 
			InventorySlot::RANDOM_FIRST, InventorySlot::RANDOM_FIRST + 39, 10);
		m_gui_slots->getSlots().at(m_gui_slots->getSlots().size() - 1)->clas = "trash";
		horizont->appendChild(m_gui_slots);
		m_gui_action_slots->showTitle(false);

		//space between normal slots and armor slot column
		filler = new GUIBlank();
		filler->dim = { 0,5 };
		filler->isAlwaysPacked = false;
		horizont->appendChild(filler);
		
		auto armorSlots = new GUISlots(&m_disgusting_player->getInventory(), *HUD::get(),
			InventorySlot::ARMOR_HELMET, InventorySlot::ARMOR_BOOTS, 1);
		
		horizont->appendChild(armorSlots);
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
			if(SUtil::startsWith(cs,"player_"))
				HUD::get()->unregisterGUIEntity(cs);

			//remove all except our entity
		}
		m_gui_action_slots->showTitle(true);
	}
}

GUIEntityConsole::GUIEntityConsole()
{
	for (int i = 0; i < m_max_lines; ++i)
	{
		auto templat = new GUIText(GameFonts::smallFont);
		templat->dim = { 300,50 };
		templat->isAlwaysPacked = false;
		
		m_lines.push_back(templat);
	}
}

void GUIEntityConsole::clearChat()
{
	m_messages.clear();
	updateChat();
}

void GUIEntityConsole::addMessage(const std::string& message)
{
	m_messages.push_back(message);
	if (m_messages.size() > m_lines.size())
		m_messages.erase(m_messages.begin());

	updateChat();
}

void GUIEntityConsole::updateChat()
{
	for (int i = 0; i < m_lines.size(); ++i)
		if(i<m_messages.size())
			m_lines[i]->setText(m_messages[i]);
		else
			m_lines[i]->setText("nic");
}


void GUIEntityConsole::update(World& w)
{
}

void GUIEntityConsole::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::MouseMove)
		return;
	if (KeyTypeEvent::getKeyNumber(e) == KeyCode::F)
	{
		ND_INFO("INF");
	}
}

void GUIEntityConsole::render(BatchRenderer2D& renderer)
{
}

void GUIEntityConsole::onAttachedToHUD(HUD& hud)
{
	m_col = new GUIColumn(GUIAlign::LEFT);
	m_col->setAlignment(GUIAlign::LEFT_DOWN);
	for (auto& m_line : m_lines)
		m_col->appendChild(m_line);
	
	auto entryBox = new GUITextBox();
	
	entryBox->width = 300;
	entryBox->height= 100;
	entryBox->fontMaterial = GameFonts::smallFont;
	m_col->width = 300;
	m_col->height = 500;

	entryBox->clas = "chat_box";
	entryBox->onValueEntered = [this, entryBox](GUIElement& e)
	{
		addMessage(entryBox->getValue());
		entryBox->setGainFocus(!entryBox->getValue().empty());
		entryBox->setValue("");
	};
	m_col->appendChild(entryBox);
	
	hud.appendChild(m_col,getID());
}

void GUIEntityConsole::open(bool open)
{
	ND_INFO("opening hehhee");
	m_opened = true;
}
const std::string& GUIEntityConsole::getID() const
{
	static std::string id = "console";
	return id;
}

