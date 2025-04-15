#define ND_TERRAIN_APP

#include "core/App.h"
#include "scene/EditorLayer.h"
#include "TerrainLayer.h"

using namespace nd;

class TerrainApp :public App
{
public:
	TerrainApp()
	{
		AppInfo info;
		info.io.enableSCENE = true;
		info.io.enableIMGUI = true;

		init(info);
		auto editor = new EditorLayer();
		m_LayerStack.pushLayer(editor);
		m_LayerStack.pushLayer(new TerrainLayer(*editor));
	}
};


#ifdef ND_TERRAIN_APP
int main()
{
	Log::init();

	TerrainApp t;

	t.start();

	return 0;

}
#endif
