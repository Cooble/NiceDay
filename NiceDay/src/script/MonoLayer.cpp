#include "ndpch.h"
#include "MonoLayer.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include "core/App.h"
#include "core/NBT.h"
#include "files/FileUtil.h"
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
}
static void l_warn(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_WARN(c);
}
static void l_error(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_ERROR(c);
}
static void l_trace(MonoString* s)
{
	auto c = mono_string_to_utf8(s);
	ND_TRACE(c);
}
static MonoDomain* domain;
static MonoImage* image;
static MonoAssembly* assembly;

static bool callCSMethod(const char* methodName,MonoObject** ex=nullptr)
{
	//Build a method description object
	MonoMethodDesc* TypeMethodDesc;
	TypeMethodDesc = mono_method_desc_new(methodName, NULL);
	if (!TypeMethodDesc)
		return false;

	//Search the method in the image
	MonoMethod* method;
	method = mono_method_desc_search_in_image(TypeMethodDesc, image);
	if (!method) {
		ASSERT(false, "C# method not found: {}", methodName);
		return false;
	}

	mono_runtime_invoke(method, nullptr, nullptr, ex);
	if (ex && *ex)
		return false;

	return true;
}
static void initInternalCalls()
{
	mono_add_internal_call("Log::nd_trace(string)", l_trace);
	mono_add_internal_call("Log::nd_info(string)", l_info);
	mono_add_internal_call("Log::nd_warn(string)", l_warn);
	mono_add_internal_call("Log::nd_error(string)", l_error);
}

void MonoLayer::onAttach()
{
	std::string monoPath;
	App::get().getSettings().loadSet("MonoInstallationPath (with folders /lib and /etc)", monoPath, std::string("C:/Program Files/Mono"));
	mono_set_dirs((monoPath + "/lib").c_str(), (monoPath + "/etc").c_str());
	//Init a domain
	domain = mono_jit_init("MonoScripter");
	if (!domain)
	{
		ND_ERROR("mono_jit_init failed");
		return;
	}

	initInternalCalls();

	//Open a assembly in the domain
	std::string assPath;
	App::get().getSettings().loadSet("ManagedDLL_FileName",assPath,std::string("Managedd.dll"));
	assPath = FileUtil::getAbsolutePath(assPath.c_str());
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
	callCSMethod("Entry:onAttach()");
}

void MonoLayer::onDetach()
{
	callCSMethod("Entry:onDetach()");
}

void MonoLayer::onUpdate()
{
	callCSMethod("Entry:onUpdate()");
}

void MonoLayer::onImGuiRender()
{
}