#include "ndpch.h"
#include "GUIBasic.h"
#include "core/App.h"
#include "graphics/API/Texture.h"
#include "GUIContext.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"
#include "core/AppGlobals.h"
#include "audio/player.h"
#include "graphics/FontMaterial.h"


GUIText::GUIText() : GUIElement(GETYPE::Text)
{
   isAlwaysPacked = true;
   isNotSpatial = true;
}

GUIText::GUIText(FontMaterial* mat) : GUIText()
{
   fontMaterial = mat;
}

bool GUIText::packDimensions()
{
   auto lastW = width;
   width = widthPadding() + fontMaterial->font->getTextWidth(m_text);
   height = fontMaterial->font->lineHeight + heightPadding();
   if (lastW != width)
	  onDimensionChange();
   markDirty();
   return lastW != width;
}

void GUIText::setText(const std::string& val)
{
   m_text = val;
   auto lastDim = this->dim;
   packDimensions();
   if (getParent() &&lastDim!=this->dim)//inform parent of changed size
	  getParent()->onChildChange();
   markDirty();
}

GUITextBox::GUITextBox() : GUIElement(GETYPE::TextBox), cursorMesh(1)
{
   setPadding(10);
}

void GUITextBox::setValue(const std::string& val)
{
   isDirty = true;
   m_text = val;
   cursorPos = m_text.size();
   textClipOffset = 0;
}

void GUITextBox::onValueModified()
{
}

void GUITextBox::moveCursor(int delta)
{
   int oldcur = cursorPos;
   cursorPos += delta;
   cursorPos = std::clamp(cursorPos, 0, (int)m_text.size());

   isDirty = oldcur != cursorPos;
}

bool GUITextBox::packDimensions()
{
   if (dimInherit == GUIDimensionInherit::WIDTH_HEIGHT)
	  return false;

	
   glm::vec2 lastDim=dim;
   glm::vec2 newDim;

   newDim.x = widthPadding() + fontMaterial->font->getTextWidth(m_text);
   newDim.y = fontMaterial->font->lineHeight + heightPadding();

	
   if (!isWidth(dimInherit))
	  width = newDim.x;
   if (!isHeight(dimInherit))
	  height = newDim.y;

   bool change = lastDim != dim;
   if (change)
	  onDimensionChange();
   markDirty();
   return change;
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
		 cursorPos = m_text.size();
		 isDirty = true;
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
		 case KeyCode::ENTER:
			if (onValueEntered)
			   onValueEntered(*this);
		 case KeyCode::ESCAPE:
			GUIContext::get().setFocusedElement(nullptr);
			m_has_total_focus = false;
			e.handled = true;
			break;
		 case KeyCode::BACKSPACE:
			if (m_text.size() && cursorPos > 0)
			{
			   m_text = m_text.substr(0, cursorPos - 1) + m_text.substr(cursorPos);
			   isDirty = true;
			   moveCursor(-1);
			   onValueModified();
			}
			break;
		 case KeyCode::DELETE_KEY:
			if (cursorPos < m_text.size())
			{
			   m_text = m_text.substr(0, cursorPos) + m_text.substr(cursorPos + 1);
			   isDirty = true;
			   onValueModified();
			}
			break;

		 case KeyCode::LEFT:
			moveCursor(-1);
			break;
		 case KeyCode::RIGHT:
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
		 if (key != KeyCode::UNKNOWN)
		 {
			m_text.insert(m_text.begin() + cursorPos, (char)key);
			moveCursor(1);
			onValueModified();
		 }
	  }
	  break;
   default:;
   }
}

void GUITextBox::setGainFocus(bool cond)
{
   GUIContext::get().setFocusedElement(cond ? this : nullptr);
   m_has_total_focus = cond;
}

GUIButton::GUIButton()
   : GUIElement(GETYPE::Button)
{
}

