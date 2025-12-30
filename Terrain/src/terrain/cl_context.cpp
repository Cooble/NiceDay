#include "ndpch.h"
#include "cl_context.h"

#include "files/FUtil.h"

void EroCLContext::init(EulerGround& g, Euler::EulerSettings& s)
{
	initializeContext();

	if (width == g.width)
	{
		upload_all(g);
		upload_params(g, s);
		return;
	}
	width = g.width;
	height = g.height;


	// Create buffers
	size_t sz = width * height;
	b.buf_terrain_height =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
	b.buf_new_terrain_height =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
	b.buf_flux =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(gvec4) * sz);
	b.buf_sediment =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
	b.buf_new_sediment =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
	b.buf_velocity =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(gvec2) * sz);
	b.buf_water_height =
		cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);

	// read only
	b.buf_original_height =
		cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * sz);
	b.buf_perlin_map =
		cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * sz);


	// Bind buffers to kernels

	//ero1
	kernels[0].setArg(0, b.buf_water_height);

	//ero2
	kernels[1].setArg(0, b.buf_terrain_height);
	kernels[1].setArg(1, b.buf_water_height);
	kernels[1].setArg(2, b.buf_flux);

	//ero3
	kernels[2].setArg(0, b.buf_water_height);
	kernels[2].setArg(1, b.buf_flux);
	kernels[2].setArg(2, b.buf_velocity);

	//ero4
	kernels[3].setArg(0, b.buf_terrain_height);
	kernels[3].setArg(1, b.buf_new_terrain_height);
	kernels[3].setArg(2, b.buf_water_height);
	kernels[3].setArg(3, b.buf_sediment);
	kernels[3].setArg(4, b.buf_velocity);
	kernels[3].setArg(5, b.buf_perlin_map);
	kernels[3].setArg(6, b.buf_original_height);

	//ero5
	kernels[4].setArg(0, b.buf_sediment);
	kernels[4].setArg(1, b.buf_new_sediment);
	kernels[4].setArg(2, b.buf_velocity);

	//ero7
	kernels[5].setArg(0, b.buf_terrain_height);

	upload_all(g);
	upload_params(g, s);
}

void EroCLContext::upload_params(EulerGround& g, Euler::EulerSettings& s)
{
	//ero1
	auto idx = 1;
	kernels[0].setArg(idx++, g.width);
	kernels[0].setArg(idx++, g.height);
	kernels[0].setArg(idx++, s.K_rain);
	kernels[0].setArg(idx++, s.K_evaporation);
	kernels[0].setArg(idx++, s.K_dt);

	//ero2
	idx = 3;
	kernels[1].setArg(idx++, g.width);
	kernels[1].setArg(idx++, g.height);
	kernels[1].setArg(idx++, s.K_dt);

	//ero3
	idx = 3;
	kernels[2].setArg(idx++, g.width);
	kernels[2].setArg(idx++, g.height);
	kernels[2].setArg(idx++, s.K_dt);

	//ero4
	idx = 7;
	kernels[3].setArg(idx++, g.width);
	kernels[3].setArg(idx++, g.height);
	kernels[3].setArg(idx++, s.K_sediment_capacity);
	kernels[3].setArg(idx++, s.K_tilt_minimum);
	kernels[3].setArg(idx++, s.K_s_dissolving);
	kernels[3].setArg(idx++, s.K_d_depositing);

	//ero5
	idx = 3;
	kernels[4].setArg(idx++, g.width);
	kernels[4].setArg(idx++, g.height);
	kernels[4].setArg(idx++, s.K_dt);

	//ero7
	idx = 1;
	kernels[5].setArg(idx++, g.width);
	kernels[5].setArg(idx++, g.height);
	idx++; // offset x
	idx++; // offset y
	kernels[5].setArg(idx++, s.K_landSlideSpeed);
	kernels[5].setArg(idx++, s.K_landSlideCutoffAngle);
	kernels[5].setArg(idx++, s.K_dt);
}

void EroCLContext::upload_all(EulerGround& g)
{
	queue.enqueueWriteBuffer(b.buf_terrain_height, CL_FALSE, 0, sizeof(float) * width * height,
	                         g.terrain_height.data());
	queue.enqueueWriteBuffer(b.buf_new_terrain_height, CL_FALSE, 0, sizeof(float) * width * height,
	                         g.new_terrain_height.data());
	queue.enqueueWriteBuffer(b.buf_original_height, CL_FALSE, 0, sizeof(float) * width * height,
	                         g.original_height.data());
	queue.enqueueWriteBuffer(b.buf_flux, CL_FALSE, 0, sizeof(gvec4) * width * height, g.flux.data());
	queue.enqueueWriteBuffer(b.buf_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.sediment.data());
	queue.enqueueWriteBuffer(b.buf_new_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.new_sediment.data());
	queue.enqueueWriteBuffer(b.buf_velocity, CL_FALSE, 0, sizeof(gvec2) * width * height, g.velocity.data());
	queue.enqueueWriteBuffer(b.buf_water_height, CL_FALSE, 0, sizeof(float) * width * height, g.water_height.data());
	queue.enqueueWriteBuffer(b.buf_perlin_map, CL_FALSE, 0, sizeof(float) * width * height,
		g.perlin_map.data());
	queue.finish();
}

