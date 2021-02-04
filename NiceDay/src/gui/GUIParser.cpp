#include "GUIParser.h"

#include "GUIElement.h"
#include "GUIBasic.h"
#include "GUIFactory.h"
#include "rapidxml.hpp"
#include "core/NBT.h"


static std::string aliases(const std::string& name)
{
   if (name == "align")
	  return "alignment";
   if (name == "pad")
	  return "padding";
   if (name == "packed")
	  return "isAlwaysPacked";
   if (name == "inherit")
	  return "dimInherit";
   return name;
}
using namespace rapidxml;
static std::unordered_map<std::string, NBT> styles;
static std::unordered_map<std::string, NBT> global_styles;


static void parseStyle(xml_node<char>* node)
{
  
   auto nameAtrib = node->first_attribute("name");
   if (!nameAtrib) {
	  ND_ERROR("Invalid gui xml style missing name");
	  return;
   }
   std::string name = nameAtrib->value();
   NBT out;

   //look for parents of style
   for (xml_attribute<>* attr = node->first_attribute("parent"); attr; attr = attr->next_attribute("parent"))
   {
	  auto it = styles.find(std::string(attr->value()));
	  if (it == styles.end())
		 ND_WARN("Unknown parent style gui xml: {}", attr->value());
	  else
		 for (auto& pair : it->second.maps())
			out[aliases(pair.first)] = pair.second;
   }
	//load other attribs
   for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
   {
	  if (strcmp(attr->name(), "name") == 0)
		 name = attr->value();
	  else
		 out[aliases(std::string(attr->name()))] = attr->value();
   }
    styles[name] = out;

}
static GUIElement* parseNode(xml_node<char>* node, GUIElement* source = nullptr)
{
   if (strcmp((const char*)node->name(), "style") == 0)
   {
	  parseStyle(node);
	  return nullptr;
   }
   GUIFactory::begin();

   auto style = node->first_attribute("style");
   if (style) {
	  auto it = styles.find(style->value());
	  if (it == styles.end())
	  {
		 ND_WARN("Unknown style name {}", style->value());
		 return nullptr;
	  }
	  GUIFactory::setStyle(it->second);
   }
   //attribs
   for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	  GUIFactory::setAttrib(aliases(std::string(attr->name())), NBT(attr->value()));
   GUIFactory::setAttrib("type", NBT(node->name()));
   GUIFactory::setAttrib("value", NBT(node->value()));
   //create element from set attribs

   GUIElement* element = source ? GUIFactory::end(*source) : GUIFactory::end();
   if (!element)
	  return nullptr;

   // splits have special children
   auto vert = dynamic_cast<GUIVerticalSplit*>(element);
   if (vert) {
	  if (node->first_node()) {
		 if (SUtil::equals(node->name(), "SliderView"))
		 {
			auto left = node->first_node()->name();

			if (SUtil::equals(left, "View"))
			   parseNode(node->first_node(), vert->isPrimaryLeft() ? vert->getRight()->getFirstChild() : vert->getLeft()->getFirstChild());
			else if (SUtil::equals(left, "VSlider"))
			   parseNode(node->first_node(), vert->isPrimaryLeft() ? vert->getLeft()->getFirstChild() : vert->getRight()->getFirstChild());

			if (node->first_node()->next_sibling())
			{
			   auto right = node->first_node()->next_sibling()->name();
			   if (SUtil::equals(right, "View"))
				  parseNode(node->first_node()->next_sibling(), vert->isPrimaryLeft() ? vert->getRight()->getFirstChild() : vert->getLeft()->getFirstChild());
			   else if (SUtil::equals(right, "VSlider"))
				  parseNode(node->first_node()->next_sibling(), vert->isPrimaryLeft() ? vert->getLeft()->getFirstChild() : vert->getRight()->getFirstChild());
			}
		 }
		 else {
			auto e = parseNode(node->first_node());
			if (e) vert->getLeft()->appendChild(e);
			if (node->first_node()->next_sibling()) {
			   auto e = parseNode(node->first_node()->next_sibling());
			   vert->getRight()->appendChild(e);
			}
		 }
	  }
	  return element;
   }
   auto hor = dynamic_cast<GUIHorizontalSplit*>(element);
   if (hor) {
	  if (node->first_node()) {
		 auto e = parseNode(node->first_node());
		 if (e)	hor->getUp()->appendChild(e);
		 if (node->first_node()->next_sibling()) {
			auto e = parseNode(node->first_node()->next_sibling());
			if (e) hor->getDown()->appendChild(e);
		 }
	  }
	  return element;
   }
   auto view = dynamic_cast<GUIView*>(element);
   if (view) {
	  if (!view->id.empty())
		 view->getInside()->id = view->id + ".in";
	  for (auto child = node->first_node(); child; child = child->next_sibling())
	  {
		 auto ch = parseNode(child);
		 if (ch)
			view->getInside()->appendChild(ch);
	  }
	  return element;
   }

   if (element)
	  for (auto child = node->first_node(); child; child = child->next_sibling())
	  {
		 auto ch = parseNode(child);
		 if (ch)
			element->appendChild(ch);
	  }


   return element;
}

void GUIParser::setGlobalStyle(const std::string& name, const NBT& style)
{
   global_styles[name] = style;
   for (auto& pair : global_styles)
	  styles[pair.first] = pair.second;
}

GUIElement* GUIParser::parse(const std::string& text)
{
   styles.clear();//copy globals styles to styles
   for (auto& pair : global_styles)
	  styles[pair.first] = pair.second;

   try {
	  xml_document<> doc;    // character type defaults to char
	  doc.parse<0>((char*)text.c_str());    // 0 means default parse flags
	  for (auto child = doc.first_node(); child; child = child->next_sibling())
	  {
		 auto ch = parseNode(child);
		 if (ch)
			return ch;
	  }
   }
   catch (...) {
	  ND_ERROR("Invalid xml gui");
   }
   return nullptr;
}

GUIElement* GUIParser::parseWindow(const std::string& text, GUIWindow& window)
{
   styles.clear();//copy globals styles to styles
   for (auto& pair : global_styles)
	  styles[pair.first] = pair.second;

   try {
	  xml_document<> doc;    // character type defaults to char
	  doc.parse<0>((char*)text.c_str());    // 0 means default parse flags
	  for (auto child = doc.first_node(); child; child = child->next_sibling())
	  {
		 auto ch = parseNode(child, &window);
		 if (ch)
			return ch;
	  }
   }
   catch (...) {
	  ND_ERROR("Invalid xml gui");
   }
   return nullptr;
}
