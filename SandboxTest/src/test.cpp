#define ND_TEST
#include "core/App.h"
#include "imgui.h"
#include "graphics/font/FontParser.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/API/Texture.h"
#include "graphics/API/Shader.h"
#include "graphics/API/VertexArray.h"
#include "graphics/GContext.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/Sprite.h"
#include "graphics/FontMaterial.h"
#include "GUITestLayer.h"
#include "file/test_DynamicSaver.h"
#include "ConsoleTestLayer.h"
#include "MandelBrotLayer.h"
#include "scene/SceneLayer.h"

static bool textChange;
static int textSize;

extern BatchRenderer2D* g_batch_render;

class TestLayer:public Layer
{
private:
	Texture* fontTexture;
	Shader* fontShader;
	TextMesh* textMesh;
	Font font;
	char textInput[1024];
	Sprite m_sprite;
	FontMaterial* fontMat;
	BatchRenderer2D* renderer2d;
	
public:
	TestLayer() = default;
	
	inline void onAttach() override
	{

		renderer2d = new BatchRenderer2D();
		static SpriteSheetResource res(Texture::create(
			TextureInfo("res/images/player.png")
			.filterMode(TextureFilterMode::NEAREST)
			.format(TextureFormat::RGBA)), 4, 4);
		m_sprite = Sprite(&res);
		m_sprite.setSpriteIndex(0, 0);
		float size = 0.3f;
		m_sprite.setPosition(glm::vec3(-size / 2, -size / 2, 0));
		m_sprite.setSize(glm::vec2(size, size));
		
		textMesh = new TextMesh(300);
		ND_INFO("TestLayer attached");

		FontParser::parse(font,"arial.fnt");
		font.xSpace = -10;
		font.ySpace= -5;

		TextBuilder::buildMesh("Wwi\nihesKarelbgngb\nllo this is \njust very interest",1000,
			font, *textMesh,TextBuilder::ALIGN_CENTER);
		
		fontTexture = Texture::create(TextureInfo(font.texturePath));

		fontMat = new FontMaterial();
		fontMat->color = glm::vec4(0, 0.5, 0.1, 1);
		fontMat->border_color = glm::vec4(1, 1, 1, 1);
		fontMat->font = &font;
		fontMat->texture = fontTexture;

		ND_INFO("font loaded");
	}
	void onDetach() override
	{
		delete renderer2d;
	}
	void onRender() override
	{
		
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		
		renderer2d->begin(Renderer::getDefaultFBO());
		renderer2d->submitColorQuad(glm::vec3(0, 0, 0), glm::vec2(0.3f, 0.2f), glm::vec4(0, 1, 0, 1));
		renderer2d->submit(m_sprite);
		renderer2d->push(glm::scale(glm::mat4(1.f), glm::vec3(1.f / App::get().getWindow()->getWidth(), 1.f / App::get().getWindow()->getHeight(), 1)));
		renderer2d->submitText(*textMesh, fontMat);
		renderer2d->pop();
		renderer2d->flush();
	}
	void onUpdate() override
	{
		if(textChange)
		{
			textChange = false;
			TextBuilder::buildMesh(std::string(textInput).substr(0,textSize),1000,
				font, *textMesh, TextBuilder::ALIGN_CENTER);
		}
		GUIContext::get().onUpdate();
	}
	static int textBack(ImGuiInputTextCallbackData* data)
	{
		textSize = data->BufTextLen;
		textChange = true;
		return 1;
	}
	void onImGuiRender() override
	{

		/*bool t = true;
		ImGui::ShowDemoWindow(&t);
		if(ImGui::Begin("Karel"))
		{
			ImGui::Text("Sup dawg");
			ImGui::InputTextMultiline("Texttik", textInput, sizeof(textInput), ImVec2(0, 0), 
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize, &textBack);
			static float c[4];
			static float cBo[4];
			static float width[2];
			if(ImGui::ColorPicker4("text", c)|| ImGui::ColorPicker4("border", cBo)||ImGui::InputFloat2("thick", width))
			{
				fontMat->color = *(glm::vec4*) & c;
				fontMat->border_color = *(glm::vec4*) & cBo;
				fontMat->thickness = glm::vec2(width[0], width[1]);
			}

		}
		ImGui::End();*/
	}
};
class TestApp:public App
{
public:
	TestApp()
	{
		//m_LayerStack.pushLayer(new GUITestLayer());
		AppInfo info;
		info.enableSCENE = true;
		info.enableIMGUI= true;
		
		init(info);
		m_LayerStack.pushLayer(new ConsoleTestLayer());
		m_LayerStack.pushLayer(new SceneLayer());
		//m_LayerStack.pushLayer(new MandelBrotLayer());
	}
	
};
#ifdef ND_TEST
int main()
{
	Log::init();
	TestApp t;

	t.start();
	
	
	ND_INFO("EXxit test");
	ND_WAIT_FOR_INPUT;
	return 0;
	
}
#endif