
#pragma once

#include "mrender/mrender.hpp"
#include "mapp/app.hpp"
#include "mcore/math.hpp"

class CameraOrbitController
{
public:
	CameraOrbitController(std::shared_ptr<mrender::Camera> camera);

	void onEvent(mapp::Event& event);
	void onUpdate(const float& dt);

	[[nodiscard]] std::shared_ptr<mrender::Camera> getCamera() { return mCamera; }
private:
	std::shared_ptr<mrender::Camera> mCamera;

	bool mMousePressed;
	float mDistanceFromTarget, mYaw, mPitch;
	mcore::Vector<float, 3> mTargetPosition = { mCamera->getSettings().mLookAt };
	mcore::Vector<float, 3> mPosition = { mCamera->getSettings().mPosition };
	
};