#include "DropletSim.h"

#include <glm/vec2.hpp>
#include <glm/gtx/quaternion.hpp>

#include "imgui.h"
#include "glm/gtc/noise.hpp"

#include "TUtils.h"
#include "core/Core.h"
#include "TerrainLayer.h"


void Droplet::init(Ground& g)
{
	auto w = g.width;
	auto h = g.height;
	pos.x = (std::rand() % (w - 2)) + 1;
	pos.y = (std::rand() % (h - 2)) + 1;
	oldHeight = ter::interpolate2D(g.terrain_height, w, h, pos.x, pos.y);
	sediment = 0;
	water = 1;
	direction = gvec2(0, 0.1);
	speed = 0;
}

bool Droplet::step(Ground& g)
{
	using namespace ter;
	auto w = g.width;
	auto h = g.height;

	// 2. gradient
	grad = interPol(
		gradAt(g.terrain_height, (int)pos.x, (int)pos.y, w),
		gradAt(g.terrain_height, (int)pos.x, (int)(pos.y + 1), w),
		gradAt(g.terrain_height, (int)(pos.x + 1), (int)pos.y, w),
		gradAt(g.terrain_height, (int)(pos.x + 1), (int)(pos.y + 1), w),
		pos - (gvec2)glm::ivec2(pos)
	);


	// 3. update velocity
	auto dirRaw = direction * (1 - pMomentum) - grad * pMomentum;
	direction = glm::normalize(dirRaw);

	// handle result of normalize(0)
	if (glm::length2(dirRaw) < 0.000000000001)
		direction = { 0, 1 };


	auto oldPos = glm::ivec2(pos);
	auto oldOffset = pos - gvec2(oldPos);

	// 4. move to next cell based on velocity
	pos += glm::normalize(direction);

	auto newPos = glm::ivec2(pos);
	if (glm::clamp(pos, (gfloat)0, (gfloat)(w - 1.01)) != pos)
		return false; //we are out of map


	auto newHeight = interpolate2D(g.terrain_height, w, h, pos.x, pos.y);
	auto heightDiff = newHeight - oldHeight;

	capacity = glm::max(-heightDiff * speed * water * pCapacity, pMinSlope);

	if (sediment > capacity || heightDiff > 0)
	{
		//wanna deposit
		auto toDeposit = (heightDiff > 0) ? glm::min(heightDiff, sediment) : (sediment - capacity) * pDeposition;
		sediment -= toDeposit;


		// deposit at interpolated old Position
		g.terrain_height[oldPos.y * w + oldPos.x] += toDeposit * (1 - oldOffset.x) * (1 - oldOffset.y);
		g.terrain_height[oldPos.y * w + oldPos.x + 1] += toDeposit * oldOffset.x * (1 - oldOffset.y);
		g.terrain_height[(oldPos.y + 1) * w + oldPos.x] += toDeposit * (1 - oldOffset.x) * oldOffset.y;
		g.terrain_height[(oldPos.y + 1) * w + oldPos.x + 1] += toDeposit * oldOffset.x * oldOffset.y;
	}
	else
	{
		// wanna erode
		auto toErode = glm::min(-heightDiff, (capacity - sediment) * pErosion);

		auto& t00 = g.terrain_height[oldPos.y * w + oldPos.x];
		auto& t01 = g.terrain_height[oldPos.y * w + oldPos.x + 1];
		auto& t10 = g.terrain_height[(oldPos.y + 1) * w + oldPos.x];
		auto& t11 = g.terrain_height[(oldPos.y + 1) * w + oldPos.x + 1];

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


		gfloat o = g.terrain_height[oldPos.y * w + oldPos.x];
		ASSERT(o >= 0, "fock");

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
	if (ImGui::BeginTable("Droplet", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
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
