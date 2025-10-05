#pragma once
#include "Core/World.h"

// Forward-declare PomodoroTimer to avoid including GameState.h here
class PomodoroTimer;

class SceneManager;

class InteractionSystem {
public:
    void Update(World& world, SceneManager& sceneManager);
    
private:
    Entity FindPlayerEntity(World& world);
    void HandleBuildingEntry(World& world, SceneManager& sceneManager, Entity player);
    void HandleInteriorInteraction(World& world, int currentScene);
};