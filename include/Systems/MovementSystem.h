// ============================================================================
// include/Systems/MovementSystem.h
// ============================================================================
#pragma once
#include "Core/World.h"
// Forward-declare PomodoroTimer to avoid including GameState.h here
class PomodoroTimer;

class MovementSystem {
public:
    void UpdatePlayerInput(World& world, float deltaTime, int currentScene);
    void UpdateAIWander(World& world, float deltaTime, int currentScene);
    
private:
    bool CheckEntityCollision(World& world, Entity entity, int currentScene);
};