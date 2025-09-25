#include "DropletSim.h"

#include <ImGuiFileDialog.h>
#include <glm/vec2.hpp>
#include <glm/gtx/quaternion.hpp>

#include "imgui.h"
#include "glm/gtc/noise.hpp"

#include "TUtils.h"
#include "core/Core.h"
#include "TerrainLayer.h"
#include "core/NBT.h"




#define REINTERPRET_AS(Type, Value) (*reinterpret_cast<Type*>(&Value))

int Droplet::balls = 0;

void Droplet::init(BaseGround& g)
{
	auto w = g.width;
	auto h = g.height;
	pos.x = (std::rand() % (w - 2)) + 1;
	pos.y = (std::rand() % (h - 2)) + 1;
	oldHeight = ter::interpolate2D(REINTERPRET_AS(std::vector<gfloat>, g.terrain_height), w, h, pos.x, pos.y);
	sediment = 0;
	water = 1;
	direction = gvec2(0, 0.1);
	speed = 0;

	kernel.resize((MAX_RADIUS * 2 + 1) * (MAX_RADIUS * 2 + 1));
	balls++;
}




void erodePrimitive(std::vector<gfloat>& terrain, glm::ivec2 oldipos, gvec2 oldOffset,int w, gfloat heightDiff, gfloat capacity, gfloat& sediment, gfloat pErosion)
{
	// we can only erode if we have enough capacity and the height difference is negative
	if (capacity <= 0 || heightDiff >= 0)
		return;

	// calculate how much terrain we can erode (cannot go lower than 0)
	auto toErode = glm::min(-heightDiff, (capacity - sediment) * pErosion);

	auto& t00 = terrain[oldipos.y * w + oldipos.x];
	auto& t01 = terrain[oldipos.y * w + oldipos.x + 1];
	auto& t10 = terrain[(oldipos.y + 1) * w + oldipos.x];
	auto& t11 = terrain[(oldipos.y + 1) * w + oldipos.x + 1];

	// calculate how much terrain we can erode (cannot go lower than 0)
	auto e00 = glm::min(t00, toErode * (1 - oldOffset.x) * (1 - oldOffset.y));
	auto e01 = glm::min(t01, toErode * oldOffset.x * (1 - oldOffset.y));
	auto e10 = glm::min(t10, toErode * (1 - oldOffset.x) * oldOffset.y);
	auto e11 = glm::min(t11, toErode * oldOffset.x * oldOffset.y);

	auto reallyEroded = e00 + e01 + e10 + e11;
	sediment += reallyEroded;

	t00 -= e00;
	t01 -= e01;
	t10 -= e10;
	t11 -= e11;
}
void erodeInRadius(std::vector<gfloat>& terrain, gvec2 pos, int w, int radius, std::vector<gfloat>& kernel, gfloat& sediment,gfloat pErosion,gfloat capacity,gfloat heightDiff)
{
	// we can only erode if we have enough capacity and the height difference is negative
	if (capacity <= 0 || heightDiff >= 0)
		return;

	// calculate how much terrain we can erode (cannot go lower than 0)
	auto maxErode = glm::min(-heightDiff, (capacity - sediment) * pErosion);


	// based on distance from origin
	auto x0 = (int)std::max(0.f, pos.x - radius);
	auto y0 = (int)std::max(0.f, pos.y - radius);
	auto x1 = std::min(w, x0 + 2 * radius + 1);
	auto y1 = std::min(w, y0 + 2 * radius + 1);

	auto rowSize = x1 - x0;

	// prepare kernel, actually calculate totalMass
	gfloat totalMass = 0;
	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			auto kernelIdx = (y - y0) * rowSize + (x - x0);
			auto dist = glm::distance(gvec2(x, y), pos);
			//if (dist < radius)
			//{
			kernel[kernelIdx] = glm::max((gfloat)0, 1 - dist / radius);
			totalMass += kernel[kernelIdx];
			//}
		}
	}
	// erode
	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			auto idx = y * w + x;
			auto kernelIdx = (y - y0) * rowSize + (x - x0);

			auto toErode = glm::min(terrain[idx], maxErode * kernel[kernelIdx] / totalMass);
			terrain[idx] -= toErode;
			sediment += toErode;
		}
	}
}

