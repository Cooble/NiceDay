#pragma once
#include "graphics/API/Shader.h"
#include "graphics/API/Texture.h"
#include "core/sids.h"

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
};
class Material
{
public:
	typedef Ref<Material> MaterialPtr;
private:
	std::string m_name;
	char* m_ubo;
	ShaderPtr m_shader;
	const UniformLayout* m_matLayout;
	std::unordered_map<Strid, size_t> m_offsets;
	//each texture has a slot associated and index in m_textures array
	std::unordered_map<Strid, int> m_tex_indexes;
	std::vector<TexturePtr> m_textures;
public:
	Material(const MaterialInfo& info);
	static MaterialPtr create(const MaterialInfo& info);

	MaterialPtr copy(const char* name);

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
		ASSERT(m_matLayout->size == sizeof(BigStruct), "Size of struct doesn't correspond to shader uniform");
		memcpy(m_ubo, &src, sizeof(BigStruct));
	}

	template <typename T>
	T& getValue(StringId name)
	{
		auto t = getPointerToValue(name);
		ASSERT(t, "invalid field name");
		return *(T*)t;
	}

	template <>
	TexturePtr& getValue(StringId name)
	{
		auto it = m_tex_indexes.find(name());
		ASSERT(it != m_tex_indexes.end(), "INvalid texture name");
		return m_textures[it->second];
	}

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
	

	void* getPointerToValue(StringId name);

	const auto& getShader() const { return m_shader; }
	const auto& getLayout() const { return *m_matLayout; }
	int getTextureCount() const { return m_textures.size(); }
	auto& getName() const { return m_name; }
};
typedef Ref<Material> MaterialPtr;

