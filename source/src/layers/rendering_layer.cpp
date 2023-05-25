#include "layers/rendering_layer.hpp"
#include "mapp/app.hpp"
#include "imgui_impl_mapp.hpp"
#include "mrender/imgui_impl_mrender.hpp"

#include <imgui.h>
#include <iostream>

void RenderingLayer::onInit(mapp::AppContext& context)
{
	mContext = &context;

	// MRENDER
	mrender::RenderSettings renderSettings;
	renderSettings.mRendererName = "MyRenderer";
	renderSettings.mNativeDisplay = mContext->getWindow()->getNativeDisplay();
	renderSettings.mNativeWindow = mContext->getWindow()->getNativeWindow();
	renderSettings.mResolutionWidth = mContext->getWindow()->getParams().mWidth;
	renderSettings.mResolutionHeight = mContext->getWindow()->getParams().mHeight;
	
	mRenderContext.initialize(renderSettings);

	mRenderer = mrender::Renderer::make(renderSettings.mRendererName);
	if (mRenderer)
	{
		mRenderSystems = std::move(mRenderer->setupRenderSystems(mRenderContext));

		std::cout << "Enabled these render systems:" << std::endl;
		for (auto& renderSystem : mRenderSystems)
		{
			std::cout << "noname" << std::endl;
			renderSystem->init(mRenderContext);
		}
	}
	
	// IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	ImGui_ImplMapp_Init(mContext->getWindow()->getNativeWindow());
	ImGui_Implbgfx_Init(255);
}

void RenderingLayer::onShutdown()
{
	// MRENDER
	mRenderContext.cleanup();

	// IMGUI
	ImGui_Implbgfx_Shutdown();
	ImGui_ImplMapp_Shutdown();
	ImGui::DestroyContext();
}

void RenderingLayer::onEvent(mapp::Event& event)
{
	ImGui_ImplMapp_ProcessEvent(event, mContext->getWindow());

	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			mRenderContext.reset(0, e.getWidth(), e.getHeight());

			return 0;
		});
}

void RenderingLayer::onUpdate(const float& dt)
{
	// IMGUI
	ImGui_Implbgfx_NewFrame();
	ImGui_ImplMapp_NewFrame();
	ImGui::NewFrame();
}

void RenderingLayer::onRender(const float& dt)
{
	// MRENDER
	for (auto& renderSystem : mRenderSystems)
	{
		renderSystem->render(mRenderContext);
	}

	// IMGUI TEST
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	if (ImGui::Begin("Rendering Framework | mrender"))
	{
		ImGui::Text("Some text about the renderer");
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::vector<std::string_view> allRenderers = mRenderer->getNames();
			static const char* currentItem = mRenderContext.getSettings().mRendererName.data();
			if (ImGui::BeginCombo("Renderer", currentItem))
			{
				for (int n = 0; n < allRenderers.size(); n++)
				{
					bool isSelected = (currentItem == allRenderers[n]);
					if (ImGui::Selectable(allRenderers[n].data(), isSelected))
					{
						currentItem = allRenderers[n].data();
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			const mrender::RenderStats* renderStats = mRenderContext.getStats();
			ImGui::Text("Num draw calls		: %u", renderStats->mNumDrawCalls);
			ImGui::Text("Runtime memory		: %umb / 16000mb", renderStats->mRuntimeMemory);
			ImGui::Text("Render Resolution	 : %ux%u", mRenderContext.getSettings().mResolutionWidth, mRenderContext.getSettings().mResolutionHeight);
			ImGui::Text("VSync				 : %s", mRenderContext.getSettings().mVSync ? "true" : "false");

		}
		ImGui::End();
	}
	ImGui::PopStyleVar();
}

void RenderingLayer::onPostRender(const float& dt)
{
	// IMGUI
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(mContext->getWindow()->getParams().mWidth),
		static_cast<float>(mContext->getWindow()->getParams().mHeight));
	io.DeltaTime = dt;

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_Implbgfx_RenderDrawData(ImGui::GetDrawData());
}
