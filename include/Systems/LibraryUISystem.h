#pragma once
#include "Core/World.h"

class LibraryUISystem {
public:
    void Render(World& world, int currentScene);
    void HandleInput(World& world);
    
private:
    void RenderLibraryUI(LibraryData& library, int guiHeight);
};
