#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/window.hpp"

int main(int argc, char** argv)
{
    // Create and initialize the Window
    mapp::WindowParams windowParams;
    windowParams.mTitle = "My App";
    windowParams.mWidth = 720;
    windowParams.mHeight = 480;
    mapp::Window* window = mapp::Window::create(windowParams);

    // Create the application
    mapp::App* app = new mapp::App(window);

    // Push all layers
    app->pushLayer(new ApplicationLayer());

    // Run the application
    app->run();

    // Clean up and shutdown
    delete app;
    delete window;

    return 0;
}
