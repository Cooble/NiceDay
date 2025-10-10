#pragma once
#include "graphics/API/Texture.h"
#include "Material.h"
#include "Camm.h"
#include "Mesh.h"
#include "graphics/API/FrameBuffer.h"

namespace nd {
namespace AtelierDim {
	constexpr int width = 120;
	constexpr int height = 120;
};

class Atelier
{
	static Atelier g_instance;

private:
	Atelier() = default;

	FrameBufferPtr m_fbo{};
	TexturePtr m_background{};
	MeshPtr m_sphere{};
	MaterialPtr m_enviroment{};
	MaterialPtr m_default_material{};

	struct Env
	{
		glm::mat4 view;
		glm::mat4 proj;

		glm::vec3 sunPos;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 camera_pos;

		float constant;
		float linear;
		float quadratic;
	} m_env;

	std::unordered_map<Strid, TexturePtr> m_photos;

	std::vector<MaterialPtr> m_pending_work;
	std::vector<MeshPtr> m_pending_work_mesh;
public:

	static Atelier& get()
	{
		return g_instance;
	}
	static void unloadAll()
	{
		// reset singleton
		g_instance = Atelier();
	}

	void init();
	void snapshot(TexturePtr& photo, MaterialPtr& mat);
	void snapshot(TexturePtr& photo, MeshPtr& mat);

	TexturePtr getPhoto(MaterialPtr& mat);
	TexturePtr getPhoto(MeshPtr& mat);

	//new texture will be available after makePendingPhotos()
	TexturePtr assignPhotoWork(MaterialPtr& mat);
	TexturePtr assignPhotoWork(MeshPtr& mat);

	//will render pending photos
	void makePendingPhotos();
};
}
