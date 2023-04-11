#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/window.hpp"
#include "mapp/layer.hpp"

#include "mapp/platform-win/window_win.hpp"

#include <iostream>

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
    std::cout << "Main called." << std::endl;

    mapp::WindowWin* windowsWindow = new mapp::WindowWin("My App", 1280, 720);

    Application* app = new Application(windowsWindow);
    app->run();

    return 0;
}
