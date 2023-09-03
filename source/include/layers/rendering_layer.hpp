#pragma once

#include "mapp/mapp.hpp"
#include "mrender/mrender.hpp"

static uint32_t toUnorm(float _value, float _scale)
{
    return uint32_t(round(std::clamp(_value, 0.0f, 1.0f) * _scale));
}

static void packRgba8(void* _dst, const float* _src)
{
    uint8_t* dst = (uint8_t*)_dst;
    dst[0] = uint8_t(toUnorm(_src[0], 255.0f));
    dst[1] = uint8_t(toUnorm(_src[1], 255.0f));
    dst[2] = uint8_t(toUnorm(_src[2], 255.0f));
    dst[3] = uint8_t(toUnorm(_src[3], 255.0f));
}

static uint32_t encodeNormalRgba8(float _x, float _y, float _z, float _w = 0.0f)
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

static std::vector<Vertex> sCubeVertices =
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

static const std::vector<uint16_t> sCubeIndices =
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
	RenderingLayer(mapp::Window& window, const mrender::RenderSettings& renderSettings);
	~RenderingLayer();
	virtual void update(const float dt) override;
	virtual void postUpdate(const float dt) override;

private:
	mrender::GfxContext* mGfxContext = nullptr;

	// @temp
	mrender::CameraHandle mCamera = INVALID_HANDLE;
	mrender::RenderableList mRenderables;
};