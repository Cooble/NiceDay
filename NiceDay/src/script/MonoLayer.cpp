#include "ndpch.h"
#include "MonoLayer.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include "core/App.h"
#include "core/NBT.h"
#include "files/FUtil.h"

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
	try {
		 suk = std::filesystem::copy_file(f, t,std::filesystem::copy_options::overwrite_existing);
	}catch (std::exception& e)
	{
		ND_ERROR("Cannot copy {} to {}\n{}", f, t,e.what());
	}
	//ND_TRACE("Copying file from {} to {} and {}",f,t,suk);
	mono_free(f);
	mono_free(t);

	return suk;
}
static void nd_profile_begin_session(MonoString* name,MonoString* path)
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

static MonoObject* callCSMethod(const char* methodName,void* obj=nullptr,void** params=nullptr, MonoObject** ex=nullptr)
{
	MonoMethodDesc* TypeMethodDesc = mono_method_desc_new(methodName, NULL);
	if (!TypeMethodDesc)
		return nullptr;

	//Search the method in the image
	MonoMethod* method = mono_method_desc_search_in_image(TypeMethodDesc, image);
	if (!method) {
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
		for (auto splitter = SUtil::SplitIterator<true, char>(path, ';'); splitter; ++splitter) {
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
	if(monoDir.empty())
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
	std::string assPath= ASS_NAME;
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
	MonoBoolean b=hotSwapEnable;
	void* array = { &b };
	entryInstance=callCSMethod("ND.Entry:Init", nullptr, &array);
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
	callEntryMethod("OnAttach",entryInstance);

	MonoObject* e;
	int size = 0;
	try {
		size = *(int*)mono_object_unbox(callCSMethod("ND.Entry:GetLayersSize", entryInstance));
	}catch(...)
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
