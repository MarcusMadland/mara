#pragma once

#include "mrender/mrender.hpp"

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

uint32_t toUnorm(float _value, float _scale);
void packRgba8(void* _dst, const float* _src);
uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f);

std::shared_ptr<mrender::Texture> loadTexture(std::shared_ptr<mrender::RenderContext>& context, std::string_view filename);
std::shared_ptr<mrender::Geometry> loadGeometry(std::shared_ptr<mrender::RenderContext>& context, std::string_view filename);
