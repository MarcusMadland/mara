#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mrender/testing.hpp"

void ApplicationLayer::onInit()
{
    mapp::Window* window = mapp::App::getInstance().getWindow();

    void* nativeWindow = window->getNativeWindow();
    void* nativeDisplay = window->getNativeWindow();

    Backend::init(window->getWidth(), window->getHeight(), nativeWindow, nativeDisplay);
}

void ApplicationLayer::onShutdown()
{
    Backend::shutdown();
}

void ApplicationLayer::onUpdate(const float& dt)
{
    Backend::render();
}
