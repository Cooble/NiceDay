#ifdef nottitit


#include "ndpch.h"
#include "MonoLayer.h"
//#include <mono/jit/jit.h>
//#include <mono/metadata/assembly.h>
//#include <mono/metadata/debug-helpers.h>
//#include <mono/metadata/object.h>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include "core/App.h"
#include "core/NBT.h"
#include "files/FUtil.h"

// Standard headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>


#ifdef ND_DEBUG
#define ASS_NAME "ManagedCored.dll"
#else
#define ASS_NAME "ManagedCore.dll"
#endif

#ifdef ND_NOT_EXISTS

namespace nd {

#ifdef ND_DEBUG
#define ASS_NAME "ManagedCored.dll"
#else
#define ASS_NAME "ManagedCore.dll"
#endif
//#pragma comment(lib, "C:/Program Files/Mono/lib/mono-2.0-sgen.lib")
/*	types for args in internal calls
 *
static MonoClass*
find_system_class(const char* name)
{
	if (!strcmp(name, "void"))
		return mono_defaults.void_class;
	else if (!strcmp(name, "char")) return mono_defaults.char_class;
	else if (!strcmp(name, "bool")) return mono_defaults.boolean_class;
	else if (!strcmp(name, "byte")) return mono_defaults.byte_class;
	else if (!strcmp(name, "sbyte")) return mono_defaults.sbyte_class;
	else if (!strcmp(name, "uint16")) return mono_defaults.uint16_class;
	else if (!strcmp(name, "int16")) return mono_defaults.int16_class;
	else if (!strcmp(name, "uint")) return mono_defaults.uint32_class;
	else if (!strcmp(name, "int")) return mono_defaults.int32_class;
	else if (!strcmp(name, "ulong")) return mono_defaults.uint64_class;
	else if (!strcmp(name, "long")) return mono_defaults.int64_class;
	else if (!strcmp(name, "uintptr")) return mono_defaults.uint_class;
	else if (!strcmp(name, "intptr")) return mono_defaults.int_class;
	else if (!strcmp(name, "single")) return mono_defaults.single_class;
	else if (!strcmp(name, "double")) return mono_defaults.double_class;
	else if (!strcmp(name, "string")) return mono_defaults.string_class;
	else if (!strcmp(name, "object")) return mono_defaults.object_class;
	else
		return NULL;
}*/
static void l_info(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_INFO(c);
	mono_free(c);
}

static void l_warn(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_WARN(c);
	mono_free(c);
}

static void l_error(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_ERROR(c);
	mono_free(c);
}

static void l_trace(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_TRACE(c);
	mono_free(c);
}

static mono_bool nd_copy_file(MonoString* from, MonoString* to)
{
	auto f = mono_string_to_utf8(from);
	auto t = mono_string_to_utf8(to);
	bool suk = false;
	try
	{
		suk = std::filesystem::copy_file(f, t, std::filesystem::copy_options::overwrite_existing);
	}
	catch (std::exception& e)
	{
		ND_ERROR("Cannot copy {} to {}\n{}", f, t, e.what());
	}
	//ND_TRACE("Copying file from {} to {} and {}",f,t,suk);
	mono_free(f);
	mono_free(t);

	return suk;
}

static void nd_profile_begin_session(MonoString* name, MonoString* path)
{
	auto n = mono_string_to_utf8(name);
	auto p = mono_string_to_utf8(path);
	ND_PROFILE_BEGIN_SESSION(n, p);
	mono_free(n);
	mono_free(p);
}

static void nd_profile_end_session()
{
	ND_PROFILE_END_SESSION();
}

static MonoString* nd_current_config()
{
	return mono_string_new(mono_domain_get(), ND_CONFIG);
}

static MonoDomain* domain;
static MonoImage* image;
static MonoAssembly* assembly;
static MonoObject* entryInstance;

static MonoObject* callCSMethod(const char* methodName, void* obj = nullptr, void** params = nullptr,
                                MonoObject** ex = nullptr)
{
	MonoMethodDesc* TypeMethodDesc = mono_method_desc_new(methodName, NULL);
	if (!TypeMethodDesc)
		return nullptr;

	//Search the method in the image
	MonoMethod* method = mono_method_desc_search_in_image(TypeMethodDesc, image);
	if (!method)
	{
		ASSERT(false, "C# method not found: {}", methodName);
		return nullptr;
	}

	auto b = mono_runtime_invoke(method, obj, params, ex);
	if (ex && *ex)
		return nullptr;

	return b;
}

static std::string hotSwapLoc = "ND.EntryHotSwap:";
static std::string coldLoc = "ND.EntryCold:";
static bool happyLoad = false;

MonoObject* MonoLayer::callEntryMethod(const char* methodName, void* obj, void** params, MonoObject** ex)
{
	std::string s = hotSwapEnable ? hotSwapLoc : coldLoc;
	return callCSMethod((s + methodName).c_str(), obj, params, ex);
}

static void initInternalCalls()
{
	mono_add_internal_call("ND.Log::nd_trace(string)", l_trace);
	mono_add_internal_call("ND.Log::nd_info(string)", l_info);
	mono_add_internal_call("ND.Log::nd_warn(string)", l_warn);
	mono_add_internal_call("ND.Log::nd_error(string)", l_error);
	mono_add_internal_call("ND.Log::ND_COPY_FILE(string,string)", nd_copy_file);
	mono_add_internal_call("ND.Log::ND_PROFILE_BEGIN_SESSION(string,string)", nd_profile_begin_session);
	mono_add_internal_call("ND.Log::ND_PROFILE_END_SESSION()", nd_profile_end_session);
	mono_add_internal_call("ND.Log::ND_CURRENT_CONFIG()", nd_current_config);
}

static std::string getMonoInstallationDirectory()
{
	auto path = std::getenv("PATH");
	SUtil::replaceWith(path, '\\', '/');

	if (path)
		for (auto splitter = SUtil::SplitIterator<true, char>(path, ';'); splitter; ++splitter)
		{
			std::string_view vi = *splitter;
			if (vi[vi.size() - 1] == '/')
				vi = std::string_view(vi.data(), vi.size() - 1);
			if (SUtil::endsWith(vi, "Mono"))
				return std::string(vi);
		}
	auto s = "C:/Program Files/Mono";
	if (std::filesystem::exists(s))
		return s;
	s = "C:/Program Files (x86)/Mono";
	if (std::filesystem::exists(s))
		return s;


	return "";
}


void MonoLayer::onAttach()
{
	std::string monoPath;

	auto monoDir = getMonoInstallationDirectory();
	if (monoDir.empty())
	{
		ND_ERROR("Cannot find mono installation dir");
		return;
	}
	mono_set_dirs((monoDir + "/lib").c_str(), (monoDir + "/etc").c_str());
	//Init a domain
	domain = mono_jit_init("MonoScripter");
	if (!domain)
	{
		ND_ERROR("mono_jit_init failed");
		return;
	}

	initInternalCalls();

	//Open a assembly in the domain
	std::string assPath = ASS_NAME;
	//App::get().getSettings().loadSet("ManagedDLL_FileName",assPath,std::string("Managedd.dll"));
	assPath = FUtil::getAbsolutePath(assPath.c_str());
	assembly = mono_domain_assembly_open(domain, assPath.c_str());
	if (!assembly)
	{
		ND_ERROR("mono_domain_assembly_open failed");
		return;
	}

	//Get a image from the assembly
	image = mono_assembly_get_image(assembly);
	if (!image)
	{
		ND_ERROR("mono_assembly_get_image failed");
		return;
	}
	MonoClass* entryClass = mono_class_from_name(image, "ND", "Entry");
	if (!entryClass)
	{
		ND_ERROR("Cannot load Entry.cs");
	}
	MonoBoolean b = hotSwapEnable;
	void* array = {&b};
	entryInstance = callCSMethod("ND.Entry:Init", nullptr, &array);
	if (!entryInstance)
		return;

	/*
		{
			//Get the class
			MonoClass* dogclass;
			dogclass = mono_class_from_name(image, "", "Dog");
			if (!dogclass)
			{
				std::cout << "mono_class_from_name failed" << std::endl;
				system("pause");
				return 1;
			}

			//Create a instance of the class
			MonoObject* dogA;
			dogA = mono_object_new(domain, dogclass);
			if (!dogA)
			{
				std::cout << "mono_object_new failed" << std::endl;
				system("pause");
				return 1;
			}

			//Call its default constructor
			mono_runtime_object_init(dogA);


			//Build a method description object
			MonoObject* result;
			MonoMethodDesc* BarkMethodDesc;
			char* BarkMethodDescStr = "Dog:Bark(int)";
			BarkMethodDesc = mono_method_desc_new(BarkMethodDescStr, NULL);
			if (!BarkMethodDesc)
			{
				std::cout << "mono_method_desc_new failed" << std::endl;
				system("pause");
				return 1;
			}

			//Search the method in the image
			MonoMethod* method;
			method = mono_method_desc_search_in_image(BarkMethodDesc, image);
			if (!method)
			{
				std::cout << "mono_method_desc_search_in_image failed" << std::endl;
				system("pause");
				return 1;
			}

			//Set the arguments for the method
			void* args[1];
			int barkTimes = 10;
			args[0] = &barkTimes;



			MonoObject * pException = NULL;
			//Run the method
			std::cout << "Running the method: " << BarkMethodDescStr << std::endl;
			mono_runtime_invoke(method, dogA, args, &pException);
			if(pException)
			{
				system("pause");
			}

		}*/
	callEntryMethod("OnAttach", entryInstance);

	MonoObject* e;
	int size = 0;
	try
	{
		size = *(int*)mono_object_unbox(callCSMethod("ND.Entry:GetLayersSize", entryInstance));
	}
	catch (...)
	{
		ND_ERROR("Cannot load mono GetLayersSize");
		happyLoad = false;
	}
	happyLoad = size || hotSwapEnable;
}

void MonoLayer::onDetach()
{
	callEntryMethod("OnDetach", entryInstance);
}

void MonoLayer::onUpdate()
{
	callEntryMethod("OnUpdate", entryInstance);
}

void MonoLayer::onImGuiRender()
{
}

void MonoLayer::reloadAssembly()
{
	callEntryMethod("ReloadAssembly", entryInstance);
}

bool MonoLayer::isMonoLoaded()
{
	return happyLoad;
}
}

