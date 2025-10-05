#pragma once
#include "Core/World.h"
#include "Core/Components.h"

class SceneManager {
private:
    int currentSceneId;
    int previousSceneId;
    Position playerPositionBeforeTransition;

public:
    SceneManager();
    
    int GetCurrentScene() const;
    void EnterBuilding(World& world, Entity player, int interiorSceneId, float spawnX, float spawnY);
    void ExitToMain(World& world, Entity player);
};
