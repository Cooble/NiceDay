#include "GUIItemContainer.h"

#include "MainWindow.h"
#include "gui/GUIBasic.h"

constexpr int GUI_ITEM_CONTAINER_SIZE = 60;
constexpr int GUI_ITEM_ITEM_SIZE = 32;

GUIItemContainer::GUIItemContainer()
	:GUIElement(GETYPE::ItemContainer)
{
	dim = { GUI_ITEM_CONTAINER_SIZE ,GUI_ITEM_CONTAINER_SIZE };
	setPadding((GUI_ITEM_CONTAINER_SIZE- GUI_ITEM_ITEM_SIZE)/2);
}

void GUIItemContainer::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);
	if (e.getEventType() == Event::EventType::MouseMove)
		return;
	if (m_container && onContainerEventConsumer&&m_slotIndex!=-1&& m_slotIndex<m_container->getItemsSize())
		onContainerEventConsumer(m_container->getID(), m_slotIndex, m_container, e);
	if (!isNotSpatial && e.getEventType() == Event::EventType::MousePress)
		e.handled = true;
}

void GUIItemContainer::setContainer(Inventory* c, int slot)
{
	m_container = c;
	m_slotIndex = slot;
}
void GUIItemContainer::setContainerSlot(int slot)
{
	m_slotIndex = slot;
}

const ItemStack* GUIItemContainer::getItemStack() const
{

	if (m_container == nullptr||m_slotIndex==-1||m_slotIndex>=m_container->getItemsSize())
		return nullptr;
	return m_container->getItemStack(m_slotIndex);
}

GUIItemTitle::GUIItemTitle():GUIBlank()
{
	setPadding(10);
	isAlwaysPacked = true;
	dim = { 100,50 };
	isVisible = true;
	isNotSpatial = true;

	auto mat = GameFonts::smallFont;
	auto row = new GUIColumn(GUIAlign::LEFT);
	row->space = -5;
	row->isAlwaysPacked = true;
	
	m_title = new GUIText(mat);
	m_title->setText("Name");
	row->appendChild(m_title);

	m_meta = new GUIText(mat);
	m_meta->packToZeroWhenEmpty = true;
	m_meta->setText("Metadata");
	row->appendChild(m_meta);

	row->setAlignment(GUIAlign::LEFT_UP);
	appendChild(row);
}

void GUIItemTitle::setTitle(const std::string& s)
{
	m_title->setText(s);
}

void GUIItemTitle::setMeta(const std::string& s)
{
	m_meta->setText(s);
}