void GUIButton::onMyEvent(Event& e)
{
   GUIElement::onMyEvent(e);

   if (e.getEventType() == Event::EventType::MousePress)
   {
	  if (soundClick != "")
		 Sounder::get().playSound(soundClick, soundVolume);
	  if (onPressed)
		 onPressed(*this);
   }
   else if (e.getEventType() == Event::EventType::MouseFocusGain)
   {
	  if (soundFocus != "")
		 Sounder::get().playSound(soundFocus, soundVolume);
	  if (onFocusGain)
		 onFocusGain(*this);
   }
   else if (e.getEventType() == Event::EventType::MouseFocusLost)
   {
	  if (onFocusLost)
		 onFocusLost(*this);
   }
}

GUITextButton::GUITextButton(const std::string& text, FontMaterial* material)
{
   setMaterial(material);
   getTextElement()->setText(text);
}

GUIImageButton::GUIImageButton()
{
   auto t = new GUIText();
   t->setAlignment(GUIAlign::CENTER);
   GUIButton::appendChild(t);
}

void GUIImageButton::setImage(Sprite* image)
{
   clearChildren();
   auto t = new GUIImage();
   t->setAlignment(GUIAlign::CENTER);
   t->setImage(image);
   GUIButton::appendChild(t);
}

GUIImageButton::GUIImageButton(Sprite* image)
{
   setImage(image);
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
   isDirty = b != value;
   value = b;
}

void GUICheckBox::onMyEvent(Event& e)
{
   GUIElement::onMyEvent(e);

   if (e.getEventType() == Event::EventType::MousePress)
   {
	  value = !value;
	  isDirty = true;
	  if (on_pressed)
		 on_pressed(*this);
   }
}

GUIImage::GUIImage()
   :
   GUIElement(GETYPE::Image)
{
   isNotSpatial = true;
}

bool GUIImage::packDimensions()
{
   auto old = dim;
   if (image)
	  this->dim = image->getSize();
   return old != dim;
}

void GUIImage::setImage(Sprite* sprite)
{
   this->image = sprite;
   this->dim = sprite->getSize();
   onDimensionChange();
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
	  m_draggedCursor = m.getPos() - GUIContext::get().getStackPos() - pos;


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
	  if (m_resizeMode != invalid && isResizable)
	  {
		 m_draggedCursor = m.getPos();
		 m_resize_x = x;
		 m_resize_y = y;
		 m_resize_w = width;
		 m_resize_h = height;
	  }
   }
   if (e.getEventType() == Event::EventType::MouseMove && m_is_pressed)
   {
	  auto& m = static_cast<MousePressEvent&>(e);
	  auto delta = m.getPos() - m_draggedCursor;
	  if (m_resizeMode != invalid && isResizable && isMoveable)
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
	  else if (isMoveable)
		 pos = m.getPos() - m_draggedCursor;
   }
}

GUIColumn::GUIColumn(GUIAlign childAlignment) : GUIElement(GETYPE::Column)
{
   isNotSpatial = true;
   isAlwaysPacked = true;
   child_alignment = childAlignment;
   space = 5;
   isVisible = false;
}

