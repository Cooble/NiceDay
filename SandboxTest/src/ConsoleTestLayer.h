#pragma once
#include "layer/Layer.h"
#include "graphics/BatchRenderer2D.h"



constexpr int BUFF_LINE_SIZE=200;
constexpr int BUFF_LINES_COUNT=100;

struct FontMaterial;
struct CursorProp;
class SingleTextureEffect;
class FrameBufferPingPong;
class ConsoleTestLayer: public Layer
{
private:
	std::vector<std::string> lines;
	char chars[BUFF_LINES_COUNT][BUFF_LINE_SIZE];
	FontMaterial* fontMat;
	BatchRenderer2D renderer;
	CursorProp* cursor;
	ShaderPtr deformationShader;
	SingleTextureEffect* deformationEffect;
	FrameBufferPingPong* fbos;
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
	void onEvent(Event& e) override;
	void onRender() override;
	void onImGuiRender() override;
};

