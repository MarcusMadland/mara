#include "layers/rendering_layer.hpp"
#include "mapp/app.hpp"

#include <iostream>

void RenderingLayer::onInit(mapp::AppContext& context)
{
	mContext = &context;

	mrender::RenderSettings renderSettings;
	renderSettings.mRendererName = "GI-1.0";
	renderSettings.mNativeDisplay = mContext->getWindow()->getNativeDisplay();
	renderSettings.mNativeWindow = mContext->getWindow()->getNativeWindow();
	renderSettings.mResolutionWidth = mContext->getWindow()->getParams().mWidth;
	renderSettings.mResolutionHeight = mContext->getWindow()->getParams().mHeight;
	
	mRenderContext.initialize(renderSettings);

	mRenderer = Capsaicin::Renderer::make(renderSettings.mRendererName);
	if (mRenderer)
	{
		mTechniques = std::move(mRenderer->setupRenderTechniques(mRenderContext));

		for (auto& technique : mTechniques)
		{
			technique->init(mRenderContext);
		}
	}
	else
	{
		std::cout << "Unknown renderer passed in to rendersettings" << std::endl;
	}
	
}

void RenderingLayer::onShutdown()
{
	mRenderContext.cleanup();
}

void RenderingLayer::onEvent(mapp::Event& event)
{
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
	for (auto& technique : mTechniques)
	{
		technique->render(mRenderContext);
	}
}
