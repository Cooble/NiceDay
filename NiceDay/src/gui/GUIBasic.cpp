#include "ndpch.h"
#include "GUIBasic.h"
#include "App.h"

GUILabel::GUILabel() : GUIElement(GETYPE::Label)
{
	is_final_element = true;
}

void GUILabel::setValue(const std::string& val)
{
	m_value = val;
	markDirty();
}

GUIButton::GUIButton() : GUIElement(GETYPE::Button)
{
	is_final_element = true;
}

void GUIButton::setValue(const std::string& val)
{
	m_value = val;
	markDirty();
}

void GUIButton::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);

	if (e.getEventType() == Event::EventType::MousePress)
	{
		if (on_pressed)
			on_pressed(*this);
	}
}

GUIWindow::GUIWindow(): GUIElement(GETYPE::Window)
{
	setPadding(10);
}

void GUIWindow::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);

	if (e.getEventType() == Event::EventType::MousePress)
	{
		auto& m = static_cast<MousePressEvent&>(e);
		m_draggedCursor = m.getPos() - glm::vec2(x, y);
	}
	if (e.getEventType() == Event::EventType::MouseMove && is_pressed)
	{
		auto& m = static_cast<MousePressEvent&>(e);
		pos = m.getPos() - m_draggedCursor;
	}
}


GUIColumn::GUIColumn(): GUIElement(GETYPE::Column)
{
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
}


void GUIColumn::appendChild(GUIElement* element)
{
	if (getChildren().empty())
		element->y = getParent()->height - element->height - getParent()->padding[GUI_TOP];
	else
		element->y = getChildren()[getChildren().size() - 1]->y - space - element->height;

	switch (alignment)
	{
	case GUIAlign::RIGHT:
		element->x = getParent()->width - element->width - getParent()->padding[GUI_RIGHT];
		break;
	case GUIAlign::LEFT:
		element->x = getParent()->padding[GUI_LEFT];
		break;
	case GUIAlign::CENTER:
		element->x = (getParent()->width - element->width) / 2;
		break;
	default:
		ASSERT(false, "Unsupported alignment");
		break;
	}
	GUIElement::appendChild(element);
}

GUIRow::GUIRow(): GUIElement(GETYPE::Row)
{
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
}

void GUIRow::appendChild(GUIElement* element)
{
	element->y = getParent()->height - element->height - getParent()->padding[GUI_TOP];

	switch (alignment)
	{
	case GUIAlign::LEFT:
		if (getChildren().empty())
			element->x = getParent()->padding[GUI_LEFT];
		else
		{
			auto child = getChildren()[getChildren().size() - 1];
			element->x = space + child->x + child->width;
		}
		break;
	case GUIAlign::RIGHT:

		element->x = getParent()->width - element->width - getParent()->padding[GUI_RIGHT];
		for (auto child : getChildren())
			child->x -= element->width + space;
		break;
	default:
		ASSERT(false, "Unsupported alignment");
		break;
	}
	GUIElement::appendChild(element);
}

float GUIGrid::getLowestY()
{
	float y = getParent()->height;
	for (auto child : getChildren())
		y = std::min(y, child->y);
	return y;
}

GUIGrid::GUIGrid(): GUIElement(GETYPE::Grid)
{
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
}

void GUIGrid::appendChild(GUIElement* element)
{
	if (getChildren().empty())
	{
		element->x = getParent()->padding[GUI_LEFT];
		element->y = getParent()->height - getParent()->padding[GUI_TOP] - element->height;
	}
	else
	{
		auto child = getChildren()[getChildren().size() - 1];
		if (child->x + child->width + space + element->width > getParent()->width - getParent()->padding[GUI_RIGHT])
		{
			//next row
			element->y = getLowestY() - space - element->height;
			element->x = getParent()->padding[GUI_LEFT];
		}
		else
		{
			element->x = child->x + child->width + space;
			element->y = child->y + child->height - element->height;
		}
	}

	GUIElement::appendChild(element);
}
