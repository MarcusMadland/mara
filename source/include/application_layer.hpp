#include "mapp/layer.hpp"

class ApplicationLayer : public mapp::Layer
{
    virtual void onInit() override;
    virtual void onShutdown() override;
    virtual void onUpdate(const float& dt) override;
};