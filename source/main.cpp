#include "mapp/app.hpp"
#include "application_layer.hpp"
#include "mcore/file_ops.hpp"
#include "mapp/window.hpp"
#include "mapp/event.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
#include <iostream>


class Application : public mapp::App
{
public:
    Application(mapp::Window* window) 
        : mapp::App(window)
    {
        pushLayer(new ApplicationLayer());
    };
};

class WindowSDL : public mapp::Window
{
public:
    WindowSDL(const char* name, uint32_t width, uint32_t height)
        : mapp::Window(name, width, height)
        , window(nullptr)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
            return;
        }

        window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            width,
            height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        if (window == nullptr) {
            printf(
                "Window could not be created. SDL_Error: %s\n", SDL_GetError());
            return;
        }
    }

    ~WindowSDL() 
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    virtual void onUpdate(const float& dt) override 
    { 
        // Handle Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_WINDOWEVENT) 
            {
                switch (e.window.event) 
                {
                    case SDL_WINDOWEVENT_RESIZED: 
                    {
                        mapp::WindowResizeEvent event = mapp::WindowResizeEvent(
                            e.window.data1, e.window.data2);
                        windowInfo.eventCallback(event);
                        break;
                    }
                        
                    case SDL_WINDOWEVENT_CLOSE: 
                    {
                        mapp::WindowCloseEvent event;
                        windowInfo.eventCallback(event);
                        break;
                    }  
                }
            }
        }
    }

private:
    SDL_Window* window;
};

int main(int argc, char** argv)
{
    WindowSDL* window = new WindowSDL("My Application", 1280, 720);
    Application* app = new Application(window);
    app->run();

    return 0;
}
