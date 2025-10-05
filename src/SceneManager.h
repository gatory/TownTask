#pragma once
#include "World.h"

// Scene IDs
#define SCENE_MAIN 0
#define SCENE_SHOP_INTERIOR 1

// SceneManager - handles scene transitions and tracks current scene
class SceneManager {
private:
    int currentSceneId;
    int previousSceneId;
    Position playerPositionBeforeTransition;

public:
    SceneManager() : currentSceneId(SCENE_MAIN), previousSceneId(SCENE_MAIN), playerPositionBeforeTransition{0, 0} {}

    int GetCurrentScene() const {
        return currentSceneId;
    }

    void EnterBuilding(World& world, Entity player, int interiorSceneId, float spawnX, float spawnY) {
        Position* playerPos = world.GetPosition(player);
        if (playerPos) {
            // Save player position
            playerPositionBeforeTransition = *playerPos;

            // Move to new scene
            previousSceneId = currentSceneId;
            currentSceneId = interiorSceneId;

            // Set player spawn position in interior
            playerPos->x = spawnX;
            playerPos->y = spawnY;

            // Switch from player input to AI wander
            PlayerInput* playerInput = world.GetPlayerInput(player);
            if (playerInput) {
                // Remove player input temporarily by checking for AI wander
                world.AddAIWander(player, 100.0f, 0.5f, 2.0f); // Slow wander
            }
        }
    }

    void ExitToMain(World& world, Entity player) {
        Position* playerPos = world.GetPosition(player);
        if (playerPos && currentSceneId != SCENE_MAIN) {
            // Restore player position
            playerPos->x = playerPositionBeforeTransition.x;
            playerPos->y = playerPositionBeforeTransition.y;

            // Return to main scene
            currentSceneId = SCENE_MAIN;

            // Note: AIWander component stays, but InputSystem will take priority
            // We could remove AIWander component here if needed
        }
    }
};
