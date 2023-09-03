#include "mengine.hpp"
#include "mapp/mapp.hpp"
#include "layers/application_layer.hpp"
#include "layers/rendering_layer.hpp"

namespace mengine {

void Game::run(const GameDesc& desc)
{
    mapp::Window window;
    mapp::EventQueue eventQueue;
    if (!window.create(desc.windowDesc, eventQueue))
    {
        printf("Failed to create window\n");
        return;
    }

    mapp::LayerList layers;
    layers.push_back(new ApplicationLayer());
    layers.push_back(new RenderingLayer(window, desc.renderSettings));

    float dt = 0.0f;
    bool isRunning = true;
    while (isRunning)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        // Update engine layers
        for (auto& layer : layers)
        {
            layer->update(dt);
        }

        // Update game
        this->update();
        
        // Update events
        eventQueue.update();
        while (!eventQueue.empty())
        {
            const mapp::Event& event = eventQueue.front();

            if (event.type == mapp::EventType::MouseMove)
            {
                const mapp::MouseMoveData mouse = event.data.mouseMove;
            }
            if (event.type == mapp::EventType::Close)
            {
                window.close();
                isRunning = false;
            }

            eventQueue.pop();
        }

        // Post update engine layers
        for (auto& layer : layers)
        {
            layer->postUpdate(dt);
        }

        // Post update game
        this->postUpdate();

        // Time
        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> duration = end - start;

        mLastFrameTime = dt;
        dt = duration.count();
    }
}

}