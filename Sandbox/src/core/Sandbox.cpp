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


class MainEventsLayer :public Layer
{
public:
   MainEventsLayer() = default;
   void onEvent(Event& e) override
   {
	  if (e.getEventType() == Event::EventType::Message)
	  {
		 auto message = dynamic_cast<MessageEvent&>(e);
		 if (message.isTitle("language_change"))
		 {
			AppLanguages::loadLanguage(message.getText());

			if (message.getText() == "jp")//todo font change should not be here
			{
			   GameFonts::smallFont = FontMatLib::getMaterial("res/fonts/umeboshi.fnt");
			   GameFonts::bigFont = FontMatLib::getMaterial("res/fonts/umeboshi_big.fnt");
			}
			else {
			   GameFonts::smallFont = FontMatLib::getMaterial("res/fonts/andrew_czech.fnt");
			   GameFonts::bigFont = FontMatLib::getMaterial("res/fonts/andrew_big_czech.fnt");
			}
			NBT smallFont;
			smallFont["textMaterial"] = GameFonts::smallFont->name;
			NBT bigFont;
			bigFont["textMaterial"] = GameFonts::bigFont->name;
			GUIParser::setGlobalStyle("smallFont", smallFont);
			GUIParser::setGlobalStyle("bigFont", bigFont);
		 }
	  }
   }
};

Sandbox::Sandbox() :
   App()
{
   AppInfo info;
   info.title = title;
   info.io.enableSCENE = true;
   info.io.enableMONO = true;
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
   AppLanguages::registerLanguage("English", "en");
   AppLanguages::registerLanguage("Cestina", "cs");
   AppLanguages::registerLanguage("Nihongo", "jp");
   AppLanguages::addLanguageFolder(ND_RESLOC("res/lang"));
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

