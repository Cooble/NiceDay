#pragma once
#include "NewScene.h"
#include "components.h"
#include "graphics/TextureAtlas.h"
#include "imgui.h"

namespace components_imgui_access
{
	
	void Image(ImTextureID image, const TextureAtlasUVCoords& coords, ImVec2 size);
	
	inline void Image(ImTextureID image, const TextureAtlasUVCoords& coords)
	{
		Image(image,coords, ImVec2(coords.pixelSize.x, coords.pixelSize.y));
	}
	inline void Image(StringId id, const TextureAtlasUV& atlas)
	{
		Image(reinterpret_cast<ImTextureID>(atlas.getTexture()->getID()), atlas.getSubImage(id));
	}
	inline void Image(StringId id, const TextureAtlasUV& atlas, ImVec2 size) { Image(reinterpret_cast<ImTextureID>(atlas.getTexture()->getID()), atlas.getSubImage(id), size); }

	
	void draw(Entity e, LightComponent& c);
	void draw(Entity e, TransformComponent& c);
	void draw(Entity e, ModelComponent& c);
	void draw(Entity e, CameraComponent& c);

	bool drawWindow(MaterialPtr& c);
	bool drawEntityManager();


	struct SceneWindows
	{
		bool open_material=false;
		MaterialPtr material;

		NewScene* scene;
		Entity activeCamera;
		Entity selectedEntity;

		void drawWindows();
		void init();
	};
	extern SceneWindows windows;
	extern TextureAtlasUV icons;
};



