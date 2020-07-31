#include "ndpch.h"
#include "nd_luabinder.h"
#include <sol/sol.hpp>
#include "audio/audio_handle.h"
#include "lua.h"
#include "audio/player.h"
#include "core/NBT.h"
#include "gui/GUIElement.h"
#include "gui/GUIBasic.h"
#include "gui/GUIContext.h"
#include "lua.hpp"

//#define BIND_LUA_GUI_HANDLES

static void bindBasic(sol::state& state);

static void bindGLM(sol::state& state);

static void bindSound(sol::state& state);
static int playSound(lua_State* L);
static int playMusic(lua_State* L);

static void bindNBT(sol::state& state);

#ifdef BIND_LUA_GUI_HANDLES
static void bindGUIElements(sol::state& state);
static void bindGUICast(sol::state& state);
static void bindGUIContext(sol::state& state);
#endif
static void bindFontMaterial(sol::state& state);


void nd_luabinder::bindEverything(sol::state& state)
{
	bindBasic(state);
	bindGLM(state);
	bindSound(state);
	bindNBT(state);

#ifdef BIND_LUA_GUI_HANDLES
	bindGUIElements(state);
	bindGUICast(state);
	bindGUIContext(state);
#endif
	bindFontMaterial(state);
}

static void bindBasic(sol::state& state)
{
}

static void bindGLM(sol::state& state)
{
	//glm::vec<2, float, Q> operator+(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	state.new_usertype<half_int>("half_int",
		sol::constructors<half_int(int), half_int(int, int)>(),
		"x", &half_int::x,
		"y", &half_int::y,
		"i", &half_int::i,
		sol::meta_function::addition, [](half_int& a, half_int& b) { return a + b; },
		sol::meta_function::subtraction, [](half_int& a, half_int& b) { return a - b; },
		sol::meta_function::multiplication, [](half_int& a, int b) { return a * b; },
		sol::meta_function::subtraction, [](half_int& a, int b) { return a / b; }
	);
	state.new_usertype<glm::vec2>("vec2",
		sol::constructors<glm::vec2(float, float)>(),
		"x", &glm::vec2::x,
		"y", &glm::vec2::y,
		sol::meta_function::addition, [](glm::vec2& a, glm::vec2& b) { return a + b; },
		sol::meta_function::subtraction, [](glm::vec2& a, glm::vec2& b) { return a - b; },
		sol::meta_function::multiplication,
		sol::overload([](glm::vec2& a, glm::vec2& b) { return a * b; },
			[](glm::vec2& a, float b) { return a * b; }, [](float a, glm::vec2& b) { return a * b; }),
		sol::meta_function::division,
		sol::overload([](glm::vec2& a, glm::vec2& b) { return a / b; },
			[](glm::vec2& a, float b) { return a / b; })
		);
	state.set_function("Vec2", sol::overload([](float x, float y) {return glm::vec2(x, y); }, [](float v) {return glm::vec2(v); }));
	state.new_usertype<glm::vec3>("vec3",
		sol::constructors<glm::vec3(float, float, float)>(),
		"x", &glm::vec3::x,
		"y", &glm::vec3::y,
		"z", &glm::vec3::z,
		sol::meta_function::addition, [](glm::vec3& a, glm::vec3& b) { return a + b; },
		sol::meta_function::subtraction, [](glm::vec3& a, glm::vec3& b) { return a - b; },
		sol::meta_function::multiplication,
		sol::overload([](glm::vec3& a, glm::vec3& b) { return a * b; },
			[](glm::vec3& a, float b) { return a * b; }),
		sol::meta_function::division,
		sol::overload([](glm::vec3& a, glm::vec3& b) { return a / b; },
			[](glm::vec3& a, float b) { return a / b; })
		);
	state.set_function("Vec3", sol::overload([](float x, float y,float z) {return glm::vec3(x, y,z); }, [](float v) {return glm::vec3(v); }));
	state.new_usertype<glm::vec4>("vec4",
		sol::constructors<glm::vec4(float, float, float, float)>(),
		"x", &glm::vec4::x,
		"y", &glm::vec4::y,
		"z", &glm::vec4::z,
		"w", &glm::vec4::w,
		sol::meta_function::addition, [](glm::vec4& a, glm::vec4& b) { return a + b; },
		sol::meta_function::subtraction, [](glm::vec4& a, glm::vec4& b) { return a - b; },
		sol::meta_function::multiplication,
		sol::overload([](glm::vec4& a, glm::vec4& b) { return a * b; },
			[](glm::vec4& a, float b) { return a * b; }),
		sol::meta_function::division,
		sol::overload([](glm::vec4& a, glm::vec4& b) { return a / b; },
			[](glm::vec4& a, float b) { return a / b; })
		);
	state.set_function("Vec4", sol::overload([](float x, float y, float z,float w) {return glm::vec4(x, y, z, w); }, [](float v) {return glm::vec4(v); }));
	state.set_function("normalize", sol::overload(
		[](glm::vec2 v) {return glm::normalize(v); },
		[](glm::vec3 v) {return glm::normalize(v); },
		[](glm::vec4 v) {return glm::normalize(v); }));

}