void GUIColumn::repositionChildren()
{
   auto oldW = this->width;
   auto oldH = this->height;

   if (isAlwaysPacked)
   {
	  this->width = 0;
	  this->height = 0;

	  for (auto child : getChildren())
	  {
		 this->width = std::max(this->width, child->width+widthPadding());
		 this->height += child->height + space;
	  }
	  if (this->height) {
		 this->height -= space;
		  this->height+=heightPadding();
	  }

   }
   if (getParent())
	  switch (dimInherit)
	  {
	  case GUIDimensionInherit::WIDTH:
		 this->width = getParent()->width - getParent()->widthPadding();
		 break;
	  case GUIDimensionInherit::HEIGHT:
		 this->height = getParent()->height - getParent()->heightPadding();
		 break;
	  case GUIDimensionInherit::WIDTH_HEIGHT:
		 this->width = getParent()->width - getParent()->widthPadding();
		 this->height = getParent()->height - getParent()->heightPadding();
		 break;
	  }

   float yPos = this->height-padding[GUI_TOP];
   int spacc = 0;
    for (auto child : getChildren())
   {
	  child->y = yPos - child->height - spacc;
	  spacc = space;
	  yPos = child->y;

	  switch (child_alignment)
	  {
	  case GUIAlign::RIGHT:
		 child->x = this->width - child->width-padding[GUI_RIGHT];
		 break;
	  case GUIAlign::LEFT:
		 child->x = padding[GUI_LEFT];
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
		 getParent()->onChildChange();
}

GUIRow::GUIRow(GUIAlign childAlignment) :
   GUIElement(GETYPE::Row)
{
   isNotSpatial = true;
   isAlwaysPacked = true;
   child_alignment = childAlignment;
   space = 5;
   isVisible = false;
}

void GUIRow::repositionChildren()
{
   auto oldW = this->width;
   auto oldH = this->height;


   if (isAlwaysPacked) {
	  this->width = 0;
	  this->height = 0;
	  for (auto child : getChildren())
	  {
		 this->height = std::max(this->height, child->height);
		 this->width += child->width + space;
	  }
	  if (this->width)
		 this->width -= space;
   }
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
   isVisible = false;
   isNotSpatial = true;
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

void GUIGrid::onChildChange()
{
   float lastHeight = height;
   repositionChildren();
   if (lastHeight != height)
   {
	  if (m_parent)
		 m_parent->onChildChange();
	  if (onDimChange)
		 onDimChange(*this);
   }
}


GUISlider::GUISlider()
   :
   GUIElement(GETYPE::Slider)
{
   setPadding(10);
}

void GUISlider::setQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = possibleStates - 1;
   step = 1;
   //setValue(m_value);
}

void GUISlider::setNormalQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = 1;
   step = 1.f / (possibleStates - 1);
}

static float calculateSliderValue(float normalVal, bool invertedVal, float min, float max)
{
   return min + (invertedVal ? (1 - normalVal) : normalVal) * (max - min);
}

static float quantizeSliderNormalValue(float normalVal, float step, float min, float max)
{
   if (step)
   {
	  float normalizedStep = step / (max - min);
	  return glm::round(normalVal / normalizedStep) * normalizedStep;
   }
   return normalVal;
}

void GUISlider::onMyEvent(Event& e)
{
   GUIElement::onMyEvent(e);

   if (e.getEventType() == Event::EventType::MousePress)
   {
	  auto& m = static_cast<MousePressEvent&>(e);
   }
   if (e.getEventType() == Event::EventType::MouseMove && m_is_pressed)
   {
	  auto& m = static_cast<MousePressEvent&>(e);

	  m_normalValue = std::clamp(
		 (m.getPos().x - GUIContext::get().getStackPos().x - x - padding[GUI_LEFT]) / (width - padding[GUI_LEFT]
			-
			padding[GUI_RIGHT]), 0.f, 1.f);

	  m_normalValue = quantizeSliderNormalValue(m_normalValue, step, minValue, maxValue);
	  float old = m_value;

	  m_value = calculateSliderValue(m_normalValue, invertedVal, minValue, maxValue);

	  if (old != m_value && on_changed)
		 on_changed(*this);
   }
}

void GUISlider::setValue(float v)
{
   v = glm::clamp(v, minValue, maxValue);
   v -= minValue;
   if (step)
	  v = glm::round(v / step) * step;
   m_normalValue = v / (maxValue - minValue);
   if (invertedVal)
	  m_normalValue = 1 - m_normalValue;
   m_value = v + minValue;
}

GUIVSlider::GUIVSlider()
   : GUIElement(GETYPE::VSlider)
{
   setPadding(1);
}

void GUIVSlider::onMyEvent(Event& e)
{
   GUIElement::onMyEvent(e);

   float slideHeight = height - padding[GUI_TOP] - padding[GUI_BOTTOM];
   sliderHeight = slideHeight * this->sliderRatio;

   /*if (sliderRatio == 1)
   {
	   auto old = m_normalValue;
	   m_normalValue = 1;
	   if (m_normalValue != old)
		   if (on_changed)
			   on_changed(*this);
	   return;
   }*/

   float oldNormal = m_normalValue;
   if (e.getEventType() == Event::EventType::MousePress)
   {
	  auto& m = static_cast<MousePressEvent&>(e);
	  m_draggedCursor = m.getPos();
	  auto localY = m.getPos().y - GUIContext::get().getStackPos().y - y - padding[GUI_BOTTOM];
	  bool isUnderSlider = localY < (m_normalValue* (1 - sliderRatio)* slideHeight);
	  bool isAboveSlider = localY > (m_normalValue * (1 - sliderRatio) * slideHeight + sliderHeight);
	  float placeOfClick = localY / slideHeight;
	  if (isUnderSlider || isAboveSlider)
	  {
		 m_normalValue = glm::clamp(placeOfClick - sliderRatio / 2, 0.f, 1.f - sliderRatio);
		 m_draggedCursor = {
			 0,
			 GUIContext::get().getStackPos().y + y + padding[GUI_BOTTOM] + (m_normalValue + sliderRatio / 2) *
			 slideHeight
		 };

		 m_normalValue /= (1 - sliderRatio);
		 m_normalValue = quantizeSliderNormalValue(m_normalValue, step, minValue, maxValue);
		 m_value = calculateSliderValue(m_normalValue, invertedVal, minValue, maxValue);
	  }
	  m_old_placeOfClick = m_normalValue * (1 - sliderRatio);

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }
   if (e.getEventType() == Event::EventType::MouseMove && m_is_pressed)
   {
	  auto& m = static_cast<MouseMoveEvent&>(e);
	  auto localY = m.getPos().y - m_draggedCursor.y;
	  float placeOfClick = localY / slideHeight;

	  m_normalValue = glm::clamp(m_old_placeOfClick + placeOfClick, 0.f, 1 - sliderRatio) / (1 - sliderRatio);

	  m_normalValue = quantizeSliderNormalValue(m_normalValue, step, minValue, maxValue);
	  m_value = calculateSliderValue(m_normalValue, invertedVal, minValue, maxValue);

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }

   if (e.getEventType() == Event::EventType::MouseScroll && (m_has_focus || m_scroll_focus))
   {
	  auto& m = static_cast<MouseScrollEvent&>(e);

	  float delta = m.getScrollY();
	  if (step != 0)
		 setValue(m_value + delta * step);
	  else
		 setValue(m_value + delta * ((maxValue - minValue) / 10));

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }
}


void GUIVSlider::setValue(float v)
{
   v = glm::clamp(v, minValue, maxValue);
   v -= minValue;
   if (step)
	  v = glm::round(v / step) * step;
   m_normalValue = v / (maxValue - minValue);
   if (invertedVal)
	  m_normalValue = 1 - m_normalValue;
   m_value = v + minValue;
}

void GUIVSlider::setQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = possibleStates - 1;
   step = 1;
   //setValue(m_value);
}

