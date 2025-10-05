#pragma once
#include "Core/World.h"

class CollisionSystem {
public:
    void Update(World& world, int currentScene);
    
private:
    void UpdateHitboxPositions(World& world);
    void UpdateInteractionZones(World& world);
    void ResetCollisionStates(World& world);
    void CheckPlayerBuildingInteractions(World& world, int currentScene);
    Entity FindPlayerEntity(World& world);
};
