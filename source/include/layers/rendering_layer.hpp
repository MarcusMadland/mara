#include "mapp/layer.hpp"
#include "mrender/mrender.hpp"

class RenderingLayer : public mapp::Layer
{
public:
    virtual void onInit(mapp::AppContext& context) override;
    virtual void onShutdown() override;
    virtual void onEvent(mapp::Event& event) override;
    virtual void onUpdate(const float& dt) override;
    virtual void onRender() override;

private:
    void renderUserInterface();

    void imguiImplInit();
    void imguiImplShutdown();
    void imguiImplBegin();
    void imguiImplEnd();

private:
    mapp::AppContext* mAppContext;
    std::unique_ptr<mrender::RenderContext> mRenderContext;

    bool mDrawDebugText = false;
};