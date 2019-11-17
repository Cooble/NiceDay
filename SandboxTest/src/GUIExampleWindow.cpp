#include "ndpch.h"
#include "GUIExampleWindow.h"
#include "App.h"
#include "graphics/API/Texture.h"

GUIExampleWindow::GUIExampleWindow()
{
	width = App::get().getWindow()->getWidth()/3*2;
	height = App::get().getWindow()->getHeight()/3*2;
	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	auto backTexture = Texture::create(TextureInfo("res/images/gui_back.png"));
	static SpriteSheetResource* res = new SpriteSheetResource(backTexture, 1, 1);
	auto backSprite = new Sprite(res);
	backSprite->setPosition({ -1, -1, 0.1 });
	backSprite->setSize({ 2, 2 });
	
	auto leftRow = new GUIRow();
	leftRow->setAlignment(GUIAlign::LEFT);
	auto rightRow = new GUIRow();
	rightRow->setAlignment(GUIAlign::RIGHT);

	for (int i = 0; i < 5; ++i)
	{
		auto btn = new GUIButton();
		btn->dim = { 30,30 };
		btn->setText("A");
		leftRow->appendChild(btn);
	}
	GUIButton* lastBut = nullptr;
	GUIButton* preBut = nullptr;
	for (int i = 0; i < 3; ++i)
	{
		auto btn = new GUIButton();
		btn->dim = { 40,40 };
		btn->setText("B");
		rightRow->appendChild(btn);
		lastBut = btn;
	}
	preBut = (GUIButton*)rightRow->getChildren()[rightRow->getChildren().size()-2];
	
	
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
	lowerSettingSplit->getLeftChild()->appendChild(optionsColumn);
	lowerSettingSplit->getLeftChild()->color = { 1,1,0,1 };
	lowerSettingSplit->getLeftChild()->is_always_packed = false;
	lowerSettingSplit->getLeftChild()->width = 200;

	auto textBox = new GUITextBox();
	textBox->dim = { 300,100 };
	textBox->setAlignment(GUIAlign::CENTER_UP);
	
	lowerSettingSplit->getRightChild()->appendChild(textBox);
	for (int i = 0; i < 5; ++i)
	{
		auto btn = new GUIButton();
		btn->dim = { 50 + i * 10,30 + i * 5 };
		btn->setText("ahoj " + std::to_string(i));
		btn->on_pressed = [textBox,btn](GUIElement& e)
		{
			textBox->setValue(btn->getText());
		};
		optionsColumn->appendChild(btn);
	}
	{
		auto grid = new GUIGrid();
		grid->dimension_inherit = GUIDimensionInherit::WIDTH;
		grid->setAlignment(GUIAlign::CENTER_DOWN);
		for (int i = 0; i < 25; ++i)
		{
			auto img = new GUIImage();
			img->setValue(backSprite);
			img->dim = { 30+i*2, 20+i*3 };
			grid->appendChild(img);
		}
		lowerSettingSplit->getRightChild()->appendChild(grid);

		lastBut->on_pressed = [grid, backSprite](GUIElement& e)
		{
			auto img = new GUIImage();
			img->setValue(backSprite);
			
			img->dim = { 30+std::rand()%60 , 20+ std::rand() % 60 };
			grid->appendChild(img);
		};
		preBut->on_pressed = [grid](GUIElement& e)
		{
			if(grid->getChildren().size())
				grid->removeChild(0);
		};
	}
	

	
	
	appendChild(upperLineSplit);
	
	
	
	
	
}
