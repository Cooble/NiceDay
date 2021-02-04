#include "ndpch.h"
#include "FontMaterial.h"

FontMaterial::FontMaterial(): id(currentID++)
{
}

FontMaterial* FontMatLib::getMaterial(const std::string& name)
{
	static std::unordered_map<std::string, FontMaterial> s_fonts;
	auto d = s_fonts.find(name);
	if(d==s_fonts.end())
	{
		auto font = new Font();
		if (!FontParser::parse(*font, name))
		{
			ND_WARN("Cannot parse fontMat: {}", name);
			delete font;
			return nullptr;
		}
		s_fonts.emplace(name, FontMaterial());
		FontMaterial& mat = s_fonts[name];
		mat.font = font;
		mat.texture = Texture::create(TextureInfo((SUtil::startsWith(name,"res/fonts/")?"res/fonts/":"")+mat.font->texturePath).filterMode(TextureFilterMode::LINEAR));

		mat.color = { 1, 1, 1, 1 };
		mat.border_color = { 0, 0.1, 0.7, 1 };
		mat.name = name;

		ND_TRACE("Loaded fontMaterial: {}", name);
		return &mat;
		
	}
	else
		return &d->second;
	
	
}
