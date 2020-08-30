#pragma once
#include "graphics/API/Shader.h"
#include "graphics/API/Texture.h"
#include "core/sids.h"

class NBT;

namespace MaterialFlags {
	enum:int
	{
		// default true
		FLAG_CULL_FACE = 1 << 0,
		// default true
		FLAG_DEPTH_MASK = 1 << 1,
		// default true
		FLAG_DEPTH_TEST = 1 << 2,
		// default false
		// glo.view matrix (passed globally) wont be translational 
		FLAG_CHOP_VIEW_MAT_POS = 1 << 3,
	};
	inline int DEFAULT_FLAGS = (FLAG_CULL_FACE | FLAG_DEPTH_MASK | FLAG_DEPTH_TEST);
}
// If you don't want to specify shader
// dont set shader but set layout instead
// In this case it's neccessary to specify shader in Material::bind(int,ShaderPtr)
//
struct MaterialInfo
{
	ShaderPtr shader;
	const char* structName = "MAT";
	const char* name="Unnamed Material";
	const UniformLayout* layout=nullptr;
	int flags= MaterialFlags::DEFAULT_FLAGS;
};
class MaterialLibrary;
class Material
{
public:
	friend MaterialLibrary;
	typedef Ref<Material> MaterialPtr;
private:
	Strid m_id;
	std::string m_name;
	std::string m_structName;
	char* m_ubo;
	ShaderPtr m_shader=std::shared_ptr<Shader>(nullptr);
	const UniformLayout* m_layout;
	std::unordered_map<Strid, size_t> m_offsets;
	//each texture has a slot associated and index in m_textures array
	std::unordered_map<Strid, int> m_tex_indexes;
	std::vector<TexturePtr> m_textures;
	int m_flags;
	void updateLayoutFromShader();
	MaterialPtr copy(const char* name);
public:
	Material(const Material&) = delete;
	Material(Material&&) = delete;
	Material(const MaterialInfo& info);
	static MaterialPtr create(const MaterialInfo& info);

	void setShader(ShaderPtr& shader);
	void serialize(NBT& nbt);
	static MaterialPtr deserialize(NBT& nbt);

	Strid getID() const { return m_id; }
	// will upload uniforms to shader
	// the textures will bind to slots starting from slotBindingOffset
	// set shader if was not specified in MaterialInfo
	void bind(int slotBindingOffset, const ShaderPtr& shader);
	void bind(int slotBindingOffset = 0)
	{
		ShaderPtr ptr;
		bind(slotBindingOffset, ptr);
	}

	// struct with same layout as in shader
	// texture in struct must be represented with int and it's needed to set texture using setValue(name,TexturePtr&)
	// btw. don't use vec3, it's not good for your health (though it might work with the current implementation)
	template <typename BigStruct>
	void setRaw(BigStruct& src)
	{
		ASSERT(m_layout->size == sizeof(BigStruct), "Size of struct doesn't correspond to shader uniform");
		memcpy(m_ubo, &src, sizeof(BigStruct));
	}
	// retrieves value by shortened name (without layout name) e.g. "color"
	template <typename T>
	T& getValue(StringId name)
	{
		auto t = getPointerToValue(name);
		ASSERT(t, "invalid field name");
		return *(T*)t;
	}
	// retrieves value by shortened name (without layout name) e.g. "color"
	template <>
	TexturePtr& getValue(StringId name)
	{
		auto it = m_tex_indexes.find(name());
		ASSERT(it != m_tex_indexes.end(), "INvalid texture name");
		return m_textures[it->second];
	}
	// retrieves value by full name as in layout e.g. "mat.color"
	template <typename T>
	T& getValueFullName(const char* name) { return getValue<T>(toShortName(name)); }
	// retrieves value by full name as in layout e.g. "mat.color"
	template <>
	TexturePtr& getValueFullName(const char* name) { return getValue<TexturePtr>(toShortName(name)); }

	template <typename T>
	void setValue(StringId name, T val)
	{
		auto t = getPointerToValue(name);
		ASSERT(t, "invalid field name");
		*(T*)t = val;
	}
	template <>
	void setValue(StringId name, TexturePtr texture)
	{
		auto it = m_tex_indexes.find(name());
		ASSERT(it != m_tex_indexes.end(), "INvalid texture name");
		m_textures[it->second] = texture;
	}
	// cuts of struct prefix e.g. from "mat.color" -> "color"
	Strid toShortName(const char* name)
	{
		ASSERT(strlen(name) > m_structName.size() + 1, "invalid name");
		return SID(name + (m_structName.size() + 1));
	}
	std::string toShortNameString(const std::string& name)
	{
		ASSERT(name.size() > m_structName.size() + 1, "invalid name");
		return name.substr(m_structName.size() + 1);
	}
	
	char* getRawAccess() { return m_ubo; }
	void* getPointerToValue(StringId name);
	void* getPointerToValueFullName(const char* c) { return getPointerToValue(toShortName(c)); }


	const auto& getShader() const { return m_shader; }
	const auto& getLayout() const { return *m_layout; }
	int getTextureCount() const { return m_textures.size(); }
	auto& getName() const { return m_name; }
	auto getFlags() const { return m_flags; }
	//changes the name but id will stay the same
	void setName(const std::string& name) { m_name = name; }
};


typedef Ref<Material> MaterialPtr;


class MaterialLibrary
{
public:
	static MaterialPtr getByName(const std::string& name);
	static MaterialPtr create(const MaterialInfo& info);
	static MaterialPtr copy(const MaterialPtr& mat, const std::string& newName);
	static void save(MaterialPtr& ptr, const std::string& path = "");
	static MaterialPtr loadOrGet(const std::string& filepath);
	static void clearUnused();
	static std::unordered_map<Strid,MaterialPtr>& getList();
	static void remove(Strid mat);

	static bool& isDirty(Strid mat);

	//returns ptr or nullptr
	static MaterialPtr& get(Strid id);

	//returns material based on filepath or nullptr if material not loaded
	static MaterialPtr& getByFilePath(Strid filePath);
};

