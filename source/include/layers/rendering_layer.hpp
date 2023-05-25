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
    virtual void onRender(const float& dt) override;
    virtual void onPostRender(const float& dt) override;

private:
    mapp::AppContext* mContext;
    mrender::RenderContext mRenderContext;
    std::unique_ptr<mrender::Renderer> mRenderer;
    std::vector<std::unique_ptr<mrender::RenderSystem>> mRenderSystems;
};