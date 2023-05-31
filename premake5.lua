-- Internal third Party directories
local MAPP_DIR        = "mapp"
local MCORE_DIR       = "mcore"
local MRENDER_DIR     = "mrender"

-- Main directory
local THIRDPARTY_DIR = "third-party"
local APPLICATION_DIR = "source"

-- Application Workspace
workspace "workspace"
	architecture "x86_64"
	startproject "application"

	configurations 
	{ 
		"Release", 
		"Debug" 
	}

	-- temp --------------------------------
	defines
	{
		--"BGFX_CONFIG_RENDERER_AGC"     
		--"BGFX_CONFIG_RENDERER_DIRECT3D9"  
		--"BGFX_CONFIG_RENDERER_DIRECT3D11" 
		--"BGFX_CONFIG_RENDERER_DIRECT3D12" 
		--"BGFX_CONFIG_RENDERER_GNM"        
		--"BGFX_CONFIG_RENDERER_METAL"      
		--"BGFX_CONFIG_RENDERER_NVN"        
		"BGFX_CONFIG_RENDERER_OPENGL"     
		--"BGFX_CONFIG_RENDERER_OPENGLES"
		--"BGFX_CONFIG_RENDERER_VULKAN"  
		--"BGFX_CONFIG_RENDERER_WEBGPU"
	}

	filter "system:macosx"
		xcodebuildsettings 
		{
			["MACOSX_DEPLOYMENT_TARGET"] = "10.9",
			["ALWAYS_SEARCH_USER_PATHS"] = "YES", 
		}

	filter "configurations:Release"
		defines
		{
			"NDEBUG",
			"BX_CONFIG_DEBUG=0"
		}
		optimize "Full"
		
	filter "configurations:Debug*"
		defines
		{
			"_DEBUG",
			"BX_CONFIG_DEBUG=1"
		}
		optimize "Debug"
		symbols "On"
	----------------------------------------

group "internal"
	include(path.join(MAPP_DIR,    "premake.lua"))
	include(path.join(MCORE_DIR,   "premake.lua"))
	include(path.join(MRENDER_DIR, "premake.lua"))
group ""

group "external"
	outputdir = "binaries/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}"
	include(path.join(THIRDPARTY_DIR, "imgui/premake5.lua"))
group ""

-- Application Project
project "application"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	location (APPLICATION_DIR)
	
	targetdir ("binaries/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")
    objdir ("intermediate/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")

	files 
	{
		path.join(APPLICATION_DIR, "**.h"),
		path.join(APPLICATION_DIR, "**.hpp"),
		path.join(APPLICATION_DIR, "**.cpp"),
	}
	
	includedirs
	{
		path.join(APPLICATION_DIR, "include"),

		path.join(MAPP_DIR,    "include"),
		path.join(MCORE_DIR,   "include"),
		path.join(MRENDER_DIR, "include"),

		path.join(THIRDPARTY_DIR, "imgui"),
	}

	links
	{ 
		"mapp", 
		--"mcore",
		"mrender",
		"imgui",
	}

	filter "system:windows"
		links { "gdi32", "kernel32", "psapi" }
	filter "system:linux"
		links { "dl", "GL", "pthread", "X11" }
	filter "system:macosx"
		links { "QuartzCore.framework", "Metal.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework" }