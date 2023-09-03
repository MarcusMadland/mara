#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class PostProcessing : public RenderSystem
{
public:
    PostProcessing(GfxContext* context);
    ~PostProcessing();

    bool init(GfxContext* context) override;
    void render(GfxContext* context) override;

private:
    ShaderHandle mShader;
    RenderStateHandle mState;

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
