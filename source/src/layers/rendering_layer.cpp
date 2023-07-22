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
	mrender::GeometryHandle cubeGeo = mGfxContext->createGeometry(layout, sCubeVertices.data(), static_cast<uint32_t>(sCubeVertices.size() * sizeof(Vertex)), sCubeIndices);

	// Textures @todo fix life time of these, the textures need to be loaded of the lifetime of the rendering layer, so make member variables?
	mrender::TextureHandle albedoTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/albedo.png");
	mrender::TextureHandle normalTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/normal.png");
	mrender::TextureHandle specularTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/specular.png");
	mrender::TextureHandle blankNormalTex = loadTexture(mGfxContext, "C:/Users/marcu/Dev/mengine/resources/blank_normal.jpg");
	static mcore::Vector<float, 4> whiteColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	static mcore::Vector<float, 4> normalColor = { 0.5f, 0.5f, 1.0f, 1.0f };
	static mcore::Vector<float, 4> redColor = { 0.8f, 0.0f, 0.0f, 1.0f };

	// Materials
	mrender::MaterialHandle textureMaterial = mGfxContext->createMaterial(shader);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_albedo", albedoTex);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_normal", normalTex);
	mGfxContext->setMaterialTextureData(textureMaterial, "u_specular", specularTex);

	mrender::MaterialHandle floorMaterial = mGfxContext->createMaterial(shader);
	mGfxContext->setMaterialUniformData(floorMaterial, "u_albedoColor", mrender::UniformData::UniformType::Vec4, &whiteColor);
	mGfxContext->setMaterialUniformData(floorMaterial, "u_normalColor", mrender::UniformData::UniformType::Vec4, &normalColor);

	mrender::MaterialHandle debugDrawMaterial = mGfxContext->createMaterial(debugDrawShader);
	mGfxContext->setMaterialUniformData(debugDrawMaterial, "u_debugColor", mrender::UniformData::UniformType::Vec4, &redColor);

	// Renderables (cubes)
	/*
	for (int x = -10; x < 10; x++)
	{
		for (int y = 1; y < 11; y++)
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
	}*/
	{
		mrender::RenderableHandle renderable = mGfxContext->createRenderable(cubeGeo, textureMaterial);
		{
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> position = { 0.0f, 1.0f, 0.0f };
			mcore::translate(translation, position);

			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> scaleVal = { 1.0f, 1.0f, 1.0f };
			mcore::scale(scale, scaleVal);

			mcore::Matrix4x4<float> model = scale * translation;

			mGfxContext->setRenderableTransform(renderable, &model[0]);
		}

		mCubes.push_back(renderable);
	}
	{
		mrender::RenderableHandle renderable = mGfxContext->createRenderable(cubeGeo, textureMaterial);
		{
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> position = { 5.0f, 1.5, 5.0f };
			mcore::translate(translation, position);

			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> scaleVal = { 1.5f, 1.5f, 1.5f };
			mcore::scale(scale, scaleVal);

			mcore::Matrix4x4<float> model = scale * translation;

			mGfxContext->setRenderableTransform(renderable, &model[0]);
		}

		mCubes.push_back(renderable);
	}
	{
		mrender::RenderableHandle renderable = mGfxContext->createRenderable(cubeGeo, textureMaterial);
		{
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> position = { 3.0f, 1.5, -1.0f };
			mcore::translate(translation, position);

			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> scaleVal = { 1.5f, 1.5f, 1.5f };
			mcore::scale(scale, scaleVal);

			mcore::Matrix4x4<float> model = scale * translation;

			mGfxContext->setRenderableTransform(renderable, &model[0]);
		}

		mCubes.push_back(renderable);
	}
	mGfxContext->setActiveRenderables(mCubes);
	{
		mrender::RenderableHandle renderable = mGfxContext->createRenderable(cubeGeo, debugDrawMaterial);
		{
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> position = { 5.0f, 5.0f, 2.0f };
			mcore::translate(translation, position);

			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> scaleVal = { 0.1f, 0.1f, 0.1f };
			mcore::scale(scale, scaleVal);

			mcore::Matrix4x4<float> model = scale * translation;

			mGfxContext->setRenderableTransform(renderable, &model[0]);
		}

		mCubes.push_back(renderable);
	}
	mGfxContext->setActiveRenderables(mCubes);
	{
		mFloor = mGfxContext->createRenderable(cubeGeo, floorMaterial);

		mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
		mcore::Vector<float, 3> position = { 0.0f, -50.0f, 0.0f };
		mcore::translate(translation, position);

		mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
		mcore::Vector<float, 3> scaleVal = { 50.0f, 50.0f, 50.0f };
		mcore::scale(scale, scaleVal);

		mcore::Matrix4x4<float> model = scale * translation;

		mGfxContext->setRenderableTransform(mFloor, &model[0]);
	}
	mGfxContext->setActiveRenderable(mFloor);

	// Renderable (lights)
	{
		mrender::LightSettings lightSettings;
		lightSettings.mType = mrender::LightSettings::Directional;
		lightSettings.mIntensity = 2.0f;
		lightSettings.mPosition[0] = 5.0f; 
		lightSettings.mPosition[1] = 5.0f;
		lightSettings.mPosition[2] = 2.0f;
		mrender::LightHandle light = mGfxContext->createLight(lightSettings);
		mGfxContext->setActiveLight(light);
	}
	/*
	for (uint32_t i = 0; i < mNumLights; i++)
	{
		mrender::LightSettings lightSettings;
		lightSettings.mType = mrender::LightSettings::Point;
		mrender::LightHandle light = mGfxContext->createLight(lightSettings);
		mGfxContext->setActiveLight(light);
	}*/
	

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
	// Update Camera
	mCamera->onUpdate(dt);

	/*
	// Update light animation
	const float speed = 0.05f;
	static float time = 0.0f;
	time += dt;

	//
	mrender::LightHandle dirLight = mGfxContext->getActiveLights().at(0);
	mrender::LightSettings settings = mGfxContext->getLightSettings(dirLight);

	float radius = 1.0f; 
	float angle = time * speed * 2.0f * 3.14159f; 
	float x = radius * cos(angle);
	float z = radius * sin(angle);

	settings.mPosition[0] = x;
	settings.mPosition[1] = z;
	settings.mPosition[2] = x;
	std::array<float, 3> dirLightPos;
	dirLightPos[0] = settings.mPosition[0];
	dirLightPos[1] = settings.mPosition[1];
	dirLightPos[2] = settings.mPosition[2];
	mGfxContext->setOption("DirectionalLightPosition", dirLightPos);
	mGfxContext->setLightSettings(dirLight, settings);

	//
	
	const float offset = 15.0f;
	for (uint32_t i = 1; i < mGfxContext->getActiveLights().size(); i++)
	{
		mrender::LightSettings settings = mGfxContext->getLightSettings(mGfxContext->getActiveLights().at(i));

		if (settings.mType == mrender::LightSettings::LightType::Directional)
			return;

		settings.mRange = 2.0f;
		settings.mIntensity = 1.5f;

		// Animate position
		float lightTime = time * speed * (sin(i / float(30) * 1.57079632679) * 0.5f + 0.5f);
		settings.mPosition[0] = sin(((lightTime + i * 0.47f) + 1.57079632679 * 1.37f)) * offset;
		settings.mPosition[1] = cos(((lightTime + i * 0.69f) + 1.57079632679 * 1.49f)) * offset;
		settings.mPosition[2] = sin(((lightTime + i * 0.37f) + 1.57079632679 * 1.57f)) * 2.0f;

		// Animate color
		uint8_t val = i & 7;
		settings.mColor[0] = val & 0x1 ? 1.0f : 0.25f,
		settings.mColor[1] = val & 0x2 ? 1.0f : 0.25f,
		settings.mColor[2] = val & 0x4 ? 1.0f : 0.25f,

		mGfxContext->setLightSettings(mGfxContext->getActiveLights().at(i), settings);
		{
			mcore::Matrix4x4<float> translation = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> position = mGfxContext->getLightSettings(mGfxContext->getActiveLights().at(i)).mPosition;
			mcore::translate(translation, position);

			mcore::Matrix4x4<float> scale = mcore::Matrix4x4<float>::identity();
			mcore::Vector<float, 3> scaleVal = { 0.1f, 0.1f, 0.1f };
			mcore::scale(scale, scaleVal);

			mcore::Matrix4x4<float> model = scale * translation;

			mGfxContext->setRenderableTransform(mLightCubes[i], &model[0]);
		}

	}
	*/
	
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

	static bool showXRayWindow = false;

	// MAIN Window
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
		ImGui::SameLine();
		if (ImGui::Button("XRay"))
		{
			showXRayWindow = !showXRayWindow;
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

	// XRAY Window
	if (showXRayWindow)
	{
		const float desiredWindowSize = mGfxContext->getSettings().mResolutionHeight - 28.0f;
		ImGui::SetNextWindowContentSize(ImVec2(desiredWindowSize, desiredWindowSize));
		if (ImGui::Begin("XRay", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			const char* items[] = { "GDiffuse", "GNormal", "Light", "GDepth" };
			static const char* current_item = "GDiffuse";
			if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
					if (ImGui::Selectable(items[n], is_selected))
						current_item = items[n];
					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
				ImGui::EndCombo();
			}
			if (current_item)
			{
				mrender::TextureHandle map = mGfxContext->getSharedBuffers().at(current_item);

				ImVec2 windowPos = ImGui::GetWindowPos();
				ImVec2 windowSize = ImGui::GetContentRegionAvail();
				ImVec2 screenSize = ImGui::GetIO().DisplaySize;

				ImVec2 uv0 = ImVec2(windowPos.x / screenSize.x, (-windowPos.y + windowSize.y) / screenSize.y);
				ImVec2 uv1 = ImVec2((windowPos.x + windowSize.x) / screenSize.x, -windowPos.y / screenSize.y);


				ImGui::Image(ImTextureID(mGfxContext->getTextureID(map)), windowSize, uv0, uv1);
			}
		}
		ImGui::End();
	}

	if (ImGui::Begin("ShadowTesting"))
	{
		mrender::TextureHandle shadow = mGfxContext->getSharedBuffers().at("ShadowMap");
		ImGui::Image(ImTextureID(mGfxContext->getTextureID(shadow)), {512, 512}, {-1.0f, 1.0f}, {0.0f, 0.0f});
	}
	ImGui::End();

	if (ImGui::Begin("Option Testing"))
	{
		// OPTION TESTING
		for (auto& option : mGfxContext->getOptions())
		{
			if (std::holds_alternative<int>(option.second.mValue))
			{
				static int value = mGfxContext->getOptionValue<int>(option.first);;
				ImGui::SliderInt(option.first.c_str(), &value, option.second.mMin, option.second.mMax);
				mGfxContext->setOption(option.first, value);
			}
			if (std::holds_alternative<bool>(option.second.mValue))
			{
				static bool value = mGfxContext->getOptionValue<bool>(option.first);;
				ImGui::Checkbox(option.first.c_str(), &value);
				mGfxContext->setOption(option.first, value);
			}
			if (std::holds_alternative<std::array<float, 3>>(option.second.mValue))
			{
				static std::array<float, 3> value = mGfxContext->getOptionValue<std::array<float, 3>>(option.first);;
				ImGui::SliderFloat3(option.first.c_str(), &value[0], option.second.mMin, option.second.mMax);
				mGfxContext->setOption(option.first, value);
				auto settings = mGfxContext->getLightSettings(mGfxContext->getActiveLights().at(0));
				settings.mPosition[0] = value[0];
				settings.mPosition[1] = value[1];
				settings.mPosition[2] = value[2];
				mGfxContext->setLightSettings(mGfxContext->getActiveLights().at(0), settings); // @todo temp
			}
		}
	}
	ImGui::End();

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

