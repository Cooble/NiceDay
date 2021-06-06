#pragma once
#include "GUIElement.h"
#include "core/NBT.h"

namespace nd {

class GUIFactory
{
private:
	static NBT s_map;
public:
	//creates gui element
	static GUIElement* end();
	// sets attribs of passed source element
	static GUIElement* end(GUIElement& source);
	static void begin();
	static void setAttrib(const std::string& name, NBT& val);

	static void setStyle(const NBT& style);
};
}
