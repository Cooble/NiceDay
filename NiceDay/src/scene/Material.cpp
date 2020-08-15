#include "Material.h"
#include "core/NBT.h"
#include "platform/OpenGL/GLShader.h"


Material::MaterialPtr Material::create(const MaterialInfo& info)
{
	return MakeRef<Material>(info);
}

Material::MaterialPtr Material::copy(const char* name)
{
	auto out = create({m_shader, m_layout->name.c_str(), name, m_shader ? nullptr : m_layout});
	memcpy(out->m_ubo, m_ubo, m_layout->size);
	out->m_tex_indexes = m_tex_indexes;
	out->m_textures = m_textures;
	out->m_flags = m_flags;
	return out;
}


void Material::setShader(ShaderPtr& shader)
{
	auto copy_offsets = m_offsets;
	auto copy_tex_indexes = m_tex_indexes;
	auto copy_textures = m_textures;
	auto copy_ubo = (char*)malloc(m_layout->size);
	memcpy(copy_ubo, m_ubo, m_layout->size);
	auto copy_layout = *m_layout;

	m_offsets.clear();
	m_tex_indexes.clear();
	m_textures.clear();
	m_layout = shader->getLayout().getLayoutByName(m_structName.c_str());
	m_shader = shader;
	updateLayoutFromShader();

	for (auto& newE : m_layout->elements)
	{
		auto oldElement = copy_layout.getElementByName(newE.name);
		if (oldElement && oldElement == newE)
		{
			if (GTypes::isTexture(newE.type))
			{
				auto it = copy_tex_indexes.find(toShortName(newE.name.c_str()));
				if (it == copy_tex_indexes.end())
					continue;
				auto ptr = copy_textures[it->second];
				this->setValue(toShortName(newE.name.c_str()), ptr);
			}
			else
				memcpy(m_ubo + newE.offset, copy_ubo + oldElement->offset, GTypes::getSize(newE.type) * newE.arraySize);
		}
	}
}

