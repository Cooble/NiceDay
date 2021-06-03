#define ND_TEST
#include "ConsoleTestLayer.h"
#include "core/App.h"

#include "scene/EditorLayer.h"

class TestApp:public App
{
public:
	TestApp()
	{
		//m_LayerStack.pushLayer(new GUITestLayer());
		AppInfo info;
		info.io.enableSCENE = false;
		info.io.enableIMGUI= false;
		
		init(info);
		m_LayerStack.pushLayer(new ConsoleTestLayer());
		//m_LayerStack.pushLayer(new EditorLayer());
		//m_LayerStack.pushLayer(new TestEnntLayer());
		//m_LayerStack.pushLayer(new MandelBrotLayer());
	}
	
};


#ifdef ND_TEST
int main()
{
	Log::init();

	
	
	TestApp t;

	t.start();
	
	return 0;
	
}
#endif