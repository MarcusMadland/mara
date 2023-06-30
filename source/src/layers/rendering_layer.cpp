#include "layers/rendering_layer.hpp"
#include "mapp/app.hpp"
#include "imgui_impl_mapp.hpp"
#include "mrender/imgui_impl_mrender.hpp"
#include "mcore/math.hpp"

#include <imgui.h>
#include <stb_image.h>
#include <iostream>

void RenderingLayer::onInit(mapp::AppContext& context)
{
	mAppContext = &context;

	// MRENDER
	mrender::RenderSettings renderSettings;
	renderSettings.mRendererName = "Deferred";
	renderSettings.mNativeDisplay = mAppContext->getWindow()->getNativeDisplay();
	renderSettings.mNativeWindow = mAppContext->getWindow()->getNativeWindow();
	renderSettings.mResolutionWidth = mAppContext->getWindow()->getParams().mWidth;
	renderSettings.mResolutionHeight = mAppContext->getWindow()->getParams().mHeight;
	renderSettings.mVSync = false;
#ifdef MENGINE_DEBUG
	renderSettings.mRenderDebugText = true;
#else
	renderSettings.mRenderDebugText = false;
#endif

	mRenderContext = mrender::createRenderContext();
	mRenderContext->initialize(renderSettings);

	// Clear color
	mRenderContext->setClearColor(0xFF00FFFF);

	// Shaders
	mRenderContext->loadShader("uber", "C:/Users/marcu/Dev/mengine/mrender/shaders/uber");

	// Geometry
	mrender::BufferLayout layout =
	{ {
		{ mrender::AttribType::Float, 3, mrender::Attrib::Position },
		{ mrender::AttribType::Uint8, 4, mrender::Attrib::Normal },
		{ mrender::AttribType::Uint8, 4, mrender::Attrib::Tangent },
		{ mrender::AttribType::Int16, 2, mrender::Attrib::TexCoord0 },
	} };
	std::shared_ptr<mrender::Geometry> cubeGeo = mRenderContext->createGeometry(layout, s_bunnyVertices.data(), static_cast<uint32_t>(s_bunnyVertices.size() * sizeof(Vertex)), s_bunnyTriList);

	// Textures @todo fix life time of these, the textures need to be loaded of the lifetime of the rendering layer, so make member variables?
	static std::shared_ptr<mrender::Texture> albedoTex = loadTexture(mRenderContext, "C:/Users/marcu/Dev/mengine/resources/albedo.png");
	static std::shared_ptr<mrender::Texture> normalTex = loadTexture(mRenderContext, "C:/Users/marcu/Dev/mengine/resources/normal.png");
	static std::shared_ptr<mrender::Texture> specularTex = loadTexture(mRenderContext, "C:/Users/marcu/Dev/mengine/resources/specular.png");
	static mcore::Vector<float, 4> whiteColor = { 0.8f, 0.8f, 0.8f, 1.0f };

	// Materials
	std::shared_ptr<mrender::Material> textureMaterial = mRenderContext->createMaterial("uber");
	textureMaterial->setUniform("u_albedo", mrender::UniformType::Sampler, albedoTex);
	textureMaterial->setUniform("u_normal", mrender::UniformType::Sampler, normalTex);
	textureMaterial->setUniform("u_specular", mrender::UniformType::Sampler, specularTex);
	
	std::shared_ptr<mrender::Material> whiteMaterial = mRenderContext->createMaterial("uber");
	whiteMaterial->setUniform("u_albedoColor", mrender::UniformType::Vec4, std::shared_ptr<void>(&whiteColor));

	// Renderables
	std::shared_ptr<mrender::Renderable> cubeRender1 = mRenderContext->createRenderable(cubeGeo, textureMaterial);
	std::shared_ptr<mrender::Renderable> cubeRender2 = mRenderContext->createRenderable(cubeGeo, textureMaterial);
	std::shared_ptr<mrender::Renderable> floorRender = mRenderContext->createRenderable(cubeGeo, whiteMaterial);

	mRenderContext->addRenderable(cubeRender1);
	mRenderContext->addRenderable(cubeRender2);
	mRenderContext->addRenderable(floorRender);

	// Camera
	mrender::CameraSettings cameraSettings;
	cameraSettings.mProjectionType = mrender::ProjectionType::Perspective;
	cameraSettings.mWidth = static_cast<float>(mRenderContext->getSettings().mResolutionWidth);
	cameraSettings.mHeight = static_cast<float>(mRenderContext->getSettings().mResolutionHeight);
	cameraSettings.mClipFar = 10000.0f;
	cameraSettings.mPosition[2] = -5.0f;
	auto camera = mRenderContext->createCamera(cameraSettings);
	mCamera = std::make_shared<CameraOrbitController>(camera);

	// ImGui
#ifdef MENGINE_DEBUG
	imguiImplInit();
#endif
}

void RenderingLayer::onShutdown()
{
	mRenderContext->cleanup();

#ifdef MENGINE_DEBUG
	imguiImplShutdown();
#endif
}

void RenderingLayer::onEvent(mapp::Event& event)
{
	// Camera events
	mCamera->onEvent(event);

	// Rendering events
	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);
	
	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			// Resize Renderer
			mrender::RenderSettings settings = mRenderContext->getSettings();
			settings.mResolutionWidth = e.getWidth();
			settings.mResolutionHeight = e.getHeight();
			mRenderContext->setSettings(settings);

			return 0;
		});

	// ImGui events
