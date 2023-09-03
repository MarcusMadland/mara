#include "render-systems/empty/empty.hpp"

namespace mrender {

Empty::Empty(GfxContext* context)
	: RenderSystem("Empty")
{
}

Empty::~Empty()
{
}

bool Empty::init(GfxContext* context)
{
	return true;
}

void Empty::render(GfxContext* context)
{
}

}	// namespace mrender