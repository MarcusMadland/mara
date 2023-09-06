#include "render-systems/post-processing/post_processing.hpp"

#include <filesystem>

namespace mrender {

PostProcessing::PostProcessing(GfxContext* context)
    : RenderSystem("Post Processing")
    , mShader(INVALID_HANDLE)
    , mState(INVALID_HANDLE)
    , mScreenQuad(INVALID_HANDLE)
{
}

PostProcessing::~PostProcessing()
{
}

bool PostProcessing::init(GfxContext* context)
{
    // Shader
    mShader = context->createShader(
        std::filesystem::current_path().string() + "/data/shaders/screen-vert.bin",
        std::filesystem::current_path().string() + "/data/shaders/screen-frag.bin");

    // Render state
    mState = context->createRenderState("Post Processing", 0
        | MRENDER_STATE_WRITE_RGB
        | MRENDER_STATE_WRITE_A);

    // Screen quad
    BufferLayout layout =
    {
        { BufferElement::AttribType::Float, BufferElement::Attrib::Position, 3 },
        { BufferElement::AttribType::Float, BufferElement::Attrib::TexCoord0, 2 },
    };
    mScreenQuad = context->createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);

    return true;
}

void PostProcessing::render(GfxContext* context)
{
    PROFILE_SCOPE(mName);

    // Set current renderpass id
    context->setActiveRenderState(mState);
    context->clear(MRENDER_CLEAR_COLOR | MRENDER_CLEAR_DEPTH);

    // Set shader uniforms
   // TextureHandle diffuseBuffer = context->getSharedBuffers().at("Combine");
    TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");

    context->setTexture(mShader, "u_buffer", diffuseBuffer, 1);

    // Submit quad
    context->submit(mScreenQuad, mShader, INVALID_HANDLE);
}

}   // namespace mrender