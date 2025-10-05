#include "Game/SceneManager.h"
#include "Core/Constants.h"

SceneManager::SceneManager() : 
    currentSceneId(SceneID::MAIN), 
    previousSceneId(SceneID::MAIN),
    playerPositionBeforeTransition{0, 0} {}

int SceneManager::GetCurrentScene() const {
    return currentSceneId;
}

void SceneManager::EnterBuilding(World& world, Entity player, int interiorSceneId, float spawnX, float spawnY) {
    Position* playerPos = world.GetPosition(player);
    if (playerPos) {
        playerPositionBeforeTransition = *playerPos;
        
        previousSceneId = currentSceneId;
        currentSceneId = interiorSceneId;
        
        playerPos->x = spawnX;
        playerPos->y = spawnY;
        
        PlayerInput* playerInput = world.GetPlayerInput(player);
            if (playerInput) {
                // Only add AIWander if the player is currently in wander/autonomous mode
                if (!playerInput->controlled) {
                    world.AddAIWander(player, GameConfig::AI_WANDER_SPEED, 
                                    GameConfig::AI_MIN_MOVE_TIME, GameConfig::AI_MAX_MOVE_TIME);
                }
            }
        // Ensure the player isn't spawned overlapping an interior hitbox (e.g., desk/NPC).
        Hitbox* phb = world.GetHitbox(player);
        if (phb) {
            phb->bounds.x = playerPos->x;
            phb->bounds.y = playerPos->y;
            const int maxNudge = 50;
            int attempts = 0;
            bool stillColliding = true;
            while (stillColliding && attempts < maxNudge) {
                stillColliding = false;
                for (Entity other : world.GetEntities()) {
                    if (other == player) continue;
                    Scene* sc = world.GetScene(other);
                    if (!sc || sc->sceneId != interiorSceneId) continue;
                    Hitbox* ohb = world.GetHitbox(other);
                    if (!ohb) continue;
                    if (CheckCollisionRecs(phb->bounds, ohb->bounds)) {
                        // Nudge player slightly to the right
                        playerPos->x += 8;
                        phb->bounds.x = playerPos->x;
                        stillColliding = true;
                        break;
                    }
                }
                attempts++;
            }
        }
    }
}

void SceneManager::ExitToMain(World& world, Entity player) {
    Position* playerPos = world.GetPosition(player);
    if (playerPos && currentSceneId != SceneID::MAIN) {
        playerPos->x = playerPositionBeforeTransition.x;
        playerPos->y = playerPositionBeforeTransition.y;
        
        currentSceneId = SceneID::MAIN;
    }
}