#endif


namespace nd
{
static hostfxr_initialize_for_runtime_config_fn init_fptr;
static hostfxr_get_runtime_delegate_fn get_delegate_fptr;
static hostfxr_close_fn close_fptr;
static hostfxr_handle cxt;

void* load_library(const char* path)
{
	HMODULE handle = LoadLibraryA(path);
	return (void*)handle;
}

void* get_export(void* h, const char* name)
{
	return (void*)GetProcAddress((HMODULE)h, name);
}

// Locate and load hostfxr.dll
void load_hostfxr()
{
	wchar_t buffer[MAX_PATH];
	size_t buffer_size = sizeof(buffer) / sizeof(wchar_t);
	int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
	if (rc != 0)
	{
		std::cerr << "Failed to locate hostfxr.dll" << std::endl;
		exit(-1);
	}
	// convert wchar_t to char*
	auto buffer_data = new char[buffer_size + 1];
	for (size_t i = 0; i < buffer_size; ++i)
	{
		buffer_data[i] = static_cast<char>(buffer[i]);
	}

	void* lib = load_library(buffer_data);
	init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
	get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
	close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

	if (!init_fptr || !get_delegate_fptr || !close_fptr)
	{
		std::cerr << "Failed to load hostfxr functions" << std::endl;
		exit(-1);
	}
}

hostfxr_handle load_runtime(const std::string& config_path)
{
	hostfxr_handle cxt = nullptr;
	int rc = init_fptr(SUtil::toWString(config_path).c_str(), nullptr, &cxt);
	if (rc != 0 || cxt == nullptr)
	{
		ND_ERROR("Failed to initialize .NET Core runtime");
	}
	return cxt;
}

load_assembly_and_get_function_pointer_fn get_load_assembly()
{
	load_assembly_and_get_function_pointer_fn load_assembly = nullptr;
	int rc = get_delegate_fptr(cxt, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly);
	if (rc != 0 || load_assembly == nullptr)
	{
		ND_ERROR("Failed to get load_assembly_and_get_function_pointer delegate");
	}
	return load_assembly;
}


void MonoLayer::onAttach()
{
	load_hostfxr();
	//todo this is very wrong /../ but for now it works
	cxt = load_runtime(FUtil::getExecutableFolderPath() + "/../ND.runtimeconfig.json");
	if (!cxt)
	{
		ND_ERROR("Failed to load runtime from config {}",
		         FUtil::getExecutableFolderPath() + "/../ND.runtimeconfig.json");
		return;
	}
	auto load_assemblyFn = get_load_assembly();


	std::string assPath = ASS_NAME;
	//App::get().getSettings().loadSet("ManagedDLL_FileName",assPath,std::string("Managedd.dll"));
	assPath = FUtil::getAbsolutePath(assPath.c_str());

	std::string type_name = "ND.Entry";
	std::string method_name = "Init";

	component_entry_point_fn init_method = nullptr;

	auto jojo = SUtil::toWString(assPath);
	//int rc = load_assemblyFn(
	//	SUtil::toWString(assPath).c_str(), 
	//	SUtil::toWString(type_name).c_str(), 
	//	SUtil::toWString(method_name).c_str(), nullptr, nullptr, (void**)&init_method);
	int rc = load_assemblyFn(
		SUtil::toWString(assPath).c_str(),
		L"TestClass",
		L"Test",
		nullptr, nullptr, (void**)&init_method);


	if (rc != 0 || init_method == nullptr)
	{
		//std::cerr << "Failed to load method 'ND.Entry:Init'" << std::endl;
		//exit(-1);
		ND_ERROR("Failed to load method 'TestEntry:Init' rc={} initMethod={}", rc,
		         reinterpret_cast<uint64_t>(init_method));
	}
}

void MonoLayer::onDetach()
{
	close_fptr(cxt);
}
}

