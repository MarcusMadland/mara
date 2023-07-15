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
	mGfxContext = mrender::createGfxContext(renderSettings);

	// Clear color
	mGfxContext->setClearColor(0x00000000);

	// Shaders
	mrender::ShaderHandle shader = mGfxContext->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_geo-vert.bin", 
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_geo-frag.bin");
	mrender::ShaderHandle debugDrawShader = mGfxContext->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/debug_draw-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/debug_draw-frag.bin");

	// Geometry
	mrender::BufferLayout layout =
	{
		{ mrender::BufferElement::AttribType::Float, mrender::BufferElement::Attrib::Position, 3 },
		{ mrender::BufferElement::AttribType::Uint8, mrender::BufferElement::Attrib::Normal, 4 },
		{ mrender::BufferElement::AttribType::Uint8, mrender::BufferElement::Attrib::Tangent, 4 },
		{ mrender::BufferElement::AttribType::Int16, mrender::BufferElement::Attrib::TexCoord0, 2 },
	};
	mrender::GeometryHandle cubeGeo = mGfxContext->createGeometry(layout, s_bunnyVertices.data(), static_cast<uint32_t>(s_bunnyVertices.size() * sizeof(Vertex)), s_bunnyTriList);

	// Textures @todo fix life time of these, the textures need to be loaded of the lifetime of the rendering layer, so make member variables?
	mrender::TextureHandle albedoTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/albedo.png");
	mrender::TextureHandle normalTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/normal.png");
	mrender::TextureHandle specularTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/specular.png");
	static mcore::Vector<float, 4> whiteColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	static mcore::Vector<float, 4> blueColor = { 0.0f, 0.0f, 0.8f, 1.0f };
	static mcore::Vector<float, 4> redColor = { 0.8f, 0.0f, 0.0f, 1.0f };

	// Materials
	mrender::MaterialHandle textureMaterial = mGfxContext->createMaterial(shader);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_albedo", albedoTex);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_normal", normalTex);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_specular", specularTex);

	mrender::MaterialHandle debugDrawMaterial = mGfxContext->createMaterial(debugDrawShader);
	mGfxContext->setMaterialUniformData(debugDrawMaterial, "u_debugColor", mrender::UniformData::UniformType::Vec4, &redColor);

	// Renderables (cubes)
	for (int x = -10; x < 10; x++)
	{
		for (int y = -10; y < 10; y++)
		{
			mrender::RenderableHandle renderable = mGfxContext->createRenderable(cubeGeo, textureMaterial);
			{
				mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
				mcore::Vector<float, 3> position = { (float)x * 3, (float)y * 3, 0.0f };
				mcore::translate(translation, position);

				mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
				mcore::Vector<float, 3> scaleVal = { 1.0f, 1.0f, 1.0f };
				mcore::scale(scale, scaleVal);

				mcore::Matrix4x4<float> model = scale * translation;

				mGfxContext->setRenderableTransform(renderable, &model[0]);
			}

			mCubes.push_back(renderable);
		}
	}
	mGfxContext->setActiveRenderables(mCubes);

	// Lights
	mrender::LightSettings lightSettings;
	mrender::LightHandle light01 = mGfxContext->createLight(lightSettings);
	mGfxContext->setActiveLight(light01);

	// Camera
	mrender::CameraSettings cameraSettings;
	cameraSettings.mProjectionType = mrender::CameraSettings::Perspective;
	cameraSettings.mWidth = static_cast<float>(mGfxContext->getSettings().mResolutionWidth);
	cameraSettings.mHeight = static_cast<float>(mGfxContext->getSettings().mResolutionHeight);
	cameraSettings.mClipFar = 1000.0f;
	cameraSettings.mPosition[2] = -5.0f;
	auto camera = mGfxContext->createCamera(cameraSettings);
	mCamera = std::make_shared<CameraOrbitController>(mGfxContext, camera);

	// ImGui
#ifdef MENGINE_DEBUG
	imguiImplInit();
