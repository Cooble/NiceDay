workspace "NiceDay"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
    flags
	{
		"MultiProcessorCompile",
        --"NoRuntimeChecks"
	}
    


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "NiceDay/vendor/glfw"
    include "NiceDay/vendor/glad"
    include "NiceDay/vendor/imgui"
group ""

project "NiceDay"
	location "NiceDay"
    kind "StaticLib"
	language "C++"
    cppdialect "C++17"
	staticruntime "on"

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
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
	}
    defines
	{
		"_CRT_SECURE_NO_WARNINGS"
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
        --buildoptions "/RTCu"
        --[Add here C++/CodeGeneration/BasicRuntimeChecks->Uninitialized vars]

	filter "configurations:Release"
		defines "ND_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ND_DIST"
		runtime "Release"
		optimize "On"

project "SandboxTest"
	location "SandboxTest"
	kind "ConsoleApp"
	language "C++"
    cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
   

	includedirs
	{
        "%{prj.name}/src",
        "NiceDay/src",
        "NiceDay/vendor",
		"NiceDay/vendor/spdlog/include",
		"NiceDay/vendor/imgui",
		"NiceDay/vendor/glm",
        "NiceDay/vendor/glfw/include",
		"NiceDay/vendor/glad/include",
		"NiceDay/vendor/stb_image"
	}
    links 
	{ 
        "NiceDay"
	}
    

	filter "system:windows"
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
        --buildoptions "/RTCu" god knows why it doesnt work

        --[Add here C++/CodeGeneration/BasicRuntimeChecks->Uninitialized vars]

	filter "configurations:Release"
		defines "ND_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ND_DIST"
		runtime "Release"
		optimize "On"
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
    cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
   

	includedirs
	{
        "%{prj.name}/src",
        "NiceDay/src",
        "NiceDay/vendor",
		"NiceDay/vendor/spdlog/include",
		"NiceDay/vendor/imgui",
		"NiceDay/vendor/glm",
        "NiceDay/vendor/glfw/include",
		"NiceDay/vendor/glad/include",
		"NiceDay/vendor/stb_image"
	}
    links 
	{ 
        "NiceDay"
	}
    

	filter "system:windows"
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
        --buildoptions "/RTCu" god knows why it doesnt work

        --[Add here C++/CodeGeneration/BasicRuntimeChecks->Uninitialized vars]

	filter "configurations:Release"
		defines "ND_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ND_DIST"
		runtime "Release"
		optimize "On"
                         