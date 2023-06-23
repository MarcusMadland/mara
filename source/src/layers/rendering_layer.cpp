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
	renderSettings.mVSync = false;

	mRenderContext = mrender::createRenderContext();
	mRenderContext->initialize(renderSettings);

	// Clear color
	mRenderContext->setClearColor(0xFF00FFFF);

	// Shaders
	mRenderContext->loadShader("simple", "C:/Users/marcu/Dev/mengine/mrender/shaders/simple");
	mRenderContext->loadShader("flat", "C:/Users/marcu/Dev/mengine/mrender/shaders/flat");

	// Geometry
	mrender::BufferLayout layout =
	{ {
		{ mrender::AttribType::Float, 3, mrender::Attrib::Position },
		{ mrender::AttribType::Uint8, 4, mrender::Attrib::Color0 },
	} };
	std::shared_ptr<mrender::Geometry> cubeGeo = mRenderContext->createGeometry(layout, s_bunnyVertices.data(), static_cast<uint32_t>(s_bunnyVertices.size() * sizeof(Vertex)), s_bunnyTriList);

	// Renerables
	std::shared_ptr<mrender::Renderable> cubeRender1 = mRenderContext->createRenderable(cubeGeo, "simple");
	std::shared_ptr<mrender::Renderable> cubeRender2 = mRenderContext->createRenderable(cubeGeo, "flat");

	mRenderContext->addRenderable(cubeRender1);
	mRenderContext->addRenderable(cubeRender2);

	// Camera
	mrender::CameraSettings cameraSettings;
	cameraSettings.mProjectionType = mrender::ProjectionType::Perspective;
	cameraSettings.mWidth = static_cast<float>(mRenderContext->getSettings().mResolutionWidth);
	cameraSettings.mHeight = static_cast<float>(mRenderContext->getSettings().mResolutionHeight);
	cameraSettings.mPosition[2] = -5.0f;
	mCamera = mRenderContext->createCamera(cameraSettings);

	// ImGui
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
			// Resize Renderer
			mrender::RenderSettings settings = mRenderContext->getSettings();
			settings.mResolutionWidth = e.getWidth();
			settings.mResolutionHeight = e.getHeight();
			mRenderContext->setSettings(settings);

			// Resize Camera
			mrender::CameraSettings cameraSettings = mCamera->getSettings();
			cameraSettings.mWidth = static_cast<float>(mRenderContext->getSettings().mResolutionWidth);
			cameraSettings.mHeight = static_cast<float>(mRenderContext->getSettings().mResolutionHeight);
			mCamera->setSettings(cameraSettings);

			return 0;
		});
}

void RenderingLayer::onUpdate(const float& dt)
{
	
}

void RenderingLayer::onRender()
{
	// Render
	mRenderContext->render(mCamera);
	
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
		static float msHighest = 0;
		if (!(counter % 10))
		{
			fps = 1 / deltaTime;
			ms = 1000 * deltaTime;
			if (ms > msHighest) msHighest = ms;
		}

		if (mDrawDebugText)
		{
			mRenderContext->submitDebugTextOnScreen(x, y, "cpu(application):  %.2f ms [%.2f ms]", ms, msHighest);
			mRenderContext->submitDebugTextOnScreen(x, y + 1, "cpu(mrender):      %.2f ms [%.2f ms]", 0, 0);
			mRenderContext->submitDebugTextOnScreen(x, y + 2, "gpu:               %.2f ms [%.2f ms]", 0, 0);
			mRenderContext->submitDebugTextOnScreen(x, y + 3, "framerate:         %.2f fps", fps);
			mRenderContext->submitDebugTextOnScreen(x, y + 4, "textures:          %.2f / 1454 MiB", 0);

			mRenderContext->submitDebugTextOnScreen(x - 20, y, mrender::Color::Red, true, false, "Too many meshes!!", 0);
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
		if (ImGui::Button("Recompile & load shaders"))
		{
			const char* scriptPath = "C:/Users/marcu/Dev/mengine/compile-shaders-win.bat";
			int result = system(scriptPath);
			if (result == 0)
			{
				mRenderContext->reloadShaders();
			}
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

			ImGui::Text("Num render states	: %u", mRenderContext->getRenderStateCount());
			ImGui::Text("Num buffers	 : %u", mRenderContext->getBuffers().size());
			ImGui::Text("Num draw calls		: %u", 0);
			ImGui::Text("Render Resolution	: %ux%u", mRenderContext->getSettings().mResolutionWidth, mRenderContext->getSettings().mResolutionHeight);
			

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
}

void RenderingLayer::imguiImplInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplMApp_Init(mAppContext->getWindow()->getNativeWindow());
	ImGui_ImplMRender_Init(50);
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
