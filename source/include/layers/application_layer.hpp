#include "mapp/mapp.hpp"

class ApplicationLayer : public mapp::Layer
{
public:
	virtual void update(const float dt) override;
	virtual void postUpdate(const float dt) override;
};
