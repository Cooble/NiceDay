#define ND_TERRAIN_APP

#include <span>
#include <valarray>

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


void measureol(std::span<const int> sizes)
{
	constexpr int flagsTotal = 1;
	constexpr const char* flagNames[] = {
		"rain",
		"flow",
		"eros",
		"slid"
	};
	NBT stats;

	Euler e;

	for (auto type : {Euler::SIMD_PARALLEL, Euler::CPU_SIMD, Euler::OPENCL})
	{
		e.sim_type = type;
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
	}
	NBT::saveAsCSV("sim_times_chunk_sizes.csv", stats, ',', "Size");
}

void measureTypes(
	std::span<const int> sizes,
	std::initializer_list<Euler::SimType> types = {Euler::SIMD_PARALLEL, Euler::CPU_SIMD, Euler::OPENCL},
	int cycles = 80)
{
	NBT stats;

	Euler e;

	for (auto type : types)
	{
		e.sim_type = type;
		for (auto size : sizes)
		{
			EulerGround g;
			g.resize(size);


			Euler::EulerSettings s;

			e.init(g, &s, false);
			uint64_t micros;
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
			ND_BUG("Euler size {} simType {} took {} us per step", size, Euler::sim_type_names[e.sim_type], micros / cycles);
			stats[std::to_string(size)][std::string(Euler::sim_type_names[e.sim_type])] += (double)micros / (double)cycles;
		}
	}
	NBT::saveAsCSV("sim_times_OMP_longerjbjb2.csv", stats, ',', "Size");
}

void measureChunksSimdMD(
	std::span<const int> sizes,
	std::initializer_list<int> chunkSizes = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512},
	int cycles = 80)
{
	NBT stats;

	Euler e;
	e.sim_type = Euler::SIMD_PARALLEL;
	for (auto size : sizes)
	{
		for (auto cs : chunkSizes)
		{
			EulerGround g;
			g.resize(size);
			g.pc.initialize(1, g.height - 1, cs);

			Euler::EulerSettings s;
			e.init(g, &s, false);

			uint64_t micros;
			{
				e.refreshParams(g, s);
				e.step(g); //warmup
				e.step(g);
				e.stepRender(g);

				TimerStaper t("");
				for (int i = 0; i < cycles; ++i)
					e.step(g);
				e.stepRender(g);

				micros = t.getUS();
			}
			ND_BUG("Euler size {}-{} took {} us per step", size, cs, micros / cycles);
			stats[std::to_string(size)][std::to_string(cs)] += (double)micros * 1000.0 / (double)cycles / (double)(size * size);
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


static constexpr auto chunkSizes = std::to_array<int>({
	128, 160, 192,
	256, 320, 384,
	512, 640, 768,
	1024, 1280, 1536,
	2048, 2560, 3072,
	4096, 5120, 6144,
	8192, 10240, 12288,
});
static constexpr auto chunkSizesPlus2 = []
{
	auto arr = chunkSizes;
	for (auto& v : arr)
		v += 2;
	return arr;
}();


// expect args: size simType
// e.g. "./Terrain 2048 SIMD_PARALLEL"
static void fromCmd(int argc, char* argv[])
{
	// extract size from args
	int size = 2048;
	if (argc > 1)
	{
		size = std::atoi(argv[1]);
	}
	size += 2; //for simd tail ignore

	Euler::SimType t = Euler::SIMD_PARALLEL;
	if (argc > 2)
	{
		t =
			std::string(argv[2]) == "SIMD_PARALLEL" ? Euler::SIMD_PARALLEL : 
		(std::string(argv[2]) == "CPU_SIMD" ? Euler::CPU_SIMD : 
			(std::string(argv[2]) == "OPENCL" ? Euler::OPENCL : 
				(std::string(argv[2]) == "CPU_PARALLEL" ? Euler::CPU_PARALLEL 
					: Euler::SIMD_PARALLEL)));
	}
	ND_BUG("Using size: {}", size);
	ND_BUG("Using type: {}", Euler::sim_type_names[t]);

	Euler e;
	e.sim_type = t;
	EulerGround g;
	g.resize(size);
	Euler::EulerSettings s;
	e.init(g, &s, false);
	constexpr int cycles = 100;
	{
		e.refreshParams(g, s);
		TimerStaper t("");
		for (int i = 0; i < cycles; ++i)
			e.step(g);
		e.stepRender(g);
		auto micros = t.getUS();
		ND_BUG("Euler size {} simType {} took {} us per step", size, Euler::sim_type_names[e.sim_type], micros / cycles);
	}
}

int main(int argc, char* argv[])
{
	Log::init();


	fromCmd(argc, argv);
	return 0;


	//measureChunksSimdMD(std::span{chunkSizesPlus2}.first(15));
	measureTypes(std::span{chunkSizesPlus2}.first(21).last(3), {Euler::SIMD_PARALLEL, Euler::CPU_SIMD}, 100);
	return 0;


	//measureChunkSizes();
	//measureStatsOld();
	//measureCacheMisses(size);
	return 0;
}
#endif
