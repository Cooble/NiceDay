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
		"eros",
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

	Euler e1;
	e1.sim_type = Euler::CPU_BASIC;
	EulerGround g;
	g.resize(8192);
	Euler::EulerSettings s;
	e1.init(g, &s);

	//todo work here
	//e.ero2_simd_old(g);

	NBT stats;

	uint64_t micros1;
	constexpr int cycles = 100;
	{
		TimerStaper t("");
		for (int i = 0; i < cycles && false; ++i)
			e1.step(g);
		micros1 = t.getUS();
	}
	ND_BUG("Euler CPU BASIC took {} us per step", micros1 / cycles);

	Euler e2;
	e2.init(g, &s);
	e2.sim_type = Euler::CPU_SIMD;
	uint64_t micros2;
	{
		TimerStaper t("");
		for (int i = 0; i < cycles && false; ++i)
			e2.step(g);
		micros2 = t.getUS();
	}
	ND_BUG("Euler CPU SIMD took {} us per step", micros2 / cycles);

	{
		Euler e3;
		e3.sim_type = Euler::OPENCL;
		e3.init(g, &s);
		e3.cl->upload_all(g);
		e3.cl->upload_params(g, s);
		uint64_t micros3;
		{
			TimerStaper t("");
			for (int i = 0; i < cycles; ++i)
			{
				e3.step(g);
			}
			e3.stepRender(g);
			micros3 = t.getUS();
		}
		ND_BUG("Euler OPENCL took {} us per step", micros3 / cycles);
	}
	{
		Euler e3;
		e3.sim_type = Euler::OPENCL;
		e3.init(g, &s);
		e3.cl->upload_all(g);
		e3.cl->upload_params(g, s);
		uint64_t micros3;
		{
			TimerStaper t("");
			for (int i = 0; i < cycles; ++i)
			{
				e3.step(g);
				e3.stepRender(g);
			}
			micros3 = t.getUS();
		}
		ND_BUG("Euler OPENCL_EVERY+DOWNLOAD took {} us per step", micros3 / cycles);
	}
}

#endif
/*
#include <CL/cl.h>
#include <iostream>
#include <vector>

const char* kernelSource = R"CLC(
__kernel void vecAdd(__global const float* A, __global const float* B, __global float* C) {
    int id = get_global_id(0);
    C[id] = A[id] + 3.5*B[id];
}
)CLC";

int main() {
    // Size of vectors
    const int N = 10;
    std::vector A(N, 1.0f);
    std::vector B(N, 2.2f);
    std::vector C(N, 0.0f);

    // 1. Get OpenCL platforms
    cl_uint numPlatforms;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

    // 2. Get first GPU device
    cl_device_id device;
    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    // 3. Create OpenCL context
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);

    // 4. Create command queue
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, nullptr);

    // 5. Create buffers
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * N, A.data(), nullptr);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * N, B.data(), nullptr);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        sizeof(float) * N, nullptr, nullptr);

    // 6. Create and build program
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, nullptr);
    clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    // 7. Create kernel
    cl_kernel kernel = clCreateKernel(program, "vecAdd", nullptr);

    // 8. Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);

    // 9. Execute kernel
    size_t globalSize = N;
    clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);

    // 10. Read results
    clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, sizeof(float) * N, C.data(), 0, nullptr, nullptr);

    // 11. Print results
    std::cout << "Result: ";
    for (float v : C) std::cout << v << " ";
    std::cout << std::endl;

    // 12. Cleanup
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}

*/
