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

project "NiceDay"
	location "NiceDay"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    --pchheader "hzpch.h"
	--pchsource "NiceDay/src/hzpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"ND_PLATFORM_WINDOWS",
			"ND_BUILD_DLL",
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

	filter "configurations:Release"
		defines "ND_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ND_DIST"
		runtime "Release"
		optimize "On"
        