static void bindSound(sol::state& state)
{
	state.new_usertype<SoundHandle>("Sound",
		"play", sol::overload(&SoundHandle::play, [](SoundHandle& h) { h.play(); }),
		"stop", sol::overload(&SoundHandle::stop, [](SoundHandle& h) { h.stop(); }),
		"setPitch", sol::overload(&SoundHandle::setPitch, [](SoundHandle& h, float p)
			{
				h.setPitch(p);
			}),
		"setVolume",
				sol::overload(&SoundHandle::setVolume, [](SoundHandle& h, float p)
					{
						h.setVolume(p);
					}),
				"pause", &SoundHandle::pause,
						"open", &SoundHandle::open,
						"setLoop", &SoundHandle::setLoop,
						"isPlaying", &SoundHandle::isPlaying
						);
	state.new_usertype<MusicHandle>("Music",
		"play", sol::overload(&MusicHandle::play, [](MusicHandle& h) { h.play(); }),
		"stop", sol::overload(&MusicHandle::stop, [](MusicHandle& h) { h.stop(); }),
		"setPitch", sol::overload(&MusicHandle::setPitch, [](MusicHandle& h, float p)
			{
				h.setPitch(p);
			}),
		"setVolume",
				sol::overload(&MusicHandle::setVolume, [](MusicHandle& h, float p)
					{
						h.setVolume(p);
					}),
				"pause", &MusicHandle::pause,
						"open", &MusicHandle::open,
						"setLoop", &MusicHandle::setLoop,
						"isPlaying", &MusicHandle::isPlaying
						);

	lua_register(state.lua_state(), "playSound", playSound);
	lua_register(state.lua_state(), "playMusic", playMusic);
}

static int playSound(lua_State* L)
{
	auto numberOfArgs = lua_gettop(L);
	float volume = 1;
	float pitch = 1;
	if (numberOfArgs == 0)
	{
		ND_ERROR("Lua error invaliad number of args");
		return 0;
	}
	const char* c = lua_tostring(L, 1);
	if (numberOfArgs > 1)
		volume = lua_tonumber(L, 2);
	if (numberOfArgs > 2)
		pitch = lua_tonumber(L, 3);
	Sounder::get().playSound(c, volume, pitch);

	return 0;
}

static int playMusic(lua_State* L)
{
	auto numberOfArgs = lua_gettop(L);
	float volume = 1;
	float pitch = 1;
	if (numberOfArgs == 0)
	{
		ND_ERROR("Lua error invaliad number of args");
		return 0;
	}
	const char* c = lua_tostring(L, 1);
	if (numberOfArgs > 1)
		volume = lua_tonumber(L, 2);
	if (numberOfArgs > 2)
		pitch = lua_tonumber(L, 3);
	Sounder::get().playMusic(c, volume, pitch);

	return 0;
}


static int nbtRetrieve(lua_State* L);
static int nbtIndex(lua_State* L);
static int nbtNewIndex(lua_State* L);
static int nbtSetValue(lua_State* L);
static int nbtGetValue(lua_State* L);


