#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/window.hpp"

class Application : public mapp::App
{
public:
    Application(mapp::Window* window) : mapp::App(window)
    {
        pushLayer(new ApplicationLayer());
    }
};

int main(int argc, char** argv)
{
    // Create window
    mapp::WindowParams windowParams;
    windowParams.title = "My App";
    windowParams.width = 1920;
    windowParams.height = 1080;
    mapp::Window* window = mapp::Window::create(windowParams);
    //window->setFullscreen(false);

    // Create app
    Application* app = new Application(window);
    app->run();

    return 0;
}
