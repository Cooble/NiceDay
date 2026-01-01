#include <span>
#include <valarray>

#include "core/App.h"
#include "core/NBT.h"
#include "scene/EditorLayer.h"
#include "terrain/cl_context.h"
#include "terrain/TerrainLayer.h"

#define ND_TERRAIN_APP // if not enabled, debug mode only


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
		m_target_tps = 30;
		init(info);
		auto editor = new EditorLayer();
		m_LayerStack.pushLayer(editor);
		m_LayerStack.pushLayer(new TerrainLayer(*editor));
	}
};

// expect args: size simType cycles
// e.g. "./Terrain 2048 SIMD_PARALLEL 500"
static void benchmarkFromCmd(int argc, char* argv[])
{
	// extract size from args
	int size = 4096;
	int cycles = 100;

	if (argc > 1)
	{
		std::string arg1 = argv[1];
		if (arg1 == "-h" || arg1 == "--help")
		{
			ND_INFO("\nUsage: {} [size] [simType] [cycles]\n\n"
				"Description:\n"
				"  Runs the Euler terrain simulation benchmark.\n\n"
				"Arguments:\n"
				"  size     : Integer grid size (default: 4096)\n"
				"  simType  : Execution backend (default: SIMD_PARALLEL)\n"
				"  cycles   : Number of simulation steps (default: 100)\n\n"
				"Available simTypes:\n"
				"  CPU - basic cpu\n"
				"  PARALLEL - parallel std::for_each\n"
				"  SIMD - AVX512\n"
				"  SIMD_PARALLEL - AVX512 & parallel std::for_each\n"
				"  SIMD_OMP - AVX512 & OpenMP\n"
				"  OPENCL\n"
				, argv[0]);
			std::exit(0);
		}
		size = std::atoi(argv[1]);
		if (!size)
			size = 4096;
	}

	size += 2; //for simd tail ignore

	Euler::SimType t = Euler::SIMD_PARALLEL;
	if (argc > 2)
	{
		std::string_view simTypeStr = argv[2];

		if (simTypeStr == "SIMD_PARALLEL")
		{
			t = Euler::SIMD_PARALLEL;
		}
		else if (simTypeStr == "SIMD")
		{
			t = Euler::CPU_SIMD;
		}
		else if (simTypeStr == "OPENCL")
		{
			t = Euler::OPENCL;
		}
		else if (simTypeStr == "PARALLEL")
		{
			t = Euler::CPU_PARALLEL;
		}
		else if (simTypeStr == "SIMD_OMP")
		{
			t = Euler::SIMD_PARALLEL_OMP;
		}
		else if (simTypeStr == "CPU")
		{
			t = Euler::CPU_BASIC;
		}
	}

	// Parse cycles if provided
	if (argc > 3)
	{
		cycles = std::atoi(argv[3]);
		if (cycles <= 0)
			cycles = 100;
	}

	ND_BUG("Using size: {}", size);
	ND_BUG("Using type: {}", Euler::sim_type_names[t]);
	ND_BUG("Using cycles: {}", cycles);

	Euler e;
	e.sim_type = t;
	EulerGround g;
	g.resize(size);
	Euler::EulerSettings s;
	e.init(g, &s, false);

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



//#undef ND_TERRAIN_APP

#ifdef ND_TERRAIN_APP
int main(int argc, char* argv[])
{
	// CLI if program args
	if (argc > 1)
	{
		Log::init();
		ResourceMan::init();
		benchmarkFromCmd(argc, argv);
		return 0;
	}

	TerrainApp t;
	t.start();
	return 0;
}
#endif










// =====================================================
// Benchmarking utilities
// =====================================================

// Deliberately ignore big sizes for slow sim types, to not spend eternity
bool ignoreBig(size_t size, Euler::SimType type)
{
	switch (type)
	{
	case Euler::CPU_BASIC:
		return size > 3072;
	case Euler::CPU_PARALLEL:
		return size > 3072;
	default:
		return false;
	}
}

// Runs benchmarks for different simulation types and sizes
// Saves results to a CSV file
void measureTypes(const char* filename,
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
			if (ignoreBig(size, type))
				continue;

			EulerGround g;
			g.resize(size);


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
			ND_BUG("Euler size {} simType {} took {} us per step", size, Euler::sim_type_names[e.sim_type], micros / cycles);
			stats[std::to_string(size)][std::string(Euler::sim_type_names[e.sim_type])] += (double)micros / (double)cycles;
		}
	}
	NBT::saveAsCSV(filename, stats, ',', "Size");
}


// no longer used
void measureFlags(std::span<const int> sizes)
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

	for (auto type : { Euler::SIMD_PARALLEL, Euler::CPU_SIMD, Euler::OPENCL })
	{
		e.sim_type = type;
		for (auto size : sizes)
		{
			auto chunkSizes = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
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

// no significant effect of different batch sizes for threads on performance
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

// Texture sizes
static constexpr auto chunkSizes = std::to_array<int>({
	128, 160, 192,
	256, 320, 384,
	512, 640, 768,
	1024, 1280, 1536,
	2048, 2560, 3072,
	4096, 5120, 6144,
	8192, 10240, 12288,
	16384, 20480
	});
static constexpr auto chunkSizesPlus2 = []
	{
		auto arr = chunkSizes;
		for (auto& v : arr)
			v += 2;
		return arr;
	}();

#ifndef ND_TERRAIN_APP
// Standalone benchmark mode
int main(int argc, char* argv[])
{
	Log::init();
	ResourceMan::init();

	// CLI if program args
	if (argc > 1)
	{
		benchmarkFromCmd(argc, argv);
		return 0;
	}

	measureTypes("total2.csv", std::span{chunkSizesPlus2}.first(18), {Euler::CPU_BASIC, Euler::CPU_PARALLEL, Euler::CPU_SIMD, Euler::SIMD_PARALLEL, Euler::SIMD_PARALLEL_OMP, Euler::OPENCL}, 200);
}
#endif