void Material::bind(int slotBindingOffset, const ShaderPtr& shader)
{
	ASSERT(m_shader || shader, "You need to specify shader on creation or now!");

	ShaderPtr sha = shader ? shader : m_shader;
	sha->bind();
	if (!m_layout)
		return;
	int localTexIdx = 0;
	auto sh = std::static_pointer_cast<GLShader>(sha);
	for (auto& element : m_layout->elements)
	{
		/*if(shader)//skip uniforms which dont exist in shader
		{
			auto ee = shader->getLayout().getLayoutByName(m_structName.c_str())->getElementByName(element.name);
			if(!ee/*|| ((UniformElement*)ee)!=(UniformElement*)&element*/ //we just assume that names are enough
		/*	continue;
	}*/
		if (GTypes::isTexture(element.type))
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
		case g_typ::TEXTURE_2D:
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

void Material::updateLayoutFromShader()
{
	int textures = 0;
	for (auto& element : m_layout->elements)
	{
		auto ss = toShortName(element.name.c_str());
		m_offsets[ss] = element.offset; //cut the first 4 chars "mat."
		if (GTypes::isTexture(element.type))
		{
			m_tex_indexes[ss] = textures++;
		}
	}
	m_textures.resize(textures);

	m_ubo = (char*)malloc(m_layout->size);
}

Material::Material(const MaterialInfo& info)
	: m_name(info.name), m_shader(info.shader), m_flags(info.flags), m_structName(info.structName),m_id(StringId(info.name)())
{
	ASSERT((info.shader || info.layout)&& !(info.shader && info.layout), "You need to specify shader xor layout");
	if (!info.shader)
		m_layout = info.layout;
	else m_layout = m_shader->getLayout().getLayoutByName(info.structName);
	if (!m_layout)
	{
		ND_WARN("Shader is missing mat");
		return;
	}
	updateLayoutFromShader();
}

void* Material::getPointerToValue(StringId name)
{
	auto it = m_offsets.find(name());
	ASSERT(it != m_offsets.end(), "invalid field name");
	return m_ubo + it->second;
}

#define SAVV(tyyye) list.save(element.name.substr(m_layout->name.size() + 1), *(tyyye*)(m_ubo + element.offset));break;

void Material::serialize(NBT& nbt)
{
	nbt.save("structName", m_layout->name);
	nbt.save("name", m_name);
	nbt.save("shader", m_shader->getFilePath());
	auto& list = nbt["elements"];
	for (auto& element : m_layout->elements)
	{
		switch (element.type)
		{
		case g_typ::UNSIGNED_INT: SAVV(uint32_t);
		case g_typ::FLOAT: SAVV(float);
		case g_typ::VEC2: SAVV(glm::vec2);
		case g_typ::VEC3: SAVV(glm::vec3);
		case g_typ::VEC4: SAVV(glm::vec4);
		case g_typ::INT: SAVV(int);
		case g_typ::IVEC2: SAVV(glm::ivec2);
		case g_typ::IVEC3: SAVV(glm::ivec3);
		case g_typ::IVEC4: SAVV(glm::ivec4);
		case g_typ::TEXTURE_2D:
		case g_typ::TEXTURE_CUBE:
			{
				auto& tex = m_textures[m_tex_indexes[SID(element.name)]];
				if (tex)
					list.save(element.name.substr(m_layout->name.size() + 1), tex->getFilePath());
				break;
			}
		case g_typ::MAT3: //ignore matrix
		case g_typ::MAT4:
			break;
		default:
			ASSERT(false, "Unsupported type");
			break;
		}
	}
}

#define LODD(tyyye) list.loadIfExists(element.name.substr(offsetSize), *(tyyye*)(m_ubo + element.offset));break;

MaterialPtr Material::deserialize(NBT& nbt)
{
	auto mat = Material::create({
		ShaderLib::loadOrGetShader(nbt["shader"].string()), nbt["structName"].c_str(), nbt["name"].c_str(), nullptr
	});
	auto offsetSize = mat->getLayout().name.size() + 1;
	auto m_ubo = mat->getRawAccess();
	auto& list = nbt["elements"];
	for (auto& element : mat->getLayout().elements)
		switch (element.type)
		{
		case g_typ::UNSIGNED_INT: LODD(uint32_t);
		case g_typ::FLOAT: LODD(float);
		case g_typ::VEC2: LODD(glm::vec2);
		case g_typ::VEC3: LODD(glm::vec3);
		case g_typ::VEC4: LODD(glm::vec4);
		case g_typ::INT: LODD(int);
		case g_typ::IVEC2: LODD(glm::ivec2);
		case g_typ::IVEC3: LODD(glm::ivec3);
		case g_typ::IVEC4: LODD(glm::ivec4);
		case g_typ::TEXTURE_2D:
		case g_typ::TEXTURE_CUBE:
			{
				TextureType type = element.type == g_typ::TEXTURE_2D ? TextureType::_2D : TextureType::_CUBE_MAP;
				TextureInfo info;
				info.type(type);
				info.file_path = list[element.name.substr(offsetSize)].string();
				if (info.file_path.empty())
					break;

				TexturePtr ptr = std::shared_ptr<Texture>(Texture::create(info));
				mat->setValue(element.name.substr(offsetSize), ptr);
				break;
			}
		case g_typ::MAT3: //ignore matrix
		case g_typ::MAT4:
			break;
		default:

			break;
		}
	return mat;
}


static MaterialPtr invalid;
static std::unordered_map<Strid,MaterialPtr> m_materials;
static std::unordered_map<Strid,bool> m_dirties;

MaterialPtr MaterialLibrary::getByName(const std::string& name)
{
	for (auto& mat : m_materials)
		if (mat.second->getName() == name)
			return mat.second;
	return invalid;
}

MaterialPtr MaterialLibrary::create(const MaterialInfo& info)
{
	std::string possibleName = info.name;
	int currentIndex = 1;
	while (true)
		if (m_materials.find(SID(possibleName)) != m_materials.end())
			possibleName = info.name + std::to_string(currentIndex++);
		else break;
	
	MaterialInfo newMat{info.shader, info.structName, possibleName.c_str(), info.layout, info.flags};
	auto mat = Material::create(newMat);
	m_materials[SID(possibleName)]=mat;
	m_dirties[SID(possibleName)] = true;
	return mat;
}

MaterialPtr MaterialLibrary::copy(const MaterialPtr& mat, const std::string& newName)
{
	auto out = mat->copy(newName.c_str());
	m_materials[out->getID()] = out;
	m_dirties[out->getID()] = true;

	return out;

}

void MaterialLibrary::save(MaterialPtr& ptr, const std::string& path)
{
	NBT e;
	ptr->serialize(e);
	NBT::saveToFile(path, e);
}

MaterialPtr MaterialLibrary::loadOrGet(const std::string& filepath)
{
	auto it = m_materials.find(SID(filepath));
	if (it != m_materials.end())
		return it->second;
	NBT e;
	NBT::loadFromFile(filepath, e);
	if (e.size() == 0)
		return invalid;
	auto m = Material::deserialize(e);
	m_materials[m->getID()]=m;
	m_dirties[m->getID()] = true;

	return m;
}

void MaterialLibrary::clearUnused()
{
	std::vector<Strid> toRemove;
	toRemove.reserve(m_materials.size());
	for (auto& material : m_materials)
		if (material.second.use_count() == 1) 
			toRemove.push_back(material.first);
	for (auto id : toRemove) {
		m_materials.erase(id);
		m_dirties.erase(id);
	}
}

std::unordered_map<Strid,MaterialPtr>& MaterialLibrary::getList()
{
	return m_materials;
}

void MaterialLibrary::remove(Strid mat)
{
	auto it = m_materials.find(mat);
	if (it == m_materials.end())
		return;
	m_materials.erase(it);
	m_dirties.erase(m_dirties.find(mat));
}

bool& MaterialLibrary::isDirty(Strid mat)
{
	auto it = m_dirties.find(mat);
	ASSERT(it != m_dirties.end(), "This material is not in lib");
	return it->second;
}
static std::shared_ptr<Material> nullMat(nullptr);
MaterialPtr& MaterialLibrary::get(Strid id)
{
	auto it = m_materials.find(id);
	if (it == m_materials.end())
		return nullMat;
	//ASSERT(it != m_materials.end(), "This material is not in lib");
	return it->second;
}
