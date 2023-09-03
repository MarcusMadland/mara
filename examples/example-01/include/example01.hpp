#pragma once

#include "mengine.hpp"
#include "mrender/mrender.hpp"

class Example01 : public mengine::Game
{
public:
	Example01();
	~Example01();
	virtual void update() override;
	virtual void postUpdate() override;

private:
	mrender::RenderableList mRenderables;
};