#include "MainWindow.h"
#include "core/App.h"
#include "event/MessageEvent.h"
#include "gui/GUIContext.h"
#include "GUICustomRenderer.h"
#include "Translator.h"
#include "world/WorldsProvider.h"
#include "window_messeages.h"
#include "event/ControlMap.h"
#include "event/KeyEvent.h"
#include "core/NBT.h"
#include "files/FUtil.h"
#include "gui/GUIParser.h"

using namespace nd;


FontMaterial* GameFonts::bigFont;
FontMaterial* GameFonts::smallFont;




static float logoTransient = -1.3;

MainWindow::MainWindow(const MessageConsumer& c)
	:m_messenger(c) {
	width = APwin()->getWidth();
	height = APwin()->getHeight();
	isVisible = false;
	isMoveable = false;
	isResizable = false;
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;


	setCenterPosition(APwin()->getWidth(), APwin()->getHeight());

	auto material = GameFonts::bigFont;

	auto col = new GUIColumn();
	col->isAlwaysPacked = true;
	col->setAlignment(GUIAlign::CENTER);


	long long micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	int rand = micros % 6;
	auto logo = Texture::create(TextureInfo("res/images/logos/" + std::to_string(rand) + ".png").filterMode(TextureFilterMode::LINEAR));
	auto res = new SpriteSheetResource(logo, 1, 1);

	m_logo = new GUIImage();
	m_logo->setImage(new Sprite(res));
	m_logo->image->setSize({ logo->width(), logo->height() });
	m_logo->packDimensions();
	m_logo->isAlwaysPacked = true;
	m_logo->scale = 0;
	col->appendChild(m_logo);

	auto dims = new GUIElement(GETYPE::Blank);
	dims->dim = { 0,25 };
	dims->isVisible = false;
	col->appendChild(dims);

	//scale japanese
	auto gengo = App::get().getSettings()["language"];
	float maxScale = 1.2 + (gengo.string() == "jp" ? 0.2 : 0);
	float minScale = 0.7 + (gengo.string() == "jp" ? 0.3 : 0);



	//auto playBtn = new GUISpecialTextButton(Font::colorizeBorder(Font::BLACK)+"&0P&1l&2a&3y &4P&5l&6a&7y &8P&9l&aa&by &cP&dl&ea&fy!", material);
	auto playBtn = new GUISpecialTextButton(ND_TRANSLATE("main.btn.play_play"), material);
	//auto playBtn = new GUISpecialTextButton("Play", material);
	playBtn->dim = { 200,50 };
	playBtn->maxScale = maxScale;
	playBtn->minScale = minScale;
	playBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenWorldSelection));
		logoTransient = -1;
	};
	col->appendChild(playBtn);

	auto playNew = new GUISpecialTextButton(ND_TRANSLATE("main.btn.play"), material);
	playNew->dim = { 200,50 };
	playNew->maxScale = maxScale;
	playNew->minScale = minScale;
	playNew->onPressed = [this](GUIElement& e)
	{
		logoTransient = -1;
		m_messenger(MessageEvent(WindowMess::OpenSkin));
	};
	col->appendChild(playNew);

	auto setBtn = new GUISpecialTextButton(ND_TRANSLATE("main.btn.settings"), material);
	setBtn->dim = { 200,50 };
	setBtn->maxScale = maxScale;
	setBtn->minScale = minScale;
	setBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenSettings));
	};
	col->appendChild(setBtn);

	auto exitBtn = new GUISpecialTextButton(ND_TRANSLATE("main.btn.exit"), material);
	exitBtn->dim = { 200,50 };
	exitBtn->maxScale = maxScale;
	exitBtn->minScale = minScale;
	exitBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenExit));
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

		m_logo->renderAngle = std::sin((scaleUp + 3.14159 / 2) / 2) / 8;

		float s = std::clamp(logoTransient, 0.f, 1.5f);
		if (s > 1.25f)
			s = 1.25 - (s - 1.25);
		m_logo->scale = (((1 + std::sin(scaleUp)) / 2) * 0.15 + 1) * s;
	}

}