static void bindNBT(sol::state& state)
{
	auto namespac = state["NBType"].get_or_create<sol::table>();

	namespac["T_NUMBER_FLOAT"] = NBT::NBTType::T_NUMBER_FLOAT;
	namespac["T_NUMBER_INT"] = NBT::NBTType::T_NUMBER_INT;
	namespac["T_NUMBER_UINT"] = NBT::NBTType::T_NUMBER_UINT;
	namespac["T_STRING"] = NBT::NBTType::T_STRING;
	namespac["T_BOOL"] = NBT::NBTType::T_BOOL;
	namespac["T_MAP"] = NBT::NBTType::T_MAP;
	namespac["T_ARRAY"] = NBT::NBTType::T_ARRAY;
	namespac["T_NULL"] = NBT::NBTType::T_NULL;

	state.new_usertype<NBT>("NBT",
		sol::constructors<NBT()>(),
		"isArray", &NBT::isArray,
		"isMap", &NBT::isMap,
		"isBool", &NBT::isBool,
		"isContainer", &NBT::isContainer,
		"isFloat", &NBT::isFloat,
		"isInt", &NBT::isInt,
		"isNull", &NBT::isNull,
		"isNumber", &NBT::isNumber,
		"isString", &NBT::isString,
		"isUInt", &NBT::isUInt,
		"toNumber", [](NBT& t) { return (double)t; },
		"toString", [](NBT& t) { return t.string(); },
		"toBool", [](NBT& t) { return (bool)t; },
		"toVec2", [](NBT& t) { return (glm::vec2)t; },
		"arrays", [](NBT& t) { return t.arrays(); },
		"maps", [](NBT& t) { return t.maps(); },
		sol::meta_method::new_index, &nbtNewIndex,
		sol::meta_method::index, &nbtIndex,
		"nbt", &nbtRetrieve,
		"getValue", &nbtGetValue,
		"setValue", &nbtSetValue
		);
}

//sets current nbt's value (use nil to erase it to T_NULL)
static int nbtSetValue(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		return luaL_error(L, "expecting exactly 2 arguments");
	}
	NBT& nbt = sol::stack::get_usertype<NBT>(L, 1);
	auto type = lua_type(L, 2);

	//array access
	if (type == LUA_TNUMBER)
	{
		nbt = NBT(lua_tonumber(L, 2));
	}
	else if (type == LUA_TBOOLEAN)
	{
		nbt = NBT(lua_toboolean(L, 2));
	}
	else if (type == LUA_TNIL)
	{
		nbt = NBT();
	}
	else if (type == LUA_TSTRING)
	{
		std::string s = lua_tostring(L, 2);
		nbt = s;
	}
	else if (type == LUA_TUSERDATA)
	{
		if (sol::stack::check_usertype<NBT>(L, 2))
		{
			nbt = sol::stack::get_usertype<NBT>(L, 2);
		}
		else
			return luaL_error(L, "LUA Invalid type passed to nbt");
	}
	return 0;
}

//get current nbt's value (=string/bool/number). if nil or container -> returns itself
static int nbtGetValue(lua_State* L)
{
	NBT& nbt = sol::stack::get_usertype<NBT>(L, 1);

	if (nbt.isString())
	{
		sol::stack::push(L, nbt.string());
		return 1;
	}
	if (nbt.isNumber())
	{
		sol::stack::push(L, (double)nbt);
		return 1;
	}
	if (nbt.isBool())
	{
		sol::stack::push(L, (bool)nbt);
		return 1;
	}
	sol::stack::push(L, sol::make_reference(L, &nbt));
	return 1;
}

//returns always nbt reference or nothing if nbt is not proper container at specified key
static int nbtRetrieve(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		return luaL_error(L, "expecting exactly 2 arguments");
	}
	NBT& nbt = sol::stack::get_usertype<NBT>(L, 1);
	auto type = lua_type(L, 2);

	//array access
	if (type == LUA_TNUMBER && lua_isinteger(L, 2))
	{
		auto index = lua_tointeger(L, 2);
		if (!nbt.isArray() && !nbt.isNull())
			return 0;
		auto& n = nbt[index];
		sol::stack::push(L, sol::make_reference(L, &n));
		return 1;
	}
	//map access
	if (type == LUA_TSTRING && lua_isstring(L, 2))
	{
		std::string key = lua_tostring(L, 2);
		if (!nbt.isMap() && !nbt.isNull())
			return 0;
		auto& n = nbt[key];
		sol::stack::push(L, sol::make_reference(L, &n));
		return 1;
	}
	return 0;
}

