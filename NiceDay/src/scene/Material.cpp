#include "Material.h"
#include "platform/OpenGL/GLShader.h"


Material::MaterialPtr Material::create(const MaterialInfo& info)
{
	return MakeRef<Material>(info);
}

Material::MaterialPtr Material::copy(const char* name)
{
	auto out = create({ m_shader,m_matLayout->name.c_str(),name });
	memcpy(out->m_ubo, m_ubo, m_matLayout->size);
	out->m_tex_indexes = m_tex_indexes;
	out->m_textures = m_textures;
	return out;
}

void Material::bind(int slotBindingOffset, const ShaderPtr& shader)
{
	ASSERT(m_shader || shader, "You need to specify shader on creation or now!");

	ShaderPtr sha = shader?shader:m_shader;
	sha->bind();
	if (!m_matLayout)
		return;
	int localTexIdx = 0;
	auto sh = std::static_pointer_cast<GLShader>(sha);
	for (auto& element : m_matLayout->elements)
	{
		if (element.type == g_typ::TEXTURE)
			for (int i = 0; i < element.arraySize; ++i)
			{
				auto& t = m_textures[localTexIdx];
				if (t)
					t->bind(slotBindingOffset + localTexIdx);
				localTexIdx++;
			}
		switch (element.type)
		{
		case g_typ::FLOAT:
		case g_typ::VEC2:
		case g_typ::VEC3:
		case g_typ::VEC4:
			sh->setUniformfv(element.name, GTypes::getSize(element.type) / sizeof(float), element.arraySize,
			                 (float*)(m_ubo + element.offset));
			break;
		case g_typ::UNSIGNED_INT:
			sh->setUniformuiv(element.name, GTypes::getSize(element.type) / sizeof(float), element.arraySize,
			                  (uint32_t*)(m_ubo + element.offset));
			break;
		case g_typ::TEXTURE:
		case g_typ::INT:
		case g_typ::IVEC2:
		case g_typ::IVEC3:
		case g_typ::IVEC4:
			sh->setUniformiv(element.name, GTypes::getSize(element.type) / sizeof(float), element.arraySize,
			                 (int*)(m_ubo + element.offset));
			break;
		case g_typ::MAT3:
			sh->setUniformMat3v(element.name, element.arraySize, (glm::mat3*)(m_ubo + element.offset));
			break;
		case g_typ::MAT4:
			sh->setUniformMat4v(element.name, element.arraySize, (glm::mat4*)(m_ubo + element.offset));
			break;

		case g_typ::INVALID:
		case g_typ::BYTE:
		case g_typ::UNSIGNED_BYTE:
		case g_typ::SHORT:
		case g_typ::UNSIGNED_SHORT:
			ASSERT(false, "currently unsupported type");
			break;
		default: ;
		}
	}
}

Material::Material(const MaterialInfo& info)
	: m_name(info.name),m_shader(info.shader)
{
	ASSERT((info.shader || info.layout)&& !(info.shader && info.layout),"You need to specify shader xor layout");
	if (!info.shader)
		m_matLayout = info.layout;
	else m_matLayout = m_shader->getLayout().getLayoutByName(info.structName);
	if (!m_matLayout)
	{
		ND_WARN("Shader is missing mat");
		return;
	}
	int textures = 0;
	for (auto& element : m_matLayout->elements) {
		std::string ss = element.name.substr(m_matLayout->name.size() + 1);
		m_offsets[SID(ss)] = element.offset; //cut the first 4 chars "mat."
		m_tex_indexes[SID(ss)] = textures;
		if (element.type == g_typ::TEXTURE)
			textures++;
	}
	m_textures.resize(textures);
	
	m_ubo = (char*)malloc(m_matLayout->size);
	
}

void* Material::getPointerToValue(StringId name)
{
	auto it = m_offsets.find(name());
	ASSERT(it != m_offsets.end(), "invalid field name");
	return m_ubo + it->second;
}
