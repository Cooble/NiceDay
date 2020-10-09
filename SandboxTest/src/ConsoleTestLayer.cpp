#include "ConsoleTestLayer.h"
#include "graphics/GContext.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/FontMaterial.h"
#include "core/App.h"
#include "event/KeyEvent.h"
#include "graphics/Effect.h"
#include "GLFW/glfw3.h"
#include "platform/OpenGL/GLRenderer.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "core/AppGlobals.h"
#include "imgui.h"

#include "imgui_internal.h"

static TextMesh textMesh;
static int cursorBlink = 10;
constexpr int cursorBlinkRate = 30;
static bool cursorEnable = false;
static std::string randomText;
GaussianBlur* blur;
constexpr int MAX_LINES = 20;

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
static int radnomWriteSpeed = 0;

void ConsoleTestLayer::randomWriter()
{
	//if (!(++radnomWriteSpeed % 2))
	//	return;


	if (currentLetter < randomText.size())
	{
		char c = randomText[currentLetter++];
		if (c == '\n')
			onEvent(KeyPressEvent(GLFW_KEY_ENTER, 0, false));
		else
			onEvent(KeyTypeEvent(randomText[currentLetter]));
	}
	else currentLetter = 0;
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
	/*char tmpname[L_tmpnam];
	std::tmpnam(tmpname);
	std::string cmd = command + " >> " + tmpname;
	std::system(cmd.c_str());
	std::ifstream file(tmpname, std::ios::in | std::ios::binary);
	std::string result;
	if (file)
	{
		while (!file.eof())
			result.push_back(file.get());
		file.close();
	}
	remove(tmpname);
	return result;*/
}
static Texture* smallTexture;

static ShaderPtr modelShader;

static Texture* texture;


