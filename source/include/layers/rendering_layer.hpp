#include "mapp/layer.hpp"

#include "mrender/renderers/renderer.hpp"
#include "mrender/renderers/gi/gi.hpp"

class RenderingLayer : public mapp::Layer
{
public:
    virtual void onInit(mapp::AppContext& context) override;
    virtual void onShutdown() override;
    virtual void onEvent(mapp::Event& event) override;
    virtual void onUpdate(const float& dt) override;

private:
    mapp::AppContext* mContext;
    mrender::RenderContext mRenderContext;
    std::unique_ptr<Capsaicin::Renderer> mRenderer;
    std::vector<std::unique_ptr<Capsaicin::RenderTechnique>> mTechniques;
};