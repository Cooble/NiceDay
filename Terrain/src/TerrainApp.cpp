#define ND_TERRAIN_APP

#include "core/App.h"
#include "scene/EditorLayer.h"
#include "terrain/TerrainLayer.h"

using namespace nd;

class TerrainApp :public App
{
public:
	TerrainApp()
	{
		AppInfo info;
		info.io.enableSCENE = true;
		info.io.enableIMGUI = true;
		info.io.enableMONO = false;
		m_target_tps = 120;
		init(info);
		auto editor = new EditorLayer();
		m_LayerStack.pushLayer(editor);
		m_LayerStack.pushLayer(new TerrainLayer(*editor));

	}
};

//#undef ND_TERRAIN_APP

#ifdef ND_TERRAIN_APP
int main()
{
	Log::init();

	TerrainApp t;

	t.start();

	return 0;

}
#else
int main()
{
    Log::init();

    Euler e;
    EulerGround g;
    g.resize(1024);
    e.init(g);

    //todo work here
    //e.ero2_simd(g);


    e.step(g);
    e.step(g);
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