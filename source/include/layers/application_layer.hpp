#include "mapp/layer.hpp"

class ApplicationLayer : public mapp::Layer
{
public:
    virtual void onInit(mapp::AppContext& context) override;
    virtual void onEvent(mapp::Event& event) override;

private:
    mapp::AppContext* mContext;
};