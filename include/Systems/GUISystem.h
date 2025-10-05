#pragma once
#include "Core/World.h"
#include "Game/GameState.h"

class GUISystem {
public:
    void Render(World& world, int currentScene, PomodoroTimer& pomodoroTimer, int guiHeight);
    
private:
    void RenderPomodoroGUI(World& world, PomodoroTimer& timer, int guiHeight);
};