#include "mengine.hpp"

#include "example01.hpp" // Game
#include "renderers/deferred/deferred.hpp" // Renderer

void mappMain(int argc, const char** argv)
{
    mengine::GameDesc gameDesc;
    gameDesc.windowDesc.name = "Example01";
    gameDesc.windowDesc.title = "Example01 - Game";
    gameDesc.windowDesc.width = 540;
    gameDesc.windowDesc.height = 960;
    gameDesc.windowDesc.fullscreen = false;
    gameDesc.renderSettings.mGraphicsAPI = mrender::RenderSettings::GraphicsAPI::OpenGL;    
    gameDesc.renderSettings.mRendererName = "Deferred";
    gameDesc.renderSettings.mVSync = true;
    gameDesc.renderSettings.mEnableMultithreading = false;
    gameDesc.renderSettings.mResolutionWidth = 540;
    gameDesc.renderSettings.mResolutionHeight = 960;

    Example01* game = new Example01();
    game->run(gameDesc);

    delete game;
}
