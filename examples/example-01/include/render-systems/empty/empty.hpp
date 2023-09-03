#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class Empty : public RenderSystem
{
public:
	Empty(GfxContext* context);
	~Empty();

	bool init(GfxContext* context) override;
	void render(GfxContext* context) override;
};

}	// namespace mrender