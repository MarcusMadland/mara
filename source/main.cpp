#include "mapp/app.hpp"
#include "mapp/window.hpp"
#include "layers/application_layer.hpp"
#include "layers/rendering_layer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv)
{
    // Create and initialize the Window
    mapp::WindowParams windowParams;
    windowParams.mTitle = "My Application";
    windowParams.mWidth = 1280;
    windowParams.mHeight = 720;
    mapp::Window* window = mapp::Window::create(windowParams);

    // Create the application
    mapp::App* app = new mapp::App(window);

    // Push all layers
    app->pushLayer(new ApplicationLayer());
    app->pushLayer(new RenderingLayer());

    // Run the application
    app->run();

    // Clean up and shutdown
    delete app;
    delete window;

    return 0;
}
