#include "mapp/layer.hpp"
#include "mrender/mrender.hpp"
#include "mrender/utils/timable.hpp"
#include "utils/loader.hpp"

#include "utils/camera_orbit_controller.hpp"

static std::vector<Vertex> s_bunnyVertices =
{
    {-1.0f,  1.0f,  1.0f, encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0,      0,      0 },
    { 1.0f,  1.0f,  1.0f, encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0, 0x7fff,      0 },
    {-1.0f, -1.0f,  1.0f, encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0,      0, 0x7fff },
    { 1.0f, -1.0f,  1.0f, encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0, 0x7fff, 0x7fff },
    {-1.0f,  1.0f, -1.0f, encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0,      0,      0 },
    { 1.0f,  1.0f, -1.0f, encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0, 0x7fff,      0 },
    {-1.0f, -1.0f, -1.0f, encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0,      0, 0x7fff },
    { 1.0f, -1.0f, -1.0f, encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0, 0x7fff, 0x7fff },
    {-1.0f,  1.0f,  1.0f, encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0,      0,      0 },
    { 1.0f,  1.0f,  1.0f, encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0, 0x7fff,      0 },
    {-1.0f,  1.0f, -1.0f, encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0,      0, 0x7fff },
    { 1.0f,  1.0f, -1.0f, encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0, 0x7fff, 0x7fff },
    {-1.0f, -1.0f,  1.0f, encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0,      0,      0 },
    { 1.0f, -1.0f,  1.0f, encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0, 0x7fff,      0 },
    {-1.0f, -1.0f, -1.0f, encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0,      0, 0x7fff },
    { 1.0f, -1.0f, -1.0f, encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0, 0x7fff, 0x7fff },
    { 1.0f, -1.0f,  1.0f, encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0,      0,      0 },
    { 1.0f,  1.0f,  1.0f, encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
    { 1.0f, -1.0f, -1.0f, encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
    { 1.0f,  1.0f, -1.0f, encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
    {-1.0f, -1.0f,  1.0f, encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0,      0,      0 },
    {-1.0f,  1.0f,  1.0f, encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
    {-1.0f, -1.0f, -1.0f, encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
    {-1.0f,  1.0f, -1.0f, encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
};

static const std::vector<uint16_t> s_bunnyTriList =
{
    0,  2,  1,
     1,  2,  3,
     4,  5,  6,
     5,  7,  6,

     8, 10,  9,
     9, 10, 11,
    12, 13, 14,
    13, 15, 14,

    16, 18, 17,
    17, 18, 19,
    20, 21, 22,
    21, 23, 22,
};

class RenderingLayer : public mapp::Layer, mrender::Timable
{
public:
    virtual void onInit(mapp::AppContext& context) override;
    virtual void onShutdown() override;
    virtual void onEvent(mapp::Event& event) override;
    virtual void onUpdate(const float& dt) override;
    virtual void onRender() override;

private:
    void imguiUpdate();
    void imguiImplInit();
    void imguiImplShutdown();

private:
    mapp::AppContext* mAppContext;
    std::shared_ptr<mrender::RenderContext> mRenderContext;
    std::shared_ptr<CameraOrbitController> mCamera;

    bool mDrawDebugText = false;
};