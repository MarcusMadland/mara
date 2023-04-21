#include "application_layer.hpp"
#include "mapp/app.hpp"
#include "mrender/testing.hpp"
#include "mapp/input.hpp"

#include <iostream>

void ApplicationLayer::onInit()
{
	std::cout << "Application layer called." << std::endl;
	mapp::Window* window = mapp::App::getInstance().getWindow();
	Backend::init(window->getParams().width, window->getParams().height, window->getNativeWindow(), window->getNativeDisplay());
}

void ApplicationLayer::onShutdown()
{
   Backend::shutdown();
}

void ApplicationLayer::onUpdate(const float& dt)
{
   Backend::render();
}

void ApplicationLayer::onEvent(mapp::Event& event)
{
	mapp::EventDispatcher dispatcher = mapp::EventDispatcher(event);

	// APP
	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			std::cout << "Resized screen: x: " << e.getWidth() << " y: " << e.getHeight() << std::endl;

			Backend::resize(e.getWidth(), e.getHeight());

			return 0;
		});

	// KEYBOARD
	dispatcher.dispatch<mapp::KeyPressedEvent>(
		[&](const mapp::KeyPressedEvent& e)
		{
			std::cout << "Pressed key: " << e.getKeyCode() << std::endl;

			// If you press ESCAPE we shut down the application and close the window
			if (e.getKeyCode() == MAPP_KEY_ESCAPE)
			{
				mapp::App::getInstance().shutdown();
			}

			return false;
		});

	dispatcher.dispatch<mapp::KeyPressingEvent>(
		[&](const mapp::KeyPressingEvent& e)
		{
			//std::cout << "Pressing key: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::KeyReleasedEvent>(
		[&](const mapp::KeyReleasedEvent& e)
		{
			std::cout << "Released key: " << e.getKeyCode() << std::endl;

			return false;
		});

	// MOUSE
	dispatcher.dispatch<mapp::MouseButtonPressedEvent>(
		[&](const mapp::MouseButtonPressedEvent& e)
		{
			std::cout << "Pressed mouse: " << e.getMouseButton() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseButtonReleasedEvent>(
		[&](const mapp::MouseButtonReleasedEvent& e)
		{
			std::cout << "Released mouse: " << e.getMouseButton() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseMovedEvent>(
		[&](const mapp::MouseMovedEvent& e)
		{
			//std::cout << "Moved mouse: x: " << e.getX() << " y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::MouseScrolledEvent>(
		[&](const mapp::MouseScrolledEvent& e)
		{
			std::cout << "Moved scrolled: x: " << e.getXOffset() << " y: " << e.getYOffset() << std::endl;

			return false;
		});

	// GAMEPAD
	dispatcher.dispatch<mapp::GamepadKeyPressedEvent>(
		[&](const mapp::GamepadKeyPressedEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " Pressed: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadKeyReleasedEvent>(
		[&](const mapp::GamepadKeyReleasedEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " Released: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadKeyPressingEvent>(
		[&](const mapp::GamepadKeyPressingEvent& e)
		{
			//std::cout << "ID: " << e.getControlledID() << " Pressing: " << e.getKeyCode() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadLeftJoystickEvent>(
		[&](const mapp::GamepadLeftJoystickEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " JoystickL Left X: " << e.getX() << " Y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadRightJoystickEvent>(
		[&](const mapp::GamepadRightJoystickEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " JoystickR Right X: " << e.getX() << " Y: " << e.getY() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadLeftTriggerEvent>(
		[&](const mapp::GamepadLeftTriggerEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " TriggerL X: " << e.getX() << std::endl;

			return false;
		});

	dispatcher.dispatch<mapp::GamepadRightTriggerEvent>(
		[&](const mapp::GamepadRightTriggerEvent& e)
		{
			std::cout << "ID: " << e.getControlledID() << " TriggerR X: " << e.getX() << std::endl;

			return false;
		});
}
