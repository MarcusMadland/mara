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

	dispatcher.dispatch<mapp::WindowResizeEvent>(
		[&](const mapp::WindowResizeEvent& e)
		{
			std::cout << "Resized screen: x: " << e.getWidth() << " y: " << e.getHeight() << std::endl;

			Backend::resize(e.getWidth(), e.getHeight());

			return 0;
		});

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

	dispatcher.dispatch<mapp::KeyReleasedEvent>(
		[&](const mapp::KeyReleasedEvent& e)
		{
			std::cout << "Released key: " << e.getKeyCode() << std::endl;

			return false;
		});

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
}