void EroCLContext::download_all(EulerGround& g)
{
	queue.enqueueReadBuffer(b.buf_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.terrain_height.data());
	queue.enqueueReadBuffer(b.buf_new_terrain_height, CL_FALSE, 0, sizeof(float) * width * height,
	                        g.new_terrain_height.data());
	queue.enqueueReadBuffer(b.buf_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.sediment.data());
	queue.enqueueReadBuffer(b.buf_new_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.new_sediment.data());
	queue.enqueueReadBuffer(b.buf_velocity, CL_FALSE, 0, sizeof(gvec2) * width * height, g.velocity.data());
	queue.enqueueReadBuffer(b.buf_flux, CL_FALSE, 0, sizeof(gvec4) * width * height, g.flux.data());
	queue.enqueueReadBuffer(b.buf_water_height, CL_FALSE, 0, sizeof(float) * width * height, g.water_height.data());
	queue.finish();
}

void EroCLContext::download_graphics(EulerGround& g)
{
	queue.finish(); // wait for all operations to finish
	queue.enqueueReadBuffer(b.buf_terrain_height, CL_TRUE, 0, sizeof(float) * width * height, g.terrain_height.data());
	queue.enqueueReadBuffer(b.buf_water_height, CL_TRUE, 0, sizeof(float) * width * height, g.water_height.data());
	queue.finish(); // wait for read to finish
	//ND_BUG("center height : {}", g.terrain_height[700]);
}

void EroCLContext::step(Euler::EulerSettings& s)
{
	// range from 1,1 - width-1 ,height-1 to avoid boundary issues
	cl::NDRange global(width - 2, height - 2);
	cl::NDRange of(1, 1);


	if (s.e_rain)
		queue.enqueueNDRangeKernel(kernels[0], of, global, cl::NullRange);
	if (s.e_flow)
	{
		queue.enqueueNDRangeKernel(kernels[1], of, global, cl::NullRange);
		queue.enqueueNDRangeKernel(kernels[2], of, global, cl::NullRange);
	}
	if (s.e_erosion)
	{
		queue.enqueueNDRangeKernel(kernels[3], of, global, cl::NullRange);
		queue.enqueueCopyBuffer(b.buf_new_terrain_height, b.buf_terrain_height, 0, 0, sizeof(float) * width * height);
		queue.enqueueNDRangeKernel(kernels[4], of, global, cl::NullRange);
		queue.enqueueCopyBuffer(b.buf_new_sediment, b.buf_sediment, 0, 0, sizeof(float) * width * height);
	}
	if (s.e_landslide)
	{
		global = cl::NDRange((width - 1) / 2, (height - 1) / 2);

		// 4 passes for land slide
		for (int pass = 0; pass < 4; pass++)
		{
			kernels[5].setArg(3, pass & 1); // offset x
			kernels[5].setArg(4, (pass >> 1) & 1); // offset y
			queue.enqueueNDRangeKernel(kernels[5], of, global, cl::NullRange);
		}
	}
}

void EroCLContext::initializeContext()
{
	if (initialized)
		return;
	initialized = true;

	// Platform & device
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.empty()) throw std::runtime_error("No OpenCL platform found");
	platform = platforms[0];
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
	if (devices.empty()) throw std::runtime_error("No OpenCL device found");
	device = devices[0];
	context = cl::Context(device);
	queue = cl::CommandQueue(context, device);

	// Print device info
	ND_INFO("Using OpenCL device: {}", device.getInfo<CL_DEVICE_NAME>());
	ND_INFO("  Version: {}", device.getInfo<CL_DEVICE_VERSION>());
	ND_INFO("  Vendor: {}", device.getInfo<CL_DEVICE_VENDOR>());
	ND_INFO("  Driver version: {}", device.getInfo<CL_DRIVER_VERSION>());


	// Build program
	std::string src = nd::FUtil::readFileString(ND_RESLOC("res/../Terrain/src/terrain/ero.h"));
	program = cl::Program(context, src);
	try
	{
		program.build({device});
	}
	catch (...)
	{
		ND_ERROR("Build log:\n {}", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
		throw;
	}
	ND_ERROR("Build log:\n {}", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));

	// Create all kernels
	const char* names[] = {
		"ero1_kernel", "ero2_kernel", "ero3_kernel",
		"ero4_kernel", "ero5_kernel", "ero7_kernel"
	};

	for (const char* name : names)
	{
		try
		{
			kernels.emplace_back(program, name);
		}
		catch (...)
		{
			ND_ERROR("Failed to create kernel {}", name);
			throw;
		}
	}
}
