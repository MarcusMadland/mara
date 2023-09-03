#pragma once

#include "mapp/mapp.hpp"
#include "mrender/mrender.hpp"

// @todo Temp stuff to get a game going
namespace mengine {

struct GameDesc
{
	mapp::WindowDesc windowDesc;
	mrender::RenderSettings renderSettings;
};

class Game
{
public:
	virtual void update() = 0;
	virtual void postUpdate() = 0;

	void run(const GameDesc& desc);

private:
	float mLastFrameTime = 0.0f;
};


}