void ConsoleTestLayer::onAttach()
{
	
	ZeroMemory(chars, BUFF_LINES_COUNT * BUFF_LINE_SIZE);
	image = Texture::create(TextureInfo(ND_RESLOC("res/images/gui_back.png")));
	cursor = new CursorProp;
	cursor->cursorCharacter = '_';
	cursor->cursorPos = 20;

	fontMat = FontMatLib::getMaterial("res/fonts/consolas.fnt");
	textMesh.reserve(50000);
	lines.emplace_back("#008800Line1 is nice");
	lines.emplace_back("Line2 is nicer no doubt about this \nreally this seems to be somehing else");
	lines.emplace_back("#008800ls");
	rebuildMesh();

	deformationShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/Deformation.shader"));
	deformationEffect = new SingleTextureEffect
		(TextureInfo().size(1920, 1080).wrapMode(TextureWrapMode::CLAMP_TO_EDGE));
	fbos = new FrameBufferPingPong(TextureInfo().size(1920, 1080).wrapMode(TextureWrapMode::CLAMP_TO_EDGE));

	
	/*skyTex = Texture::create(TextureInfo(TextureType::_CUBE_MAP,ND_RESLOC("res/images/skymap2/*.png")));

	skyShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/CubeMap.shader"));
	texture = Texture::create(TextureInfo(ND_RESLOC("res/models/vector.png")));
	
	modelShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/Model.shader"));
	modelShader->bind();
	std::static_pointer_cast<GLShader>(modelShader)->setUniform1i("u_material.diffuse", 0);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform1i("u_material.specular", 1);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform1f("u_material.shines", 64);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform3f("u_light.ambient", 0.1f, 0.1f, 0.1f);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform3f("u_light.diffuse", 0.5f, 0.5f, 0.5f);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform3f("u_light.specular", 1.f, 1.f, 1.f);

	std::static_pointer_cast<GLShader>(modelShader)->setUniform1f("u_light.constant", 1.f);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform1f("u_light.linear", 0.0014f);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform1f("u_light.quadratic", 0.000007f);
	
	modelShader->unbind();

	crateTexture = Texture::create(TextureInfo(ND_RESLOC("res/models/crate.png")));
	crateSpecular = Texture::create(TextureInfo(ND_RESLOC("res/models/crate_specular.png")));
	crate.set(Colli::buildMesh(ND_RESLOC("res/models/cube.fbx")));
	

	vector.set(Colli::buildMesh(ND_RESLOC("res/models/vector.obj")));
	//vector.set(MeshFactory::buildFromObj("res/models/vector.obj"));
	//dragon.set(MeshFactory::buildFromObj("res/models/dragon.obj"));
	dragon.set(MeshFactory::readBinaryFile(ND_RESLOC("res/models/dragon.bin")));
	//MeshFactory::writeBinaryFile(ND_RESLOC("res/models/dragon.bin"), *dragon.mesh);
	wire.set(MeshFactory::buildWirePlane(40,40));
	sky.set(MeshFactory::buildCube());*/

	
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
	//randomWriter();
	//randomWriter();
	//randomWriter();
	AppGlobals::get().nbt["corsorEnable"] = cursorEnable;
	AppGlobals::get().nbt["corsorLoca"] = cursor->cursorPos;



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
	std::string currentLine = lines[lines.size() - 1];
	if (c != -1)
	{
		if (c == '\t')
		{
			lines[lines.size() - 1] += "    ";
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
	return;
	if (key == KeyCode::ENTER)
	{
		while (lines.size() > MAX_LINES)
		{
			lines.erase(lines.begin());
		}
		lines[0] = "#008800";
		lines.emplace_back("#008800");

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
	if (key == KeyCode::F11)
	{
		APwin()->setFullScreen(!APwin()->isFullscreen());
	}
}


static int blurAmount = 1;

void ConsoleTestLayer::onRender()
{
	
	deformationEffect->defaultBind();
	Gcon.setClearColor(0, 0.1f, 0, 1);
	Gcon.clear(BuffBit::COLOR);
	renderer.begin(Renderer::getDefaultFBO());
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
	



	renderer.pop();
	renderer.pop();
	renderer.pop();
	renderer.flush();
	deformationEffect->defaultUnbind();
	fbos->bind();
	Gcon.clear(BuffBit::COLOR);
	deformationShader->bind();
	std::static_pointer_cast<GLShader>(deformationShader)->setUniform1f("u_on_screen", m_on_screen_float);
	deformationEffect->getTexture()->bind(0);
	Effect::getDefaultVAO().bind();
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	auto outTex = fbos->getOutputTexture();
	if (blurAmount > 0) {
		fbos->flip();
		//TimerStaper t("b");
		Effecto::Blurer::blur(*fbos, outTex, blurAmount);
		//Effect::render(fbos->getOutputTexture(),);
	}
	else {
		fbos->unbind();
		Renderer::getDefaultFBO()->bind();
		
		Effect::render(outTex, Renderer::getDefaultFBO());
	}
	Gcon.enableDepthTest(true);

	/*
	//transofrms
	glm::mat4 proj(1.f);
	glm::mat4 view(1.f);
	glm::mat4 world(1.f);
	static float rotation = 0;
	rotation += 0.03;
	//world = glm::rotate(world, rotation, { 0, 0, 1 });
	modelShader->bind();
	proj = glm::perspective(glm::quarter_pi<float>(),
		(GLfloat)APwin()->getWidth() / (GLfloat)APwin()->getHeight(),
		1.f, 100.0f);

	//draw sky
	skyShader->bind();
	std::static_pointer_cast<GLShader>(skyShader)->setUniformMat4("u_viewMat", glm::mat4(glm::mat3(camm->getViewMatrix()))*glm::scale(glm::mat4(1.f),{20,20,20}));
	std::static_pointer_cast<GLShader>(skyShader)->setUniformMat4("u_projMat", proj);

	sky.vao->bind();
	skyTex->bind(0);
	glDepthMask(GL_FALSE);
	GLCall(glDisable(GL_CULL_FACE));

	Gcon.cmdDrawArrays(Topology::TRIANGLES, sky.vbo->getSize()/sky.vbo->getLayout().getStride());
	glDepthMask(GL_TRUE);
	GLCall(glEnable(GL_CULL_FACE));

	

	dragon.vao->bind();
	modelShader->bind();
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", glm::rotate(glm::mat4(1.f), rotation,{0,1,0})*world);
	std::static_pointer_cast<GLShader>(modelShader)->setUniformVec3f("u_camera_pos", lookEditor?editorCam->pos:playerCam->pos);
	std::static_pointer_cast<GLShader>(modelShader)->setUniformVec3f("u_light.pos", lightPos);
	
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_viewMat", camm->getViewMatrix());
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_projMat", proj);
	std::static_pointer_cast<GLShader>(modelShader)->setUniformVec4f("u_material.color", dragonColor);
	Gcon.cmdDrawElements(Topology::TRIANGLES, dragon.vio->getCount());

	texture->bind(0);
	vector.vao->bind();
	world = glm::translate(glm::mat4(1.f), {0.1, 0.5, 0.1});
	world = glm::scale(world, {10, 10, 10});
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform4f("u_material.color", 0.2, 1, 1, 1);
	Gcon.cmdDrawElements(Topology::TRIANGLES, vector.vio->getCount());

	crateTexture->bind(0);
	crateSpecular->bind(1);
	crate.vao->bind();
	world = glm::translate(glm::mat4(1.f), { 0.1, 0.5, 0.1 });
	world = glm::scale(world, { 10, 10, 10 });
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world);
	std::static_pointer_cast<GLShader>(modelShader)->setUniform4f("u_material.color", 0,0,0,0);
	Gcon.cmdDrawElements(Topology::TRIANGLES, crate.vio->getCount());

	texture->bind(0);
	vector.vao->bind();
	auto cam = dynamic_cast<EditorCam*>(editorCam);
	if (cam)
	{
		if (cam->fullRotRelative){
			world = glm::translate(glm::mat4(1.f), cam->farPoint);
			world = glm::scale(world, { 2, 4, 2 });
			glm::quat t({3.14159f/2,0,0});
				
			std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world);
			std::static_pointer_cast<GLShader>(modelShader)->setUniform4f("u_material.color", 1, 0, 0, 1);
			std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world * glm::toMat4(t));
			Gcon.cmdDrawElements(Topology::TRIANGLES, vector.vio->getCount());
		}
		if(!lookEditor)
		{
			world = glm::translate(glm::mat4(1.f), cam->pos);
			world = glm::scale(world, { 5, 5, 5 });
			auto rot = cam->getLookingMat();
			
			std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world*rot);
			std::static_pointer_cast<GLShader>(modelShader)->setUniform4f("u_material.color", 0, 1, 0, 1);
			Gcon.cmdDrawElements(Topology::TRIANGLES, vector.vio->getCount());
		}
		
	}


	world = glm::translate(glm::mat4(1.f), {
		                       -20 * 4 + glm::floor(camm->pos.x / 4) * 4, 0,
		                       -20 * 4 + glm::floor(camm->pos.z / 4) * 4
	                       });
	world = glm::scale(world, {4, 1, 4});
	std::static_pointer_cast<GLShader>(modelShader)->setUniform4f("u_material.color", 0.5f, 0.5f, 0.5f, 1);
	std::static_pointer_cast<GLShader>(modelShader)->setUniformMat4("u_worldMat", world);
	wire.vao->bind();
	Gcon.cmdDrawArrays(Topology::LINES, wire.vbo->getSize()/wire.vbo->getLayout().getStride());*/


}

static void ShowExampleAppDockSpace(bool* p_open)
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MyDockSpace", p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	ImGuiID dock_up_id;
	static ImGuiID dock_right_id;
	static ImGuiID dock_left_id=0;
	ImGuiID dock_down_id;
	// DockSpace
	static bool k = true;
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None| ImGuiDockNodeFlags_AutoHideTabBar;
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	/*if (!ImGui::DockBuilderGetNode(dockspace_id)||k) {
		k = false;
		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);

		ImGuiID dock_main_id = dockspace_id;
		dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.05f, nullptr, &dock_main_id);
		dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
		dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
		dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
		dock_down_right_id = ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

		ImGui::DockBuilderDockWindow("Actions", dock_up_id);
		ImGui::DockBuilderDockWindow("Hierarchy", dock_right_id);
		ImGui::DockBuilderDockWindow("Inspector", dock_left_id);
		ImGui::DockBuilderDockWindow("Console", dock_down_id);
		ImGui::DockBuilderDockWindow("Project", dock_down_right_id);
		ImGui::DockBuilderDockWindow("Scene", dock_main_id);

		// Disable tab bar for custom toolbar
		ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_up_id);
		node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		ImGui::DockBuilderFinish(dock_main_id);
	}*/
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

	if (dock_left_id == 0) {
		
		ImGui::DockBuilderRemoveNodeChildNodes(dockspace_id);
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.5f, &dock_left_id, &dock_right_id);
		ImGui::DockBuilderDockWindow("Sup2", dock_right_id);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Docking"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0))     dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			ImGui::Separator();
			if (ImGui::MenuItem("Close DockSpace", NULL, false, p_open != NULL))
				*p_open = false;
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();

	static bool firstOpen = true;
	ImGui::SetNextWindowDockID(dock_left_id, ImGuiCond_Once);
	ImGui::Begin("Sup", &firstOpen);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hello there");
	ImGui::End();

	
	static bool secondOpen = true;
	//ImGui::SetNextWindowDockID(dock_right_id, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::Begin("Sup2", &secondOpen,ImGuiWindowFlags_NoDecoration);
	ImGui::PopStyleVar(2);
	ImGui::Image((ImTextureID)image->getID(), ImVec2(image->width(), image->height()),ImVec2(0,1),ImVec2(1,0));
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Hello here");
	ImGui::End();
}

void ConsoleTestLayer::onImGuiRender()
{
	static bool dockOpen = true;
	//ShowExampleAppDockSpace(&dockOpen);
	
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
