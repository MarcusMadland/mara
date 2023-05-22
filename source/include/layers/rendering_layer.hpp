#include "mapp/layer.hpp"

#include "mrender/renderers/renderer.hpp"
#include "mrender/renderers/techniques/testing.hpp"

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
    mrender::TestingTechnique mTechnique;
};