GUIWorldEntry::GUIWorldEntry(MessageConsumer* c) :m_messenger(c)
{
	auto material = GameFonts::smallFont;

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
		playBtn->getImageElement()->image->setSpriteIndex(2, 0);
	};
	playBtn->onPressed = [this](GUIElement& e)
	{
		auto data = ND_TEMP_EMPLACE(WindowMessageData::World);
		data->worldName = this->getWorldName();
		auto m = MessageEvent(WindowMess::OpenPlayWorld, 0, data);
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
		auto m = MessageEvent(WindowMess::ActionDeleteWorld, 0, data);
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


SelectWorldWindow::SelectWorldWindow(const MessageConsumer& c)
	:m_messenger(c) {


   GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/world_select.xml"), *this);

	auto material = GameFonts::bigFont;
	auto materialSmall = GameFonts::smallFont;

	//createbtn
	auto createNewBtn = get<GUITextButton>("btn.create_world");
	//create txtbox
	auto textBox = get<GUITextBox>("txtBox");
	
	auto onWorldCrea = [textBox, this](GUIElement& e)
	{
		if (textBox->getValue().empty())
			return;

		auto data = ND_TEMP_EMPLACE(WindowMessageData::World);
		data->worldName = textBox->getValue();
		auto m = MessageEvent(WindowMess::ActionGenerateWorld, 0, data);
		m_messenger(m);
	};
	textBox->onValueEntered = onWorldCrea;
	createNewBtn->onPressed = onWorldCrea;

	m_world_slider = get<GUIVSlider>("slider");

	//view column
	m_world_column = get<GUIColumn>("keyColumn");

	//back btn
	auto bckBtn = new GUITextButton(ND_TRANSLATE("btn.back"), material);
	bckBtn->dim = { 700, bckBtn->getTextElement()->height + bckBtn->heightPadding() };

	get<GUIButton>("btn.back")->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenBack));
	};
}

void SelectWorldWindow::setWorlds(const std::vector<WorldInfoData>& worlds)
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
}

PauseWindow::PauseWindow(const MessageConsumer& c)
	:m_messenger(c) {

	width = APwin()->getWidth();
	height = APwin()->getHeight();
	setCenterPosition(APwin()->getWidth(), APwin()->getHeight());

	isVisible = false;
	isMoveable = false;
	isResizable = false;

	setAlignment(GUIAlign::CENTER);
	dimInherit = GUIDimensionInherit::WIDTH_HEIGHT;

	auto material = GameFonts::bigFont;
	auto materialSmall = GameFonts::smallFont;

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
	auto createNewBtn = new GUITextButton(ND_TRANSLATE("btn.continue"), materialSmall);
	createNewBtn->setAlignment(GUIAlign::CENTER);
	createNewBtn->isAlwaysPacked = true;
	createNewBtn->setPadding(5);
	createNewBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenBack));
	};
	col->appendChild(createNewBtn);


	//goToMainScreenBtn
	auto goToMainScreenBtn = new GUITextButton(ND_TRANSLATE("btn.save_go_menu"), materialSmall);
	goToMainScreenBtn->setAlignment(GUIAlign::CENTER);
	goToMainScreenBtn->isAlwaysPacked = true;
	goToMainScreenBtn->setPadding(5);
	goToMainScreenBtn->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::ActionWorldQuit));
	};
	col->appendChild(goToMainScreenBtn);


	//Title
	auto title = new GUIText(material);
	title->setText(ND_TRANSLATE("title.pause"));
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