void GUIVSlider::setNormalQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = 1;
   step = 1.f / (possibleStates - 1);
}

GUIHSlider::GUIHSlider()
   : GUIElement(GETYPE::HSlider)
{
   setPadding(1);
}

void GUIHSlider::onMyEvent(Event& e)
{
   GUIElement::onMyEvent(e);

   float slideWidth = width - widthPadding();
   sliderWidth = slideWidth * this->sliderRatio;

   float oldNormal = m_normalValue;
   if (e.getEventType() == Event::EventType::MousePress)
   {
	  auto& m = static_cast<MousePressEvent&>(e);
	  m_draggedCursor = m.getPos();
	  auto localX = m.getPos().x - GUIContext::get().getStackPos().x - x - padding[GUI_LEFT];
	  bool isUnderSlider = localX < (m_normalValue* (1 - sliderRatio)* slideWidth);
	  bool isAboveSlider = localX > (m_normalValue * (1 - sliderRatio) * slideWidth + sliderWidth);
	  float placeOfClick = localX / slideWidth;
	  if (isUnderSlider || isAboveSlider)
	  {
		 m_normalValue = glm::clamp(placeOfClick - sliderRatio / 2, 0.f, 1.f - sliderRatio);
		 m_draggedCursor = {
			 GUIContext::get().getStackPos().x + x + padding[GUI_LEFT] + (m_normalValue + sliderRatio / 2) *
			 slideWidth,
			 0
		 };

		 m_normalValue /= (1 - sliderRatio);
		 m_normalValue = quantizeSliderNormalValue(m_normalValue, step, minValue, maxValue);
		 m_value = calculateSliderValue(m_normalValue, invertedVal, minValue, maxValue);
	  }
	  m_old_placeOfClick = m_normalValue * (1 - sliderRatio);

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }
   if (e.getEventType() == Event::EventType::MouseMove && m_is_pressed)
   {
	  auto& m = static_cast<MouseMoveEvent&>(e);
	  auto localX = m.getPos().x - m_draggedCursor.x;
	  float placeOfClick = localX / slideWidth;

	  m_normalValue = glm::clamp(m_old_placeOfClick + placeOfClick, 0.f, 1 - sliderRatio) / (1 - sliderRatio);

	  m_normalValue = quantizeSliderNormalValue(m_normalValue, step, minValue, maxValue);
	  m_value = calculateSliderValue(m_normalValue, invertedVal, minValue, maxValue);

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }

   if (e.getEventType() == Event::EventType::MouseScroll && (m_has_focus || m_scroll_focus))
   {
	  auto& m = static_cast<MouseScrollEvent&>(e);

	  float delta = m.getScrollY();
	  if (step != 0)
		 setValue(m_value + delta * step);
	  else
		 setValue(m_value + delta * ((maxValue - minValue) / 10));

	  if (oldNormal != m_normalValue && on_changed)
		 on_changed(*this);
   }
}

