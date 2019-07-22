#include "ndpch.h"
/*#include "TextureManager.h"
#include "Texture.h"

Texture* TextureManager::getTexture(const char* path)
{
	auto it = s_textures.find(path);
	if (it != s_textures.end())
		return it->second;
	auto& t = s_textures[path];
	t = Texture::create(TextureInfo(path));
	return t;
}

void TextureManager::destroyTextures()
{
	for (auto t : s_textures)
		delete t.second;
}*/
