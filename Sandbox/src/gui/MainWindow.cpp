#include "MainWindow.h"
#include "core/App.h"
#include "event/MessageEvent.h"
#include "layer/CommonMessages.h"
#include "gui/GUIContext.h"
#include "GUICustomRenderer.h"
#include "world/WorldsProvider.h"

static float logoTransient = -1.3;

MainWindow::MainWindow()
{
	width = App::get().getWindow()->getWidth();
	height = App::get().getWindow()->getHeight();
	isVisible = false;
	isMoveable = false;
	isResizable = false;
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;


	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	auto material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");

	auto col = new GUIColumn();
	col->setAlignment(GUIAlign::CENTER);

	
	long long micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	int rand = micros % 6;
	auto logo = Texture::create(TextureInfo("res/images/logos/" + std::to_string(rand) + ".png").filterMode(TextureFilterMode::LINEAR));
	auto res = new SpriteSheetResource(logo, 1, 1);

	m_logo = new GUIImage();
	m_logo->setImage(new Sprite(res));
	m_logo->image->setSize({ logo->getWidth(), logo->getHeight() });
	m_logo->isAlwaysPacked = true;
	m_logo->scale = 0;
	col->appendChild(m_logo);
	
	auto dims = new GUIElement(GETYPE::Blank);
	dims->dim = {50,50};
	dims->isVisible = false;
	col->appendChild(dims);
	
	auto playBtn = new GUISpecialTextButton("Play", material);
	playBtn->dim = {200,50};
	playBtn->maxScale = 1.2;
	playBtn->minScale = 0.7;
	auto point = this;
	playBtn->onPressed=[point](GUIElement& e)
	{
		GUIContext::get().closeWindow(point->id);
		GUIContext::get().getWindows().push_back(new PlayWindow());
		
		logoTransient = -1;
	};
	col->appendChild(playBtn);

	auto playNew = new GUISpecialTextButton("New", material);
	playNew->dim = { 200,50 };
	playNew->maxScale = 1.2;
	playNew->minScale = 0.7;
	playNew->onPressed = [](GUIElement& e)
	{

		auto dat = App::get().getBufferedAllocator().allocate(CommonMessages::PlayMessage());
		dat->type = CommonMessages::PlayMessage::PLAY;
		App::get().fireEvent(MessageEvent("PlayNewBtnEvent", 0, dat));

		logoTransient = -1;
	};
	col->appendChild(playNew);
	
	auto setBtn = new GUISpecialTextButton("Settings", material);
	setBtn->dim = { 200,50 };
	setBtn->maxScale = 1.2;
	setBtn->minScale = 0.7;
	point = this;
	setBtn->onPressed = [point](GUIElement& e)
	{
	
	};
	col->appendChild(setBtn);

	auto exitBtn = new GUISpecialTextButton("Exit", material);
	exitBtn->dim = { 200,50 };
	exitBtn->maxScale = 1.2;
	exitBtn->minScale = 0.7;
	exitBtn->onPressed = [](GUIElement& e)
	{
		App::get().fireEvent(WindowCloseEvent());
	};
	col->appendChild(exitBtn);


	appendChild(col);
}

void MainWindow::update()
{
	GUIWindow::update();

	{//logo animation
		static float scaleUp = 0;
		logoTransient += 0.06;
		scaleUp += 0.01;

		m_logo->renderAngle = std::sin((scaleUp + 3.14159 / 2)/2) / 8;

		float s = std::clamp(logoTransient, 0.f, 1.5f);
		if (s > 1.25f)
			s = 1.25 - (s - 1.25);
		m_logo->scale = (((1 + std::sin(scaleUp)) / 2) * 0.15 + 1)*s;
	}
	
}

