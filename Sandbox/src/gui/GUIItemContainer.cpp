#include "GUIItemContainer.h"

GUIItemContainer::GUIItemContainer()
	:GUIElement(GETYPE::ItemContainer)
{
	setPadding(16);
}

void GUIItemContainer::onMyEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::MouseMove)
		return;
	if (m_container && onContainerEventConsumer)
		onContainerEventConsumer(m_container->getID(), m_slotIndex, e);
	if (!isNotSpacial && e.getEventType() == Event::EventType::MousePress)
		e.handled = true;
}

void GUIItemContainer::setContainer(Inventory* c, int slot)
{
	m_container = c;
	m_slotIndex = slot;
}

const ItemStack* GUIItemContainer::getItemStack() const
{
	
	if (m_container == nullptr)
		return nullptr;
	return m_container->getItemStack(m_slotIndex);
}