void GUIHSlider::setValue(float v)
{
   v = glm::clamp(v, minValue, maxValue);
   v -= minValue;
   if (step)
	  v = glm::round(v / step) * step;
   m_normalValue = v / (maxValue - minValue);
   if (invertedVal)
	  m_normalValue = 1 - m_normalValue;
   m_value = v + minValue;
}

void GUIHSlider::setQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = possibleStates - 1;
   step = 1;
}

void GUIHSlider::setNormalQuantization(int possibleStates)
{
   minValue = 0;
   maxValue = 1;
   step = 1.f / (possibleStates - 1);
}

GUIBlank::GUIBlank() : GUIElement(GETYPE::Blank)
{
   isVisible = false;
   isNotSpatial = true;
   isAlwaysPacked = false;
}

GUIHorizontalSplit::GUIHorizontalSplit(GUIElement* eUp, GUIElement* eDown, bool isUpMain) : GUIHorizontalSplit()
{
   setPrimaryUp(isUpMain);
   getUp()->appendChild(eUp);
   getDown()->appendChild(eDown);
}

GUIHorizontalSplit::GUIHorizontalSplit() : GUIElement(GETYPE::SplitHorizontal)
{
   getChildren().reserve(2);
   getChildren().push_back(new GUIBlank());
   getChildren().push_back(new GUIBlank());
   getUp()->setParent(this);
   getDown()->setParent(this);

   getUp()->dimInherit = GUIDimensionInherit::WIDTH;
   getDown()->dimInherit = GUIDimensionInherit::WIDTH;

   dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
   isVisible = true;
   isNotSpatial = true;
   space = 5;
   color = { 0,1,0,1 };
   setAlignment(GUIAlign::CENTER);
}

void GUIHorizontalSplit::setPrimaryUp(bool isUpMain)
{
   if (isUpMain) {
	  getDown()->isAlwaysPacked = false;
	  getUp()->isAlwaysPacked = true;
   }
   else {
	  getUp()->isAlwaysPacked = false;
	  getDown()->isAlwaysPacked = true;
   }
   m_is_up_main = isUpMain;

}

void GUIHorizontalSplit::repositionChildren()
{
   auto upC = getUp();
   auto downC = getDown();

   downC->pos = { 0, 0 };

   if (m_is_up_main)
   {
	  upC->pos = { 0, height - upC->height };

	  glm::vec2 newDim = { width, height - upC->height - space };
	  if ((newDim - downC->dim) != glm::vec2(0, 0))
	  {
		 downC->dim = newDim;
		 downC->onDimensionChange();
	  }
   }
   else
   {
	  upC->pos = { 0, downC->height + space };
	  glm::vec2 newDim = { width, height - downC->height - space };
	  if ((newDim - upC->dim) != glm::vec2(0, 0))
	  {
		 upC->dim = newDim;
		 upC->onDimensionChange();
	  }
   }
}

