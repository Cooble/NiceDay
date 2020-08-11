#pragma once
#include "graphics/API/Texture.h"
#include "Material.h"
#include "Camm.h"
#include "Mesh.h"

namespace AtelierDim
{
	constexpr int width=120;
	constexpr int height=120;
};
class Atelier
{
private:
	Atelier() { init(); }

	FrameBuffer* m_fbo;
	TexturePtr m_background=nullptr;
	MeshPtr m_sphere;
	MaterialPtr m_enviroment=nullptr;
	MaterialPtr m_default_material = nullptr;
	
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
		static Atelier a;
		return a;
	}
	void init();
	void snapshot(TexturePtr& photo,MaterialPtr& mat);
	void snapshot(TexturePtr& photo,MeshPtr& mat);
	
	TexturePtr getPhoto(MaterialPtr& mat);
	TexturePtr getPhoto(MeshPtr& mat);

	//new texture will be available after makePendingPhotos()
	TexturePtr assignPhotoWork(MaterialPtr& mat);
	TexturePtr assignPhotoWork(MeshPtr& mat);

	//will render pending photos
	void makePendingPhotos();
};
