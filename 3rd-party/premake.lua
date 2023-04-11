project "imgui"
	kind "StaticLib"
	language "C++"

	targetdir ("../binaries/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")
	objdir ("../intermediate/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")

	files
	{
		"imgui/imconfig.h",
		"imgui/imgui.h",
		"imgui/imgui.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_internal.h",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h",
		"imgui/imgui_demo.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

project "sdl"
	language "C++"
	cppdialect "C++17"  

	filter "system:windows"               -- SDL needs to be a DLL on windows for
		kind          "SharedLib"         -- some reason :)
		staticruntime "off"

	filter "system:macosx"
		kind          "StaticLib"
		staticruntime "on"

	filter "system:linux"
		kind          "StaticLib"
		staticruntime "on"

	targetdir ("../binaries/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")
	objdir ("../intermediate/" .. "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" .. "/%{prj.name}")

	flags {
		              "NoRuntimeChecks", -- Only used on Visual Studio.
		              "NoBufferSecurityCheck"
	}

	vectorextensions  "SSE"               -- Necessary to run x32.

	includedirs      { "sdl/include", "include" }

	filter "system:windows"
		links {
			         "setupapi",
			         "winmm",
			         "imm32",
			         "version",
		}
	filter {}

	files {
		-- All platforms.
		-- Header files.                                    -- C files.
		"sdl/include/*.h",									    
														    
		"sdl/src/audio/disk/*.h",                               "sdl/src/atomic/*.c",
		"sdl/src/audio/dummy/*.h",						        "sdl/src/audio/disk/*.c",
		"sdl/src/audio/*.h",								    "sdl/src/audio/dummy/*.c",
		"sdl/src/dynapi/*.h",								    "sdl/src/audio/*.c",
		"sdl/src/events/blank_cursor.h",					    "sdl/src/cpuinfo/*.c",
		"sdl/src/events/default_cursor.h",				        "sdl/src/dynapi/*.c",
		"sdl/src/events/SDL_clipboardevents_c.h",			    "sdl/src/events/*.c",
		"sdl/src/events/SDL_displayevents_c.h",			        "sdl/src/file/*.c",
		"sdl/src/events/SDL_dropevents_c.h",				    "sdl/src/haptic/*.c",
		"sdl/src/events/SDL_events_c.h",					    "sdl/src/joystick/hidapi/*.c",
		"sdl/src/events/SDL_gesture_c.h",					    "sdl/src/joystick/*.c",
		"sdl/src/events/SDL_keyboard_c.h",				        "sdl/src/libm/*.c",
		"sdl/src/events/SDL_mouse_c.h",					        "sdl/src/power/*.c",
		"sdl/src/events/SDL_sysevents.h",					    "sdl/src/render/opengl/*.c",
		"sdl/src/events/SDL_touch_c.h",					        "sdl/src/render/opengles2/*.c",
		"sdl/src/events/SDL_windowevents_c.h",			        "sdl/src/render/SDL_render.c",
		"sdl/src/haptic/SDL_syshaptic.h",					    "sdl/src/render/SDL_yuv_sw.c",
		"sdl/src/joystick/hidapi/*.h",					        "sdl/src/render/software/*.c",
		--[["src/joystick/hidapi/SDL_hidapijoystick_c.h",]]     "sdl/src/*.c",
		"sdl/src/joystick/SDL_hidapijoystick_c.h",		        "sdl/src/sensor/dummy/SDL_dummysensor.c",
		"sdl/src/joystick/SDL_joystick_c.h",				    "sdl/src/sensor/SDL_sensor.c",
		"sdl/src/joystick/SDL_sysjoystick.h",				    "sdl/src/stdlib/*.c",
		"sdl/src/libm/*.h",								        "sdl/src/thread/generic/SDL_syscond.c",
		"sdl/src/render/opengl/*.h",						    "sdl/src/thread/*.c",
		"sdl/src/render/opengles/*.h",					        "sdl/src/thread/windows/SDL_sysmutex.c",
		"sdl/src/render/SDL_yuv_sw_c.h",					    "sdl/src/thread/windows/SDL_syssem.c",
		"sdl/src/render/software/*.h",					        "sdl/src/thread/windows/SDL_systhread.c",
		"sdl/src/render/SDL_sysrender.h",					    "sdl/src/thread/windows/SDL_systls.c",
		"sdl/src/SDL_dataqueue.h",						        "sdl/src/timer/*.c",
		"sdl/src/SDL_error_c.h",							    "sdl/src/timer/windows/SDL_systimer.c",
		"sdl/src/sensor/dummy/*.h",						        "sdl/src/video/dummy/*.c",
		"sdl/src/sensor/*.h",								    "sdl/src/video/*.c",
		"sdl/src/thread/*.h",								    "sdl/src/video/yuv2rgb/*.c",
		"sdl/src/timer/*.h",
		"sdl/src/video/dummy/*.h",
		"sdl/src/video/*.h",
	}

	filter "system:windows"
		files {
			-- Windows specific files.
			-- Header files.                                -- C files.
			"sdl/include/SDL_config_windows.h",				    
														    
			"sdl/src/audio/directsound/*.h",                    "sdl/src/audio/directsound/*.c",
			"sdl/src/audio/wasapi/*.h",						    "sdl/src/audio/winmm/*.c",
			"sdl/src/audio/winmm/*.h",						    "sdl/src/audio/wasapi/*.c",
			--[["src/windows/SDL_directx.h",]]  		    "sdl/src/core/windows/*.c",
			"sdl/src/core/windows/*.h",						    "sdl/src/filesystem/windows/*.c",
			"sdl/src/haptic/windows/*.h",					    "sdl/src/haptic/windows/*.c",
			"sdl/src/joystick/windows/*.h",					    "sdl/src/joystick/windows/*.c",
			"sdl/src/render/direct3d11/SDL_shaders_d3d11.h",    "sdl/src/hidapi/windows/*.c",
			"sdl/src/render/direct3d/*.h",					    "sdl/src/loadso/windows/*.c",
			"sdl/src/render/SDL_d3dmath.h",					    "sdl/src/power/windows/*.c",
			"sdl/src/thread/windows/*.h",					    "sdl/src/render/direct3d11/*.c",
			"sdl/src/video/windows/SDL_vkeys.h",			    "sdl/src/render/direct3d/*.c",
			"sdl/src/video/windows/SDL_windowsclipboard.h",	    "sdl/src/render/SDL_d3dmath.c",
			"sdl/src/video/windows/SDL_windowsevents.h",	    "sdl/src/video/windows/*.c",
			"sdl/src/video/windows/SDL_windowsframebuffer.h",
			"sdl/src/video/windows/SDL_windowskeyboard.h",
			"sdl/src/video/windows/SDL_windowsmessagebox.h",
			"sdl/src/video/windows/SDL_windowsmodes.h",
			"sdl/src/video/windows/SDL_windowsmouse.h",
			"sdl/src/video/windows/SDL_windowsopengl.h",
			"sdl/src/video/windows/SDL_windowsshape.h",
			"sdl/src/video/windows/SDL_windowsvideo.h",
			"sdl/src/video/windows/SDL_windowsvulkan.h",
			"sdl/src/video/windows/SDL_windowswindow.h",
			"sdl/src/video/windows/wmmsg.h",
		}

		
	inlining          "Explicit"             -- General optimisation options.
	intrinsics        "Off"

	filter "system:windows"
		systemversion "latest"
		defines {
			          "_WINDOWS"
		}

	filter "configurations:Debug"
		defines {
			          "_DEBUG"
		}
		runtime       "Debug"
		symbols       "On"

	filter "configurations:Release"
		defines {
			          "NDEBUG"
		}
		runtime       "Release"
		optimize      "Speed"