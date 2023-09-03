#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class Deferred : public RenderSystem
{
public:
    Deferred(GfxContext* context);
    ~Deferred();

    bool init(GfxContext* context) override;
    void render(GfxContext* context) override;

private:
    CameraHandle mShadowCamera;
    RenderStateHandle mShadowState;
    FramebufferHandle mShadowFramebuffer;
    ShaderHandle mShadowShader;

    RenderStateHandle mGeometryState;
    FramebufferHandle mGeometryFramebuffer;

    RenderStateHandle mLightState;
    FramebufferHandle mLightFramebuffer;
    ShaderHandle mPointLightShader;
    ShaderHandle mSpotLightShader;
    ShaderHandle mDirectionalLightShader;

    RenderStateHandle mCombineState;
    FramebufferHandle mCombineFramebuffer;
    ShaderHandle mCombineShader;

    GeometryHandle mScreenQuad;
    struct VertexData
    {
        float x;
        float y;
        float z;
        float texX;
        float texY;
    };
    std::vector<VertexData> mQuadVertices =
    {
        { -1.0f,  1.0f, 0.0f,  0.0f,0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f,0.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f,1.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f,1.0f },
    };
    const std::vector<uint16_t> mQuadIndices =
    {
        0, 1, 2, 1, 3, 2,
    };
};

}   // namespace mrender
