#include "SceneLayer.h"
#include "graphics/API/Texture.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Mesh.h"
#include "Colli.h"
#include "Scene.h"
#include "Camm.h"


static Model* dragon;

void SceneLayer::onAttach()
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/ignore/OpenSans-Regular.ttf").c_str(), 20);

	//adding JP (merging with current font) -> nihongode ok kedo, mada nai to omou
	/*ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/NotoSansCJKjp-Medium.otf").c_str(), 20, &config, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->Build();*/

	m_scene = new Scene;

	//adding crate
	{
		auto crateTexture = Texture::create(TextureInfo(ND_RESLOC("res/models/crate.png")));
		auto crateSpecular = Texture::create(TextureInfo(ND_RESLOC("res/models/crate_specular.png")));
		auto crate = new GModel;
		crate->set(Colli::buildMesh(ND_RESLOC("res/models/cube.fbx")));
		crate->textures.push_back(crateTexture);
		crate->textures.push_back(crateSpecular);
		auto mo = new Model("crate");
		mo->model = crate;
		mo->pos = { 1,2,1 };
		m_scene->addObject(mo);
	}
	//adding dragoon
	{
		auto d = new GModel;
		d->set(Colli::buildMesh(ND_RESLOC("res/models/dragon.obj")));
		dragon = new Model("dragon");
		dragon->color = { 0,0,0,1 };
		dragon->model = d;
		m_scene->addObject(dragon);
	}

}

void SceneLayer::onDetach()
{
}

void SceneLayer::onUpdate()
{
	m_scene->update();
}

void SceneLayer::onRender()
{
	m_scene->render();
}


void SceneLayer::onImGuiRender()
{
	m_scene->imGuiRender();
	static bool open = true;
	if (ImGui::Begin("ConsoleTest", &open))
	{
		//ImGui::Checkbox("Look editor", &lookEditor);
		//ImGui::Checkbox("event editor", &eventEditor);
		//ImGui::SliderInt("blur", &blurAmount, 0, 20);
		//ImGui::SliderFloat3("pos", (float*)&editorCam->pos, -10, 10);
		//ImGui::SliderFloat3("angle", (float*)&editorCam->angles, -3.14159f, 3.14159f);
		//ImGui::SliderFloat3("lightPos", (float*)&lightPos, 0, 50);
		ImGui::ColorPicker3("Dragon color", (float*)&dragon->color);

	}
	ImGui::End();
}

void SceneLayer::onEvent(Event& e)
{
	m_scene->onEvent(e);

}