void GUIHorizontalSplit::onDimensionChange()
{
   auto upC = getUp();
   auto downC = getDown();
   if (m_is_up_main)
   {
	  upC->onDimensionChange();
	  upC->pos = { 0, height - upC->height };

	  downC->dim = { width, height - upC->height - space };
	  downC->pos = { 0, 0 };
	  downC->onDimensionChange();
   }
   else
   {
	  downC->onDimensionChange();
	  downC->pos = { 0, 0 };

	  upC->dim = { width, height - downC->height - space };
	  upC->pos = { 0, downC->height + space };
	  upC->onDimensionChange();
   }
}

void GUIHorizontalSplit::appendChild(GUIElement* element)
{
   ASSERT(false, "Invalid operation");
}

void GUIHorizontalSplit::destroyChild(int index)
{
   ASSERT(false, "Invalid operation");
}


GUIVerticalSplit::GUIVerticalSplit(bool isLeftMain) : GUIVerticalSplit()
{
   setPrimaryLeft(isLeftMain);
}

GUIVerticalSplit::GUIVerticalSplit() : GUIElement(GETYPE::SplitVertical)
{
   getChildren().reserve(2);
   getChildren().push_back(new GUIBlank());
   getChildren().push_back(new GUIBlank());
   getRight()->setParent(this);
   getLeft()->setParent(this);

   getRight()->dimInherit = GUIDimensionInherit::HEIGHT;
   getLeft()->dimInherit = GUIDimensionInherit::HEIGHT;

   dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
   isVisible = false;
   isNotSpatial = true;
   space = 5;
   setAlignment(GUIAlign::CENTER);
}

void GUIVerticalSplit::setPrimaryLeft(bool left)
{
   if (left) {
	  getLeft()->isAlwaysPacked = true;
	  getRight()->isAlwaysPacked = false;
   }
   else {
	  getRight()->isAlwaysPacked = true;
	  getLeft()->isAlwaysPacked = false;
   }
   m_is_left_main = left;

}

void GUIVerticalSplit::repositionChildren()
{
   auto rightC = getRight();
   auto leftC = getLeft();

   leftC->pos = { 0, 0 };

   if (m_is_left_main)
   {
	  rightC->pos = { leftC->width + space, 0 };
	  glm::vec2 newDim = { width - leftC->width - space, height };
	  if ((newDim - rightC->dim) != glm::vec2(0, 0))
	  {
		 rightC->dim = newDim;
		 rightC->onDimensionChange();
	  }
   }
   else
   {
	  rightC->pos = { width - rightC->width, 0 };

	  glm::vec2 newDim = { width - rightC->width - space, height };
	  if ((newDim - leftC->dim) != glm::vec2(0, 0))
	  {
		 leftC->dim = newDim;
		 leftC->onDimensionChange();
	  }
   }
}

void GUIVerticalSplit::onDimensionChange()
{
   auto rightC = getRight();
   auto leftC = getLeft();

   // apply padding for two children
   // ignore padding between those two (used space instead)
   //rightC->paddingVec = paddingVec;
  // leftC->paddingVec = paddingVec;

   leftC->pos = { padding[GUI_LEFT],padding[GUI_BOTTOM] };
   if (!m_is_left_main)
   {
	  rightC->onDimensionChange();
	  rightC->pos = { width - rightC->width-padding[GUI_RIGHT], padding[GUI_BOTTOM] };

	  leftC->dim = { width - rightC->width - space- widthPadding(), height-heightPadding() };
	  leftC->onDimensionChange();
   }
   else
   {
	  leftC->onDimensionChange();

	  rightC->dim = { width - leftC->width - space-widthPadding(), height-heightPadding() };
	  rightC->pos = { leftC->width + space+ padding[GUI_LEFT], padding[GUI_BOTTOM] };
	  rightC->onDimensionChange();
   }
}

void GUIVerticalSplit::appendChild(GUIElement* element)
{
   ASSERT(false, "Invalid operation");
}

