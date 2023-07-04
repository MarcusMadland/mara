#include "utils/loader.hpp"

#include <stb_image.h>

uint32_t toUnorm(float _value, float _scale)
{
	return uint32_t(round(std::clamp(_value, 0.0f, 1.0f) * _scale));
}

void packRgba8(void* _dst, const float* _src)
{
	uint8_t* dst = (uint8_t*)_dst;
	dst[0] = uint8_t(toUnorm(_src[0], 255.0f));
	dst[1] = uint8_t(toUnorm(_src[1], 255.0f));
	dst[2] = uint8_t(toUnorm(_src[2], 255.0f));
	dst[3] = uint8_t(toUnorm(_src[3], 255.0f));
}

uint32_t encodeNormalRgba8(float _x, float _y, float _z, float _w)
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

mrender::TextureHandle loadTexture(mrender::GfxContext* context, std::string_view filename)
{
	stbi_set_flip_vertically_on_load(true);
	int width = 0, height = 0, channels = 0;
	const uint8_t* data = stbi_load(filename.data(), &width, &height, &channels, 4);
	return context->createTexture(data, mrender::TextureFormat::RGBA8, 0, width, height, 4);
}