ControlsWindow::ControlsWindow(const MessageConsumer& c)
	:m_messenger(c) {

	auto material = GameFonts::bigFont;
	auto materialSmall = GameFonts::smallFont;
	GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/settings_template.xml"), *this);


	// title
	get<GUIText>("template.title")->setText(ND_TRANSLATE("title.controls"));

	//custom column fill
	auto keyColumn = get<GUIColumn>("keyColumn");
	for (auto& pair : ControlMap::getControlsList())
	{
	   auto button = new GUISpecialTextButton(pair.first + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*pair.second.pointer), materialSmall);
	   button->minScale = 1;
	   button->maxScale = 1;
	   button->setPadding(10);
	   button->packDimensions();
	   keyColumn->appendChild(button);
	   button->onPressed = [button, titlo = pair.first](GUIElement& e)
	   {
		  if (GUIContext::get().getFocusedElement() != &e)
		  {
			 GUIContext::get().setFocusedElement(&e);
			 button->getTextElement()->setText(Font::BLUE + titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
		  else {
			 auto loc = APin().getMouseLocation();
			 if (!e.contains(loc.x, loc.y))
			 {
				GUIContext::get().setFocusedElement(nullptr);
				e.onMyEventFunc(MouseFocusLost(0, 0), e);
			 }

		  }
	   };
	   button->onMyEventFunc = [button, titlo = pair.first](Event& eve, GUIElement& e)
	   {

		  if (GUIContext::get().getFocusedElement() == &e)
		  {
			 if (eve.getEventType() == Event::EventType::KeyPress)
			 {
				auto m = static_cast<KeyPressEvent&>(eve);
				ControlMap::setValueAtPointer(titlo, (uint64_t)m.getKey());
				button->getTextElement()->setText(titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName((uint64_t)m.getKey()));
				GUIContext::get().setFocusedElement(nullptr);
			 }
		  }
		  else if (eve.getEventType() == Event::EventType::MouseFocusGain)
		  {
			 button->getTextElement()->setText(Font::GREEN + titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
		  else if (eve.getEventType() == Event::EventType::MouseFocusLost)
		  {
			 button->getTextElement()->setText(titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
	   };
	}
	for (auto& pair : ControlMap::getControlsList())
	{
	   auto button = new GUISpecialTextButton(pair.first + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*pair.second.pointer), materialSmall);
	   button->minScale = 1;
	   button->maxScale = 1;
	   button->setPadding(10);
	   button->packDimensions();
	   keyColumn->appendChild(button);
	   button->onPressed = [button, titlo = pair.first](GUIElement& e)
	   {
		  if (GUIContext::get().getFocusedElement() != &e)
		  {
			 GUIContext::get().setFocusedElement(&e);
			 button->getTextElement()->setText(Font::BLUE + titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
		  else {
			 auto loc = APin().getMouseLocation();
			 if (!e.contains(loc.x, loc.y))
			 {
				GUIContext::get().setFocusedElement(nullptr);
				e.onMyEventFunc(MouseFocusLost(0, 0), e);
			 }

		  }
	   };
	   button->onMyEventFunc = [button, titlo = pair.first](Event& eve, GUIElement& e)
	   {

		  if (GUIContext::get().getFocusedElement() == &e)
		  {
			 if (eve.getEventType() == Event::EventType::KeyPress)
			 {
				auto m = static_cast<KeyPressEvent&>(eve);
				ControlMap::setValueAtPointer(titlo, (uint64_t)m.getKey());
				button->getTextElement()->setText(titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName((uint64_t)m.getKey()));
				GUIContext::get().setFocusedElement(nullptr);
			 }
		  }
		  else if (eve.getEventType() == Event::EventType::MouseFocusGain)
		  {
			 button->getTextElement()->setText(Font::GREEN + titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
		  else if (eve.getEventType() == Event::EventType::MouseFocusLost)
		  {
			 button->getTextElement()->setText(titlo + ": " + Font::colorize(Font::BLACK, "#013220") + ControlMap::getKeyName(*ControlMap::getButtonData(titlo)->pointer));
		  }
	   };
	}

	// back buttons
	auto onPressed = [this](GUIElement& e)
	{
	   m_messenger(MessageEvent(WindowMess::OpenBack));
	};
	get<GUIButton>("btn.back")->onPressed = onPressed;
	get<GUIButton>("btn.save_back")->onPressed = onPressed;
}
SettingsWindow::SettingsWindow(const MessageConsumer& c) :m_messenger(c) {

	auto material = GameFonts::bigFont;
	auto materialSmall = GameFonts::smallFont;
	GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/settings_template.xml"), *this);


	// title
	get<GUIText>("template.title")->setText(ND_TRANSLATE("title.settings"));

	//custom column fill
   auto keyColumn = get<GUIColumn>("keyColumn");
	//nav controls button
	auto navCon = new GUITextButton(ND_TRANSLATE("btn.controls"), materialSmall);
	navCon->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenControls));
	};
	navCon->setPadding(5);
	navCon->isAlwaysPacked = true;
	keyColumn->appendChild(navCon);
	auto lang = new GUITextButton(ND_TRANSLATE("btn.lang"), materialSmall);
	lang->setPadding(5);
	lang->onPressed = [this](GUIElement& e)
	{
		m_messenger(MessageEvent(WindowMess::OpenLanguage));
	};
	lang->isAlwaysPacked = true;
	keyColumn->appendChild(lang);

	// back buttons
	auto onPressed = [this](GUIElement& e)
	{
	   m_messenger(MessageEvent(WindowMess::OpenBack));
	};
	get<GUIButton>("btn.back")->onPressed = onPressed;
	get<GUIButton>("btn.save_back")->onPressed = onPressed;
}
LanguageWindow::LanguageWindow(const MessageConsumer& c)
	:m_messenger(c) {

   GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/settings_template.xml"), *this);

	auto material = GameFonts::bigFont;
	auto materialSmall = GameFonts::smallFont;

	// title
	get<GUIText>("template.title")->setText(ND_TRANSLATE("title.language"));

	// custom column fill
	auto keyColumn = get<GUIColumn>("keyColumn");

	auto& list = AppLanguages::getLanguages();
	for (auto& lang : list)
	{
		bool selected = lang.abbrev == AppLanguages::getCurrentLanguage();

		auto btn = new GUITextButton((selected ? Font::colorizeBorder(Font::BLACK) + Font::GREEN : "") + lang.name, materialSmall);
		if (!selected) {
			btn->onPressed = [this, abbrev = lang.abbrev](GUIElement& e)
			{
			   App::get().fireEvent(MessageEvent("language_change", abbrev));
				
				m_messenger(MessageEvent(WindowMess::OpenBack));//go up
				m_messenger(MessageEvent(WindowMess::OpenLanguage));//go back again
			};
		}

		btn->setPadding(5);
		btn->isAlwaysPacked = true;
		keyColumn->appendChild(btn);
	}

	// back button
	auto onPressed = [this](GUIElement& e)
	{
	   m_messenger(MessageEvent(WindowMess::OpenBack));
	};

	get<GUIButton>("btn.back")->onPressed = onPressed;
	get<GUIButton>("btn.save_back")->onPressed = onPressed;
}

SkinWindow::SkinWindow(const MessageConsumer& c)
	:m_messenger(c) {

	width = APwin()->getWidth();
	height = APwin()->getHeight();
	setCenterPosition(APwin()->getWidth(), APwin()->getHeight());
	GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/world_select.xml"), *this);
}

void SkinWindow::onMyEvent(Event& e)
{
	GUIWindow::onMyEvent(e);
	auto ee = dynamic_cast<MousePressEvent*>(&e);
	if (ee)
	{
		GUIContext::get().setFocusedElement(this);
	}
	if (KeyPressEvent::getKeyNumber(e) == KeyCode::R)
	{
	   clearChildren();
	   GUIParser::parseWindow(FUtil::readFileString("res/xml_gui/world_select.xml"), *this);
	}
	else if (KeyPressEvent::getKeyNumber(e) == KeyCode::B)
	{
		GUIContext::get().setFocusedElement(nullptr);
		m_messenger(MessageEvent(WindowMess::OpenBack));
	}
}
