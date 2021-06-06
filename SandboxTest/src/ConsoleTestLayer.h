#pragma once
#include "layer/Layer.h"
#include "graphics/BatchRenderer2D.h"


namespace nd {
struct CursorProp;
class FrameBufferPingPong;
class SingleTextureEffect;
}

constexpr int BUFF_LINE_SIZE=200;
constexpr int BUFF_LINES_COUNT=100;

class ConsoleTestLayer: public nd::Layer
{
private:
	std::vector<std::u32string> lines;
	char chars[BUFF_LINES_COUNT][BUFF_LINE_SIZE];
	nd::FontMaterial* fontMat;
	nd::BatchRenderer2D renderer;
	nd::CursorProp* cursor;
	nd::ShaderPtr deformationShader;
	nd::SingleTextureEffect* deformationEffect;
	nd::FrameBufferPingPong* fbos;
	float m_on_screen_float=2;

	int sizeOfLines();
	void rebuildMesh(bool cursorAtEnd=true);
	void randomWriter();
	std::string command(const std::string& command);
public:
	ConsoleTestLayer() = default;
	~ConsoleTestLayer() = default;

	void onAttach() override;
	void onUpdate() override;
	void onEvent(nd::Event& e) override;
	void onRender() override;
	void onImGuiRender() override;
};

