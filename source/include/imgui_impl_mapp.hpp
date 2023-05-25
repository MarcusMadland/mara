#pragma once

#include "mapp/platform.hpp"
#include "mapp/input.hpp"
#include "mapp/event.hpp"
// @todo write it similar to sdl

#ifdef MAPP_PLATFORM_WIN32

#include <iostream>
#include "../../third-party/imgui/examples/imgui_impl_win32.h"

static bool sMousePressed[3] = { false,false,false };
static float sMouseScrollAxis = 0.0f;

void ImGui_ImplMapp_Init(void* window) { ImGui_ImplWin32_Init(window); } // should this be init for mrender?
void ImGui_ImplMapp_Shutdown() { ImGui_ImplWin32_Shutdown(); }
void ImGui_ImplMapp_NewFrame()
{
	ImGui_ImplWin32_NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = sMousePressed[0];
	io.MouseDown[1] = sMousePressed[1];
	io.MouseDown[2] = sMousePressed[2];

	ImGui::GetPlatformIO().Monitors.resize(0);
}
void ImGui_ImplMapp_ProcessEvent(mapp::Event& event, mapp::Window* window) 
{ 
	ImGuiIO& io = ImGui::GetIO();

	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			//io.DisplaySize = ImVec2(static_cast<float>(e.getWidth()),
			//static_cast<float>(e.getHeight()));

			return 0;
		});

	dispatcher.dispatch<mapp::WindowCloseEvent>(
		[&](const mapp::WindowCloseEvent& e)
		{
			
			return 0;
		});

	dispatcher.dispatch<mapp::MouseButtonPressedEvent>(
		[&](const mapp::MouseButtonPressedEvent& e)
		{
			if (e.getKeyCode() == MAPP_MOUSE_LEFT)
			{
				sMousePressed[0] = true;
			}

			if (e.getKeyCode() == MAPP_MOUSE_RIGHT)
			{
				sMousePressed[1] = true;
			}

			if (e.getKeyCode() == MAPP_MOUSE_MIDDLE)
			{
				sMousePressed[2] = true;
			}

			return 0;
		});

	dispatcher.dispatch<mapp::MouseButtonReleasedEvent>(
		[&](const mapp::MouseButtonReleasedEvent& e)
		{
			sMousePressed[0] = false;
			sMousePressed[1] = false;
			sMousePressed[2] = false;
			
			return 0;
		});

	dispatcher.dispatch<mapp::MouseScrolledEvent>(
		[&](const mapp::MouseScrolledEvent& e)
		{
			io.MouseWheel = e.getYOffset();

			return 0;
		});
}


#endif