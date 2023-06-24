
#include "utils/camera_orbit_controller.hpp"

#include "mapp/input.hpp"

CameraOrbitController::CameraOrbitController(std::shared_ptr<mrender::Camera> camera)
	: mCamera(std::move(camera)), mMousePressed(false), mDistanceFromTarget(5.0f), mYaw(0.0f), mPitch(0.0f)
{
	
}

void CameraOrbitController::onEvent(mapp::Event& event)
{
	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	dispatcher.dispatch<mapp::MouseButtonPressedEvent>(
		[&](const mapp::MouseButtonPressedEvent& e)
		{
			if (e.getKeyCode() == MAPP_MOUSE_LEFT)
			{
				mMousePressed = true;
			}

			return false;
		});

	dispatcher.dispatch<mapp::MouseButtonReleasedEvent>(
		[&](const mapp::MouseButtonReleasedEvent& e)
		{
			if (e.getKeyCode() == MAPP_MOUSE_LEFT)
			{
				mMousePressed = false;
			}

			return false;
		});

	dispatcher.dispatch<mapp::MouseMovedEvent>(
		[&](const mapp::MouseMovedEvent& e)
		{
			static float lastMouseX = 0;
			static float lastMouseY = 0;

			float mouseX = e.getX();
			float mouseY = e.getY();

			float deltaX = mouseX - lastMouseX;
			float deltaY = mouseY - lastMouseY;

			lastMouseX = mouseX;
			lastMouseY = mouseY;

			// Sensitivity controls the speed of camera rotation
			float sensitivity = 0.5f;

			if (mMousePressed)
			{
				// Update camera yaw and pitch based on mouse movement
				mYaw -= deltaX * sensitivity;
				mPitch += deltaY * sensitivity;

				// Clamp the pitch to avoid flipping the camera
				const float maxPitch = 89.0f;
				const float minPitch = -89.0f;
				mPitch = std::clamp(mPitch, minPitch, maxPitch);
			}
			return false;
		});

	dispatcher.dispatch<mapp::MouseScrolledEvent>(
		[&](const mapp::MouseScrolledEvent& e)
		{
			mDistanceFromTarget -= static_cast<float>(e.getYOffset());

			return false;
		});

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			mrender::CameraSettings settings = mCamera->getSettings();
			settings.mWidth = static_cast<float>(e.getWidth());
			settings.mHeight = static_cast<float>(e.getHeight());
			mCamera->setSettings(settings);

			return 0;
		});
}

void CameraOrbitController::onUpdate(const float& dt)
{
	mTargetPosition = { mCamera->getSettings().mLookAt };
	mPosition = { mCamera->getSettings().mPosition };

	mPosition[0] = mTargetPosition[0] + mDistanceFromTarget * std::cos(mcore::toRadians(mYaw)) * std::cos(mcore::toRadians(mPitch));
	mPosition[1] = mTargetPosition[1] + mDistanceFromTarget * std::sin(mcore::toRadians(mPitch));
	mPosition[2] = mTargetPosition[2] + mDistanceFromTarget * std::sin(mcore::toRadians(mYaw)) * std::cos(mcore::toRadians(mPitch));

	// Update the camera settings
	mrender::CameraSettings settings = mCamera->getSettings();
	settings.mPosition[0] = mPosition[0];
	settings.mPosition[1] = mPosition[1];
	settings.mPosition[2] = mPosition[2];
	settings.mLookAt[0] = mTargetPosition[0];
	settings.mLookAt[1] = mTargetPosition[1];
	settings.mLookAt[2] = mTargetPosition[2];
	mCamera->setSettings(settings);
	
}
