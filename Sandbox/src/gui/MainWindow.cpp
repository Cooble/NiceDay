#include "MainWindow.h"
#include "core/App.h"
#include "event/MessageEvent.h"
#include "layer/CommonMessages.h"
#include "gui/GUIContext.h"
#include "GUICustomRenderer.h"
#include "world/WorldsProvider.h"
#include "window_messeages.h"

static float logoTransient = -1.3;

MainWindow::MainWindow(const MessageConsumer& c)
	:m_messenger(c) {
	width = App::get().getWindow()->getWidth();
	height = App::get().getWindow()->getHeight();
	isVisible = false;
	isMoveable = false;
	isResizable = false;
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;


	setCenterPosition(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	auto material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");

	auto col = new GUIColumn();
	col->isAlwaysPacked = true;
	col->setAlignment(GUIAlign::CENTER);

	
	long long micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	int rand = micros % 6;
	auto logo = Texture::create(TextureInfo("res/images/logos/" + std::to_string(rand) + ".png").filterMode(TextureFilterMode::LINEAR));
	auto res = new SpriteSheetResource(logo, 1, 1);

	m_logo = new GUIImage();
	m_logo->setImage(new Sprite(res));
	m_logo->image->setSize({ logo->getWidth(), logo->getHeight() });
	m_logo->packDimensions();
	m_logo->isAlwaysPacked = true;
	m_logo->scale = 0;
	col->appendChild(m_logo);
	
	auto dims = new GUIElement(GETYPE::Blank);
	dims->dim = {0,25};
	dims->isVisible = false;
	col->appendChild(dims);
	
	auto playBtn = new GUISpecialTextButton("Play", material);
	playBtn->dim = {200,50};
	playBtn->maxScale = 1.2;
	playBtn->minScale = 0.7;
	playBtn->onPressed=[this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::MenuPlay));
		logoTransient = -1;
	};
	col->appendChild(playBtn);

	auto playNew = new GUISpecialTextButton("New", material);
	playNew->dim = { 200,50 };
	playNew->maxScale = 1.2;
	playNew->minScale = 0.7;
	playNew->onPressed = [](GUIElement& e)
	{
		logoTransient = -1;
	};
	col->appendChild(playNew);
	
	auto setBtn = new GUISpecialTextButton("Settings", material);
	setBtn->dim = { 200,50 };
	setBtn->maxScale = 1.2;
	setBtn->minScale = 0.7;
	setBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::MenuSettings));
	};
	col->appendChild(setBtn);

	auto exitBtn = new GUISpecialTextButton("Exit", material);
	exitBtn->dim = { 200,50 };
	exitBtn->maxScale = 1.2;
	exitBtn->minScale = 0.7;
	exitBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::MenuExit));
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

