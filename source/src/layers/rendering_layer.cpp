#include "layers/rendering_layer.hpp"

#include "mapp/app.hpp"

void RenderingLayer::onInit(mapp::AppContext& context)
{
	mContext = &context;

	mrender::RenderSettings renderSettings;
	renderSettings.mNativeDisplay = mContext->getWindow()->getNativeDisplay();
	renderSettings.mNativeWindow = mContext->getWindow()->getNativeWindow();
	renderSettings.mResolutionWidth = mContext->getWindow()->getParams().mWidth;
	renderSettings.mResolutionHeight = mContext->getWindow()->getParams().mHeight;
	
	mRenderContext.initialize(renderSettings);
	mTechnique.initialize(mRenderContext);
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
	mTechnique.render();
}