bool Droplet::step(BaseGround& g)
{
	using namespace ter;
	auto w = g.width;
	auto h = g.height;

	auto& terrain_height = REINTERPRET_AS(std::vector<gfloat>, g.terrain_height);
	// 2. gradient
	grad = interPol(
		gradAt(terrain_height, (int)pos.x, (int)pos.y, w),
		gradAt(terrain_height, (int)pos.x, (int)(pos.y + 1), w),
		gradAt(terrain_height, (int)(pos.x + 1), (int)pos.y, w),
		gradAt(terrain_height, (int)(pos.x + 1), (int)(pos.y + 1), w),
		pos - (gvec2)glm::ivec2(pos)
	);


	// 3. update velocity
	auto dirRaw = direction * (1 - pMomentum) - grad * pMomentum;
	direction = glm::normalize(dirRaw);

	// handle result of normalize(0)
	if (glm::length2(dirRaw) < 0.000000000001)
		direction = {0, 1};


	auto oldipos = glm::ivec2(pos);
	auto oldpos = pos;
	auto oldOffset = pos - gvec2(oldipos);

	// 4. move to next cell based on velocity
	pos += direction;

	auto newPos = glm::ivec2(pos);
	if (glm::clamp(pos, (gfloat)0, (gfloat)(w - 1.01)) != pos)
		return false; //we are out of map


	auto newHeight = interpolate2D(REINTERPRET_AS(std::vector<gfloat>,g.terrain_height), w, h, pos.x, pos.y);
	auto heightDiff = newHeight - oldHeight;

	capacity = glm::max(-heightDiff * speed * water * pCapacity, pMinSlope);

	if (sediment > capacity || heightDiff > 0)
	{
		//wanna deposit
		auto toDeposit = (heightDiff > 0) ? glm::min(heightDiff, sediment) : (sediment - capacity) * pDeposition;
		sediment -= toDeposit;


		// deposit at interpolated old Position
		g.terrain_height[oldipos.y * w + oldipos.x] += toDeposit * (1 - oldOffset.x) * (1 - oldOffset.y);
		g.terrain_height[oldipos.y * w + oldipos.x + 1] += toDeposit * oldOffset.x * (1 - oldOffset.y);
		g.terrain_height[(oldipos.y + 1) * w + oldipos.x] += toDeposit * (1 - oldOffset.x) * oldOffset.y;
		g.terrain_height[(oldipos.y + 1) * w + oldipos.x + 1] += toDeposit * oldOffset.x * oldOffset.y;
	}
	else
	{
		// wanna erode
		if(pRadius >0)
			erodeInRadius(REINTERPRET_AS(std::vector<gfloat>,g.terrain_height), pos, w, pRadius, kernel, sediment, pErosion, capacity, heightDiff);
		else
			erodePrimitive(REINTERPRET_AS(std::vector<gfloat>, g.terrain_height), oldipos, oldOffset, w, heightDiff, capacity, sediment, pErosion);
	}

	speed = glm::sqrt(glm::max((gfloat)0, speed * speed - heightDiff * pGravity));

	if (speed < 0.00000001) //tno speed pick some random direction
		direction = gvec2(rand() % 64 - 32, rand() % 64 - 32);

	water *= 1 - pEvaporation;

	oldHeight = newHeight;

	return water > 0.01; // if no water is left this is the end
}

