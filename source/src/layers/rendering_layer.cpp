#include "layers/rendering_layer.hpp"

#include "mrender/mrender.hpp"
#include "mcore/math.hpp"

#include <filesystem>

RenderingLayer::RenderingLayer(mapp::Window& window, const mrender::RenderSettings& renderSettings)
{
	// Render Context
	mrender::RenderSettings settings = renderSettings;
	settings.mNativeDisplay = nullptr;
#ifdef XWIN_WIN32
	settings.mNativeWindow = window.getHwnd();
#elif defined(XWIN_COCOA) || defined(XWIN_UIKIT)
	settings.mNativeWindow = window.getWindowHandle();
#endif
	mGfxContext = mrender::createGfxContext(settings);

	// @temp
	{
		// Camera
		mrender::CameraSettings cameraSettings;
		cameraSettings.mProjectionType = mrender::CameraSettings::Perspective;
		cameraSettings.mWidth = settings.mResolutionWidth;
		cameraSettings.mHeight = settings.mResolutionHeight;
		cameraSettings.mClipFar = 1000.0f;
		cameraSettings.mPosition[2] = -8.0f;
		cameraSettings.mPosition[1] = 2.0f;
		mCamera = mGfxContext->createCamera(cameraSettings);

		// Renderables (material)
		mrender::ShaderHandle geometryShader = mGfxContext->createShader(
			std::filesystem::current_path().string() + "/data/shaders/deferred_geo-vert.bin",
			std::filesystem::current_path().string() + "/data/shaders/deferred_geo-frag.bin");
		mrender::MaterialHandle whiteMaterial = mGfxContext->createMaterial(geometryShader);
		static float whiteColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		static float normalColor[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
		mGfxContext->setMaterialUniformData(whiteMaterial, "u_albedoColor", mrender::UniformData::UniformType::Vec4, &whiteColor);
		mGfxContext->setMaterialUniformData(whiteMaterial, "u_normalColor", mrender::UniformData::UniformType::Vec4, &normalColor);

		// Renderables (geometry)
		mrender::BufferLayout layout =
		{
			{ mrender::BufferElement::AttribType::Float, mrender::BufferElement::Attrib::Position, 3 },
			{ mrender::BufferElement::AttribType::Uint8, mrender::BufferElement::Attrib::Normal, 4 },
			{ mrender::BufferElement::AttribType::Uint8, mrender::BufferElement::Attrib::Tangent, 4 },
			{ mrender::BufferElement::AttribType::Int16, mrender::BufferElement::Attrib::TexCoord0, 2 },
		};
		mrender::GeometryHandle cubeGeo = mGfxContext->createGeometry(layout, sCubeVertices.data(), static_cast<uint32_t>(sCubeVertices.size() * sizeof(Vertex)), sCubeIndices);

		// Renderables
		mRenderables.push_back(mGfxContext->createRenderable(cubeGeo, whiteMaterial));
		mGfxContext->setActiveRenderables(mRenderables);

		// Lights
		mrender::LightSettings lightSettings;
		lightSettings.mType = mrender::LightSettings::Directional;
		lightSettings.mIntensity = 2.0f;
		lightSettings.mPosition[0] = 5.0f;
		lightSettings.mPosition[1] = 5.0f;
		lightSettings.mPosition[2] = -5.0f;
		mrender::LightHandle light = mGfxContext->createLight(lightSettings);
		mGfxContext->setActiveLight(light);
	}
}

RenderingLayer::~RenderingLayer()
{
	delete mGfxContext;
}

void RenderingLayer::update(const float dt)
{
	
	// Update cube rotation 
	const float speed = 50.0f;
	static float time = 0.0f;
	time += dt;

	mcore::Matrix4x4<float> modelMatrix;
	mcore::rotateY(modelMatrix, time * speed);
	mGfxContext->setRenderableTransform(mRenderables.at(0), &modelMatrix[0]);
}

void RenderingLayer::postUpdate(const float dt)
{
	mGfxContext->setClearColor(0xff00ffff);

	mGfxContext->render(mCamera);
	mGfxContext->submitDebugText(50, 1, mrender::Color::White, "Build v. 133201092023 (res: %ux%u)", mGfxContext->getSettings().mResolutionWidth, mGfxContext->getSettings().mResolutionHeight);

	mGfxContext->swapBuffers();
}
