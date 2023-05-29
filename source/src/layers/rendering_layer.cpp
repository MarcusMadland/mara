#include "layers/rendering_layer.hpp"
#include "mapp/app.hpp"
#include "imgui_impl_mapp.hpp"
#include "mrender/imgui_impl_mrender.hpp"

#include <imgui.h>
#include <iostream>

void RenderingLayer::onInit(mapp::AppContext& context)
{
	mAppContext = &context;

	// MRENDER
	mrender::RenderSettings renderSettings;
	renderSettings.mRendererName = "MyRenderer";
	renderSettings.mNativeDisplay = mAppContext->getWindow()->getNativeDisplay();
	renderSettings.mNativeWindow = mAppContext->getWindow()->getNativeWindow();
	renderSettings.mResolutionWidth = mAppContext->getWindow()->getParams().mWidth;
	renderSettings.mResolutionHeight = mAppContext->getWindow()->getParams().mHeight;
	renderSettings.mVSync = true;

	mRenderContext = mrender::createRenderContext();
	mRenderContext->initialize(renderSettings);

	imguiImplInit();
}

void RenderingLayer::onShutdown()
{
	mRenderContext->cleanup();

	imguiImplShutdown();
}

void RenderingLayer::onEvent(mapp::Event& event)
{
	ImGui_ImplMApp_ProcessEvent(event, mAppContext->getWindow());

	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			mrender::RenderSettings settings = mRenderContext->getSettings();
			settings.mResolutionWidth = e.getWidth();
			settings.mResolutionHeight = e.getHeight();
			mRenderContext->setSettings(settings);

			return 0;
		});
}

void RenderingLayer::onRender()
{
	// Render
	mRenderContext->render();

	// IMGUI TEST
	imguiImplBegin();
	renderUserInterface();
	imguiImplEnd();

	// Debug text (application performance)
	{
		uint16_t y = 2;
		uint16_t x = 45;
		const float deltaTime = mAppContext->getApp()->getDeltaTime();
		static uint32_t counter = 0; counter++;
		static float fps = 0;
		static float ms = 0;
		if (!(counter % 10))
		{
			fps = 1 / deltaTime;
			ms = 1000 * deltaTime;
		}

		if (mDrawDebugText)
		{
			mRenderContext->submitDebugTextOnScreen(x, y, "cpu(application):  %.2f ms [26.05 ms]", ms);
			mRenderContext->submitDebugTextOnScreen(x, y + 1, "cpu(mrender):      %.2f ms [16.12 ms]", 0);
			mRenderContext->submitDebugTextOnScreen(x, y + 2, "gpu:               %.2f ms [24.93 ms]", 0);
			mRenderContext->submitDebugTextOnScreen(x, y + 3, "framerate:         %.2f fps", fps);
			mRenderContext->submitDebugTextOnScreen(x, y + 4, "textures:          %.2f / 1454 MiB", 0);
		}
	}

	// Swap buffers
	mRenderContext->frame();
}

void RenderingLayer::renderUserInterface()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	if (ImGui::Begin(" MRender | Rendering Framework", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::Text("A 3D Rendering framework with support\nfor PBR and GI");
		if (ImGui::Button("Reload shaders"))
		{
			mRenderContext->reloadShaders();
		}
		ImGui::Checkbox("Draw stats", &mDrawDebugText);
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::vector<std::string_view> allRenderers = mRenderContext->getRenderer()->getNames();
			static const char* currentItem = mRenderContext->getSettings().mRendererName.data();
			if (ImGui::BeginCombo("Renderer", currentItem))
			{
				for (int n = 0; n < allRenderers.size(); n++)
				{
					bool isSelected = (currentItem == allRenderers[n].data());
					if (ImGui::Selectable(allRenderers[n].data(), isSelected))
					{
						currentItem = allRenderers[n].data();
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				mrender::RenderSettings settings = mRenderContext->getSettings();
				settings.mRendererName = currentItem;
				mRenderContext->setSettings(settings);

				ImGui::EndCombo();
			}

			ImGui::Text("Num draw calls		: %u", 0);
			ImGui::Text("Render Resolution	 : %ux%u", mRenderContext->getSettings().mResolutionWidth, mRenderContext->getSettings().mResolutionHeight);
			
			static bool vSync = mRenderContext->getSettings().mVSync;
			if (ImGui::Checkbox("VSync	", &vSync))
			{
				mrender::RenderSettings settings = mRenderContext->getSettings();
				settings.mVSync = vSync;
				mRenderContext->setSettings(settings);
			}

		}

		if (ImGui::CollapsingHeader("Render Systems", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (uint32_t i = 0; i < mRenderContext->getRenderSystems().size(); i++)
			{
				ImGui::Text(mRenderContext->getRenderSystems()[i]->getName().data());
			}
		}

		if (ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& shader : mRenderContext->getShaders())
			{
				ImGui::Text(shader.first.data());
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

	//ImGui::ShowDemoWindow();
}

void RenderingLayer::imguiImplInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplMApp_Init(mAppContext->getWindow()->getNativeWindow());
	ImGui_ImplMRender_Init(255);
}

void RenderingLayer::imguiImplShutdown()
{
	ImGui_ImplMRender_Shutdown();
	ImGui_ImplMApp_Shutdown();
	ImGui::DestroyContext();
}

void RenderingLayer::imguiImplBegin()
{
	ImGui_ImplMRender_NewFrame();
	ImGui_ImplMApp_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(mAppContext->getWindow()->getParams().mWidth),
		static_cast<float>(mAppContext->getWindow()->getParams().mHeight));
}

void RenderingLayer::imguiImplEnd()
{
	
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplMRender_RenderDrawData(ImGui::GetDrawData());
}
