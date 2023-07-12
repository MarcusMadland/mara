#pragma once

#include "mrender/mrender.hpp"



uint32_t toUnorm(float _value, float _scale);
void packRgba8(void* _dst, const float* _src);
uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f);

mrender::TextureHandle loadTexture(mrender::GfxContext* context, std::string_view filename);
