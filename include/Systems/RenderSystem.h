#pragma once
#include "Core/World.h"
#include "raylib.h"

class RenderSystem {
public:
    void LoadTextures(World& world);
    void UnloadTextures(World& world);
    void RenderBuildingInteriorBackgrounds(World& world, int currentScene, int guiHeight, int gameHeight);
    void RenderEntities(World& world, int currentScene, int guiHeight);
    void RenderSpeechBubbles(World& world, Texture2D speechBubbleTexture);
    // Render the main scene background texture stretched to the full screen
    void RenderMainBackground(World& world);

    // Dedicated main background texture (optional)
    Texture2D mainBackgroundTexture;
    bool mainBackgroundLoaded = false;
    
private:
    float CalculateRenderY(Entity e, World& world, int currentScene, int guiHeight);
};