#ifdef MENGINE_DEBUG
	ImGui_ImplMApp_ProcessEvent(event, mAppContext->getWindow());
#endif
}

void RenderingLayer::onUpdate(const float& dt)
{
	mCamera->onUpdate(dt);
}

void RenderingLayer::onRender()
{
	mrender::PROFILE_SCOPE("RenderingLayer");

	float deltaTime = mAppContext->getApp()->getDeltaTime();
	static float rotationSpeed = 20.0f;
	static float accumulatedTime = 0.0f;
	static float rotationAngle = 0.0f;
	accumulatedTime += deltaTime;
	float targetRotationAngle = rotationSpeed * accumulatedTime;

	{
		mcore::Vector<float, 3> position = { -1.5f, 0.0f, 0.0f };
		mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
		mcore::translate(translation, position);
		mcore::Matrix4x4<float> rotation = mcore::Matrix4x4<float>::identity();
		mcore::rotateX(rotation, targetRotationAngle);
		mcore::Matrix4x4<float> model = rotation * translation;
		mRenderContext->getRenderables()[0]->setTransform(&model[0]);
	}
	{
		mcore::Vector<float, 3> position = { 1.5f, 0.0f, 0.0f };
		mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
		mcore::translate(translation, position);
		mcore::Matrix4x4<float> rotation = mcore::Matrix4x4<float>::identity();
		mcore::rotateY(rotation, targetRotationAngle);
		mcore::Matrix4x4<float> model = rotation * translation;
		mRenderContext->getRenderables()[1]->setTransform(&model[0]);
	}
	{
		mcore::Vector<float, 3> position = { 0.0f, -1.5f, 0.0f };
		mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
		mcore::translate(translation, position);
		mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
		mcore::scale(scale, {10.0f, 0.01f, 10.0f });
		mcore::Matrix4x4<float> model = scale * translation;
		mRenderContext->getRenderables()[2]->setTransform(&model[0]);

	}

	// Render
	mRenderContext->render(mCamera->getCamera());
	
	// Render ImGui
#ifdef MENGINE_DEBUG
	imguiUpdate();
#endif

	// Debug text (application performance)

	constexpr uint16_t textY = 2;
	constexpr uint16_t textX = 40;

	if (mDrawDebugText)
	{
		float cpuRender = mProfileResults.at("RenderingLayer");
		static float cpuRenderHighest = 0.0f;
		if (cpuRender > cpuRenderHighest) cpuRenderHighest = cpuRender;

		float gpu = 0.0f;// @todo handle stats in rendercontext
		static float gpuHighest = 0.0f;
		if (gpu > gpuHighest) gpuHighest = gpu;

		float fps = 0.0f;
		float texture = 0.0f;

		mRenderContext->submitDebugTextOnScreen(textX, textY + 0, "%-15s %.2f ms [%.2f ms]", "cpu(game):", 0, 0);
		mRenderContext->submitDebugTextOnScreen(textX, textY + 1, "%-15s %.2f ms [%.2f ms]", "cpu(render):", cpuRender, cpuRenderHighest);
		mRenderContext->submitDebugTextOnScreen(textX, textY + 2, "%-15s %.2f ms [%.2f ms]", "gpu:", gpu, gpuHighest);
		mRenderContext->submitDebugTextOnScreen(textX, textY + 3, "%-15s %.2f fps", "framerate:", fps);
		mRenderContext->submitDebugTextOnScreen(textX, textY + 4, "%-15s %.2f / 1454 MiB", "textures:", texture);

		//mRenderContext->submitDebugTextOnScreen(textX - 20, textY, mrender::Color::Red, true, false, "Too many meshes!!", 0);
	}
	
	
	// Swap buffers
	mRenderContext->swapBuffers();
}

void RenderingLayer::imguiUpdate()
{
	ImGui_ImplMRender_NewFrame();
	ImGui_ImplMApp_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(mAppContext->getWindow()->getParams().mWidth),
		static_cast<float>(mAppContext->getWindow()->getParams().mHeight));

	// Render mrender debug window
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

			ImGui::Text("%-23s: %u", "Buffers", mRenderContext->getBuffers().size());
			ImGui::Text("%-23s: %u", "Draw Calls", 0);
			ImGui::Text("%-23s: %ux%u", "Resolution", mRenderContext->getSettings().mResolutionWidth, mRenderContext->getSettings().mResolutionHeight);

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
				auto name = mRenderContext->getRenderSystems()[i]->getName().data();
				uint32_t size = mRenderContext->getRenderSystems()[i]->mProfileResults.size();
				bool hasTree = false;
				std::vector<std::pair<std::string_view, float>> childProfiles;
				for (auto& profileResult : mRenderContext->getRenderSystems()[i]->mProfileResults)
				{
					if (profileResult.first == name)
					{
						hasTree = ImGui::TreeNodeEx(name, size > 1 ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf, "%-20s: %.3fms", profileResult.first.data(), profileResult.second);
					}
					else
					{
						childProfiles.push_back({ profileResult.first, profileResult.second });
					}
				}
				if (hasTree)
				{
					for (auto& child : childProfiles)
					{
						if (ImGui::TreeNodeEx(child.first.data(), ImGuiTreeNodeFlags_Leaf, "%-17s: %.3fms", child.first.data(), child.second))
						{
							ImGui::TreePop();
						}
						
					}
					ImGui::TreePop();
				}
				
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
	//

	ImGui::Render();
	ImGui::EndFrame();
	ImGui_ImplMRender_RenderDrawData(ImGui::GetDrawData());
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

