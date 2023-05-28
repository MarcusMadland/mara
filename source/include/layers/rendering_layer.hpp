#include "mapp/layer.hpp"
#include "mrender/handler/render_context.hpp"
#include "mrender/renderers/renderer.hpp"

class RenderingLayer : public mapp::Layer
{
public:
    virtual void onInit(mapp::AppContext& context) override;
    virtual void onShutdown() override;
    virtual void onEvent(mapp::Event& event) override;
    virtual void onRender() override;

private:
    void renderUserInterface();

    void imguiImplInit();
    void imguiImplShutdown();
    void imguiImplBegin();
    void imguiImplEnd();

private:
    mapp::AppContext* mAppContext;
    mrender::RenderContext mRenderContext;
    mrender::RenderSettings mRenderSettings;

    bool mDrawDebugText = false;
};