#endif
}

void RenderingLayer::onShutdown()
{
#ifdef MENGINE_DEBUG
	imguiImplShutdown();
#endif
}

void RenderingLayer::onEvent(mapp::Event& event)
{
	// Camera events
	ImGuiIO io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		mCamera->onEvent(event);
	}

	// Rendering events
	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);
	
	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			// Resize Renderer
			mrender::RenderSettings settings = mGfxContext->getSettings();
			settings.mResolutionWidth = e.getWidth();
			settings.mResolutionHeight = e.getHeight();
			mGfxContext->setSettings(settings);

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

	// Render
	mGfxContext->render(mCamera->getCamera());
	
	// Render ImGui
#ifdef MENGINE_DEBUG
	imguiUpdate();
#endif

	// Debug text (application performance)

	constexpr uint16_t textY = 2;
	constexpr uint16_t textX = 40;

	if (mDrawDebugText)
	{
		mrender::Stats* stats = mGfxContext->getStats();

		// CPU
		float cpuRender = stats->mCpuTime;
		static float cpuRenderHighest = 0.0f;
		if (cpuRender > cpuRenderHighest) cpuRenderHighest = cpuRender;

		// GPU
		float gpuRender = stats->mGpuTime;
		static float gpuRenderHighest = 0.0f;
		if (gpuRender > gpuRenderHighest) gpuRenderHighest = gpuRender;

		// FPS
		float fps = 1 / deltaTime;

		// MEMORY
		float texture = stats->mTextureMemoryUsed;

		mGfxContext->submitDebugText(textX, textY + 0, "%-15s %.2f ms [%.2f ms]", "cpu(game):", 0, 0);
		mGfxContext->submitDebugText(textX, textY + 1, "%-15s %.2f ms [%.2f ms]", "cpu(render):", cpuRender, cpuRenderHighest);
		mGfxContext->submitDebugText(textX, textY + 2, "%-15s %.2f ms [%.2f ms]", "gpu:", gpuRender, gpuRenderHighest);
		mGfxContext->submitDebugText(textX, textY + 3, "%-15s %.2f fps", "framerate:", fps);
		mGfxContext->submitDebugText(textX, textY + 4, "%-15s %.2f / 1454 MiB", "textures:", texture);

		{
			mcore::Vector<float, 3> position = { -1.5f, 0.0f, 0.0f };
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::translate(translation, position);
			mcore::Vector<float, 3> size = { 0.2f, 0.2f, 0.2f };
			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::scale(scale, size);
			mcore::Matrix4x4<float> model = scale * translation;
			//mGfxContext->submitDebugCube(&model[0], mrender::Color::Red);
		}

		//mGfxContext->submitDebugText(textX - 20, textY, mrender::Color::Red, true, false, "Too many vertices", 0);
		//mGfxContext->submitDebugText(textX - 20, textY, mrender::Color::Red, true, false, "Too many this", 0);
		//mGfxContext->submitDebugText(textX - 20, textY, mrender::Color::Red, true, false, "Too many that", 0);
	}
	
	// Swap buffers
	mGfxContext->swapBuffers();
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
				mGfxContext->reloadShaders();
			}
		}
		ImGui::Checkbox("Draw stats", &mDrawDebugText);
		ImGui::Separator();
		
		if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::vector<std::string_view> allRenderers = mGfxContext->getRenderer()->getNames();
			static const char* currentItem = mGfxContext->getSettings().mRendererName.data();
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
				mrender::RenderSettings settings = mGfxContext->getSettings();
				settings.mRendererName = currentItem;
				mGfxContext->setSettings(settings);

				ImGui::EndCombo();
			}
			mrender::Stats* stats = mGfxContext->getStats();
			ImGui::Text("%-23s: %u", "Buffers", mGfxContext->getSharedBuffers().size());
			ImGui::Text("%-23s: %u", "Draw Calls", stats->mNumDrawCalls);
			ImGui::Text("%-23s: %ux%u", "Resolution", mGfxContext->getSettings().mResolutionWidth, mGfxContext->getSettings().mResolutionHeight);

			static bool vSync = mGfxContext->getSettings().mVSync;
			if (ImGui::Checkbox("VSync	", &vSync))
			{
				mrender::RenderSettings settings = mGfxContext->getSettings();
				settings.mVSync = vSync;
				mGfxContext->setSettings(settings);
			}

		}
		
		if (ImGui::CollapsingHeader("Render Systems", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (uint32_t i = 0; i < mGfxContext->getRenderSystems().size(); i++)
			{
				auto name = mGfxContext->getRenderSystems()[i]->getName().data();
				uint32_t size = mGfxContext->getRenderSystems()[i]->mProfileResults.size();
				bool hasTree = false;
				std::vector<std::pair<std::string_view, float>> childProfiles;
				for (auto& profileResult : mGfxContext->getRenderSystems()[i]->mProfileResults)
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

		if (ImGui::CollapsingHeader("Shaders"))
		{
			for (auto& shader : mGfxContext->getActiveShaders())
			{
				ImGui::Text("%-20s", shader.first.c_str());
			}
		}
		
		if (ImGui::CollapsingHeader("Data"))
		{
			mrender::Stats* stats = mGfxContext->getStats();
			
			ImGui::Text("%-23s: %u", "Cameras", stats->mNumCameras);
			ImGui::Text("%-23s: %u", "Framebuffers", stats->mNumFramebuffers);
			ImGui::Text("%-23s: %u", "RenderStates", stats->mNumRenderStates);
			ImGui::Text("%-23s: %u", "Materials", stats->mNumMaterials);
			ImGui::Text("%-23s: %u", "Textures", stats->mNumTextures);
			ImGui::Text("%-23s: %u", "Shaders", stats->mNumShaders);
			ImGui::Text("%-23s: %u", "Geometries", stats->mNumGeometries);
			ImGui::Text("%-23s: %u", "Renderables", stats->mNumRenderables);
			ImGui::Text("%-23s: %u", "Lights", stats->mNumLights);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

	if (ImGui::Begin("Buffers"))
	{
		mrender::TextureHandle shadowMap = mGfxContext->getSharedBuffers().at("ShadowMap");
		mrender::TextureHandle diffuse = mGfxContext->getSharedBuffers().at("GDiffuse");
		mrender::TextureHandle normal = mGfxContext->getSharedBuffers().at("GNormal");
		mrender::TextureHandle specular = mGfxContext->getSharedBuffers().at("GSpecular");
		mrender::TextureHandle position = mGfxContext->getSharedBuffers().at("GPosition");
		mrender::TextureHandle light = mGfxContext->getSharedBuffers().at("Light");

		ImGui::Image(ImTextureID(mGfxContext->getTextureID(shadowMap)), ImVec2(128, 128), ImVec2(-1, 1), ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(diffuse)), ImVec2(217, 128), ImVec2(-1, 1), ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(normal)), ImVec2(217, 128), ImVec2(-1, 1), ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(specular)), ImVec2(217, 128), ImVec2(-1, 1), ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(position)), ImVec2(217, 128), ImVec2(-1, 1), ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(light)), ImVec2(217, 128), ImVec2(-1, 1), ImVec2(0, 0));
	}
	ImGui::End();
	
	static mrender::LightSettings lightSettings;
	if (ImGui::Begin("Light"))
	{
		ImGui::SliderFloat3("Position", lightSettings.mPosition, -20.0f, 20.0f);
		ImGui::SliderFloat3("RGB", lightSettings.mColor, 0.0f, 1.0f);
		ImGui::SliderFloat("Intensity", &lightSettings.mIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Range", &lightSettings.mRange, 0.0f, 10.0f);


	}
	ImGui::End();
	mGfxContext->setLightSettings(mGfxContext->getActiveLights()[0], lightSettings);

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

