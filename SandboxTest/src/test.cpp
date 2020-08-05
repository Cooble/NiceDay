#define ND_TEST
#include "core/App.h"

#include "graphics/API/Shader.h"
#include "scene/SceneLayer.h"

class TestApp:public App
{
public:
	TestApp()
	{
		//m_LayerStack.pushLayer(new GUITestLayer());
		AppInfo info;
		info.io.enableSCENE = true;
		info.io.enableIMGUI= true;
		
		init(info);
		//m_LayerStack.pushLayer(new ConsoleTestLayer());
		m_LayerStack.pushLayer(new SceneLayer());
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