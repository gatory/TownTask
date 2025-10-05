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
    
private:
    float CalculateRenderY(Entity e, World& world, int currentScene, int guiHeight);
};
