#include "ConsoleTestLayer.h"
#include "graphics/GContext.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/FontMaterial.h"
#include "core/App.h"
#include "event/KeyEvent.h"
#include "graphics/Effect.h"
#include "graphics/font/TextBuilder.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "core/AppGlobals.h"
#include "imgui.h"

#include "imgui_internal.h"
#include "files/FUtil.h"

using namespace nd;

static TextMesh textMesh;
static int cursorBlink = 10;
constexpr int cursorBlinkRate = 30;
static bool cursorEnable = false;
static std::string randomText;
GaussianBlur* blur;
constexpr int MAX_LINES = 20;

float line=0;
int rollme = 0;
const float lineSpeed = 0.003;

std::string SHADER_PATH;
static Texture* image;
int ConsoleTestLayer::sizeOfLines()
{
	int out = 0;
	for (auto& line : lines)
		out += line.size();
	return out;
}

void ConsoleTestLayer::rebuildMesh(bool cursorAtEnd)
{
	if (cursorAtEnd)
		cursor->cursorPos = sizeOfLines();
	TextBuilder::buildMesh(lines, *fontMat->font, textMesh, TextBuilder::ALIGN_LEFT, {-1000, -1000, 10000, 10000},
	                       cursor);
}


static int currentLetter = 0;

void ConsoleTestLayer::randomWriter()
{
	if (currentLetter < randomText.size())
	{
		char c = randomText[currentLetter++];
		if (c == '\n')
			onEvent(KeyPressEvent(KeyCode::ENTER, 0, false));
		else
			onEvent(KeyTypeEvent(randomText[currentLetter],0));
	}
	else
	{
	    currentLetter = 0;

		auto list = FUtil::fileList("C:/D/Dev/NiceDay/NiceDay/src",FUtil::FileSearchFlags_OnlyFiles | FUtil::FileSearchFlags_Recursive);
		if (list.empty())
			return;
		randomText = list[std::rand() % list.size()];
		ND_BUG("Printing file: {}", randomText);
		randomText = FUtil::readFileString(randomText);

	}
}

static std::string exec(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

std::string ConsoleTestLayer::command(const std::string& command)
{
	ND_TRACE("Command issued: {}", command);
	return exec(command.c_str());
}


void ConsoleTestLayer::onAttach()
{

	SHADER_PATH = ND_RESLOC("res/shaders/Deformation.shader");
	ZeroMemory(chars, BUFF_LINES_COUNT * BUFF_LINE_SIZE);
	image = Texture::create(TextureInfo(ND_RESLOC("res/images/gui_back.png")));
	cursor = new CursorProp;
	cursor->cursorCharacter = '*';
	cursor->cursorPos = 20;

	fontMat = FontMatLib::getMaterial("res/fonts/consolas.fnt");
	textMesh.reserve(50000);
	lines.emplace_back(U"#008800Line1 is nice");
	lines.emplace_back(U"Line2 is nicer no doubt about this \nreally this seems to be somehing else");
	lines.emplace_back(U"#008800ls");
	rebuildMesh();

	deformationShader = Shader::create(SHADER_PATH);
	deformationEffect = new SingleTextureEffect
		(TextureInfo().size(1920, 1080).wrapMode(TextureWrapMode::CLAMP_TO_EDGE));
	fbos = new FrameBufferPingPong(TextureInfo().size(1920, 1080).wrapMode(TextureWrapMode::CLAMP_TO_EDGE));
	
	randomText =
		R"(
{
#if USE_MAP_BUF
	m_vbo->unMapPointer();
#else
	m_vbo->bind();
	m_vbo->changeData((char*)m_buff, sizeof(VertexData)*MAX_VERTICES, 0);
	//m_vbo->changeData((char*)m_buff, sizeof(VertexData)*(m_indices_count/6)*4, 0);
#endif
	m_vao->bind();
	m_ibo->bind();
	m_shader->bind();

	for (int i = 0; i < m_textures.size(); ++i)
		m_textures[i]->bind(i);

	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, nullptr));

}
	)";
}

static void showCursor()
{
	cursorBlink = 0;
	cursorEnable = true;
}

void ConsoleTestLayer::onUpdate()
{
	if (m_on_screen_float > 0)
	{
		m_on_screen_float -= 0.07f;
	}
	if (cursorBlink++ > cursorBlinkRate)
	{
		cursorBlink = 0;
		cursorEnable = !cursorEnable;
	}
	if((line-=lineSpeed)<-0.2)
	{
		line = 1.2;
	}
	//randomWriter();
	

	(++rollme)%=360;


	if(rollme<5)
	{
			while (currentLetter < randomText.size() - 1)
			{
				char c = randomText[currentLetter++];
				if (c == '\n') {
					onEvent(KeyPressEvent(KeyCode::ENTER, 0, false));
					break;
				}
				onEvent(KeyTypeEvent(randomText[currentLetter],0));
			}
	}else
	{
		randomWriter();
		randomWriter();
	}
	AppGlobals::get().nbt["corsorEnable"] = cursorEnable;
	AppGlobals::get().nbt["corsorLoca"] = cursor->cursorPos;


	static auto lastTime = FUtil::lastWriteTime(SHADER_PATH);
	static int refreshTime = 30;
	if(refreshTime--==0)
	{
		refreshTime = 30;
        if (auto last = FUtil::lastWriteTime(SHADER_PATH); last!=lastTime)
		{
			lastTime = last;
            try
            {
			    deformationShader = Shader::create(SHADER_PATH);
			    ND_INFO("Reload shader");
            }
            catch (...)
			{
				ND_ERROR("Cannot parse shader");
            }
		}
			
	}
	

}

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(6) 
         << std::hex << i;
  return stream.str();
}

