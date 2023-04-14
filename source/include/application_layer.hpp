#include "mapp/layer.hpp"

class ApplicationLayer : public mapp::Layer
{
public:
    virtual void onInit() override;
    virtual void onShutdown() override;
    virtual void onUpdate(const float& dt) override;
    virtual void onEvent(mapp::Event& event) override;
};