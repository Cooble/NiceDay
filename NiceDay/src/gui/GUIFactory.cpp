#include "GUIFactory.h"
#include "GUIBasic.h"
#include "Translator.h"

#include "graphics/FontMaterial.h"

namespace nd {

static glm::vec4 getVec4(const std::string& c)
{
	glm::vec4 out;
	int size;
	SUtil::parseNumbers<4>(c, glm::value_ptr(out), &size);
	if (size == 1)
		return glm::vec4(out.x, out.x, out.x, out.x);
	return out;
}

static glm::vec4 getColor(const std::string& c)
{
	glm::vec4 dims(0, 0, 0, 1);
	SUtil::parseNumbers<4>(c, glm::value_ptr(dims));
	return dims;
}

static GUIAlign getAlignment(const char* c)
{
	if (strcmp("LEFT_DOWN", c) == 0) return GUIAlign::LEFT_DOWN;
	if (strcmp("LEFT", c) == 0) return GUIAlign::LEFT;
	if (strcmp("LEFT_UP", c) == 0) return GUIAlign::LEFT_UP;
	if (strcmp("CENTER_DOWN", c) == 0) return GUIAlign::CENTER_DOWN;
	if (strcmp("CENTER", c) == 0) return GUIAlign::CENTER;
	if (strcmp("CENTER_UP", c) == 0) return GUIAlign::CENTER_UP;
	if (strcmp("RIGHT_DOWN", c) == 0) return GUIAlign::RIGHT_DOWN;
	if (strcmp("RIGHT", c) == 0) return GUIAlign::RIGHT;
	if (strcmp("RIGHT_UP", c) == 0) return GUIAlign::RIGHT_UP;
	if (strcmp("UP", c) == 0) return GUIAlign::UP;
	if (strcmp("DOWN", c) == 0) return GUIAlign::DOWN;
	return GUIAlign::INVALID;
}

static GUIDimensionInherit getDimInherit(const char* c)
{
	if (strcmp("WIDTH", c) == 0) return GUIDimensionInherit::WIDTH;
	if (strcmp("HEIGHT", c) == 0) return GUIDimensionInherit::HEIGHT;
	if (strcmp("WIDTH_HEIGHT", c) == 0) return GUIDimensionInherit::WIDTH_HEIGHT;
	return GUIDimensionInherit::INVALID;
}

static GUIElement* buildElement(const std::string& c, const NBT& nbt)
{
#define ND_FAC_BUIL(nameo) case SID(#nameo): out= new GUI ## nameo (); break;
	GUIElement* out = nullptr;
	switch (SID(c))
	{
	ND_FAC_BUIL(Blank);
	ND_FAC_BUIL(Text);
	ND_FAC_BUIL(Button);
	ND_FAC_BUIL(Image);
	ND_FAC_BUIL(Window);
	ND_FAC_BUIL(TextBox);
	ND_FAC_BUIL(CheckBox);

	ND_FAC_BUIL(Column);
	ND_FAC_BUIL(Row);
	ND_FAC_BUIL(Grid);

	ND_FAC_BUIL(HorizontalSplit);
	ND_FAC_BUIL(VerticalSplit);
	ND_FAC_BUIL(Slider);
	ND_FAC_BUIL(VSlider);
	ND_FAC_BUIL(View);

	ND_FAC_BUIL(TextButton);
	ND_FAC_BUIL(ImageButton);
	ND_FAC_BUIL(SpecialTextButton);

	case SID("SliderView"):
		out = createGUISliderView(nbt["slider"].isString() ? nbt["slider"] == "LEFT" : true);
		break;
	}
	return out;
}

#define ND_GUI_F(name) if(!s_map[#name].isNull()) {element->name=s_map[#name];}
#define ND_GUI_FBOOL(name) if(!s_map[#name].isNull()) {element->name=(bool)s_map[#name];}

NBT GUIFactory::s_map;

GUIElement* GUIFactory::end()
{
	auto element = buildElement(s_map["type"].string(), s_map);
	if (!element)
	{
		ND_WARN("invalid gui xml struct: {}", s_map["type"].string());
		return nullptr;
	}
	return end(*element);
}

GUIElement* GUIFactory::end(GUIElement& source)
{
	GUIElement* element = &source;
	// auto type = getType(s_map["type"].c_str());
	auto textMaterial = FontMatLib::getMaterial(s_map["textMaterial"].string());

	{
		//todo remove checking if source matches xml
		auto xml = buildElement(s_map["type"].string(), s_map);
		if (!xml)
		{
			ND_WARN("invalid gui xml struct: {}", s_map["type"].string());
			return nullptr;
		}
		if (xml->type != element->type)
		{
			delete xml;
			ND_WARN("Cannot parse xml, expected element: {}, but found: {}", (int)element->type, s_map["type"].string());
			return nullptr;
		}
		delete xml;
	}
	{
		//extract width height from dim
		auto& nbt = s_map["dim"];
		if (nbt.isString())
		{
			int dims[2];
			SUtil::parseNumbers<2>(nbt.string(), dims);
			s_map["width"] = dims[0];
			s_map["height"] = dims[1];
		}
	}
	{
		//convert string color to vec4
		auto& nbt = s_map["color"];
		if (nbt.isString())
			nbt = getColor(nbt.string());
	}

	if (s_map["dimInherit"].isString())
	{
		element->dimInherit = getDimInherit(s_map["dimInherit"].c_str());
	}
	if (s_map["alignment"].isString())
	{
		element->alignment = getAlignment(s_map["alignment"].c_str());
	}
	ND_GUI_FBOOL(isNotSpatial);
	ND_GUI_FBOOL(isAlwaysPacked);
	ND_GUI_FBOOL(isEnabled);
	ND_GUI_FBOOL(isVisible);
	ND_GUI_F(color);
	ND_GUI_F(space);
	ND_GUI_F(width);
	ND_GUI_F(height);
	if (!s_map["id"].isNull()) { element->id = s_map["id"].string(); }
	if (!s_map["class"].isNull()) { element->clas = s_map["class"].string(); }
	if (!s_map["margin"].isNull()) element->marginVec = getVec4(s_map["margin"].string());
	if (!s_map["padding"].isNull()) element->paddingVec = getVec4(s_map["padding"].string());

	// if element has not value and has id then id becomes translated value as well (and is not textBox)
	auto& valu = s_map["value"];
	if (valu.isString() && valu.string().empty() && !element->id.empty() && !dynamic_cast<GUITextBox*>(element))
		valu = Translator::translateTryWithKey(element->id);

	auto text = dynamic_cast<GUIText*>(element);
	if (text)
	{
		text->fontMaterial = textMaterial;
		if (s_map["value"].isString())
			text->setText(s_map["value"].string());
	}
	auto text0 = dynamic_cast<GUITextBox*>(element);
	if (text0)
	{
		text0->fontMaterial = textMaterial;
		if (s_map["value"].isString())
			text0->setValue(s_map["value"].string());
	}
	auto text1 = dynamic_cast<GUITextButton*>(element);
	if (text1)
	{
		text1->setMaterial(textMaterial);
		if (s_map["value"].isString())
			text1->getTextElement()->setText(s_map["value"].string());
	}
	auto special = dynamic_cast<GUISpecialTextButton*>(element);
	if (special)
	{
		if (s_map["maxScale"].isString())
			special->maxScale = s_map["maxScale"];
		if (s_map["minScale"].isString())
			special->minScale = s_map["minScale"];
	}
	auto row = dynamic_cast<GUIRow*>(element);
	if (row)
	{
		if (s_map["childAlignment"].isString())
			row->child_alignment = getAlignment(s_map["childAlignment"].c_str());
		if (s_map["space"].isString())
			row->space = s_map["space"];
	}
	auto column = dynamic_cast<GUIColumn*>(element);
	if (column)
	{
		if (s_map["childAlignment"].isString())
			column->child_alignment = getAlignment(s_map["childAlignment"].c_str());
		if (s_map["space"].isString())
			column->space = s_map["space"];
	}
	auto vert = dynamic_cast<GUIVerticalSplit*>(element);
	if (vert)
	{
		if (s_map["primary"].isString())
			vert->setPrimaryLeft(s_map["primary"].string() == "LEFT");
	}
	auto hor = dynamic_cast<GUIHorizontalSplit*>(element);
	if (hor)
	{
		if (s_map["primary"].isString())
			hor->setPrimaryUp(s_map["primary"].string() == "UP");
	}
	auto window = dynamic_cast<GUIWindow*>(element);
	if (window)
	{
		if (!s_map["isMoveable"].isNull())
			window->isMoveable = s_map["isMoveable"];
		if (!s_map["isResizable"].isNull())
			window->isResizable = s_map["isResizable"];
	}
	auto view = dynamic_cast<GUIView*>(element);

	if (view && s_map["insideColor"].isString())
		view->getInside()->color = getColor(s_map["insideColor"].string());


	return element;
}

void GUIFactory::begin()
{
	s_map.maps().clear();
}

void GUIFactory::setAttrib(const std::string& name, NBT& val)
{
	s_map[name] = std::move(val);
}

void GUIFactory::setStyle(const NBT& style)
{
	for (auto& pair : style.maps())
		s_map[pair.first] = pair.second;
}
}
