#define ND_TERRAIN_APP

#include "core/App.h"
#include "core/NBT.h"
#include "scene/EditorLayer.h"
#include "terrain/cl_context.h"
#include "terrain/TerrainLayer.h"

using namespace nd;

class TerrainApp : public App
{
public:
	TerrainApp()
	{
		AppInfo info;
		info.io.enableSCENE = true;
		info.io.enableIMGUI = true;
		info.io.enableMONO = false;
		m_target_tps = 60;
		init(info);
		auto editor = new EditorLayer();
		m_LayerStack.pushLayer(editor);
		m_LayerStack.pushLayer(new TerrainLayer(*editor));
	}
};

#undef ND_TERRAIN_APP

#ifdef ND_TERRAIN_APP
int main()
{
	Log::init();

	TerrainApp t;

	t.start();

	return 0;
}
#else

void measureStatsOld()
{
	//std::vector sizes = { 128, 256, 512, 1024, 2048, 4096, 8192 };
	std::vector sizes = {
		128, /* 160, 192, // ~2^7 range
		256, 320, 384, // ~2^8 range
		512, 640, 768, // ~2^9 range
		1024, 1280, 1536, // ~2^10 range
		2048, 2560, 3072, // ~2^11 range
		4096, 5120, 6144, // ~2^12 range
		8192 /*, 10240, 12288, // ~2^13 range
		16384, 20480*/
	};
	// to allow simd to ignore tails
	for (auto& a : sizes)
		a += 2;

	constexpr int flagsTotal = 1;
	constexpr const char* flagNames[] = {
		"rain",
		"flow",
		"evap",
		"slid"
	};


	NBT stats;
	//auto sim_types = {Euler::CPU_BASIC, Euler::CPU_SIMD, Euler::OPENCL};
	auto sim_types = {Euler::CPU_BASIC, Euler::CPU_SIMD, Euler::CPU_PARALLEL, Euler::SIMD_PARALLEL, Euler::OPENCL};

	for (int turn = 0; turn < 1; ++turn)
		for (auto simType : sim_types)
		{
			Euler e;
			e.sim_type = simType;
			for (auto size : sizes)
			{
				// skip waiting for Godot
				if (simType == Euler::CPU_BASIC && size > 4000)
					continue;
				if (simType == Euler::CPU_PARALLEL && size > 4000)
					continue;
				if (simType == Euler::CPU_SIMD && size > 10240)
					continue;


				EulerGround g;
				g.resize(size);

				for (int flagIdx = 0; flagIdx < flagsTotal; flagIdx++)
				{
					Euler::EulerSettings s;
					// disable all
					//ZeroMemory(&s.e_rain, sizeof(bool) * flagsTotal);
					// enable one
					//bool* flagPtr = &s.e_rain + flagIdx;
					//*flagPtr = true;


					e.init(g, &s, false);
					uint64_t micros;
					constexpr int cycles = 80;
					{
						e.refreshParams(g, s);
						e.step(g); //warmup
						e.step(g);

						TimerStaper t("");
						for (int i = 0; i < cycles; ++i)
							e.step(g);
						e.stepRender(g);

						micros = t.getUS();
					}
					ND_BUG("Euler size {} simType {}-{} took {} us per step", size, Euler::sim_type_names[simType], flagNames[flagIdx], micros / cycles);
					stats[std::to_string(size)][std::string(Euler::sim_type_names[simType])] += (double)micros / (double)cycles;
				}
			}
		}
	NBT::saveAsCSV("sim_times_std_par.csv", stats, ',', "Size");
}

void measureChunkSizes()
{
	//std::vector sizes = { 128, 256, 512, 1024, 2048, 4096, 8192 };
	std::vector sizes = {
		128, 160, 192, // ~2^7 range
		256, 320, 384, // ~2^8 range
		512, 640, 768, // ~2^9 range
		1024, 1280, 1536, // ~2^10 range
		2048, 2560, 3072, // ~2^11 range
		4096, 5120, 6144, // ~2^12 range
		8192 /*, 10240, 12288, // ~2^13 range
		16384, 20480*/
	};
	// to allow simd to ignore tails
	for (auto& a : sizes)
		a += 2;

	constexpr int flagsTotal = 1;
	constexpr const char* flagNames[] = {
		"rain",
		"flow",
		"eros",
		"evap",
		"slid"
	};


	NBT stats;

	Euler e;
	e.sim_type = Euler::SIMD_PARALLEL;
	for (auto size : sizes)
	{
		auto chunkSizes = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
		for (auto cs : chunkSizes)
		{
			EulerGround g;
			g.resize(size);
			g.pc.initialize(1, g.height - 1, cs);


			for (int flagIdx = 0; flagIdx < flagsTotal; flagIdx++)
			{
				Euler::EulerSettings s;
				// disable all
				//ZeroMemory(&s.e_rain, sizeof(bool) * flagsTotal);
				// enable one
				//bool* flagPtr = &s.e_rain + flagIdx;
				//*flagPtr = true;


				e.init(g, &s, false);
				uint64_t micros;
				constexpr int cycles = 80;
				{
					e.refreshParams(g, s);
					e.step(g); //warmup
					e.step(g);

					TimerStaper t("");
					for (int i = 0; i < cycles; ++i)
						e.step(g);
					e.stepRender(g);

					micros = t.getUS();
				}
				ND_BUG("Euler size {} simType {}-{} took {} us per step", size, Euler::sim_type_names[e.sim_type], flagNames[flagIdx], micros / cycles);
				stats[std::to_string(size)][std::string(Euler::sim_type_names[e.sim_type]) + std::string("_") + std::to_string(cs)] += (double)micros / (double)cycles;
			}
		}
	}
	NBT::saveAsCSV("sim_times_chunk_sizes.csv", stats, ',', "Size");
}

void measureCacheMisses(int size)
{
	size += 2; //for simd tail ignore
	Euler e;
	e.sim_type = Euler::SIMD_PARALLEL;

	EulerGround g;
	g.resize(size);


	Euler::EulerSettings s;


	e.init(g, &s, false);
	uint64_t micros;
	constexpr int cycles = 100;
	{
		e.refreshParams(g, s);
		e.step(g); //warmup
		e.step(g);

		TimerStaper t("");
		for (int i = 0; i < cycles; ++i)
			e.step(g);
		e.stepRender(g);

		micros = t.getUS();
	}
}

int main(int argc, char* argv[])
{
	Log::init();

	// extract size from args
	int size = 1024;
	if (argc > 1 )
	{
		size = std::atoi(argv[1]);
		ND_BUG("Using size from args: {}", size);
	}

	//measureChunkSizes();
	//measureStatsOld();
	measureCacheMisses(size);
	return 0;
}
#endif