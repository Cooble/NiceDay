#pragma once
#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#include "EulerSim.h"  // contains all 7 kernels guarded by #ifdef __OPENCL_VERSION__

// ---------------------------------------------------------
// GPU Context and Buffers
// ---------------------------------------------------------
class EroCLContext {
public:
    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
    cl::Program program;

    // GPU Buffers corresponding to EulerGround

    struct {
        cl::Buffer
            buf_terrain_height,
            buf_new_terrain_height,
            buf_original_height,
            buf_water_height,
            buf_sediment,
            buf_new_sediment,
            buf_flux,
            buf_velocity,
            buf_perlin_map;
	} b;
    // Kernel handles
    std::vector<cl::Kernel> kernels;

    int width=0, height=0;

    void init(EulerGround& g, Euler::EulerSettings& s);
    void upload_params(EulerGround& g, Euler::EulerSettings& s);

    void upload_all(EulerGround& g);
    void download_all(EulerGround& g);
    void download_graphics(EulerGround& g);


    void step(Euler::EulerSettings& s);

private:
    cl::Platform platform;
    bool initialized = false;
    void initializeContext();
};
