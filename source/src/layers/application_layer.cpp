#include "layers/application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/input.hpp"
#include "mapp/window.hpp"

#include <iostream>

void ApplicationLayer::onInit(mapp::AppContext& context)
{
	mContext = &context;
}

void ApplicationLayer::onEvent(mapp::Event& event)
{
	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	// APP
	dispatcher.dispatch<mapp::WindowCloseEvent>(
		[&](const mapp::WindowCloseEvent& e)
		{
			std::cout << "[Window] Close" << std::endl;

			return 0;
		});

	// KEYBOARD
	dispatcher.dispatch<mapp::KeyPressedEvent>(
		[&](const mapp::KeyPressedEvent& e)
		{
			std::cout << "[Key] Pressed: " << e.getKeyCode() << std::endl;

			// If you press ESCAPE we shut down the application and close the window
			if (e.getKeyCode() == MAPP_KEY_ESCAPE)
			{
				mContext->getApp()->shutdown();
			}

			// If you press F11 we set the window to fullscreen if windowed and vice versa
			if (e.getKeyCode() == MAPP_KEY_F11)
			{
				const bool isFullscreen = mContext->getWindow()->getIsFullscreen();
				mContext->getWindow()->setFullscreen(!isFullscreen);
			}

			return false;
		});

	dispatcher.dispatch<mapp::KeyPressingEvent>(
		[&](const mapp::KeyPressingEvent& e)
		{
			//std::cout << "[Key] Pressing: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::KeyReleasedEvent>(
		[&](const mapp::KeyReleasedEvent& e)
		{
			std::cout << "[Key] Released: " << e.getKeyCode() << std::endl;

			return false;
		});

	// MOUSE
	dispatcher.dispatch<mapp::MouseButtonPressedEvent>(
		[&](const mapp::MouseButtonPressedEvent& e)
		{
			std::cout << "[Mouse] Pressed: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseButtonReleasedEvent>(
		[&](const mapp::MouseButtonReleasedEvent& e)
		{
			std::cout << "[Mouse] Released: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseMovedEvent>(
		[&](const mapp::MouseMovedEvent& e)
		{
			//std::cout << "[Mouse] Moved: x: " << e.getX() << " y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseScrolledEvent>(
		[&](const mapp::MouseScrolledEvent& e)
		{
			std::cout << "[Mouse] Scrolled: y: " << e.getYOffset() << std::endl;

			return false;
		});

	// GAMEPAD
	dispatcher.dispatch<mapp::GamepadKeyPressedEvent>(
		[&](const mapp::GamepadKeyPressedEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " Pressed: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadKeyReleasedEvent>(
		[&](const mapp::GamepadKeyReleasedEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " Released: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadKeyPressingEvent>(
		[&](const mapp::GamepadKeyPressingEvent& e)
		{
			//std::cout << "[Gamepad] ID: " << e.getControlledID() << " Pressing: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadLeftJoystickEvent>(
		[&](const mapp::GamepadLeftJoystickEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " JoystickL Left X: " << e.getX() << " Y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadRightJoystickEvent>(
		[&](const mapp::GamepadRightJoystickEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " JoystickR Right X: " << e.getX() << " Y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadLeftTriggerEvent>(
		[&](const mapp::GamepadLeftTriggerEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " TriggerL X: " << e.getX() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadRightTriggerEvent>(
		[&](const mapp::GamepadRightTriggerEvent& e)
		{
			std::cout << "[Gamepad] ID: " << e.getControlledID() << " TriggerR X: " << e.getX() << std::endl;

			return false;
		});
}