GUIWorldEntry::GUIWorldEntry()
{
	auto material = FontMatLib::getMaterial("res/fonts/andrew.fnt");

	static SpriteSheetResource* res = new SpriteSheetResource(
		Texture::create(TextureInfo("res/images/gui_atlas.png").filterMode(TextureFilterMode::NEAREST)),
		4, 4);
	
	setPadding(10);
	isAlwaysPacked = false;
	isVisible = true;
	dimInherit = GUIDimensionInherit::WIDTH;
	height = 100;
	m_world_name = new GUIText(material);
	m_world_name->setAlignment(GUIAlign::LEFT_UP);
	appendChild(m_world_name);

	
	auto playBtn = new GUIImageButton(new Sprite(res));
	playBtn->isVisible = false;
	playBtn->dim = { 22,22 };
	playBtn->getImageElement()->image->setSpriteIndex(2, 0);
	playBtn->getImageElement()->dim = { 32,32 };
	playBtn->getImageElement()->setAlignment(GUIAlign::LEFT_DOWN);
	playBtn->onFocusGain = [playBtn](GUIElement& e)
	{
		playBtn->getImageElement()->image->setSpriteIndex(1, 0);
	};
	playBtn->onFocusLost = [playBtn](GUIElement& e)
	{
		playBtn->getImageElement()->image->setSpriteIndex(2,0);
	};
	playBtn->onPressed= [this](GUIElement& e)
	{
		auto dat = App::get().getBufferedAllocator().allocate(CommonMessages::PlayMessage());
		dat->type = CommonMessages::PlayMessage::PLAY;
		dat->worldName = m_world_name->getText();
		App::get().fireEvent(MessageEvent("Play", 0, dat));
	};

	auto deleteBtn = new GUIImageButton(new Sprite(res));
	deleteBtn->isVisible = false;
	deleteBtn->dim = { 22,22 };
	deleteBtn->getImageElement()->image->setSpriteIndex(0, 1);
	deleteBtn->getImageElement()->dim = { 32,32 };
	deleteBtn->getImageElement()->setAlignment(GUIAlign::LEFT_DOWN);
	deleteBtn->onFocusGain = [deleteBtn](GUIElement& e)
	{
		deleteBtn->getImageElement()->image->setSpriteIndex(1, 1);
	};
	deleteBtn->onFocusLost = [deleteBtn](GUIElement& e)
	{
		deleteBtn->getImageElement()->image->setSpriteIndex(0, 1);
	};
	deleteBtn->onPressed = [deleteBtn, this](GUIElement& e)
	{
		WorldsProvider::get().deleteWorld(m_world_name->getText());
		for (int i = 0; i < m_parent->getChildren().size(); ++i)
		{
			if(m_parent->getChildren()[i]==this)
			{
				//todo fix messes with pos stack
				m_parent->removeChild(i);
				return;
			}
		}
	};
	deleteBtn->setAlignment(GUIAlign::RIGHT_DOWN);

	auto row = new GUIRow();
	row->isAlwaysPacked = true;
	row->setAlignment(GUIAlign::LEFT_DOWN);
	row->appendChild(playBtn);
	

	appendChild(row);
	appendChild(deleteBtn);
	
}

void GUIWorldEntry::setWorldName(const std::string& name)
{
	m_world_name->setText(name);
}

const std::string& GUIWorldEntry::getWorldName()
{
	return m_world_name->getText();
}

