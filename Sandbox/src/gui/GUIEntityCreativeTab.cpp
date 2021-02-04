#include "GUIEntityCreativeTab.h"
#include "inventory/CreativeInventory.h"


GUICreativeTab::GUICreativeTab(HUD& hud) :GUIElement(GETYPE::Other)
{
	//isAlwaysPacked = true;
	isAlwaysPacked = false;
	width = 500;
	height = 400;
	// size per line
	m_item_per_line_count = 6;
	//how many lines
	m_item_lines_count = 6;
	//isVisible = true;
	/*
	auto view = createGUISliderView(false);
	view->width = width;
	view->height = height;
	GUIView* src = dynamic_cast<GUIView*>(view->getLeft()->getFirstChild());
	//m_world_slider = dynamic_cast<GUIVSlider*>(view->getRight()->getFirstChild());
	src->setPadding(0);
	src->getInside()->color = {0,0,0,0};
	src->getInside()->appendChild(slots);
	appendChild(view);*/
	m_inv = new CreativeInventory();

	m_slots = new GUISlots(m_inv, hud, 0, m_item_lines_count*m_item_per_line_count -1, m_item_per_line_count);
	m_slots->setAlignment(GUIAlign::LEFT_UP);

	
	auto slider = new GUIVSlider();
	slider->dimInherit = GUIDimensionInherit::HEIGHT;
	slider->setAlignment(GUIAlign::RIGHT);
	slider->width = 20;
	int totalLines = glm::ceil((float)m_inv->getItemsSize() / m_item_per_line_count);
	slider->setQuantization(totalLines);
	slider->on_changed = [slider, totalLines,slots = m_slots, inventorySize = m_inv->getItemsSize(),linesCount = m_item_lines_count,perLineCount = m_item_per_line_count](GUIElement& e)
	{
		int i = (totalLines-slider->getValue()-1) * perLineCount;
		slots->updateIndexes(i, i+linesCount*perLineCount-1);
	};
	slider->setValue(0);

	appendChild(m_slots);
	appendChild(slider);
	
}

GUICreativeTab::~GUICreativeTab()
{
	delete m_inv;
}

Inventory* GUIEntityCreativeTab::getInventory()
{
	return m_element->getInventory();
}

void GUIEntityCreativeTab::onAttachedToHUD(HUD& hud)
{
	m_element = new GUICreativeTab(hud);
	m_element->setAlignment(GUIAlign::RIGHT_UP);
	hud.appendChild(m_element, getID());
}

void GUIEntityCreativeTab::onDetached()
{
}

const std::string& GUIEntityCreativeTab::getID() const
{
	static std::string c = "player_creative";
	return c;
}
