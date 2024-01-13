#pragma once

#include "../../mara/include/mara/mara.h"

#include "../graphics/source/src/version.h"

namespace ImGui {

void ShowDebugGamepad()
{

}

void ShowDebugBuild()
{
	constexpr U8 color = 0xF;

	const graphics::Stats* graphicsStats = graphics::getStats();

	graphics::dbgTextPrintf(2, graphicsStats->textHeight - 3, color, "Build: Version 1.%d.%d (commit: " GRAPHICS_REV_SHA1 ")", 
		GRAPHICS_API_VERSION, GRAPHICS_REV_NUMBER); // @todo Using graphics module versioning atm, create own.
	graphics::dbgTextPrintf(2, graphicsStats->textHeight - 2, color, "Built: %s @ %s", __DATE__, __TIME__);
}

void ShowDebugStats()
{
	constexpr U32 x = 40;
	constexpr U32 offset = 14;
	constexpr U8 color = 0xA;
	constexpr U32 plotLength = 500;

	constexpr F64 cpuLimit = 40.0;
	constexpr F64 gpuLimit = 25.0;
	constexpr F64 fpsLimit = 60.0;
	constexpr F64 texLimit = 1464.0f;

	const graphics::Stats* graphicsStats = graphics::getStats();

	static F64 cpu = 0.0f;
	static F64 cpuHighest = 0.0f;
	static F64 gpu = 0.0f;
	static F64 gpuHighest = 0.0f;
	static F64 framerate = 0.0f;
	static F64 texture = 0.0f;

	{
		const F64 cpuToMs = 1000.0 / graphicsStats->cpuTimerFreq;
		cpu = F64(graphicsStats->cpuTimeFrame) * cpuToMs;
		if (cpu > cpuHighest) cpuHighest = cpu;

		const F64 gpuToMs = 1000.0 / graphicsStats->gpuTimerFreq;
		gpu = F64(graphicsStats->gpuTimeEnd - graphicsStats->gpuTimeBegin) * gpuToMs;
		if (gpu > gpuHighest) gpuHighest = gpu;

		framerate = 1 / mara::getDeltaTime();
		texture = F64(graphicsStats->textureMemoryUsed) / (1024.0 * 1024.0);
	}

	ImGui::SetNextWindowPos({ (F32)graphicsStats->width - (230), 110 });
	ImGui::SetNextWindowSize({ 0, 20 });
	ImGui::Begin("Framerate Plot", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	{
		static F32 framerateData[plotLength] = { 0 }; // Assuming a fixed-size buffer
		for (I32 i = 0; i < plotLength - 1; ++i) {
			framerateData[i] = framerateData[i + 1];
		}
		framerateData[plotLength - 1] = framerate; // Add the new framerate value
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PlotLines("##framerate", framerateData, IM_ARRAYSIZE(framerateData), 0, "", 0.0f, 120.0f, ImVec2(150, 20));
		ImGui::PopStyleColor();
	}
	ImGui::End();

	graphics::dbgTextPrintf(graphicsStats->textWidth - x, 2, cpuLimit < cpu ? 0x4 : color, "cpu(game):");
	graphics::dbgTextPrintf(graphicsStats->textWidth - x, 3, color, "cpu(render):");
	graphics::dbgTextPrintf(graphicsStats->textWidth - x, 4, gpuLimit < gpu ? 0x4 : color, "gpu:");
	graphics::dbgTextPrintf(graphicsStats->textWidth - x, 5, framerate < fpsLimit ? 0x4 : color, "framerate:");
	graphics::dbgTextPrintf(graphicsStats->textWidth - x, 6, texLimit < texture ? 0x4 : color, "textures:");

	graphics::dbgTextPrintf(graphicsStats->textWidth - (x - offset), 2, cpuLimit < cpu ? 0x4 : color, "%.2f ms [%.2f ms]", cpu, cpuHighest);
	graphics::dbgTextPrintf(graphicsStats->textWidth - (x - offset), 3, color, "00.00 ms [0.00 ms]");
	graphics::dbgTextPrintf(graphicsStats->textWidth - (x - offset), 4, gpuLimit < gpu ? 0x4 : color, "%.2f ms [%.2f ms]", gpu, gpuHighest);
	graphics::dbgTextPrintf(graphicsStats->textWidth - (x - offset), 5, framerate < fpsLimit ? 0x4 : color, "%.2f fps", framerate);
	graphics::dbgTextPrintf(graphicsStats->textWidth - (x - offset), 6, texLimit < texture ? 0x4 : color, "%.2f / %.0f MiB", texture, texLimit);
}

void BeginDeveloperMenu(const char* _name)
{
	const graphics::Stats* stats = graphics::getStats();

	// Styles
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

	// Colors
	ImGui::PushStyleColor(ImGuiCol_Border, { 0.8f, 0.8f, 0.8f, 0.8f });
	ImGui::PushStyleColor(ImGuiCol_Separator, { 0.8f, 0.8f, 0.8f, 0.8f });
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.2f, 0.2f, 0.2f, 0.8f });
	ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_CheckMark, { 1.0f, 1.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_NavHighlight, { 0.3f, 0.3f, 0.3f, 1.0f });

	// Main Window
	ImGui::SetNextWindowPos({ 50.0f, 25.0f });
	ImGui::SetNextWindowSizeConstraints({ 300.0f, 10.0f }, { 1920.0f, 1080.0f });
	ImGui::Begin(_name, (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowFocus();
	ImGui::Text(_name);
	ImGui::Separator();

	// Pause Window
	ImGui::SetNextWindowPos({ stats->width - 100.0f, stats->height - 50.0f });
	ImGui::Begin("PAUSED", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	ImGui::Text("PAUSED");
	ImGui::End();
}

void EndDeveloperMenu()
{
	// Styles
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	// Colors
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	//ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	// Main Window
	ImGui::End();
}

bool DeveloperMenuButton(const char* _text)
{
	base::FilePath text = "   ";
	text.join(_text, false);
	text.join("...", false);
	return ImGui::Button(text.getCPtr());
}

bool DeveloperMenuCheckbox(const char* _text, bool* _value)
{
	return ImGui::Checkbox(_text, _value);
}

void DeveloperMenuText(const char* _text)
{
	base::FilePath text = "   ";
	text.join(_text, false);
	ImGui::Text(text.getCPtr());
}

}
