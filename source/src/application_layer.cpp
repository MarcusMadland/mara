#include "application_layer.hpp"
#include "mapp/app.hpp"
#include "mrender/testing.hpp"

#include <iostream>

void ApplicationLayer::onInit()
{
	std::cout << "Application layer called." << std::endl;
   // Backend::init(window->getWidth(), window->getHeight(), nativeWindow, nativeDisplay);
}

void ApplicationLayer::onShutdown()
{
   //Backend::shutdown();
}

void ApplicationLayer::onUpdate(const float& dt)
{
   //Backend::render();
}
