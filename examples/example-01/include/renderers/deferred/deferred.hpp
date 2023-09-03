#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class DeferredRenderer : public Renderer::Registrar<DeferredRenderer>
{
public:
    static constexpr std::string_view Name = "Deferred";

    DeferredRenderer() {}

    std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(
        mrender::GfxContext* context) override;

};

}   // namespace mrender

