#pragma once
#include "layer/Layer.h"
#include "graphics/API/Shader.h"
#include "graphics/API/Texture.h"
#include "graphics/font/FontParser.h"
#include "graphics/FontMaterial.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"
#include "gui/GUIRenderer.h"
#include "App.h"
#include "graphics/Sprite.h"

inline static BatchRenderer2D* g_batch_render;

class GUITestLayer : public Layer
{
private:
	GUIContextID currentContext;
	GUIRenderer guiRender;
	Texture* fontTexture;
	Texture* backTexture;
	Sprite* backSprite;
	Shader* fontShader;
	Font font;

	FontMaterial* fontMat;

public:
	GUITestLayer()
	{
		m_name = "gui_test";
	};

	inline void onAttach() override
	{
		g_batch_render = new BatchRenderer2D();

		ND_INFO("TestLayer attached");

		FontParser::parse(font, "res/fonts/basic.fnt");
		//font.xSpace = -10;
		font.ySpace = -5;

		fontTexture = Texture::create(TextureInfo(font.texturePath).filterMode(TextureFilterMode::NEAREST));
		backTexture = Texture::create(TextureInfo("res/images/gui_back.png"));
		static SpriteSheetResource* res = new SpriteSheetResource(backTexture, 1, 1);
		backSprite = new Sprite(res);
		backSprite->setPosition({-1, -1, 0.1});
		backSprite->setSize({2, 2});

		fontMat = new FontMaterial();
		fontMat->color = {1, 1, 1, 1};
		fontMat->border_color = {0, 0, 0, 0};
		fontMat->font = &font;
		fontMat->texture = fontTexture;
		guiRender.m_font_material = fontMat;

		currentContext = GUIContext::create();
		guiRender.setContext(&GUIContext::get());

		ND_INFO("font loaded");


		auto& context = GUIContext::get();
		for (int i = 0; i < 1; ++i)
		{
			context.getWindows().push_back(new GUIWindow());
			auto& window = *context.getFocusedWindow();
			window.dim = {1200 + i * 50, 400};
			window.setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
			window.x += i * 50;
			window.y += i * 50;

			{
				auto layout = new GUIColumn();
				layout->alignment = GUIAlign::LEFT;
				window.appendChild(layout);


				auto box = new GUICheckBox();
				box->setText("Happiness: Enabled", "Happiness: Disabled");
				box->setValue(false);
				box->dim = {500, 42};
				layout->appendChild(box);

				auto slider = new GUISlider();
				slider->dim = {400, 30};
				slider->setValue(0);
				slider->on_changed = [](GUIElement& e)
				{
					ND_INFO("val " + std::to_string((static_cast<GUISlider&>(e).getValue())));
				};
				slider->dividor = 10;
				layout->appendChild(slider);

				auto textBox = new GUITextBox();
				textBox->dim = {400, 50};
				layout->appendChild(textBox);
				textBox->setValue("herro");

				auto btn = new GUIButton();
				btn->setText("buttonek");
				btn->dim = { fontMat->font->getTextWidth("buttonek") + 10, fontMat->font->lineHeight + 6 };
				layout->appendChild(btn);

				auto lbl = new GUILabel();
				lbl->setValue("LABEL 01");
				lbl->dim = { 300, 50 };
				layout->appendChild(lbl);

				auto img = new GUIImage();
				img->setValue(backSprite);
				img->dim = { 300, 300 };
				layout->appendChild(img);
				
				continue;
			}

			{
				auto col = new GUIColumn();
				col->alignment = GUIAlign::RIGHT;
				window.appendChild(col);

				for (int i = 0; i < 3; ++i)
				{
					auto btn = new GUIButton();
					btn->setText("Clickme" + std::to_string(i));
					btn->dim = {200, 50};
					btn->on_pressed = [](GUIElement& e)
					{
						ND_INFO("I have pressed this shit jaj " + static_cast<GUIButton&>(e).getText());
					};
					col->appendChild(btn);
				}
				{
					auto btn = new GUILabel();
					btn->setValue("ABCDEDGHIJK");
					btn->dim = {300, 50};
					col->appendChild(btn);
				}
				{
					auto btn = new GUILabel();
					btn->setValue("B");
					btn->dim = {50, 50};
					col->appendChild(btn);
				}
				{
					auto btn = new GUILabel();
					btn->setValue("A");
					btn->dim = {50, 50};
					col->appendChild(btn);
				}
			}
		}


		/*auto lbl = new GUILabel();
		lbl->setValue("This is label i think");
		lbl->dim = { 150,50 };
		lbl->pos = { 200,100 };
		window.appendChild(lbl);*/
	}

	void onDetach() override
	{
		delete g_batch_render;
	}

	void onRender() override
	{
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);

		g_batch_render->begin();

		g_batch_render->submit(*backSprite);

		g_batch_render->push(
			glm::translate(
				glm::mat4(1.f),
				{-1.f, -1.f, 0}));
		g_batch_render->push(
			glm::scale(
				glm::mat4(1.f),
				{2.f / App::get().getWindow()->getWidth(), 2.f / App::get().getWindow()->getHeight(), 1}));


		guiRender.render(*g_batch_render);

		g_batch_render->pop();
		g_batch_render->pop();
		g_batch_render->flush();
	}

	void onEvent(Event& e) override
	{
		bool flipped = e.isInCategory(Event::EventCategory::Mouse);
		if (flipped)
			static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
		GUIContext::get().onEvent(e);
		if (flipped)
			static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
	}
};
