#pragma once
#include "layer/Layer.h"
#include "graphics/API/Shader.h"
#include "graphics/API/Texture.h"
#include "graphics/font/FontParser.h"
#include "graphics/FontMaterial.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"
#include "gui/GUIRenderer.h"
#include "core/App.h"
#include "graphics/Sprite.h"
#include "GUIExampleWindow.h"
#include "globals.h"


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
	FontMaterial* g_fontMat;


public:
	GUITestLayer()
	{
		m_name = "gui_test";
	};

	inline void onAttach() override
	{
		g_batch_render = new BatchRenderer2D();

		ND_INFO("TestLayer attached");

		FontParser::parse(font, "res/fonts/andrew.fnt");
		font.xSpace = 0;
		font.ySpace = -5;

		fontTexture = Texture::create(TextureInfo("res/fonts/"+font.texturePath).filterMode(TextureFilterMode::NEAREST));
		backTexture = Texture::create(TextureInfo("res/images/gui_back.png"));
		static SpriteSheetResource* res = new SpriteSheetResource(backTexture, 1, 1);
		backSprite = new Sprite(res);
		backSprite->setPosition({-1, -1, 0.1});
		backSprite->setSize({2, 2});

		g_fontMat = FontMatLib::getMaterial("res/fonts/andrew.fnt");
		guiRender.m_font_material = g_fontMat;

		currentContext = GUIContext::create();
		guiRender.setContext(&GUIContext::get());

		ND_INFO("font loaded");


		auto& context = GUIContext::get();
		/*for (int i = 0; i < 1; ++i)
		{
			context.getWindows().push_back(new GUIWindow());
			auto& window = *context.getFocusedWindow();
			window.dim = { 1200 + i * 50, 400 };
			window.setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
			window.x += i * 50;
			window.y += i * 50;
			window.isResizable = false;
			continue;
			{
				auto layout = new GUIGrid();
				layout->width = 500;
				for (int i = 0; i < 25; ++i)
				{

					auto btn = new GUITextButton("ss",g_fontMat);
					btn->dim = { 30 + i * 5,30 };
					layout->appendChild(btn);
					if (i == 10)
					{
						auto img = new GUIImage();
						img->setImage(backSprite);
						img->dim = { 100, 100 };
						layout->appendChild(img);
					}
				}
				layout->setAlignment(GUIAlign::LEFT_UP);
				window.appendChild(layout);
			}

			{
				auto layout = new GUIColumn();
				layout->child_alignment = GUIAlign::CENTER;
				layout->setAlignment(GUIAlign::RIGHT_UP);

				auto row = new GUIRow(GUIAlign::CENTER);

				auto box = new GUICheckBox();
				box->setText("Happiness: Enabled", "Happiness: Disabled");
				box->setValue(false);
				box->dim = { 500, 42 };
				row->appendChild(box);
				{
					auto btn = new GUITextButton("buttonek",g_fontMat);
					btn->dim ={ 200, g_fontMat->font->lineHeight+btn->heightPadding()};
					btn->onPressed = [](GUIElement& e)
					{
						ND_INFO("clicked 1");
					};
					row->appendChild(btn);
				}
				layout->appendChild(row);

				auto slider = new GUISlider();
				slider->dim = { 400, 30 };
				slider->setValue(0);
				slider->on_changed = [](GUIElement& e)
				{
					ND_INFO("val " + std::to_string((static_cast<GUISlider&>(e).getValue())));
				};
				//slider->dividor = 10;
				layout->appendChild(slider);

				auto textBox = new GUITextBox();
				textBox->dim = { 400, 50 };
				layout->appendChild(textBox);
				textBox->setValue("herro");
				
				auto btn = new GUISpecialTextButton("buttonek",g_fontMat);
				btn->dim = { g_fontMat->font->getTextWidth("buttonek") + 10, g_fontMat->font->lineHeight + btn->heightPadding() };
				layout->appendChild(btn);

				auto lbl = new GUIText(g_fontMat);
				lbl->setText("yBEL 01");
				lbl->textScale = 2;
				lbl->dim = { 300, 50 };
				layout->appendChild(lbl);

				auto img = new GUIImage();
				img->setImage(backSprite);
				img->dim = { 300, 300 };
				layout->appendChild(img);
				window.appendChild(layout);

				continue;
			}

		}*/

		auto newWin = new GUIExampleWindow();
		context.getWindows().push_back(newWin);
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
		if(e.getEventType()==Event::EventType::WindowResize)
		{
			auto m = static_cast<WindowResizeEvent&>(e);
			guiRender.setScreenDimensions(m.getWidth(), m.getHeight());
		}
		bool flipped = e.isInCategory(Event::EventCategory::Mouse);
		if (flipped)
			static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
		GUIContext::get().onEvent(e);
		if (flipped)
			static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
	}

	void onUpdate() override
	{
		GUIContext::get().onUpdate();
	}
};
