#include "mapp/layer.hpp"
#include "mrender/mrender.hpp"

#include "utils/camera_orbit_controller.hpp"

inline uint32_t toUnorm(float _value, float _scale)
{
    return uint32_t(round(std::clamp(_value, 0.0f, 1.0f) * _scale));
}

inline void packRgba8(void* _dst, const float* _src)
{
    uint8_t* dst = (uint8_t*)_dst;
    dst[0] = uint8_t(toUnorm(_src[0], 255.0f));
    dst[1] = uint8_t(toUnorm(_src[1], 255.0f));
    dst[2] = uint8_t(toUnorm(_src[2], 255.0f));
    dst[3] = uint8_t(toUnorm(_src[3], 255.0f));
}

inline uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
{
    const float src[] =
    {
        _x * 0.5f + 0.5f,
        _y * 0.5f + 0.5f,
        _z * 0.5f + 0.5f,
        _w * 0.5f + 0.5f,
    };
    uint32_t dst;
    packRgba8(&dst, src);
    return dst;
}

struct Vertex
{
    float mX;
    float mY;
    float mZ;
    uint32_t mNormal;
    uint32_t mTangent;
    int16_t mU;
    int16_t mV;
};

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
    std::shared_ptr<CameraOrbitController> mCamera;

    bool mDrawDebugText = false;
};