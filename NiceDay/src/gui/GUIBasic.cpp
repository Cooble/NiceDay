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

GUITextBox::GUITextBox(): GUIElement(GETYPE::TextBox), m_cursorMesh(1)
{
	setPadding(10);
}

void GUITextBox::setValue(const std::string& val)
{
	is_dirty = true;
	m_value = val;
	cursorPos = m_value.size();
	textClipOffset = 0;
	
}

void GUITextBox::moveCursor(int delta)
{
	int oldcur = cursorPos;
	cursorPos += delta;
	cursorPos = std::clamp(cursorPos, 0, (int)m_value.size());

	is_dirty = oldcur != cursorPos;
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
			if (contains(m.getX(), m.getY()))
			{
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
				if (m_value.size() && cursorPos > 0)
				{
					m_value = m_value.substr(0, cursorPos - 1) + m_value.substr(cursorPos);
					is_dirty = true;
					moveCursor(-1);
				}
				break;
			case GLFW_KEY_DELETE:
				if (cursorPos < m_value.size())
				{
					m_value = m_value.substr(0, cursorPos) + m_value.substr(cursorPos + 1);
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
				m_value.insert(m_value.begin() + cursorPos, (char)key);
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


		constexpr float borderThickness = 10;
		m_resizeMode = invalid;
		if (m_draggedCursor.x < borderThickness)
		{
			m_resizeMode = left;
			if (m_draggedCursor.y < borderThickness)
				m_resizeMode = left_down;
			else if (m_draggedCursor.y > height - borderThickness)
				m_resizeMode = left_up;
		}
		else if (m_draggedCursor.x > width - borderThickness)
		{
			m_resizeMode = right;
			if (m_draggedCursor.y < borderThickness)
				m_resizeMode = right_down;
			else if (m_draggedCursor.y > height - borderThickness)
				m_resizeMode = right_up;
		}
		else
		{
			if (m_draggedCursor.y < borderThickness)
				m_resizeMode = down;
			else if (m_draggedCursor.y > height - borderThickness)
				m_resizeMode = up;
		}
		if (m_resizeMode != invalid && resizable)
		{
			m_draggedCursor = m.getPos();
			m_resize_x = x;
			m_resize_y = y;
			m_resize_w = width;
			m_resize_h = height;
		}
	}
	if (e.getEventType() == Event::EventType::MouseMove && is_pressed)
	{
		auto& m = static_cast<MousePressEvent&>(e);
		auto delta = m.getPos() - m_draggedCursor;
		if (m_resizeMode != invalid && resizable && moveable)
		{
			switch (m_resizeMode)
			{
			case up:
				height = m_resize_h + delta.y;
				break;
			case down:
				y = m_resize_y + delta.y;
				height = m_resize_h - delta.y;
				break;
			case left:
				x = m_resize_x + delta.x;
				width = m_resize_w - delta.x;
				break;
			case right:
				width = m_resize_w + delta.x;
				break;
			case left_up:
				x = m_resize_x + delta.x;
				width = m_resize_w - delta.x;
				height = m_resize_h + delta.y;
				break;
			case left_down:
				x = m_resize_x + delta.x;
				width = m_resize_w - delta.x;
				y = m_resize_y + delta.y;
				height = m_resize_h - delta.y;
				break;
			case right_up:
				width = m_resize_w + delta.x;
				height = m_resize_h + delta.y;
				break;

			case right_down:
				width = m_resize_w + delta.x;
				y = m_resize_y + delta.y;
				height = m_resize_h - delta.y;
				break;
			}
			onDimensionChange();
		}
		else if (moveable)
			pos = m.getPos() - m_draggedCursor;
	}
}

GUIColumn::GUIColumn(GUIAlign childAlignment) : GUIElement(GETYPE::Column)
{
	is_not_spacial = true;
	is_diplayed = false;
	is_final_element = false;
	child_alignment = childAlignment;
	space = 5;
}

void GUIColumn::repositionChildren()
{
	auto oldW = this->width;
	auto oldH = this->height;
	this->width = 0;
	this->height = 0;

	for (auto child : getChildren())
	{
		this->width = std::max(this->width, child->width);
		this->height += child->height + space;
	}
	if (this->height)
		this->height -= space;

	float yPos = this->height;
	for (auto child : getChildren())
	{
		child->y = yPos - child->height - space;

		yPos = child->y;

		switch (child_alignment)
		{
		case GUIAlign::RIGHT:
			child->x = this->width - child->width;
			break;
		case GUIAlign::LEFT:
			child->x = 0;
			break;
		case GUIAlign::CENTER:
			child->x = (this->width - child->width) / 2;
			break;
		default:
			ASSERT(false, "Unsupported alignment");
			break;
		}
	}

	if (oldW != this->width || oldH != this->height)
		if (getParent())
			getParent()->repositionChildren();
}

GUIRow::GUIRow(GUIAlign childAlignment) :
	GUIElement(GETYPE::Row)
{
	is_diplayed = false;
	is_final_element = false;
	is_not_spacial = true;
	child_alignment = childAlignment;
	space = 5;
}

void GUIRow::repositionChildren()
{
	auto oldW = this->width;
	auto oldH = this->height;
	this->width = 0;
	this->height = 0;

	for (auto child : getChildren())
	{
		this->height = std::max(this->height, child->height);
		this->width += child->width + space;
	}
	if (this->width)
		this->width -= space;

	float xPos = 0;

	for (auto child : getChildren())
	{
		child->x = xPos;
		xPos += child->width + space;

		switch (child_alignment)
		{
		case GUIAlign::UP:
			child->y = this->height - child->height;
			break;
		case GUIAlign::DOWN:
			child->y = 0;
			break;
		case GUIAlign::CENTER:
			child->y = (this->height - child->height) / 2;
			break;
		default:
			ASSERT(false, "Unsupported alignment");
			break;
		}
	}
	if (oldW != this->width || oldH != this->height)
		if (getParent())
			getParent()->repositionChildren();
}

GUIGrid::GUIGrid() :
	GUIElement(GETYPE::Grid)
{
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
	space = 5;
}

void GUIGrid::repositionChildren()
{
	float minY = 0;
	for (int i = 0; i < getChildren().size(); ++i)
	{
		auto child = getChildren()[i];
		if (i == 0)
		{
			child->x = 0;
			child->y = 0 - child->height;
			minY = std::min(child->y, minY);
		}
		else
		{
			auto lastChild = getChildren()[i - 1];
			if (lastChild->x + lastChild->width + space + child->width > this->width)
			{
				//next row
				child->y = minY - space - child->height;
				child->x = 0;
				minY = std::min(child->y, minY);
			}
			else
			{
				child->x = lastChild->x + lastChild->width + space;
				child->y = lastChild->y + lastChild->height - child->height;
				minY = std::min(child->y, minY);
			}
		}
	}
	height = -minY;
	for (auto child : getChildren())
		child->y += height;
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

GUIBlank::GUIBlank():GUIElement(GETYPE::Blank)
{
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
	is_always_packed = true;
}

GUIHorizontalSplit::GUIHorizontalSplit(GUIElement* eUp, GUIElement* eDown, bool isUpMain): GUIHorizontalSplit(isUpMain)
{
	getUpChild()->appendChild(eUp);
	getDownChild()->appendChild(eDown);
}

GUIHorizontalSplit::GUIHorizontalSplit(bool isUpMain) :GUIElement(GETYPE::SplitHorizontal)
{
	getChildren().reserve(2);
	getChildren().push_back(new GUIBlank());
	getChildren().push_back(new GUIBlank());
	getUpChild()->setParent(this);
	getDownChild()->setParent(this);

	if (isUpMain)
		getDownChild()->is_always_packed = false;
	else getUpChild()->is_always_packed = false;

	getUpChild()->dimension_inherit = GUIDimensionInherit::WIDTH;
	getDownChild()->dimension_inherit = GUIDimensionInherit::WIDTH;

	m_is_up_main = isUpMain;
	dimension_inherit = GUIDimensionInherit::WIDTH_HEIGHT;
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
	space = 5;
	setAlignment(GUIAlign::CENTER);
}

void GUIHorizontalSplit::repositionChildren()
{
	auto upC = getUpChild();
	auto downC = getDownChild();
	
	downC->pos = { 0,0 };
	
	if (m_is_up_main)
	{
		upC->pos = { 0,height - upC->height };

		glm::vec2 newDim = { width,height - upC->height-space };
		if ((newDim - downC->dim) != glm::vec2(0, 0)) {
			downC->dim = newDim;
			downC->onDimensionChange();
		}
	}
	else
	{
		upC->pos = { 0,downC->height+space };
		glm::vec2 newDim = { width,height - downC->height-space };
		if ((newDim - upC->dim) != glm::vec2(0, 0)) {
			upC->dim = newDim;
			upC->onDimensionChange();
		}
	}
}

void GUIHorizontalSplit::onDimensionChange()
{
	auto upC = getUpChild();
	auto downC = getDownChild();
	if(m_is_up_main)
	{
		upC->onParentChanged();
		upC->pos = { 0,height-upC->height };
		
		downC->dim = { width,height -upC->height-space };
		downC->pos = { 0,0 };
		downC->onDimensionChange();		
	}
	else
	{
		downC->onParentChanged();
		downC->pos = { 0,0 };

		upC->dim = { width,height - downC->height-space };
		upC->pos = { 0,downC->height+space };
		upC->onDimensionChange();
	}
}

void GUIHorizontalSplit::appendChild(GUIElement* element)
{
	ASSERT(false, "Invalid operation");
}

void GUIHorizontalSplit::removeChild(int index)
{
	ASSERT(false, "Invalid operation");
}


GUIVerticalSplit::GUIVerticalSplit(GUIElement* eUp, GUIElement* eDown, bool isleftMain) : GUIVerticalSplit(isleftMain)
{
	getRightChild()->appendChild(eUp);
	getLeftChild()->appendChild(eDown);
}

GUIVerticalSplit::GUIVerticalSplit(bool isLeftMain) :GUIElement(GETYPE::SplitVertical)
{
	getChildren().reserve(2);
	getChildren().push_back(new GUIBlank());
	getChildren().push_back(new GUIBlank());
	getRightChild()->setParent(this);
	getLeftChild()->setParent(this);

	if (isLeftMain)
		getRightChild()->is_always_packed = false;
	else getLeftChild()->is_always_packed = false;

	getRightChild()->dimension_inherit = GUIDimensionInherit::HEIGHT;
	getLeftChild()->dimension_inherit = GUIDimensionInherit::HEIGHT;

	m_is_left_main = isLeftMain;
	dimension_inherit = GUIDimensionInherit::WIDTH_HEIGHT;
	is_diplayed = false;
	is_not_spacial = true;
	is_final_element = false;
	space = 5;
	setAlignment(GUIAlign::CENTER);
}

void GUIVerticalSplit::repositionChildren()
{
	auto rightC = getRightChild();
	auto leftC = getLeftChild();

	leftC->pos = { 0,0 };
	
	if (m_is_left_main)
	{

		rightC->pos = { leftC->width+space,0 };
		glm::vec2 newDim = { width - leftC->width - space,height };
		if ((newDim - rightC->dim) != glm::vec2(0, 0)) {
			rightC->dim = newDim;
			rightC->onDimensionChange();
		}
	}
	else
	{
		rightC->pos = { width-rightC->width,0 };

		glm::vec2 newDim = { width - rightC->width - space,height };
		if ((newDim - leftC->dim) != glm::vec2(0, 0)) {
			leftC->dim = newDim;
			leftC->onDimensionChange();
		}
	}
}

void GUIVerticalSplit::onDimensionChange()
{
	auto rightC = getRightChild();
	auto leftC = getLeftChild();
	leftC->pos = { 0,0 };
	if (!m_is_left_main)
	{
		rightC->onParentChanged();
		rightC->pos = { width - rightC->width,0 };

		leftC->dim = { width - rightC->width - space,height };
		leftC->onDimensionChange();
	}
	else
	{
		leftC->onParentChanged();

		rightC->dim = { width - leftC->width - space,height };
		rightC->pos = { leftC->width + space,0 };
		rightC->onDimensionChange();
	}
}

void GUIVerticalSplit::appendChild(GUIElement* element)
{
	ASSERT(false, "Invalid operation");
}

void GUIVerticalSplit::removeChild(int index)
{
	ASSERT(false, "Invalid operation");
}