//returns value at specified key (bool,string,number) or nbt reference
static int nbtIndex(lua_State* L)
{
	NBT& nbt = sol::stack::get_usertype<NBT>(L, 1);
	auto type = lua_type(L, 2);

	//array access
	if (type == LUA_TNUMBER && lua_isinteger(L, 2))
	{
		auto index = lua_tointeger(L, 2);
		if (!nbt.isArray() && !nbt.isNull())
			return 0;
		auto& n = nbt[index];

		if (n.isString())
		{
			sol::stack::push(L, n.string());
			return 1;
		}
		if (n.isNumber())
		{
			sol::stack::push(L, (double)n);
			return 1;
		}
		if (n.isBool())
		{
			sol::stack::push(L, (bool)n);
			return 1;
		}
		sol::stack::push(L, sol::make_reference(L, &n));
		return 1;
	}
	//map access
	if (type == LUA_TSTRING && lua_isstring(L, 2))
	{
		std::string key = lua_tostring(L, 2);
		if (!nbt.isMap() && !nbt.isNull())
			return 0;
		auto& n = nbt[key];

		if (n.isString())
		{
			sol::stack::push(L, n.string());
			return 1;
		}
		if (n.isNumber())
		{
			sol::stack::push(L, (double)n);
			return 1;
		}
		if (n.isBool())
		{
			sol::stack::push(L, (bool)n);
			return 1;
		}
		sol::stack::push(L, sol::make_reference(L, &n));
		return 1;
	}
	return 0;
}

//sets value at specified key
static int nbtNewIndex(lua_State* L)
{
	constexpr auto INDEX = 3;

	NBT& nbt = sol::stack::get_usertype<NBT>(L, 1);

	auto type = lua_type(L, 2);
	//array access
	if (type == LUA_TNUMBER && lua_isinteger(L, 2))
	{
		auto index = lua_tointeger(L, 2);

		if (!nbt.isArray() && !nbt.isNull())
		{
			ND_WARN("LUA access nbt which is not array");
			return 0;
		}
		switch (lua_type(L, INDEX))
		{
		case LUA_TNUMBER:
			nbt[index] = lua_tonumber(L, INDEX);
			break;
		case LUA_TSTRING:
			nbt[index] = std::string(lua_tostring(L, INDEX));
			break;
		case LUA_TBOOLEAN:
			nbt[index] = lua_toboolean(L, INDEX);
			break;
		case LUA_TNIL:
		{
			if (index < nbt.size())
				nbt[index] = NBT();
		}
		break;
		case LUA_TUSERDATA:
		{
			if (sol::stack::check_usertype<NBT>(L, INDEX))
			{
				nbt[index] = sol::stack::get_usertype<NBT>(L, INDEX);
			}
			else
			{
				return luaL_error(L, "LUA Invalid type passed to nbt");
			}
		}
		break;
		default:
			ND_WARN("LUA Invalid type passed to nbt");
			break;
		}
	}
	//map access
	else if (type == LUA_TSTRING && lua_isstring(L, 2))
	{
		std::string key = lua_tostring(L, 2);
		if (!nbt.isMap() && !nbt.isNull())
		{
			ND_WARN("LUA access nbt which is not map");
			return 0;
		}
		switch (lua_type(L, INDEX))
		{
		case LUA_TNUMBER:
			nbt[key] = lua_tonumber(L, INDEX);
			break;
		case LUA_TSTRING:
			nbt[key] = std::string(lua_tostring(L, INDEX));
			break;
		case LUA_TBOOLEAN:
			nbt[key] = lua_toboolean(L, INDEX);
			break;
		case LUA_TNIL:
		{
			auto it = nbt.maps().find(key);
			if (it != nbt.maps().end())
				nbt.maps().erase(it);
		}
		break;
		case LUA_TUSERDATA:
		{
			if (sol::stack::check_usertype<NBT>(L, INDEX))
			{
				auto e = sol::stack::get_usertype<NBT>(L, INDEX);
				nbt[key] = e;
			}
			else
			{
				return luaL_error(L, "LUA Invalid type passed to nbt");
			}
		}
		break;
		default:
			ND_WARN("LUA Invalid type passed to nbt");
			break;
		}
	}
	return 0;
}

