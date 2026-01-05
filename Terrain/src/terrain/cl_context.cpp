#include "ndpch.h"
#include "cl_context.h"
#include "files/FUtil.h"

// -------------------------------------------------------------------------
// ERROR HANDLING HELPERS
// -------------------------------------------------------------------------

// Helper to translate OpenCL error codes to readable strings
static const char* getCLErrorString(cl_int error){
	switch (error) {
	case CL_SUCCESS:                          return "Success!";
	case CL_DEVICE_NOT_FOUND:                 return "Device not found.";
	case CL_DEVICE_NOT_AVAILABLE:             return "Device not available";
	case CL_COMPILER_NOT_AVAILABLE:           return "Compiler not available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return "Memory object allocation failure";
	case CL_OUT_OF_RESOURCES:                 return "Out of resources";
	case CL_OUT_OF_HOST_MEMORY:               return "Out of host memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE:     return "Profiling information not available";
	case CL_MEM_COPY_OVERLAP:                 return "Memory copy overlap";
	case CL_IMAGE_FORMAT_MISMATCH:            return "Image format mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return "Image format not supported";
	case CL_BUILD_PROGRAM_FAILURE:            return "Program build failure";
	case CL_MAP_FAILURE:                      return "Map failure";
	case CL_INVALID_VALUE:                    return "Invalid value";
	case CL_INVALID_DEVICE_TYPE:              return "Invalid device type";
	case CL_INVALID_PLATFORM:                 return "Invalid platform";
	case CL_INVALID_DEVICE:                   return "Invalid device";
	case CL_INVALID_CONTEXT:                  return "Invalid context";
	case CL_INVALID_QUEUE_PROPERTIES:         return "Invalid queue properties";
	case CL_INVALID_COMMAND_QUEUE:            return "Invalid command queue";
	case CL_INVALID_HOST_PTR:                 return "Invalid host pointer";
	case CL_INVALID_MEM_OBJECT:               return "Invalid memory object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return "Invalid image format descriptor";
	case CL_INVALID_IMAGE_SIZE:               return "Invalid image size";
	case CL_INVALID_SAMPLER:                  return "Invalid sampler";
	case CL_INVALID_BINARY:                   return "Invalid binary";
	case CL_INVALID_BUILD_OPTIONS:            return "Invalid build options";
	case CL_INVALID_PROGRAM:                  return "Invalid program";
	case CL_INVALID_PROGRAM_EXECUTABLE:       return "Invalid program executable";
	case CL_INVALID_KERNEL_NAME:              return "Invalid kernel name";
	case CL_INVALID_KERNEL_DEFINITION:        return "Invalid kernel definition";
	case CL_INVALID_KERNEL:                   return "Invalid kernel";
	case CL_INVALID_ARG_INDEX:                return "Invalid argument index";
	case CL_INVALID_ARG_VALUE:                return "Invalid argument value";
	case CL_INVALID_ARG_SIZE:                 return "Invalid argument size";
	case CL_INVALID_KERNEL_ARGS:              return "Invalid kernel arguments";
	case CL_INVALID_WORK_DIMENSION:           return "Invalid work dimension";
	case CL_INVALID_WORK_GROUP_SIZE:          return "Invalid work group size";
	case CL_INVALID_WORK_ITEM_SIZE:           return "Invalid work item size";
	case CL_INVALID_GLOBAL_OFFSET:            return "Invalid global offset";
	case CL_INVALID_EVENT_WAIT_LIST:          return "Invalid event wait list";
	case CL_INVALID_EVENT:                    return "Invalid event";
	case CL_INVALID_OPERATION:                return "Invalid operation";
	case CL_INVALID_GL_OBJECT:                return "Invalid OpenGL object";
	case CL_INVALID_BUFFER_SIZE:              return "Invalid buffer size";
	case CL_INVALID_MIP_LEVEL:                return "Invalid mip-map level";
	default:                                  return "Unknown";
	}
}

// Callback for asynchronous OpenCL errors (e.g., during kernel execution)
static void CL_CALLBACK contextCallBack(const char* errinfo, const void* private_info, size_t cb, void* user_data)
{
	ND_ERROR("OPENCL ASYNC ERROR: {}", errinfo);
}

// -------------------------------------------------------------------------
// BINDING HELPERS
// -------------------------------------------------------------------------