PlayWindow::PlayWindow()
{
	width = App::get().getWindow()->getWidth();
	height = App::get().getWindow()->getHeight();
	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	
	isVisible = false;
	isMoveable = false;
	isResizable = false;

	setAlignment(GUIAlign::CENTER);
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;

	auto material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");
	auto materialSmall = FontMatLib::getMaterial("res/fonts/andrew.fnt");

	auto mainCol = new GUIColumn();
	mainCol->isAlwaysPacked = true;
	mainCol->dimInherit = GUIDimensionInherit::WIDTH;
	mainCol->setAlignment(GUIAlign::CENTER);
	
	auto centerBox = new GUIBlank();
	centerBox->setPadding(10);
	centerBox->isAlwaysPacked = false;
	centerBox->dim = { 700,500 };
	centerBox->isVisible = true;
	centerBox->setAlignment(GUIAlign::CENTER);

	//create split
	auto split = new GUIVerticalSplit(false);
	split->dimInherit = GUIDimensionInherit::WIDTH;
	split->height = 40;
	
	//createbtn
	auto createNewBtn = new GUITextButton("Create", materialSmall);
	createNewBtn->setAlignment(GUIAlign::CENTER);
	createNewBtn->isAlwaysPacked = true;
	createNewBtn->setPadding(5);
	split->getRightChild()->appendChild(createNewBtn);
	split->height = createNewBtn->height+split->heightPadding();

	//create txtbox
	auto textBox = new GUITextBox();
	textBox->font = materialSmall;
	textBox->dimInherit = GUIDimensionInherit::WIDTH;
	textBox->height = materialSmall->font->lineHeight + textBox->heightPadding();
	textBox->setAlignment(GUIAlign::CENTER);
	split->getLeftChild()->appendChild(textBox);
	

	auto onWorldCrea = [textBox](GUIElement& e)
	{
		if (textBox->getValue().empty())
			return;
		for (auto& world : WorldsProvider::get().getAvailableWorlds())
		{
			if (world.name == textBox->getValue())
				return;
		}
		auto dat = App::get().getBufferedAllocator().allocate(CommonMessages::PlayMessage());
		dat->type = CommonMessages::PlayMessage::CREATE;
		dat->worldName = textBox->getValue();
		//todo here is bug with copying string or with allocation on bufstack
		App::get().fireEvent(MessageEvent("Play", 0, dat));
	};
	textBox->onValueEntered = onWorldCrea;
	createNewBtn->onPressed = onWorldCrea;

	//Column
	auto col = new GUIColumn();
	col->dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
	col->setAlignment(GUIAlign::CENTER_UP);
	centerBox->appendChild(col);

	//Title
	auto title = new GUIText(material);
	title->setText("Choose the world");
	title->setAlignment(GUIAlign::CENTER);

	auto blankTitle = new GUIBlank();
	blankTitle->setPadding(10);
	blankTitle->isAlwaysPacked = true;
	blankTitle->isVisible = true;
	
	blankTitle->appendChild(title);
	blankTitle->y = centerBox->height - blankTitle->height / 2;
	blankTitle->x = centerBox->width / 2 - blankTitle->width / 2;
	centerBox->appendChild(blankTitle);

	auto spacer = new GUIBlank();
	spacer->isAlwaysPacked = false;
	spacer->height = 50;
	col->appendChild(spacer);

	col->appendChild(split);


	//view
	auto view = createGUISliderView(false);
	view->dimInherit = GUIDimensionInherit::WIDTH;
	view->height = 300;
	GUIView* src = dynamic_cast<GUIView*>(view->getLeftChild()->getFirstChild());
	src->setPadding(0);
	src->getInside()->color = guiCRColor;

	//view column
	auto worldCol = new GUIColumn();
	worldCol->dimInherit = GUIDimensionInherit::WIDTH;
	worldCol->isAlwaysPacked = true;
	worldCol->setAlignment(GUIAlign::CENTER);

	WorldsProvider::get().rescanWorlds();
	auto& worldsList = WorldsProvider::get().getAvailableWorlds();
	for (auto& world:worldsList)
	{
		auto worlde = new GUIWorldEntry();
		worlde->setWorldName(world.name);
		worldCol->appendChild(worlde);

	}
	src->getInside()->appendChild(worldCol);


	col->appendChild(view);

	//back btn
	auto bckBtn = new GUITextButton("Back", material);
	bckBtn->dim = { 700, bckBtn->getTextElement()->height + bckBtn->heightPadding() };
	
	auto point = this;
	bckBtn->onPressed = [point](GUIElement& e)
	{
		GUIContext::get().closeWindow(point->id);
		GUIContext::get().getWindows().push_back(new MainWindow());
	};

	mainCol->appendChild(centerBox);
	mainCol->appendChild(bckBtn);
	appendChild(mainCol);
}
