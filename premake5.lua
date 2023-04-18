-- Internal third Party directories
local MAPP_DIR        = "mapp"
local MCORE_DIR       = "mcore"
local MRENDER_DIR     = "mrender"

-- Main directory
local THIRDPARTY_DIR = "3rd-party"
local APPLICATION_DIR = "source"

-- Application Workspace
workspace "application"
	startproject "application"
	configurations { "Release", "Debug" }

	if os.is64bit() and not os.istarget("windows") then
		platforms "x86_64"
	else
		platforms { "x86", "x86_64" }
	end

	-- temp
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

		-- 

	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x86_64"
		architecture "x86_64"
	filter "system:macosx"
		xcodebuildsettings {
			["MACOSX_DEPLOYMENT_TARGET"] = "10.9",
			["ALWAYS_SEARCH_USER_PATHS"] = "YES", 
		};


group "internal"
	include(path.join(MAPP_DIR,    "premake.lua"))
	include(path.join(MCORE_DIR,   "premake.lua"))
	include(path.join(MRENDER_DIR, "premake.lua"))
group ""

-- Application Project
project "application"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
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

		path.join(THIRDPARTY_DIR, "imgui/include"),
		path.join(THIRDPARTY_DIR, "sdl/include"),
	}

	-- temp
	links
	{ 
	"mapp", 
	"mrender", 
	"bgfx", 
	"bimg", 
	"bx",
	}

	defines { "MAPP_CUSTOM_PLATFORM_DETECTION"}

	filter "system:windows"
		defines { "MAPP_PLATFORM_WIN32" }
	filter "system:macosx"
		defines { "MAPP_PLATFORM_COCOA" }
		links { "Cocoa.framework",
   "QuartzCore.framework", "Metal.framework", 
-- "IOKit.framework", "CoreVideo.framework" 
}
	--
