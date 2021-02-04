#pragma once
#include "ndpch.h"
class NBT;
class GUIWindow;
class GUIElement;
class GUIParser
{
public:
	// register global style which will be shared across all parsed elements
    static void setGlobalStyle(const std::string& name, const NBT& style);
	// creates whole gui structure from xml string
	static GUIElement* parse(const std::string& text);
	// creates underlining structure of passed window from xml string
	// first gui element of xml string must be Window!!
	static GUIElement* parseWindow(const std::string& text,GUIWindow& window);
};




