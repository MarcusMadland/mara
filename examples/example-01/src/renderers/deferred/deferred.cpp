#include "renderers/deferred/deferred.hpp"
#include "render-systems/deferred/deferred.hpp"
#include "render-systems/post-processing/post_processing.hpp"
#include "render-systems/empty/empty.hpp"

namespace mrender {

std::vector<std::shared_ptr<RenderSystem>> DeferredRenderer::setupRenderSystems(GfxContext* context)
{
    std::vector<std::shared_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_shared<Deferred>(context));
    render_techniques.emplace_back(std::make_shared<PostProcessing>(context));
   
    // more ..
    return render_techniques;
}

}