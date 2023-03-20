#include "mapp/app.hpp"
#include "application_layer.hpp"
#include "mcore/file_ops.hpp"

class Application : public mapp::App
{
public:
    Application(const mapp::AppParams& params) : mapp::App(params)
    {
        pushLayer(new ApplicationLayer());
    };
};

int main(int argc, char** argv)
{
    mapp::AppParams params;
    params.name = "My Application";
    params.width = 720;

    std::unique_ptr<Application> app = std::make_unique<Application>(params);
    app->run();

    return 0;
}