void EroCLContext::bind_terrain_buffers()
{
	try {
		// ero2 (Flow) - Reads terrain
		kernels[1].setArg(0, b.buf_terrain_height);

		// ero4 (Erosion) - Reads current (0), writes new (1)
		kernels[3].setArg(0, b.buf_terrain_height);
		kernels[3].setArg(1, b.buf_new_terrain_height);

		// ero7 (Landslide) - Read/Write in-place
		kernels[5].setArg(0, b.buf_terrain_height);
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in bind_terrain_buffers: {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::bind_sediment_buffers()
{
	try {
		// ero4 (Erosion) - Writes sediment produced by erosion
		kernels[3].setArg(3, b.buf_sediment);

		// ero5 (Transport) - Reads current (0), writes new (1)
		kernels[4].setArg(0, b.buf_sediment);
		kernels[4].setArg(1, b.buf_new_sediment);
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in bind_sediment_buffers: {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}


// =========================================
// CORE IMPLEMENTATION
// ========================================

void EroCLContext::init(EulerGround& g, Euler::EulerSettings& s)
{
	try {
		initializeContext();

		if (width == g.width)
		{
			upload_all(g);
			upload_params(g, s);
			return;
		}
		width = g.width;
		height = g.height;

		size_t sz = width * height;

		// Read-Write
		b.buf_terrain_height = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
		b.buf_new_terrain_height = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
		b.buf_flux = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(gvec4) * sz);
		b.buf_sediment = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
		b.buf_new_sediment = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);
		b.buf_velocity = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(gvec2) * sz);
		b.buf_water_height = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * sz);

		// Read Only
		b.buf_original_height = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * sz);
		b.buf_perlin_map = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * sz);

		// --- Static ---

		// ero1 (Rain)
		kernels[0].setArg(0, b.buf_water_height);

		// ero2 (Flow)
		// Arg 0 (Terrain) is dynamic
		kernels[1].setArg(1, b.buf_water_height);
		kernels[1].setArg(2, b.buf_flux);

		// ero3 (Flow Update)
		kernels[2].setArg(0, b.buf_water_height);
		kernels[2].setArg(1, b.buf_flux);
		kernels[2].setArg(2, b.buf_velocity);

		// ero4 (Erosion)
		// Arg 0, 1 (Terrain)
		kernels[3].setArg(2, b.buf_water_height);
		// Arg 3 (Sediment)
		kernels[3].setArg(4, b.buf_velocity);
		kernels[3].setArg(5, b.buf_perlin_map);
		kernels[3].setArg(6, b.buf_original_height);

		// ero5 (Transport)
		// Arg 0, 1 (Sediment)
		kernels[4].setArg(2, b.buf_velocity);

		// ero7 (Landslide)
		// Arg 0 (Terrain)

		// --- Dynamic ---
		bind_terrain_buffers();
		bind_sediment_buffers();

		upload_all(g);
		upload_params(g, s);


		// prime all events as done by default, for some reason, cmd queue does not like null events in wait lists
		std::vector<cl::Event> empty;
		queue.enqueueMarkerWithWaitList(&empty, &ready_rain);
		queue.enqueueMarkerWithWaitList(&empty, &ready_erosion);
		queue.enqueueMarkerWithWaitList(&empty, &ready_landslide);
		queue.enqueueMarkerWithWaitList(&empty, &ready_sediment);
		queue.enqueueMarkerWithWaitList(&empty, &ready_velocity);


	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in init(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::upload_params(EulerGround& g, Euler::EulerSettings& s)
{
	try {
		// ero1
		auto idx = 1;
		kernels[0].setArg(idx++, g.width);
		kernels[0].setArg(idx++, g.height);
		kernels[0].setArg(idx++, s.K_rain);
		kernels[0].setArg(idx++, s.K_evaporation);
		kernels[0].setArg(idx++, s.K_dt);

		// ero2
		idx = 3;
		kernels[1].setArg(idx++, g.width);
		kernels[1].setArg(idx++, g.height);
		kernels[1].setArg(idx++, s.K_dt);

		// ero3
		idx = 3;
		kernels[2].setArg(idx++, g.width);
		kernels[2].setArg(idx++, g.height);
		kernels[2].setArg(idx++, s.K_dt);

		// ero4
		idx = 7;
		kernels[3].setArg(idx++, g.width);
		kernels[3].setArg(idx++, g.height);
		kernels[3].setArg(idx++, s.K_sediment_capacity);
		kernels[3].setArg(idx++, s.K_tilt_minimum);
		kernels[3].setArg(idx++, s.K_s_dissolving);
		kernels[3].setArg(idx++, s.K_d_depositing);

		// ero5
		idx = 3;
		kernels[4].setArg(idx++, g.width);
		kernels[4].setArg(idx++, g.height);
		kernels[4].setArg(idx++, s.K_dt);

		// ero7
		idx = 1;
		kernels[5].setArg(idx++, g.width);
		kernels[5].setArg(idx++, g.height);
		idx++; // offset x
		idx++; // offset y
		kernels[5].setArg(idx++, s.K_landSlideSpeed);
		kernels[5].setArg(idx++, s.K_landSlideCutoffAngle);
		kernels[5].setArg(idx++, s.K_dt);
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in upload_params(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::upload_all(EulerGround& g)
{
	try {
		queue.enqueueWriteBuffer(b.buf_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.terrain_height.data());
		queue.enqueueWriteBuffer(b.buf_new_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.new_terrain_height.data());
		queue.enqueueWriteBuffer(b.buf_original_height, CL_FALSE, 0, sizeof(float) * width * height, g.original_height.data());
		queue.enqueueWriteBuffer(b.buf_flux, CL_FALSE, 0, sizeof(gvec4) * width * height, g.flux.data());
		queue.enqueueWriteBuffer(b.buf_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.sediment.data());
		queue.enqueueWriteBuffer(b.buf_new_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.new_sediment.data());
		queue.enqueueWriteBuffer(b.buf_velocity, CL_FALSE, 0, sizeof(gvec2) * width * height, g.velocity.data());
		queue.enqueueWriteBuffer(b.buf_water_height, CL_FALSE, 0, sizeof(float) * width * height, g.water_height.data());
		queue.enqueueWriteBuffer(b.buf_perlin_map, CL_FALSE, 0, sizeof(float) * width * height, g.perlin_map.data());

		queue.finish();
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in upload_all(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::download_all(EulerGround& g)
{
	try {
		queue.enqueueReadBuffer(b.buf_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.terrain_height.data());
		queue.enqueueReadBuffer(b.buf_new_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.new_terrain_height.data());
		queue.enqueueReadBuffer(b.buf_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.sediment.data());
		queue.enqueueReadBuffer(b.buf_new_sediment, CL_FALSE, 0, sizeof(float) * width * height, g.new_sediment.data());
		queue.enqueueReadBuffer(b.buf_velocity, CL_FALSE, 0, sizeof(gvec2) * width * height, g.velocity.data());
		queue.enqueueReadBuffer(b.buf_flux, CL_FALSE, 0, sizeof(gvec4) * width * height, g.flux.data());
		queue.enqueueReadBuffer(b.buf_water_height, CL_FALSE, 0, sizeof(float) * width * height, g.water_height.data());

		queue.finish();
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in download_all(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::download_graphics(EulerGround& g)
{
	try {
		queue.finish();

		queue.enqueueReadBuffer(b.buf_terrain_height, CL_FALSE, 0, sizeof(float) * width * height, g.terrain_height.data());
		queue.enqueueReadBuffer(b.buf_water_height, CL_FALSE, 0, sizeof(float) * width * height, g.water_height.data());

		queue.finish(); // Wait for reads to complete
	}
	catch (cl::Error& err) {
		ND_ERROR("OpenCL Error in download_graphics(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::step(Euler::EulerSettings& s)
{
	try
	{
		// is it time to post another frame?
		if (fence_ring_buff[current_fence_idx]())
			fence_ring_buff[current_fence_idx].wait();


		// Ranges
		cl::NDRange global(width - 2, height - 2);
		cl::NDRange of(1, 1);
		cl::NDRange global_slide((width - 1) / 2, (height - 1) / 2);


		std::vector<cl::Event> waitFor;


		// 1. Rain & Evap
		waitFor = { ready_erosion };
		if (s.e_rain) {
			// Writes: buf_water_height
			queue.enqueueNDRangeKernel(kernels[0], of, global, cl::NullRange, &waitFor, &ready_rain);
		}
		else queue.enqueueMarkerWithWaitList(&waitFor, &ready_rain);

		// 2. Water Flow
		waitFor = { ready_rain, ready_landslide };
		if (s.e_flow)
		{
			cl::Event ready_flux;

			// Pass 1: Flux calculation
			// Reads: water_height, terrain (Arg 0 bound to current terrain)
			queue.enqueueNDRangeKernel(kernels[1], of, global, cl::NullRange, &waitFor, &ready_flux);

			// Pass 2: Water update
			// Reads: Flux
			// Writes: velocity, water_height
			waitFor = { ready_flux };
			queue.enqueueNDRangeKernel(kernels[2], of, global, cl::NullRange, &waitFor, &ready_velocity);
		}
		else queue.enqueueMarkerWithWaitList(&waitFor, &ready_velocity);

		// 3. Erosion Transport
		waitFor = { ready_velocity, ready_sediment };
		if (s.e_erosion)
		{
			// A. Erosion
			// Reads: Terrain (Current)
			// Writes: Terrain (New), Sediment (Current buf)
			queue.enqueueNDRangeKernel(kernels[3], of, global, cl::NullRange, &waitFor, &ready_erosion);

			// --- SWAP TERRAIN ---
			std::swap(b.buf_terrain_height, b.buf_new_terrain_height);
			bind_terrain_buffers();

			// B. Sediment Transport
			// Reads: Sediment (Current)
			// Writes: Sediment (New)
			waitFor = { ready_erosion };
			queue.enqueueNDRangeKernel(kernels[4], of, global, cl::NullRange, &waitFor, &ready_sediment);

			// --- SWAP SEDIMENT ---
			std::swap(b.buf_sediment, b.buf_new_sediment);
			bind_sediment_buffers();
		}
		else {
			queue.enqueueMarkerWithWaitList(&waitFor, &ready_erosion);
			queue.enqueueMarkerWithWaitList(&waitFor, &ready_sediment);
		}


		// 4. Landslide
		waitFor = { ready_erosion };
		if (s.e_landslide)
		{
			for (int pass = 0; pass < 4; pass++)
			{
				kernels[5].setArg(3, pass & 1);        // offset x
				kernels[5].setArg(4, (pass >> 1) & 1); // offset y

				// Reads/Writes: Terrain (In-Place)
				queue.enqueueNDRangeKernel(kernels[5], of, global_slide, cl::NullRange, &waitFor, &ready_landslide);

				waitFor = { ready_landslide };
			}
		}
		else queue.enqueueMarkerWithWaitList(&waitFor, &ready_landslide);
		

		// Fence after frame is done
		cl::Event frame_done;
		queue.enqueueBarrierWithWaitList(&waitFor, &frame_done);
		fence_ring_buff[current_fence_idx] = frame_done;
		current_fence_idx = (current_fence_idx + 1) % fence_ring_buff.size();
	}
	catch (cl::Error& err)
	{
		ND_ERROR("OpenCL Error in step(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}

void EroCLContext::initializeContext()
{
	if (initialized)
		return;
	initialized = true;

	try
	{
		// Platform & device
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.empty()) throw std::runtime_error("No OpenCL platform found");
		platform = platforms[0];

		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
		if (devices.empty()) throw std::runtime_error("No OpenCL device found");
		device = devices[0];

		// Context with Callback for Async errors
		cl_int err;
		// Passing contextCallBack to receive notifications about errors occurring in the context
		context = cl::Context(device, NULL, contextCallBack, NULL, &err);
		if (err != CL_SUCCESS) {
			ND_ERROR("Failed to create OpenCL context: {}", getCLErrorString(err));
			throw std::runtime_error("OpenCL Context Init Failed");
		}

		// Enable Out-of-Order execution
		cl_command_queue_properties props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
		// props |= CL_QUEUE_PROFILING_ENABLE; // Uncomment if profiling is needed
		queue = cl::CommandQueue(context, device, props, &err);
		if (err != CL_SUCCESS) throw cl::Error(err, "CommandQueue creation failed");

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
			program.build({ device });
		}
		catch (...)
		{
			ND_ERROR("Build log:\n {}", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
			throw;
		}

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
	catch (cl::Error& err)
	{
		ND_ERROR("OpenCL Error in initializeContext(): {} ({})", err.what(), getCLErrorString(err.err()));
		throw;
	}
}