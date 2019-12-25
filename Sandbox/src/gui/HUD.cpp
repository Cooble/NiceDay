#include "HUD.h"
#include "core/App.h"
#include "GUIItemContainer.h"
#include "core/Stats.h"
#include "event/Event.h"
#include "GLFW/glfw3.h"
#include "GUIEntityPlayer.h"


HUD* HUD::s_hud = nullptr;

HUD::HUD()
{
	s_hud = this;
	isNotSpacial = true;
	m_container_consumer = std::bind(&HUD::consumeContainerEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_hand = new GUIItemContainer();
	m_hand->isNotSpacial = true;//disable events
	m_hand->dim = { 64,64 };
	m_hand->isSlotRendered = false;
	GUIWindow::appendChild(m_hand);

	width = App::get().getWindow()->getWidth();
	height = App::get().getWindow()->getHeight();
	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	isVisible = false;
	isMoveable = false;
	isResizable = false;

	setAlignment(GUIAlign::CENTER);
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;

	//auto material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");
	//auto materialSmall = FontMatLib::getMaterial("res/fonts/andrew.fnt");
}

HUD::~HUD()
{
	for (auto& entity : m_entities)
		delete entity.entity;
}

void HUD::registerGUIEntity(GUIEntity* e)
{
	//if we have duplicate, we remove it first
	for (auto& entity : m_entities)
		if(entity.entity->getID()==e->getID())
		{
			unregisterGUIEntity(e->getID());
			break;
		}
	
	m_entities.emplace_back(e);
	e->onAttachedToHUD(*this);
	auto playerInv = dynamic_cast<GUIEntityPlayer*>(e);
	if(playerInv)
	{
		m_player = dynamic_cast<PlayerInventory*>(playerInv->getInventory());
		m_hand->setContainer(playerInv->getInventory(), InventorySlot::HAND);
	}	
}

bool HUD::isRegistered(const std::string& id)
{
	for (auto& entity : m_entities)
	{
		if (entity.entity->getID() == id)
			return true;
	}
	return false;
}

void HUD::unregisterGUIEntity(const std::string& id)
{
	for (int i = 0; i < m_entities.size(); ++i)
		if (m_entities[i].entity->getID() == id)
		{
			m_entities[i].entity->onDetached();
			delete m_entities[i].entity;
			for (auto child : m_entities[i].children)
			{
				removeChildWithID(child);
			}
			m_entities.erase(m_entities.begin() + i);
			return;
		}
}

void HUD::appendChild(GUIElement* element)
{
	ASSERT(false, "You need to use method appendChild(element,ownerID) !");
}

void HUD::update()
{
	GUIWindow::update();
	for (auto& entity : m_entities)
	{
		entity.entity->update(*Stats::world);
	}
}

void HUD::onMyEvent(Event& e)
{
	//move with item in hand
	if(e.getEventType()==Event::EventType::MouseMove)
	{
		auto m = static_cast<MouseMoveEvent&>(e);
		m_hand->pos = m.getPos()-(m_hand->dim/2.f);
	}
}

void HUD::appendChild(GUIElement* element, const std::string& ownerID)
{
	GUIWindow::appendChild(element);
	bool success=false;
	for (auto& bundle : m_entities)
	{
		if(bundle.entity->getID()==ownerID)
		{
			bundle.children.push_back(element->id);
			success = true;
			break;
		}
	}
	ASSERT(success, "Appending child of invalid ownerID");

	//put hand on the top
	m_children[m_children.size() - 1] = m_hand;
	m_children[m_children.size() - 2] = element;
}

void HUD::consumeContainerEvent(const std::string& id, int slot, Event& e)
{
	if (m_player == nullptr)
		return;
	if (e.getEventType() != Event::EventType::MousePress)
		return;
	Inventory* c = nullptr;
	for (auto& ee : m_entities)
	{
		auto entity = ee.entity->getInventory();
		if(entity==nullptr)
			continue;
		if(entity->getID()!=id)
			continue;
		c = entity;
		break;
	}
	if (c == nullptr) {
		ND_WARN("Called from GUIcontainer that has invalid owner id");
		return;
	}
	auto even = static_cast<MousePressEvent&>(e);
	bool left = even.getButton() == GLFW_MOUSE_BUTTON_LEFT;
	auto& inHand = m_player->itemInHand();

	//we have nothing in hand
	if(inHand==nullptr)
	{
		//take all from slot
		if(left)
		{
			if(App::get().getInput().isKeyPressed(GLFW_KEY_LEFT_SHIFT))
			{
				inHand = c->takeFromIndex(slot,c->getItemStack(slot)!=nullptr?(int)std::ceil(c->getItemStack(slot)->size()/2.f):-1);
				
			}else
				inHand = c->takeFromIndex(slot);
		}
		//take one from slot
		else
		{
			inHand = c->takeFromIndex(slot,1);
		}
	}
	//we have something in hand
	else
	{
		//put everything in the slot
		if(left)
		{
			//same item types -> put everyting in slot
			if(inHand->equals(c->getItemStack(slot)))
			{
				inHand = c->putAtIndex(inHand, slot);
			}
			//different items -> swap items
			else
			{
				inHand = c->swap(inHand, slot);
			}
		}
		//put one in the slot
		else
		{
			//same item types -> add one to slot
			if (inHand->equals(c->getItemStack(slot))||c->getItemStack(slot)==nullptr)
			{
				inHand = c->putAtIndex(inHand, slot,1);
			}
		}
	}
	
	
}