#ifdef BIND_LUA_GUI_HANDLES
template<typename T>
static T* guiCaster(GUIElement* pointer) noexcept
{
	return dynamic_cast<T*>(pointer);
}
template<typename T, typename... Args>
static T* nue(Args&&... args) noexcept
{
	return new T(std::forward<Args>(args)...);
}
#define ND_LUASOL_EXTERNAL_CONS(className,...)\
		state.set_function(#className, &nue<className, __VA_ARGS__>);
#define ND_LUASOL_EXTERNAL_CON(className)\
		state.set_function(#className, &nue<className>);

static void bindGUIElements(sol::state& state)
{
	{
		auto namespac = state["GUIAlign"].get_or_create<sol::table>();

		namespac["RIGHT"] = GUIAlign::RIGHT;
		namespac["RIGHT_UP"] = GUIAlign::RIGHT_UP;
		namespac["RIGHT_DOWN"] = GUIAlign::RIGHT_DOWN;
		namespac["LEFT"] = GUIAlign::LEFT;
		namespac["LEFT_UP"] = GUIAlign::LEFT_UP;
		namespac["LEFT_DOWN"] = GUIAlign::LEFT_DOWN;
		namespac["CENTER"] = GUIAlign::CENTER;
		namespac["CENTER_UP"] = GUIAlign::CENTER_UP;
		namespac["CENTER_DOWN"] = GUIAlign::CENTER_DOWN;
		namespac["UP"] = GUIAlign::UP;
		namespac["DOWN"] = GUIAlign::DOWN;
		namespac["INVALID"] = GUIAlign::INVALID;
	}
	{

		auto namespac = state["GUIDim"].get_or_create<sol::table>();

		namespac["WIDTH"] = GUIDimensionInherit::WIDTH;
		namespac["HEIGHT"] = GUIDimensionInherit::HEIGHT;
		namespac["WIDTH_HEIGHT"] = GUIDimensionInherit::WIDTH_HEIGHT;
		namespac["INVALID"] = GUIDimensionInherit::INVALID;
	}
	{

		auto namespac = state["GETYPE"].get_or_create<sol::table>();

		namespac["Window"] = GETYPE::Window;
		namespac["Text"] = GETYPE::Text;
		namespac["Button"] = GETYPE::Button;
		namespac["Column"] = GETYPE::Column;
		namespac["Row"] = GETYPE::Row;
		namespac["Grid"] = GETYPE::Grid;
		namespac["Image"] = GETYPE::Image;
		namespac["CheckBox"] = GETYPE::CheckBox;
		namespac["RadioButton"] = GETYPE::RadioButton;
		namespac["Slider"] = GETYPE::Slider;
		namespac["VSlider"] = GETYPE::VSlider;
		namespac["HSlider"] = GETYPE::HSlider;
		namespac["TextBox"] = GETYPE::TextBox;
		namespac["TextArea"] = GETYPE::TextArea;
		namespac["SplitHorizontal"] = GETYPE::SplitHorizontal;
		namespac["SplitVertical"] = GETYPE::SplitVertical;
		namespac["Vie"] = GETYPE::View;
		namespac["Blank"] = GETYPE::Blank;
		namespac["ItemContainer"] = GETYPE::ItemContainer;
		namespac["Other"] = GETYPE::Other;
	}
	{



		state["GUI_LEFT"] = GUI_LEFT;
		state["GUI_RIGHT"] = GUI_RIGHT;
		state["GUI_TOP"] = GUI_TOP;
		state["GUI_BOTTOM"] = GUI_BOTTOM;
	}


	state.new_usertype<GUIElement>("GUIElementClass", sol::no_constructor,
		"x", &GUIElement::x,
		"y", &GUIElement::y,
		"pos", &GUIElement::pos,

		"width", &GUIElement::height,
		"height", &GUIElement::height,
		"dim", &GUIElement::dim,

		"setPadding", sol::overload(&GUIElement::setPadding, [](GUIElement& e, int index, float value) {e.padding[index] = value; }),
		"dim", &GUIElement::dim,
		"dimInherit", &GUIElement::dimInherit,
		"color", &GUIElement::color,
		"clas", &GUIElement::clas,
		"id", sol::readonly(&GUIElement::id),//should be readonly so
		"color", &GUIElement::color,
		"isEnabled", &GUIElement::isEnabled,
		"renderAngle", &GUIElement::renderAngle,
		"isVisible", &GUIElement::isVisible,
		"isNotSpatial", &GUIElement::isNotSpatial,
		"isAlwaysPacked", &GUIElement::isAlwaysPacked,
		"space", &GUIElement::space,
		"type", sol::readonly(&GUIElement::type),
		"isDirty", &GUIElement::isDirty,
		"setPadding", &GUIElement::setPadding,
		"hasFocus", &GUIElement::hasFocus,
		"isPressed", &GUIElement::isPressed,
		"getAlignment", &GUIElement::getAlignment,
		"setAlignment", &GUIElement::setAlignment,
		"markDirty", &GUIElement::markDirty,
		"getParent", &GUIElement::getParent,
		"getChildren", &GUIElement::getChildren,
		"getFirstChild", &GUIElement::getFirstChild,
		"appendChild", &GUIElement::appendChild,
		"destroyChild", &GUIElement::destroyChild,
		"destroyChildWithID", &GUIElement::destroyChildWithID,
		"setCenterPosition", &GUIElement::setCenterPosition,
		"widthPadding", &GUIElement::widthPadding,
		"heightPadding", &GUIElement::heightPadding,
		"clearChildren", &GUIElement::clearChildren,
		"hasChild", &GUIElement::hasChild,
		"takeChild", &GUIElement::takeChild
		);

	state.new_usertype<GUIBlank>("GUIBlankClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUIBlank);

	state.new_usertype<GUIText>("GUITextClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"text", sol::property(&GUIText::getText, &GUIText::setText),
		"fontMaterial", &GUIText::fontMaterial
		);
	ND_LUASOL_EXTERNAL_CONS(GUIText, FontMaterial*);

	state.new_usertype<GUIButton>("GUIButtonClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"soundFocus", &GUIButton::soundFocus,
		"soundVolume", &GUIButton::soundVolume,
		"soundClick", &GUIButton::soundClick
		);
	ND_LUASOL_EXTERNAL_CON(GUIButton);

	state.new_usertype<GUIImage>("GUIImageClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"image", &GUIImage::image,
		"scale", &GUIImage::scale,
		"setImage", &GUIImage::setImage
		);
	ND_LUASOL_EXTERNAL_CON(GUIImage);

	state.new_usertype<GUIWindow>("GUIWindowClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"isMoveable", &GUIWindow::isMoveable,
		"isResizable", &GUIWindow::isResizable
		);
	ND_LUASOL_EXTERNAL_CONS(GUIWindow);

	state.new_usertype<GUITextBox>("GUITextBoxClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"fontMaterial", &GUITextBox::fontMaterial,
		"setGainFocus", &GUITextBox::setGainFocus,
		"text", sol::property(&GUITextBox::getValue, &GUITextBox::setValue)
		);
	ND_LUASOL_EXTERNAL_CON(GUITextBox);

	state.new_usertype<GUICheckBox>("GUICheckBoxClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"value", sol::property(&GUICheckBox::getValue, &GUICheckBox::setValue),
		"setText", [](GUICheckBox& e, const std::string& trueText, const std::string& falseText) {e.setText(trueText, falseText); }
	);
	ND_LUASOL_EXTERNAL_CON(GUICheckBox);

	state.new_usertype<GUIColumn>("GUIColumnClass",
		sol::no_constructor,
		sol::base_classes, sol::bases<GUIElement>()
		);
	state.set_function("GUIColumn", sol::overload(&nue<GUIColumn>, &nue<GUIColumn, GUIAlign>));

	state.new_usertype<GUIRow>("GUIRowClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CONS(GUIRow, GUIAlign);

	state.new_usertype<GUIGrid>("GUIGridClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUIGrid);

	state.new_usertype<GUIHorizontalSplit>("GUIHorizontalSplitClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"getUpChild", &GUIHorizontalSplit::getUpChild,
		"getDownChild", &GUIHorizontalSplit::getDownChild
		);
	ND_LUASOL_EXTERNAL_CONS(GUIHorizontalSplit, bool);

	state.new_usertype<GUIVerticalSplit>("GUIVerticalSplitClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>(),
		"getRightChild", &GUIVerticalSplit::getRightChild,
		"getLeftChild", &GUIVerticalSplit::getLeftChild
		);
	ND_LUASOL_EXTERNAL_CONS(GUIVerticalSplit, bool);

	state.new_usertype<GUISlider>("GUISliderClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUISlider);

	state.new_usertype<GUIVSlider>("GUIVSliderClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUIVSlider);

	state.new_usertype<GUIHSlider>("GUIHSliderClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUIHSlider);

	state.new_usertype<GUIView>("GUIViewClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CON(GUIView);

	state.new_usertype<GUITextButton>("GUITextButtonClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()
		);
	ND_LUASOL_EXTERNAL_CONS(GUITextButton, const std::string&, FontMaterial*);


	state.new_usertype<GUIImageButton>("GUIImageButtonClass", sol::no_constructor, sol::base_classes, sol::bases<GUIElement>()

		);
	//ND_LUASOL_EXTERNAL_CONS(GUIImageButton,Sprite*);

	state.new_usertype<GUISpecialTextButton>("GUISpecialTextButtonClass", sol::no_constructor,
		sol::base_classes, sol::bases<GUIElement, GUIButton, GUITextButton>(),

		"currentScale", &GUISpecialTextButton::currentScale,
		"minScale", &GUISpecialTextButton::minScale,
		"maxScale", &GUISpecialTextButton::maxScale,
		"animationSpeed", &GUISpecialTextButton::animationSpeed
		);
	ND_LUASOL_EXTERNAL_CONS(GUISpecialTextButton, const std::string&, FontMaterial*);
}

static void bindGUICast(sol::state& state)
{
	state.set_function("GUI_Blank", &guiCaster<GUIBlank>);
	state.set_function("GUI_Text", &guiCaster<GUIText>);
	state.set_function("GUI_Button", &guiCaster<GUIButton>);
	state.set_function("GUI_Image", &guiCaster<GUIImage>);
	state.set_function("GUI_Window", &guiCaster<GUIWindow>);
	state.set_function("GUI_TextBox", &guiCaster<GUITextBox>);
	state.set_function("GUI_CheckBox", &guiCaster<GUICheckBox>);
	state.set_function("GUI_Column", &guiCaster<GUIColumn>);
	state.set_function("GUI_Row", &guiCaster<GUIRow>);
	state.set_function("GUI_Grid", &guiCaster<GUIGrid>);
	state.set_function("GUI_HorizontalSplit", &guiCaster<GUIHorizontalSplit>);
	state.set_function("GUI_VerticalSplit", &guiCaster<GUIVerticalSplit>);
	state.set_function("GUI_Slider", &guiCaster<GUISlider>);
	state.set_function("GUI_VSlider", &guiCaster<GUIVSlider>);
	state.set_function("GUI_HSlider", &guiCaster<GUIHSlider>);
	state.set_function("GUI_View", &guiCaster<GUIView>);
	state.set_function("GUI_TextButton", &guiCaster<GUITextButton>);
	state.set_function("GUI_ImageButton", &guiCaster<GUIImageButton>);
	state.set_function("GUI_SpecialTextButton", &guiCaster<GUISpecialTextButton>);
}

static void bindGUIContext(sol::state& state)
{

	auto namespac = state["GUIContext"].get_or_create<sol::table>();

	namespac.set_function("openWindow", [](GUIWindow& window) {GUIContext::get().openWindow(&window); });
	namespac.set_function("destroyWindow", [](GEID window) {GUIContext::get().destroyWindow(window); });
	namespac.set_function("closeWindow", [](GEID window) {GUIContext::get().closeWindow(window); });
}
#endif

static void bindFontMaterial(sol::state& state)
{
	state.new_usertype<FontMaterial>("FontMaterial",
		sol::no_constructor,
		"color", &FontMaterial::color,
		"border_color", &FontMaterial::border_color
		);
	auto namespac = state["FontMatLib"].get_or_create<sol::table>();

	namespac.set_function("getMaterial", &FontMatLib::getMaterial);
}