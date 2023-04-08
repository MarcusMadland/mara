-- External third Party directories
local IMGUI_DIR       = "3rd-party/imgui"
local SDL_DIR         = "3rd-party/sdl"

-- Internal third Party directories
local MAPP_DIR        = "mapp"
local MCORE_DIR       = "mcore"
local MRENDER_DIR     = "mrender"

-- Main directory
local APPLICATION_DIR = "source"

-- Application Workspace
workspace "application"
	startproject "application"
	configurations { "Release", "Debug" }

group "External"
--	project "imgui"
--	project "sdl"
group ""

group "Internal"
--	project "mapp"
--	project "mcore"
--	project "mrender"
group ""

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Application Project
project "application"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location (APPLICATION_DIR)
	
	targetdir ("binaries/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")
	
	files 
	{
		path.join(APPLICATION_DIR, "**.h"),
		path.join(APPLICATION_DIR, "**.hpp"),
		path.join(APPLICATION_DIR, "**.cpp"),
	}
	
	includedirs
	{
		path.join(MAPP_DIR,    "include"),
		path.join(MCORE_DIR,   "include"),
		path.join(MRENDER_DIR, "include"),

		path.join(SDL_DIR,   "include"),
		path.join(IMGUI_DIR, "include"),
	}
	--links { "mrender", "sdl", "imgui"}