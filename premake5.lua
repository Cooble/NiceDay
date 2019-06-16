workspace "NiceDay"
	architecture "x64"
	startproject "NiceDay"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
    


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "NiceDay/vendor/glfw"
include "NiceDay/vendor/glad"
include "NiceDay/vendor/imgui"

project "NiceDay"
	location "NiceDay"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "ndpch.h"
	pchsource "NiceDay/src/ndpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb_image/**",
	}
   

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/glfw/include",
		"%{prj.name}/vendor/glad/include",
		"%{prj.name}/vendor/imgui",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/stb_image"
	}
    links 
	{ 
		"glfw",
		"glad",
		"imgui",
		"opengl32.lib"
	}
    

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"ND_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}


		--[[postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}
        ]]

	filter "configurations:Debug"
		defines "ND_DEBUG"
		runtime "Debug"
		symbols "On"
        --[Add here C++/CodeGeneration/BasicRuntimeChecks->Uninitialized vars]

	filter "configurations:Release"
		defines "ND_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ND_DIST"
		runtime "Release"
		optimize "On"
        