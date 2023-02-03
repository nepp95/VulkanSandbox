include "Dependencies.lua"

workspace "VulkanSandbox"
	architecture "x86_64"
	startproject "VulkanSandbox"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	configurations {
		"Debug",
		"Release"
	}

	flags {
		"MultiProcessorCompile"
	}

	include "VulkanSandbox/vendor/glfw"

project "VulkanSandbox"
	location "VulkanSandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
		
	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.VulkanSDK}"
	}

	links {
		"glfw",
		"%{Library.Vulkan}"
	}

	filter "configurations:Debug"
		defines "VDEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "VRELEASE"
		runtime "Release"
		optimize "On"
