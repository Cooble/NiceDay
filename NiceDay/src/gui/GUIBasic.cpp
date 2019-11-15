#include "ndpch.h"
#include "GUIBasic.h"
#include "App.h"
#include "graphics/API/Texture.h"
#include "GUIContext.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"

GUILabel::GUILabel() : GUIElement(GETYPE::Label)
{
	is_final_element = true;
}

void GUILabel::setValue(const std::string& val)
{
	m_value = val;
	markDirty();
}

GUITextBox::GUITextBox(): GUIElement(GETYPE::TextBox),m_cursorMesh(1)
{
	setPadding(10);
	
}

void GUITextBox::setValue(const std::string& val)
{
	is_dirty = true;
	m_value = val;
	cursorPos = m_value.size();
}

void GUITextBox::moveCursor(int delta)
{
	int oldcur = cursorPos;
	cursorPos += delta;
	cursorPos = std::clamp(cursorPos,0,(int)m_value.size());

	is_dirty = oldcur!=cursorPos;


}

void GUITextBox::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);
	switch (e.getEventType())
	{
	case Event::EventType::MousePress:
		{
			auto m = static_cast<MousePressEvent&>(e);
			m_has_total_focus = false;
			if (contains(m.getX(), m.getY())) {
				GUIContext::get().setFocusedElement(this);
				m_has_total_focus = true;
				cursorPos = m_value.size();
				is_dirty = true;
			}
			else if (GUIContext::get().getFocusedElement() == this) 
				GUIContext::get().setFocusedElement(nullptr);
			
		}
		break;
	
	case Event::EventType::KeyPress:
		if (GUIContext::get().getFocusedElement() == this)
		{
			auto m = static_cast<KeyPressEvent&>(e);
			switch (m.getKey())
			{
			case GLFW_KEY_ENTER:
			case GLFW_KEY_ESCAPE:
				GUIContext::get().setFocusedElement(nullptr);
				m_has_total_focus = false;
				e.handled = true;
				break;
			case GLFW_KEY_BACKSPACE:
				if (m_value.size()&&cursorPos>0)
				{
					m_value = m_value.substr(0, cursorPos-1)+m_value.substr(cursorPos);
					is_dirty = true;
					moveCursor(-1);
				}
				break;
			case GLFW_KEY_DELETE:
				if (cursorPos < m_value.size())
				{
					m_value = m_value.substr(0, cursorPos)+ m_value.substr(cursorPos+1);
					is_dirty = true;
				}
				break;

			case GLFW_KEY_LEFT:
				moveCursor(-1);
				break;
			case GLFW_KEY_RIGHT:
				moveCursor(1);
				break;
			}
		}
		break;
	case Event::EventType::KeyType:
		if (GUIContext::get().getFocusedElement() == this)
		{
			auto m = static_cast<KeyTypeEvent&>(e);
			auto key = m.getKey();
			if (key != GLFW_KEY_UNKNOWN)
			{
				m_value.insert(m_value.begin()+cursorPos,(char)key);
				moveCursor(1);
			}
		}
		break;
	default: ;
	}
}


GUIButton::GUIButton()
	:
	GUIElement(GETYPE::Button)
{
	is_final_element = true;
}

void GUIButton::setText(const std::string& val)
{
	m_text = val;
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

GUICheckBox::GUICheckBox()
	:
	GUIElement(GETYPE::CheckBox)
{
	static SpriteSheetResource* res = new SpriteSheetResource(
		Texture::create(TextureInfo("res/images/gui_atlas.png")),
		4, 4);
	static Sprite* sTrue = nullptr;
	static Sprite* sFalse = nullptr;

	if (sTrue == nullptr)
	{
		sTrue = new Sprite(res);
		sFalse = new Sprite(res);
		sTrue->setSpriteIndex(0, 3);
		sFalse->setSpriteIndex(1, 3);
	}

	spriteTrue = sTrue;
	spriteFalse = sFalse;
}

void GUICheckBox::setText(const std::string& trueText, const std::string& falseText)
{
	m_textFalse = falseText;
	m_textTrue = trueText;
	markDirty();
}

void GUICheckBox::setValue(bool b)
{
	is_dirty = b != value;
	value = b;
}

void GUICheckBox::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);

	if (e.getEventType() == Event::EventType::MousePress)
	{
		value = !value;
		is_dirty = true;
		if (on_pressed)
			on_pressed(*this);
	}
}

GUIImage::GUIImage()
	:
	GUIElement(GETYPE::Image)
{
}

void GUIImage::setValue(Sprite* sprite)
{
	this->src = sprite;
	this->dim = sprite->getSize();
}

Sprite* GUIImage::getValue()
{
	return this->src;
}

GUIWindow::GUIWindow()
	:
	GUIElement(GETYPE::Window)
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


GUIColumn::GUIColumn()
	:
	GUIElement(GETYPE::Column)
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

GUIRow::GUIRow()
	:
	GUIElement(GETYPE::Row)
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

GUIGrid::GUIGrid()
	:
	GUIElement(GETYPE::Grid)
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

GUISlider::GUISlider()
	:
	GUIElement(GETYPE::Slider)
{
	setPadding(10);
}

void GUISlider::onMyEvent(Event& e)
{
	GUIElement::onMyEvent(e);

	if (e.getEventType() == Event::EventType::MousePress)
	{
		auto& m = static_cast<MousePressEvent&>(e);
	}
	if (e.getEventType() == Event::EventType::MouseMove && is_pressed)
	{
		auto& m = static_cast<MousePressEvent&>(e);

		float old = value;
		value = std::clamp(
			(m.getPos().x - GUIContext::get().getStackPos().x - x - padding[GUI_LEFT]) / (width - padding[GUI_LEFT]
				-
				padding[GUI_RIGHT]), 0.f, 1.f);

		if (dividor)
		{
			value *= dividor;
			value = std::round(value) / dividor;;
		}

		if (old != value && on_changed)
			on_changed(*this);
	}
}

void GUISlider::setValue(float v)
{
	value = v;
}