std::u32string randomColor()
{
	int r = std::rand()%0xFFFFFF;
	return U"#" +
		SUtil::utf8toCodePoints(int_to_hex(r));
	
}
void ConsoleTestLayer::onEvent(Event& e)
{
	auto key = KeyPressEvent::getKeyNumber(e);
	
	if (key == KeyCode::F11)
	{
		APwin()->toggleFullscreen();
	}
	

	if (e.handled)
		return;

	char c = (char)KeyTypeEvent::getKeyNumber(e);
	auto currentLine = lines[lines.size() - 1];
	if (c != -1)
	{
		if (c == '\t')
		{
			lines[lines.size() - 1] += U"    ";
		}
		else
		{
			lines[lines.size() - 1] += c;
		}
		showCursor();
		rebuildMesh();
		e.handled = true;
	}
	key = KeyPressEvent::getKeyNumber(e);
	
	if (key == KeyCode::ENTER)
	{
		while (lines.size() > MAX_LINES)
		{
			lines.erase(lines.begin());
		}
		auto color = randomColor();
		//lines[0] = U"#008800";
		//lines.emplace_back(U"#008800");
		lines[0] = color;
		lines.emplace_back(color);

		rebuildMesh();
		e.handled = true;
	}
	else if (key == KeyCode::BACKSPACE)
	{
		lines[lines.size() - 1] = currentLine.substr(0, currentLine.size() - 1);
		showCursor();
		rebuildMesh();
		e.handled = true;
	}
	else if (key == KeyCode::LEFT)
	{
		if ((sizeOfLines() - lines[lines.size() - 1].size()) < cursor->cursorPos - 1)
		{
			cursor->cursorPos--;
			showCursor();
			rebuildMesh(false);
		}

		e.handled = true;
	}
	else if (key == KeyCode::RIGHT)
	{
		if (sizeOfLines() >= cursor->cursorPos)
		{
			cursor->cursorPos++;
			showCursor();
			rebuildMesh(false);
		}

		e.handled = true;
	}
	if (key == KeyCode::R)
	{
		m_on_screen_float = 3;
	}
}


static int blurAmount = 1;

void ConsoleTestLayer::onRender()
{
	
	deformationEffect->defaultBind();
	Gcon.setClearColor(0, 0.1f, 0, 1);
	Gcon.clear(BuffBit::COLOR);
	renderer.begin(deformationEffect->getFBO());
	renderer.push(
		glm::translate(
			glm::mat4(1.f),
			{-1.f, -1.f, 0}));
	renderer.push(
		glm::scale(
			glm::mat4(1.f),
			{2.f / APwin()->getWidth(), 2.f / APwin()->getHeight(), 1}));
	renderer.push(
		glm::translate(
			glm::mat4(1.f),
			{5, APwin()->getHeight() - fontMat->font->lineHeight, 0}));


	renderer.submitText(textMesh, fontMat);

	TextMesh cursorMesh(1);
	cursorMesh.setChar(0,
	                   cursor->positions.x, cursor->positions.y, cursor->positions.z+0.1, cursor->positions.w, 0x008800ff,
	                   0x0,
	                   fontMat->font->getChar(cursor->cursorCharacter));
	cursorMesh.currentCharCount = 1;
	if (cursorEnable)
		renderer.submitText(cursorMesh, fontMat);
	
	renderer.pop(3);
	renderer.flush();
	fbos->bind();
	Gcon.clear(BuffBit::COLOR);
	deformationShader->bind();
	std::static_pointer_cast<nd::internal::GLShader>(deformationShader)->setUniform1f("u_on_screen", m_on_screen_float);
	std::static_pointer_cast<nd::internal::GLShader>(deformationShader)->setUniform1f("u_line", line);
	deformationEffect->getTexture()->bind(0);
	Effect::renderDefaultVAO();
	auto outTex = fbos->getOutputTexture();

	if (blurAmount > 0) {
		fbos->flip();
		Effecto::Blurer::blur(*fbos, outTex, blurAmount);
		outTex = fbos->getOutputTexture();
	}
    Effect::render(outTex, Renderer::getDefaultFBO());
	
	Gcon.enableDepthTest(true);
}

void ConsoleTestLayer::onImGuiRender()
{	
	static bool open = false;
	if (ImGui::Begin("ConsoleTest", &open))
	{
		//ImGui::Checkbox("Look editor", &lookEditor);
		//ImGui::Checkbox("event editor", &eventEditor);
		//ImGui::SliderInt("blur", &blurAmount, 0, 20);
		//ImGui::SliderFloat3("pos", (float*)&editorCam->pos, -10, 10);
		//ImGui::SliderFloat3("angle", (float*)&editorCam->angles, -3.14159f, 3.14159f);
		//ImGui::SliderFloat3("lightPos", (float*)&lightPos, 0, 50);
		//ImGui::ColorPicker3("Dragon color", (float*)&dragon->color);

	}
	ImGui::End();
}
