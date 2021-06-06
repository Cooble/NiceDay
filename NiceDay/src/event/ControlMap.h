#pragma once
#include "ndpch.h"

namespace nd {
class NBT;

struct ControlButton
{
	std::string id;
	std::string family;
	uint64_t* pointer;
};

class ControlMap
{
private:
	static std::unordered_map<std::string, ControlButton> s_buttons;
	static std::unordered_map<uint64_t, std::string> s_button_names;
public:
	static void registerControl(const std::string& id, uint64_t* pointer, const std::string& family);
	static void registerControl(const std::string& id, uint64_t* pointer);
	static const ControlButton* getButtonData(const std::string& id);

	static void setValueAtPointer(const std::string& id, uint64_t val)
	{
		*s_buttons[id].pointer = val;
	}

	static auto& getControlsList()
	{
		return s_buttons;
	}

	static void serialize(NBT& t);
	static void deserialize(NBT& t);

	static void init();
	static const std::string& getKeyName(uint64_t key);
};
}
