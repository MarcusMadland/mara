#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/input.hpp"
#include "mapp/window.hpp"
#include "mrender/testing.hpp"

#include <iostream>

void ApplicationLayer::onInit()
{
	// Init backend renderer
	mapp::Window* window = mapp::App::getInstance().getWindow();
	Backend::init(window->getParams().width, window->getParams().height, window->getNativeWindow(), window->getNativeDisplay());
}

void ApplicationLayer::onShutdown()
{
	// Shutdown backend renderer
    Backend::shutdown();
}

void ApplicationLayer::onUpdate(const float& dt)
{
	// Render the backend renderer
    Backend::render();
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

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			std::cout << "[Window] Resize: x: " << e.getWidth() << " y: " << e.getHeight() << std::endl;

			Backend::resize(e.getWidth(), e.getHeight());

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
				mapp::App::getInstance().shutdown();
			}

			// If you press F11 we set the window to fullscreen if windowed and vice versa
			if (e.getKeyCode() == MAPP_KEY_F11)
			{
				const bool isFullscreen = mapp::App::getInstance().getWindow()->getIsFullscreen();
				mapp::App::getInstance().getWindow()->setFullscreen(!isFullscreen);
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
			std::cout << "[Mouse] Pressed: " << e.getMouseButton() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseButtonReleasedEvent>(
		[&](const mapp::MouseButtonReleasedEvent& e)
		{
			std::cout << "[Mouse] Released: " << e.getMouseButton() << std::endl;

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
