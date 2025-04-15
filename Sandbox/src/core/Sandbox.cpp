#include "Sandbox.h"

#include <codecvt>


#include "imgui.h"
#include "Translator.h"
#include "core/imgui_utils.h"
#include "layer/MainLayer.h"
#include "layer/WorldLayer.h"
#include "graphics/Sprite2D.h"
#include "layer/GUILayer.h"
#include "event/SandboxControls.h"
#include "gui/GUIParser.h"
#include "layer/PriorGenLayer.h"

#ifdef ND_DEBUG
constexpr char* title = "Niceday - Debug";
#else
constexpr char* title = "Niceday - Release";
#endif


class MainEventsLayer :public nd::Layer
{
public:
   MainEventsLayer() = default;
   void onEvent(nd::Event& e) override
   {
	  if (e.getEventType() == nd::Event::EventType::Message)
	  {
		 auto message = dynamic_cast<MessageEvent&>(e);
		 if (message.isTitle("language_change"))
		 {
			 nd::AppLanguages::loadLanguage(message.getText());

			if (message.getText() == "jp")//todo font change should not be here
			{
			   GameFonts::smallFont = nd::FontMatLib::getMaterial("res/fonts/umeboshi.fnt");
			   GameFonts::bigFont = nd::FontMatLib::getMaterial("res/fonts/umeboshi_big.fnt");
			}
			else {
			   GameFonts::smallFont = nd::FontMatLib::getMaterial("res/fonts/andrew_czech.fnt");
			   GameFonts::bigFont = nd::FontMatLib::getMaterial("res/fonts/andrew_big_czech.fnt");
			}
			NBT smallFont;
			smallFont["textMaterial"] = GameFonts::smallFont->name;
			NBT bigFont;
			bigFont["textMaterial"] = GameFonts::bigFont->name;
			 nd::GUIParser::setGlobalStyle("smallFont", smallFont);
			 nd::GUIParser::setGlobalStyle("bigFont", bigFont);
		 }
	  }
   }
};

Sandbox::Sandbox() :
   App()
{
   AppInfo info;
   info.title = title;
   //info.io.enableSCENE = true;
   info.io.enableMONO = true;
   //info.io.enableIMGUI = false;
   init(info);


   /*{
	   std::ofstream fs;
	   size_t size;
	   auto a = ImGui::FontRangeToUTF_16(ImGui::GetIO().Fonts->GetGlyphRangesJapanese(),&size);
	   fs.open("C:/Users/minek/Desktop/japaneseRange.txt", std::ios::out | std::ios::binary);
	   fs.write((const char*)a, size);
	   free((void*)a);
	   fs.close();
   }*/
   // this should not be physical but only getWindow() but it looks better this way
   // whatever just change it in the end please
   getPhysicalWindow()->setIcon(ND_RESLOC("res/images/icon.png"));
   Sprite2D::init();
   Controls::init();
   m_LayerStack.pushLayer(new MainLayer());


   //language setup
   nd::AppLanguages::registerLanguage("English", "en");
   nd::AppLanguages::registerLanguage("Cestina", "cs");
   nd::AppLanguages::registerLanguage("Nihongo", "jp");
   nd::AppLanguages::addLanguageFolder(ND_RESLOC("res/lang"));
   std::string l;
   getSettings().loadSet("language", l, "en");
   m_LayerStack.pushOverlay(new MainEventsLayer());
   fireEvent(MessageEvent("language_change", getSettings()["language"].string()));

   m_guiLayer = new GUILayer();
   auto worudo = new WorldLayer();
   m_guiLayer->setWorldLayer(worudo);
   m_LayerStack.pushLayer(worudo);
   m_LayerStack.pushOverlay(m_guiLayer);
   //m_LayerStack.pushLayer(new PriorGenLayer());
}

