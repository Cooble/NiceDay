#include "ndpch.h"
#include "GUIExampleWindow.h"
#include "core/App.h"
#include "graphics/API/Texture.h"

GUIExampleWindow::GUIExampleWindow()
{
	width = App::get().getWindow()->getWidth() / 3 * 2;
	height = App::get().getWindow()->getHeight() / 3 * 2;
	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	auto g_fontMat = FontMatLib::getMaterial("res/fonts/andrew.fnt");
	auto backTexture = Texture::create(TextureInfo("res/images/gui_back.png"));
	static SpriteSheetResource* res = new SpriteSheetResource(backTexture, 1, 1);
	auto backSprite = new Sprite(res);
	backSprite->setPosition({ -1, -1, 0.1 });
	backSprite->setSize({ 2, 2 });

	auto leftRow = new GUIRow();
	leftRow->setAlignment(GUIAlign::LEFT);
	leftRow->child_alignment = GUIAlign::CENTER;
	auto rightRow = new GUIRow();
	rightRow->setAlignment(GUIAlign::RIGHT);

	for (int i = 0; i < 1; ++i)
	{
		auto btn = new GUITextButton("Happy 100th commit :D", g_fontMat);
		btn->setPadding(10);
		btn->dim = { g_fontMat->font->getTextWidth(btn->getTextElement()->getText()) + btn->widthPadding(), 45 };
		leftRow->appendChild(btn);
	}
	GUIButton* lastBut = nullptr;
	GUIButton* preBut = nullptr;
	char* cc = "AHOY";
	for (int i = 0; i < 4; ++i)
	{
		auto btn = new GUITextButton(std::string("") + (char)(cc[i]), g_fontMat);
		btn->dim = { 35, 45 };
		rightRow->appendChild(btn);
		lastBut = btn;
	}
	preBut = (GUIButton*)rightRow->getChildren()[rightRow->getChildren().size() - 2];


	auto upperLineSplit = new GUIHorizontalSplit();
	auto lowerSettingSplit = new GUIVerticalSplit();

	//upperLineSplit->getUpChild()->color = { 0,1,0,1 };
	upperLineSplit->getUpChild()->appendChild(leftRow);
	upperLineSplit->getUpChild()->appendChild(rightRow);
	upperLineSplit->getDownChild()->appendChild(lowerSettingSplit);
	//upperLineSplit->getDownChild()->color = { 0,0,1,1 };

	auto optionsColumn = new GUIColumn();
	optionsColumn->setAlignment(GUIAlign::CENTER);
	optionsColumn->setPadding(20);
	optionsColumn->isAlwaysPacked = true;
	lowerSettingSplit->getLeftChild()->appendChild(optionsColumn);
	lowerSettingSplit->getLeftChild()->color = { 1, 1, 0, 1 };
	lowerSettingSplit->getLeftChild()->isAlwaysPacked = false;
	lowerSettingSplit->getLeftChild()->isVisible = true;
	lowerSettingSplit->getLeftChild()->width = 200;

	auto textBox = new GUITextBox();
	textBox->font = g_fontMat;
	textBox->dim = { 170, 50 };
	textBox->setAlignment(GUIAlign::CENTER_UP);

	//lowerSettingSplit->getRightChild()->appendChild(textBox);
	for (int i = 0; i < 8; ++i)
	{
		auto btn = new GUISpecialTextButton("Aloha " + std::to_string(i) + "!", g_fontMat);
		btn->dim = { 130, 40 };
		btn->minScale = 1 + (i / 18.f);
		btn->maxScale = 1.5f + (i / 18.f);
		btn->onPressed = [btn](GUIElement& e)
		{
			ND_INFO(btn->getTextElement()->getText());
		};
		optionsColumn->appendChild(btn);
	}
	optionsColumn->appendChild(textBox);
	auto sl = new GUISlider();
	sl->dim = { 130,40 };
	sl->on_changed = [textBox, sl](GUIElement& e)
	{
		textBox->setValue(std::to_string(sl->getValue()));
	};
	optionsColumn->appendChild(sl);
	auto cbox = new GUICheckBox();
	cbox->dim = { g_fontMat->font->getTextWidth("Despair: Disabled") + cbox->widthPadding() + 50,40 };
	cbox->setText("Despair: Enabled", "Despair: Disabled");
	leftRow->appendChild(cbox);

	auto grid = new GUIGrid();
	grid->dimInherit = GUIDimensionInherit::WIDTH;
	grid->setAlignment(GUIAlign::CENTER);
	grid->isAlwaysPacked = true;
	for (int i = 0; i < 30; ++i)
	{
		auto img = new GUIImage();
		img->setImage(backSprite);
		img->dim = { 100 + i * 2, 100 + i * 3 };
		grid->appendChild(img);
	}
	lastBut->onPressed = [grid, backSprite](GUIElement& e)
	{
		auto img = new GUIImage();
		img->setImage(backSprite);

		img->dim = { 100 + std::rand() % 60,100 + std::rand() % 60 };
		grid->appendChild(img);
	};
	preBut->onPressed = [grid](GUIElement& e)
	{
		if (grid->getChildren().size())
			grid->removeChild(0);
	};

	auto split = createGUISliderView(false);
	split->getRightChild()->getChildren()[0]->setPadding(5);
	split->getRightChild()->getChildren()[0]->dim = { 30,0 };
	split->getLeftChild()->getFirstChild()->getFirstChild()->appendChild(grid);

	lowerSettingSplit->getRightChild()->appendChild(split);

	appendChild(upperLineSplit);
}