GUIWorldEntry::GUIWorldEntry(MessageConsumer* c):m_messenger(c)
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
	color = { 1,0,1,1 };
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
		auto data = ND_TEMP_EMPLACE(WindowMessageData::World);
		data->worldName = this->getWorldName();
		auto m = MessageEvent(WindowMess::MenuPlayWorld,0,data);
		(*m_messenger)(m);
		
		/*auto dat = App::get().getBufferedAllocator().emplace<CommonMessages::WorldMessage>();
		dat->type = CommonMessages::WorldMessage::PLAY;
		dat->worldName = m_world_name->getText();
		App::get().fireEvent(MessageEvent("Play", 0, dat));*/
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
	deleteBtn->onPressed = [this](GUIElement& e) {

		auto data = ND_TEMP_EMPLACE(WindowMessageData::World);
		data->worldName = this->getWorldName();
		auto m = MessageEvent(WindowMess::MenuDeleteWorld, 0, data);
		(*m_messenger)(m);
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


PlayWindow::PlayWindow(const MessageConsumer& c)
:m_messenger(c){
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
	

	auto onWorldCrea = [textBox,this](GUIElement& e)
	{
		if (textBox->getValue().empty())
			return;
		
		auto data = ND_TEMP_EMPLACE(WindowMessageData::World);
		data->worldName = textBox->getValue();
		auto m = MessageEvent(WindowMess::MenuGenerateWorld, 0, data);
		m_messenger(m);
	};
	textBox->onValueEntered = onWorldCrea;
	createNewBtn->onPressed = onWorldCrea;

	//Column
	auto col = new GUIColumn();
	col->dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
	col->space = 15;
	col->setAlignment(GUIAlign::CENTER);
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
	m_world_slider = dynamic_cast<GUIVSlider*>(view->getRightChild()->getFirstChild());
	src->setPadding(0);
	src->getInside()->color = guiCRColor;
	col->appendChild(view);

	//view column
	m_world_column = new GUIColumn();
	m_world_column->dimInherit = GUIDimensionInherit::WIDTH;
	m_world_column->isAlwaysPacked = true;
	m_world_column->setAlignment(GUIAlign::CENTER);

	
	src->getInside()->appendChild(m_world_column);

	//back btn
	auto bckBtn = new GUITextButton("Back", material);
	bckBtn->dim = { 700, bckBtn->getTextElement()->height + bckBtn->heightPadding() };
	
	bckBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::MenuBack));
	};

	mainCol->appendChild(centerBox);
	mainCol->appendChild(bckBtn);
	appendChild(mainCol);
}

void PlayWindow::setWorlds(const std::vector<WorldInfoData>& worlds)
{
	m_world_column->clearChildren();
	m_world_column->adaptToParent();
	for (auto& world : worlds)
	{
		auto worlde = new GUIWorldEntry(&m_messenger);
		worlde->setWorldName(world.name);
		m_world_column->appendChild(worlde);
	}
	m_world_column->adaptToParent();
	m_world_slider->setValue(1);
	//todo fuck you  
}

PauseWindow::PauseWindow(const MessageConsumer& c)
:m_messenger(c){
	
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
	centerBox->dim = { 500,230 };
	centerBox->isVisible = true;
	centerBox->setAlignment(GUIAlign::CENTER);


	
	//Column
	auto col = new GUIColumn();
	col->dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;
	col->space = 15;
	col->setAlignment(GUIAlign::CENTER);
	centerBox->appendChild(col);

	auto spacer = new GUIBlank();
	spacer->isAlwaysPacked = false;
	spacer->height = 50;
	col->appendChild(spacer);
	
	//createbtn
	auto createNewBtn = new GUITextButton("Continue", materialSmall);
	createNewBtn->setAlignment(GUIAlign::CENTER);
	createNewBtn->isAlwaysPacked = true;
	createNewBtn->setPadding(5);
	createNewBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::MenuBack));
	};
	col->appendChild(createNewBtn);

	//goToMainScreenBtn
	auto goToMainScreenBtn = new GUITextButton("Save and go to menu", materialSmall);
	goToMainScreenBtn->setAlignment(GUIAlign::CENTER);
	goToMainScreenBtn->isAlwaysPacked = true;
	goToMainScreenBtn->setPadding(5);
	goToMainScreenBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::WorldQuit));
	};
	col->appendChild(goToMainScreenBtn);

	
	//Title
	auto title = new GUIText(material);
	title->setText("We are paused");
	title->setAlignment(GUIAlign::CENTER);

	auto blankTitle = new GUIBlank();
	blankTitle->setPadding(10);
	blankTitle->isAlwaysPacked = true;
	blankTitle->isVisible = true;

	blankTitle->appendChild(title);
	blankTitle->y = centerBox->height - blankTitle->height / 2;
	blankTitle->x = centerBox->width / 2 - blankTitle->width / 2;
	centerBox->appendChild(blankTitle);
	
	mainCol->appendChild(centerBox);
	appendChild(mainCol);

}