void GUIVerticalSplit::destroyChild(int index)
{
   ASSERT(false, "Invalid operation");
}

GUIView::GUIView() : GUIElement(GETYPE::View)
{
   getChildren().push_back(new GUIBlank());
   getInside()->setParent(this);
   getInside()->isAlwaysPacked = false;
   getInside()->color = { 0.2f, 0.2f, 0.2f, 1 };
   getInside()->dim = { 20, 20 };
   setPadding(10);
   getInside()->pos = { padding[GUI_LEFT], padding[GUI_BOTTOM] };
}

void GUIView::appendChild(GUIElement* element)
{
   ASSERT(false, "Invalid operation");
}

void GUIView::destroyChild(int index)
{
   ASSERT(false, "Invalid operation");
}

GUITextButton::GUITextButton()
{

}

void GUITextButton::setMaterial(FontMaterial* material)
{
   clearChildren();
   auto t = new GUIText(material);
   t->setAlignment(GUIAlign::CENTER);
   GUIButton::appendChild(t);
}

GUIVerticalSplit* createGUISliderView(bool sliderOnLeft)
{
   auto split = new GUIVerticalSplit(sliderOnLeft);

   auto slider = new GUIVSlider();
   auto view = new GUIView();
   auto inside = view->getInside();
   inside->isAlwaysPacked = true;
   inside->setAlignment(GUIAlign::INVALID);
   inside->dimInherit = GUIDimensionInherit::WIDTH;
   inside->setPadding(10);
   inside->x = view->padding[GUI_LEFT];

   (sliderOnLeft ? split->getLeft() : split->getRight())->appendChild(slider);
   (!sliderOnLeft ? split->getLeft() : split->getRight())->appendChild(view);

   inside->onDimChange = [slider, view](GUIElement& e)
   {
	  slider->sliderRatio = std::min(1.f, (view->height - view->heightPadding()) / (view->getInside()->height));
	  view->getInside()->x = view->padding[GUI_LEFT];
	  view->getInside()->y = view->height - view->padding[GUI_TOP] - view->getInside()->height +

		 (1 - slider->getValue()) * (view->getInside()->height - (view->height - view->heightPadding()));
   };
   view->dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
   inside->color = { 0.2, 0.2, 0.2, 1 };
   slider->dimInherit = GUIDimensionInherit::HEIGHT;
   slider->width = 20;
   slider->on_changed = [view, slider](GUIElement& e)
   {
	  view->getInside()->x = view->padding[GUI_LEFT];
	  view->getInside()->y = view->height - view->padding[GUI_TOP] - view->getInside()->height +

		 (1 - slider->getValue()) * (view->getInside()->height - (view->height - view->heightPadding()));
   };
   slider->setValue(1);

   //ensure that scrolling in view will also trigger slider
   view->onMyEventFunc = [slider](Event& e, GUIElement& v)
   {
	  if (e.getEventType() == Event::EventType::MouseFocusGain)
	  {
		 slider->setHasScrollFocus(true);
	  }
	  else if (e.getEventType() == Event::EventType::MouseFocusLost)
		 slider->setHasScrollFocus(false);
   };

   return split;
}


static float smootherstep(float x)
{
   return x * x * x * (x * (x * 6 - 15) + 10);
}

GUISpecialTextButton::GUISpecialTextButton(const std::string& text, FontMaterial* material) : GUISpecialTextButton()
{
   setMaterial(material);
   getTextElement()->setText(text);
}

GUISpecialTextButton::GUISpecialTextButton() : GUITextButton()
{
   isVisible = false;
   soundClick = "res/audio/click.ogg";
   soundFocus = "res/audio/click.ogg";
   soundVolume = 0.4f;
}

void GUISpecialTextButton::update()
{
   if (m_has_focus)
   {
	  currentScale += animationSpeed;
	  currentScale = std::min(currentScale, 1.f);
   }
   else
   {
	  currentScale -= animationSpeed;
	  currentScale = std::max(currentScale, 0.f);
   }
   getTextElement()->textScale = smootherstep(currentScale) * (maxScale - minScale) + minScale;
}
