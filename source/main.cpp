#include "application_layer.hpp"

#include "mapp/app.hpp"
#include "mapp/window.hpp"
#include "mapp/layer.hpp"

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

    Application* app = new Application(nullptr);
    app->run();

    return 0;
}
