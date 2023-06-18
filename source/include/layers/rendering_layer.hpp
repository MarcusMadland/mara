#include "mapp/layer.hpp"
#include "mrender/mrender.hpp"

struct Vertex
{
    float x;
    float y;
    float z;
    uint32_t abgr;
};

static std::vector<Vertex> s_bunnyVertices =
{
    {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
    {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
    {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
    {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const std::vector<uint16_t> s_bunnyTriList =
{
    0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};

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
    std::shared_ptr<mrender::RenderContext> mRenderContext;
    std::shared_ptr<mrender::Camera> mCamera;

    bool mDrawDebugText = false;
};