#endif


#define WINDOWS
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


// Standard headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// Provided by the AppHost NuGet package and installed as an SDK pack
#include <nethost.h>

// Header files copied from https://github.com/dotnet/core-setup
#include "MonoLayer.h"

#include <coreclr_delegates.h>
#include <hostfxr.h>

#include "files/FUtil.h"

#ifdef WINDOWS
#include <Windows.h>

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#define string_compare wcscmp

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif

using string_t = std::basic_string<char_t>;

namespace
{
	// Globals to hold hostfxr exports
	hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
	hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_run_app_fn run_app_fptr;
	hostfxr_close_fn close_fptr;
	hostfxr_get_runtime_delegate_fn hostfxr_get_runtime_delegate_fptr;
	load_assembly_fn load_assembly_fptr;
	get_function_pointer_fn get_function_pointer_fptr;

	// Forward declarations
	bool load_hostfxr(const char_t* app);
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* assembly);

	int run_component_example(const string_t& root_path);
	int run_app_example(const string_t& root_path);
}


namespace
{
	int loadManagedCore()
	{
		//
	// STEP 1: Load HostFxr and get exported hosting functions
	//
		if (!load_hostfxr(nullptr))
		{
			assert(false && "Failure: load_hostfxr()");
			return EXIT_FAILURE;
		}

		//
		// STEP 2: Initialize and start the .NET Core runtime
		//
		const string_t config_path = nd::SUtil::toWString(
			nd::FUtil::getExecutableFolderPath() + "../ND.runtimeconfig.json");
		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
		assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

		//
		// STEP 3: Load managed assembly and get function pointer to a managed method
		//
		const string_t dotnetlib_path = nd::SUtil::toWString(nd::FUtil::getExecutableFolderPath() + "ManagedCore.dll");
		const char_t* dotnet_type = STR("DotNetLib.Lib, ManagedCore");
		const char_t* dotnet_type_method = STR("Hello");
		ND_INFO("Searching for dotnetlib_path: {}", nd::SUtil::toStdString(dotnetlib_path));
		auto response = load_assembly_fptr(dotnetlib_path.c_str(), nullptr, nullptr);
		assert(response == 0 && "Failure: load_assembly_fptr()");

		return EXIT_SUCCESS;

	}
	int run_component_example()
	{
		//
// STEP 1: Load HostFxr and get exported hosting functions
//
		if (!load_hostfxr(nullptr))
		{
			assert(false && "Failure: load_hostfxr()");
			return EXIT_FAILURE;
		}

		//
		// STEP 2: Initialize and start the .NET Core runtime
		//
		const string_t config_path = nd::SUtil::toWString(
			nd::FUtil::getExecutableFolderPath() + "../ND.runtimeconfig.json");
		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
		assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

		//
		// STEP 3: Load managed assembly and get function pointer to a managed method
		//
		const string_t dotnetlib_path = nd::SUtil::toWString(nd::FUtil::getExecutableFolderPath() + "ManagedCore.dll");
		const char_t* dotnet_type = STR("DotNetLib.Lib, ManagedCore");
		const char_t* dotnet_type_method = STR("Hello");
		ND_INFO("Searching for dotnetlib_path: {}", nd::SUtil::toStdString(dotnetlib_path));
		auto response = load_assembly_fptr(dotnetlib_path.c_str(), nullptr, nullptr);
		assert(response == 0 && "Failure: load_assembly_fptr()");

	

		component_entry_point_fn hello = nullptr;
		response = get_function_pointer_fptr(
			dotnet_type,
			dotnet_type_method,
			nullptr /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&hello);
		assert(response == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");
		ND_INFO("Get function pointer response: {}", response);


		//component_entry_point_fn hello2 = nullptr;
		// <SnippetLoadAndGet>
		// Function pointer to managed delegate
		//int rc = load_assembly_and_get_function_pointer(
		//	dotnetlib_path.c_str(),
		//	dotnet_type,
		//	dotnet_type_method,
		//	nullptr /*delegate_type_name*/,
		//	nullptr,
		//	(void**)&hello2);
		//assert(hello == hello2 && "Failure: functions not mATCHINGA");


		//
		// STEP 4: Run managed code
		//
		struct lib_args
		{
			const char_t* message;
			int number;
		};
		for (int i = 0; i < 3; ++i)
		{
			// <SnippetCallManaged>
			lib_args args
			{
				STR("from host!"),
				i
			};

			hello(&args, sizeof(args));
			// </SnippetCallManaged>
		}

		// Function pointer to managed delegate with non-default signature
		typedef void (CORECLR_DELEGATE_CALLTYPE*custom_entry_point_fn)(lib_args args);
		custom_entry_point_fn custom = nullptr;
		lib_args args
		{
			STR("from host!"),
			-1
		};

		// UnmanagedCallersOnly
		auto rc = get_function_pointer_fptr(
			dotnet_type,
			STR("CustomEntryPointUnmanagedCallersOnly") /*method_name*/,
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			nullptr,
			(void**)&custom);
		assert(rc == 0 && custom != nullptr && "Failure: load_assembly_and_get_function_pointer()");
		custom(args);

		// Custom delegate type
		rc = get_function_pointer_fptr(
			dotnet_type,
			STR("CustomEntryPoint") /*method_name*/,
			STR("DotNetLib.Lib+CustomEntryPointDelegate, ManagedCore") /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&custom);
		assert(rc == 0 && custom != nullptr && "Failure: load_assembly_and_get_function_pointer()");
		custom(args);




		return EXIT_SUCCESS;
	}

	int run_app_example(const string_t& root_path)
	{
		const string_t app_path = root_path + STR("App.dll");

		if (!load_hostfxr(app_path.c_str()))
		{
			assert(false && "Failure: load_hostfxr()");
			return EXIT_FAILURE;
		}

		// Load .NET Core
		hostfxr_handle cxt = nullptr;
		std::vector<const char_t*> args{app_path.c_str(), STR("app_arg_1"), STR("app_arg_2")};
		int rc = init_for_cmd_line_fptr(args.size(), args.data(), nullptr, &cxt);
		if (rc != 0 || cxt == nullptr)
		{
			std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
			close_fptr(cxt);
			return EXIT_FAILURE;
		}

		// Get the function pointer to get function pointers
		get_function_pointer_fn get_function_pointer;
		rc = get_delegate_fptr(
			cxt,
			hdt_get_function_pointer,
			(void**)&get_function_pointer);
		if (rc != 0 || get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

		// Function pointer to App.IsWaiting
		typedef unsigned char (CORECLR_DELEGATE_CALLTYPE*is_waiting_fn)();
		is_waiting_fn is_waiting;
		rc = get_function_pointer(
			STR("App, App"),
			STR("IsWaiting"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr, nullptr, (void**)&is_waiting);
		assert(rc == 0 && is_waiting != nullptr && "Failure: get_function_pointer()");

		// Function pointer to App.Hello
		typedef void (CORECLR_DELEGATE_CALLTYPE*hello_fn)(const char*);
		hello_fn hello;
		rc = get_function_pointer(
			STR("App, App"),
			STR("Hello"),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr, nullptr, (void**)&hello);
		assert(rc == 0 && hello != nullptr && "Failure: get_function_pointer()");

		// Invoke the functions in a different thread from the main app
		std::thread t([&]
		{
			while (is_waiting() != 1)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

			for (int i = 0; i < 3; ++i)
				hello("from host!");
		});

		// Run the app
		run_app_fptr(cxt);
		t.join();

		close_fptr(cxt);
		return EXIT_SUCCESS;
	}
}

namespace nd
{
	constexpr auto s_dotnet_type_EntryCaller = STR("ND.EntryCaller, ManagedCore");
	typedef void (__cdecl* EntryCaller_Init_fn)(bool);
	typedef void (__cdecl* EntryCaller_onEvent_fn)();

	static EntryCaller_Init_fn s_EntryCaller_Init = nullptr;
	static EntryCaller_onEvent_fn s_EntryCaller_onAttach = nullptr;
	static EntryCaller_onEvent_fn s_EntryCaller_onDetach = nullptr;
	static EntryCaller_onEvent_fn s_EntryCaller_onUpdate = nullptr;

	


	void MonoLayer::onAttach()
	{
		ND_INFO("MonoLayer::onAttach");
		//run_component_example();
		loadManagedCore();

		auto response = get_function_pointer_fptr(
			s_dotnet_type_EntryCaller,
			STR("Init"),
			UNMANAGEDCALLERSONLY_METHOD /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&s_EntryCaller_Init);

		if (response!=0) {
			ND_ERROR("Failure: load Init()");
			return;
		}
		

		// Attach
		response = get_function_pointer_fptr(
			s_dotnet_type_EntryCaller,
			STR("OnAttach"),
			UNMANAGEDCALLERSONLY_METHOD /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&s_EntryCaller_onAttach);

		if (response!=0) {
			ND_ERROR("Failure: load OnAttach()");
			return;
		}

		// Detach
		response = get_function_pointer_fptr(
			s_dotnet_type_EntryCaller,
			STR("OnDetach"),
			UNMANAGEDCALLERSONLY_METHOD /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&s_EntryCaller_onDetach);

		if (response!=0) {
			ND_ERROR("Failure: load OnDetach()");
			return;
		}

		// Update
		response = get_function_pointer_fptr(
			s_dotnet_type_EntryCaller,
			STR("OnUpdate"),
			UNMANAGEDCALLERSONLY_METHOD /*delegate_type_name*/,
			nullptr,
			nullptr,
			(void**)&s_EntryCaller_onUpdate);


		if (response != 0) {
			ND_ERROR("Failure: load OnUpdate()");
			return;
		}



		// Call Init
		s_EntryCaller_Init(hotSwapEnable);


		is_mono_loaded = true;
		s_EntryCaller_onAttach();
	}

	void MonoLayer::onDetach()
	{
		s_EntryCaller_onDetach();
	}

	void MonoLayer::onUpdate()
	{
		s_EntryCaller_onUpdate();
	}

	bool MonoLayer::isMonoLoaded() const
	{
		return is_mono_loaded;
	}
}


/********************************************************************************************
 * Function used to load and activate .NET Core
 ********************************************************************************************/

namespace
{
	// Forward declarations
	void* load_library(const char_t*);
	void* get_export(void*, const char*);

#ifdef WINDOWS
	void* load_library(const char_t* path)
	{
		HMODULE h = ::LoadLibraryW(path);
		assert(h != nullptr);
		return (void*)h;
	}

	void* get_export(void* h, const char* name)
	{
		void* f = ::GetProcAddress((HMODULE)h, name);
		assert(f != nullptr);
		return f;
	}
#else
	void* load_library(const char_t* path)
	{
		void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		assert(h != nullptr);
		return h;
	}
	void* get_export(void* h, const char* name)
	{
		void* f = dlsym(h, name);
		assert(f != nullptr);
		return f;
	}
#endif

	// <SnippetLoadHostFxr>
	// Using the nethost library, discover the location of hostfxr and get exports
	bool load_hostfxr(const char_t* assembly_path)
	{
		get_hostfxr_parameters params{sizeof(get_hostfxr_parameters), assembly_path, nullptr};
		// Pre-allocate a large buffer for the path to hostfxr
		char_t buffer[MAX_PATH];
		size_t buffer_size = sizeof(buffer) / sizeof(char_t);
		int rc = get_hostfxr_path(buffer, &buffer_size, &params);
		if (rc != 0)
			return false;

		// Load hostfxr and get desired exports
		// NOTE: The .NET Runtime does not support unloading any of its native libraries. Running
		// dlclose/FreeLibrary on any .NET libraries produces undefined behavior.
		void* lib = load_library(buffer);
		init_for_cmd_line_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)get_export(
			lib, "hostfxr_initialize_for_dotnet_command_line");
		init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(
			lib, "hostfxr_initialize_for_runtime_config");
		get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
		run_app_fptr = (hostfxr_run_app_fn)get_export(lib, "hostfxr_run_app");
		close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");
		hostfxr_get_runtime_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(
			lib, "hostfxr_get_runtime_delegate");

		return (init_for_config_fptr && get_delegate_fptr && close_fptr && hostfxr_get_runtime_delegate_fptr);
	}

	// </SnippetLoadHostFxr>

	// <SnippetInitialize>
	// Load and initialize .NET Core and get desired function pointer for scenario
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;
		hostfxr_handle cxt = nullptr;
		int rc = init_for_config_fptr(config_path, nullptr, &cxt);
		if (rc != 0 || cxt == nullptr)
		{
			std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
			close_fptr(cxt);
			return nullptr;
		}

		// Get the load assembly function pointer
		rc = get_delegate_fptr(
			cxt,
			hdt_load_assembly_and_get_function_pointer,
			&load_assembly_and_get_function_pointer);
		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;


		// Get the load assembly function pointer
		hostfxr_get_runtime_delegate_fptr(cxt, hdt_load_assembly, (void**)&load_assembly_fptr);
		hostfxr_get_runtime_delegate_fptr(cxt, hdt_get_function_pointer, (void**)&get_function_pointer_fptr);


		close_fptr(cxt);
		return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
	}

	// </SnippetInitialize>
}