void Droplet::imguiRender()
{
	// ======== SLIDERS

	ImGui::PushID(123);
	ImGui::Text(" Balls: %d", balls);
	ter::SliderGFloat("Momentum", &pMomentum, 0, 1, "%.2f");
	ImGui::SetItemTooltip("How much droplet keeps its direction\nHigher values lead to deeper ravines");
	ter::SliderGFloat("Min Slope", &pMinSlope, 0, 1, "%.2f");
	ImGui::SetItemTooltip("Minimal Capacity\nDroplet can erode even on flatter terrain");
	ter::SliderGFloat("Capacity", &pCapacity, 0, 10, "%.2f");
	ImGui::SetItemTooltip("Sediment Carrying Capacity of a droplet");
	ter::SliderGFloat("Deposition", &pDeposition, 0, 1, "%.2f");
	ter::SliderGFloat("Erosion", &pErosion, 0, 1, "%.2f");
	ter::SliderGFloat("Evaporation", &pEvaporation, 0, 1, "%.2f");
	ter::SliderGFloat("Gravity", &pGravity, 0, 10, "%.2f");
	ImGui::SliderInt("Erosion Radius", &pRadius, 0, MAX_RADIUS);
	ImGui::SetItemTooltip("Radius of erosion, if 0 then erosion is done only on the 4 cells around the droplet\n"
		"Higher values lead to more erosion, but also more performance cost");
	ImGui::PopID();

	// ======== CONFIG SERIALIZATION
	if (ImGui::Button("Load Config"))
	{
		IGFD::FileDialogConfig config;
		config.path = ".";
		ImGuiFileDialog::Instance()->OpenDialog("LoadConfig", "Choose config file to open", ".json", config);
	}
	ImGui::SetItemTooltip("Load simulation parameters from file\n"
		"Config is a json file with all the parameters of the droplet simulation");
	ImGui::SameLine();
	if (ImGui::Button("Save Config"))
	{
		IGFD::FileDialogConfig config;
		config.path = ".";
		ImGuiFileDialog::Instance()->OpenDialog("SaveConfig", "Choose config file to save", ".json", config);
	}
	ImGui::SetItemTooltip("Save simulation parameters to file\n"
		"Config is a json file with all the parameters of the droplet simulation");

	if (ImGuiFileDialog::Instance()->Display("LoadConfig"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			ND_BUG("Loading config from {}", filePathName);
			nd::NBT nbt;
			nd::NBT::loadFromFile(filePathName, nbt);
			load(nbt);
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("SaveConfig"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			ND_BUG("Saving config to {}", filePathName);
			nd::NBT nbt;
			save(nbt);
			nd::NBT::saveToFile(filePathName, nbt);
		}
		ImGuiFileDialog::Instance()->Close();
	}


	// ======== INFO

	if (ImGui::BeginTable("Droplet", 2,
	                      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ter::ImGuiAddTableRow("Pos", "%.1f,%.1f", pos.x, pos.y);
		ter::ImGuiAddTableRow("Height", "%.1f", oldHeight);
		ter::ImGuiAddTableRow("Direction", "%.1f,%.1f", direction.x, direction.y);
		ter::ImGuiAddTableRow("Speed", "%.1f", speed);
		ter::ImGuiAddTableRow("Gradient", "%.1f,%.1f", grad.x, grad.y);
		ter::ImGuiAddTableRow("Sediment", "%.1f", sediment);
		ter::ImGuiAddTableRow("Water", "%.1f", water);
		ter::ImGuiAddTableRow("Old Height", "%.1f", oldHeight);
		ter::ImGuiAddTableRow("Capacity", "%.1f", capacity);
		ter::ImGuiAddTableRow("To Deposit", "%.1f", toDeposit);
		ter::ImGuiAddTableRow("To Erode", "%.1f", toErode);
		ImGui::EndTable();
	}
}

void Droplet::save(nd::NBT& src)
{
	NBT_SAVE(src, pMomentum);
	NBT_SAVE(src, pMinSlope);
	NBT_SAVE(src, pCapacity);
	NBT_SAVE(src, pDeposition);
	NBT_SAVE(src, pErosion);
	NBT_SAVE(src, pEvaporation);
	NBT_SAVE(src, pGravity);
	NBT_SAVE(src, pRadius);
}

void Droplet::load(nd::NBT& src)
{
	NBT_LOAD(src, pMomentum);
	NBT_LOAD(src, pMinSlope);
	NBT_LOAD(src, pCapacity);
	NBT_LOAD(src, pDeposition);
	NBT_LOAD(src, pErosion);
	NBT_LOAD(src, pEvaporation);
	NBT_LOAD(src, pGravity);
	NBT_LOAD(src, pRadius);
}
