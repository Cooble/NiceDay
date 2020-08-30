#include "MandelBrotLayer.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "imgui.h"
#include "graphics/API/Shader.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/Effect.h"
#include "core/NBT.h"

void FlatCam::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::MousePress)
	{
		dragging = true;
		auto& mouseEvent = dynamic_cast<MousePressEvent&>(e);
		startCursor = mouseEvent.getPos();
		startPos = pos;
	}
	else if (e.getEventType() == Event::EventType::MouseRelease)
	{
		dragging = false;
	}
	else if (e.getEventType() == Event::EventType::MouseMove && dragging)
	{
		auto& mouseEvent = dynamic_cast<MouseEvent&>(e);
		auto e = (mouseEvent.getPos() - startCursor) * unitsPerPixel;
		e.x = -e.x;//flip x
		pos = startPos + e;
	}
	else if (e.getEventType() == Event::EventType::MouseScroll)
	{
		auto& mouseEvent = dynamic_cast<MouseScrollEvent&>(e);
		auto lastU = unitsPerPixel;
		unitsPerPixel -= unitsPerPixel * mouseEvent.getScrollY() * scrollSpeed;
		if (unitsPerPixel < 0)//this should never happen
			unitsPerPixel = lastU;

		//dragging
	}


}

glm::mat4 FlatCam::getProjMatrix()
{
	auto trans = glm::mat4(1.0f);

	glm::vec2 transla = pos - APwin()->getDimensions() / 2.f * unitsPerPixel;
	glm::vec2 scal = APwin()->getDimensions() * unitsPerPixel;

	trans = glm::translate(trans, { transla.x,transla.y, 0 });
	trans = glm::scale(trans, { scal.x,scal.y, 1.f });

	return trans;
}
void FlatCam::imGuiPropsRender()
{
	ImGui::InputFloat2("Pos", (float*)&pos);
	ImGui::InputFloat("Units/Pixel", &unitsPerPixel);
	ImGui::SliderFloat("ScrollSpeed", &scrollSpeed, 0.01f, 0.5f);
}

static bool quats=false;
static FlatCam flatCam;
static ShaderPtr mandelShader;
static int mandelSteps = 20;
static int wrapAfter = 20;
static glm::vec2 dimensions = {};

static NBT settings;

void MandelBrotLayer::onAttach()
{
	NBT::loadFromFile("mandelbrot.settings", settings);
	settings.loadIfExists("x", flatCam.pos.x);
	settings.loadIfExists("y", flatCam.pos.y);
	settings.loadIfExists("zoom", flatCam.unitsPerPixel);
	settings.loadIfExists("iterations", mandelSteps);
	settings.loadIfExists("wrapAfter", wrapAfter);
	settings.loadIfExists("z", dimensions[0]);
	settings.loadIfExists("w", dimensions[1]);
	settings.loadIfExists("quats",quats);
	if(quats)
	{
		mandelShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/MandelBulb.shader"));
	}else 
		mandelShader = ShaderLib::loadOrGetShader(ND_RESLOC("res/shaders/MandelBrot.shader"));

}

void MandelBrotLayer::onDetach()
{
	settings.save("x", flatCam.pos.x);
	settings.save("y", flatCam.pos.y);
	settings.save("zoom", flatCam.unitsPerPixel);
	settings.save("iterations", mandelSteps);
	settings.save("wrapAfter", wrapAfter);
	settings.save("z", dimensions[0]);
	settings.save("w", dimensions[1]);
	settings.save("quats", quats);
	NBT::saveToFile("mandelbrot.settings", settings);
}

void MandelBrotLayer::onImGuiRender()
{
	static bool flatCamOpen = true;
	if (ImGui::Begin("FlatCam", &flatCamOpen))
	{
		flatCam.imGuiPropsRender();
		ImGui::Spacing();
		ImGui::SliderInt("Steps", &mandelSteps, 2, 1000);
		ImGui::SliderInt("WrapAfter", &wrapAfter, 2, 1000);
		ImGui::Spacing();
		if (quats) {
			ImGui::SliderFloat("Z", (float*)&dimensions, -1, 1);
			ImGui::SliderFloat("W", (float*)&dimensions[1], -1, 1);
		}
		if (ImGui::Checkbox("Quaternions", &quats))
		{
			if(quats)
			{
				mandelShader = Shader::create(ND_RESLOC("res/shaders/MandelBulb.shader"));
				
			}else
			{
				mandelShader = Shader::create(ND_RESLOC("res/shaders/MandelBrot.shader"));
			}
		}
	}
	ImGui::End();
}

void MandelBrotLayer::onRender()
{
	//MandelBrot PIe
	mandelShader->bind();

	std::static_pointer_cast<GLShader>(mandelShader)->setUniformMat4("u_uv_trans", flatCam.getProjMatrix());
	std::static_pointer_cast<GLShader>(mandelShader)->setUniform1i("u_steps", mandelSteps);
	std::static_pointer_cast<GLShader>(mandelShader)->setUniform1i("u_wrapAfter", wrapAfter);
	if(quats)
		std::static_pointer_cast<GLShader>(mandelShader)->setUniformVec2f("u_dimensions", dimensions);

	Effect::getDefaultVAO().bind();
	Effect::renderDefaultVAO();
}

void MandelBrotLayer::onEvent(Event& e)
{
	flatCam.onEvent(e);
}


