#pragma once
#include "NewScene.h"
#include "components.h"
namespace components_imgui_access
{
	void draw(Entity e, LightComponent& c);
	void draw(Entity e, TransformComponent& c);
	void draw(Entity e, ModelComponent& c);
	void draw(Entity e, CameraComponent& c);

	bool drawWindow(Mesh& c);
	bool drawWindow(MaterialPtr& c);
	bool drawEntityManager(NewScene& c,Entity& lookThroughCam);


	struct SceneWindows
	{
		bool open_material=false;
		MaterialPtr material;
		
		bool open_mesh=false;
		MeshPtr mesh;

		void drawWindows();
	};
	extern SceneWindows windows;
};



