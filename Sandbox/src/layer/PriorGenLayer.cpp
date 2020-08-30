#include "ndpch.h"
#include "PriorGenLayer.h"
#include "core/imgui_utils.h"
#include "graphics/Effect.h"
#include "event/MouseEvent.h"
#include "core/App.h"
#include "world/gen/Genom.h"
#include "core/ImGuiLayer.h"


static NBT settings;
static Genom genom;

PriorGenLayer::PriorGenLayer()
{
	m_tex = Texture::create(TextureInfo().size(10));

	auto& layers = settings["layers"];
	settings["width"] = 720;
	settings["height"] = 480;
	settings["seed"] = (uint64_t)0;
	auto& layer0 = layers[0];
	layer0["freq"] = 1.f;
	layer0["multi"] = 1.f;
	layer0["seed"] = 0;
}

void PriorGenLayer::onAttach()
{
	NBT::loadFromFile("genSettings.json", settings);
	genom.updateNBT(settings);
}

void PriorGenLayer::onDetach()
{
	NBT::saveToFile("genSettings.json", settings);
}

static glm::vec2 textureLocation{};
static glm::vec2 deltaLoc{};
static float textureZoom = 1;
static glm::vec2 lastClick = {};
static bool isDragging = false;

void PriorGenLayer::onUpdate()
{
	/*	static int divider = 1;
		if (divider-- == 0) {
			divider = 30;
			m_gen.m_time += s_time_speed;
			m_gen.genLayer0();
			m_gen.updateTexture(m_tex);
	
		}*/
}


void PriorGenLayer::onRender()
{
	genom.updateTexture(m_tex);
	auto trans = glm::mat4(1.0f);


	trans =
		glm::scale(trans,
		           {1.f / APwin()->getWidth(), 1.f / APwin()->getHeight(), 1});
	trans =
		glm::translate(trans,
		               {
			               (textureLocation.x + deltaLoc.x) * 2 * textureZoom,
			               -(textureLocation.y + deltaLoc.y) * 2 * textureZoom, 0.f
		               });
	trans =
		glm::scale(trans,
		           {m_tex->width() * textureZoom, m_tex->height() * textureZoom, 1});


	Effect::getDefaultShader()->bind();
	auto sh = std::static_pointer_cast<GLShader>(Effect::getDefaultShader());
	sh->bind();
	sh->setUniformMat4("transform", trans);

	Effect::render(m_tex,Renderer::getDefaultFBO());
}


void PriorGenLayer::onImGuiRender()
{
	static bool show = false;
	if (!ImGui::Begin("PriorGen", &show))
	{
		ImGui::End();
		return;
	}

	/*ImGui::InputInt("seed", ((int*)&s_seed));

	ImGui::InputInt("width", &s_width);
	ImGui::InputInt("height", &s_height);
	if (ImGui::Button("Update"))
	{
		m_gen.gen(s_seed, s_height - 40, s_width, s_height);
		delete m_tex;
		m_tex = m_gen.buildTexture();
	}

	ImGui::InputDouble("noise scale x", &m_gen.m_noise_scale_factor_x);
	ImGui::InputDouble("noise scale y", &m_gen.m_noise_scale_factor_y);
	ImGui::SliderDouble("noise cuttof", &m_gen.m_noise_cutoff, -1, 1);
	ImGui::Separator();
	ImGui::SliderDouble("ore start", &m_gen.m_ore_start, 0, s_height);
	ImGui::SliderDouble("ore start full", &m_gen.m_ore_start_full, 0, s_height);
	ImGui::SliderDouble("ore stop partial", &m_gen.m_ore_end_partial, 0, s_height);
	ImGui::SliderDouble("ore stop", &m_gen.m_ore_end, 0, s_height);
	ImGui::SliderDouble("ore cluster size", &m_gen.m_ore_clusterSize, 0, 2);
	//ImGui::SliderDouble("ore cluster number", &m_gen.m_ore_clusterNumber, -5, 10);
	ImGui::SliderDouble("ore scale", &m_gen.m_orescale, 0, 10);
	ImGui::SliderDoubles("ore magn: ", m_gen.m_ore_octave_magnitudes, 5, 0, 5);
	*/

	ImGuiLayer::drawNBT("Settings", settings);
	genom.updateNBT(settings);

	if (m_tex)
	{
		if ((int)settings["width"] != m_tex->width() || (int)settings["height"] != m_tex->height())
		{
			delete m_tex;
			m_tex = genom.buildTexture();
		}
	}
	else m_tex = genom.buildTexture();
	ImGui::End();
}

void PriorGenLayer::onEvent(Event& e)
{
	if (e.getEventType() == Event::EventType::MousePress)
	{
		isDragging = true;
		auto& mouseEvent = dynamic_cast<MousePressEvent&>(e);
		lastClick = mouseEvent.getPos();
	}
	else if (e.getEventType() == Event::EventType::MouseRelease)
	{
		textureLocation += deltaLoc;
		deltaLoc = {0, 0};
		isDragging = false;
	}
	else if (e.getEventType() == Event::EventType::MouseMove && isDragging)
	{
		auto& mouseEvent = dynamic_cast<MouseEvent&>(e);
		deltaLoc = (mouseEvent.getPos() - lastClick) / textureZoom;
		//dragging
	}
	else if (e.getEventType() == Event::EventType::MouseScroll)
	{
		auto& mouseEvent = dynamic_cast<MouseScrollEvent&>(e);
		textureZoom += textureZoom * mouseEvent.getScrollY() / 5.f;
		if (textureZoom < 1)
			textureZoom = 1;
		//dragging
	}
}
