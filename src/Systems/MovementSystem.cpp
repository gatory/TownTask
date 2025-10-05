#include "../include/Systems/MovementSystem.h"
#include "../include/Core/Constants.h"
#include <cstdlib>

void MovementSystem::UpdatePlayerInput(World& world, float deltaTime, int currentScene) {
    for (Entity e : world.GetEntities()) {
        PlayerInput* input = world.GetPlayerInput(e);
        if (!input) continue;
        // If this entity has an AIWander and control is disabled, skip manual input
        if (!input->controlled) continue;
    // If we're not in main scene and entity wanders (NPCs), skip player input for AI
    // but do not skip the player entity itself (player may have AIWander but still be controllable)
    if (currentScene != SceneID::MAIN && world.HasAIWander(e) && !world.HasPlayer(e)) continue;
        
        Position* pos = world.GetPosition(e);
        Sprite* sprite = world.GetSprite(e);
        Hitbox* hitbox = world.GetHitbox(e);
        if (!pos || !sprite || !hitbox) continue;
        
        float originalX = pos->x;
        float originalY = pos->y;
        
        bool moved = false;
        if (!input->frozen) {
            if (IsKeyDown(KEY_RIGHT)) { pos->x += input->speed * deltaTime; moved = true; }
            if (IsKeyDown(KEY_LEFT))  { pos->x -= input->speed * deltaTime; moved = true; }
            if (IsKeyDown(KEY_DOWN))  { pos->y += input->speed * deltaTime; moved = true; }
            if (IsKeyDown(KEY_UP))    { pos->y -= input->speed * deltaTime; moved = true; }
        }
        
        hitbox->bounds.x = pos->x;
        hitbox->bounds.y = pos->y;
        
            if (CheckEntityCollision(world, e, currentScene)) {
            pos->x = originalX;
            pos->y = originalY;
        }

        // If the player was frozen because of starting a pomodoro, moving should unfreeze
        if (moved && input->frozen) {
            input->frozen = false;
        }
        
        // Clamp position to local coordinate space: for interiors the game area is 0..GAME_HEIGHT
        pos->x = std::max(0.0f, std::min(pos->x, (float)GameConfig::SCREEN_WIDTH - sprite->width));
        if (currentScene == SceneID::MAIN) {
            pos->y = std::max(0.0f, std::min(pos->y, (float)GameConfig::SCREEN_HEIGHT - sprite->height));
        } else {
            // interior: allow y to extend into the GUI area so the player can move to the top half.
            // Positions are local to the game area but can be negative (up to -GUI_HEIGHT) so when
            // rendered with an added GUI_HEIGHT offset they occupy the full screen.
            float minY = -(float)GameConfig::GUI_HEIGHT;
            float maxY = (float)(GameConfig::GAME_HEIGHT - sprite->height);
            pos->y = std::max(minY, std::min(pos->y, maxY));
        }
    }
}

void MovementSystem::UpdateAIWander(World& world, float deltaTime, int currentScene) {
    for (Entity e : world.GetEntities()) {
        AIWander* wander = world.GetAIWander(e);
        if (!wander) continue;
        // If this entity also has PlayerInput and is controlled, skip AI movement
        PlayerInput* pi = world.GetPlayerInput(e);
        if (pi && pi->controlled) continue;

        Position* pos = world.GetPosition(e);
        Sprite* sprite = world.GetSprite(e);
        Hitbox* hb = world.GetHitbox(e);
        if (!pos || !sprite || !hb) continue;

        wander->moveTimer += deltaTime;

        if (wander->moveTimer >= wander->currentMoveTime) {
            wander->moveTimer = 0;
            wander->isMoving = !wander->isMoving;

            float range = wander->maxMoveTime - wander->minMoveTime;
            wander->currentMoveTime = wander->minMoveTime + ((float)rand() / RAND_MAX) * range;

            if (wander->isMoving) {
                // Pick a random 2D direction (normalized)
                float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
                float ry = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
                // Avoid near-zero vector
                if (fabsf(rx) < 0.2f && fabsf(ry) < 0.2f) {
                    rx = (rx < 0) ? -0.5f : 0.5f;
                    ry = (ry < 0) ? -0.5f : 0.5f;
                }
                // Normalize
                float len = sqrtf(rx*rx + ry*ry);
                wander->dirX = rx / len;
                wander->dirY = ry / len;
            } else {
                wander->dirX = 0.0f;
                wander->dirY = 0.0f;
            }
        }

        if (wander->isMoving) {
            // Attempt movement with collision checks. If blocked, try small alternate directions.
            float moveX = wander->dirX * wander->speed * deltaTime;
            float moveY = wander->dirY * wander->speed * deltaTime;

            // try primary movement
            float origX = pos->x;
            float origY = pos->y;
            pos->x += moveX;
            pos->y += moveY;
            hb->bounds.x = pos->x;
            hb->bounds.y = pos->y;

            if (CheckEntityCollision(world, e, currentScene)) {
                // revert and try X only
                pos->x = origX;
                hb->bounds.x = pos->x;
                pos->x += moveX;
                hb->bounds.x = pos->x;
                if (CheckEntityCollision(world, e, currentScene)) {
                    // revert X, try Y only
                    pos->x = origX;
                    hb->bounds.x = pos->x;
                    pos->y = origY + moveY;
                    hb->bounds.y = pos->y;
                    if (CheckEntityCollision(world, e, currentScene)) {
                        // blocked; revert fully
                        pos->x = origX;
                        pos->y = origY;
                        hb->bounds.x = pos->x;
                        hb->bounds.y = pos->y;
                        // optionally pick a new direction next tick
                        wander->isMoving = false;
                    }
                }
            }

            // keep inside local game area bounds
            float minY = 0.0f;
            float maxY = (float)GameConfig::SCREEN_HEIGHT - sprite->height;
            if (currentScene != SceneID::MAIN) {
                // interior: allow wandering into GUI area by permitting negative local y
                minY = -(float)GameConfig::GUI_HEIGHT;
                maxY = (float)(GameConfig::GAME_HEIGHT - sprite->height);
            }
            pos->x = std::max(0.0f, std::min(pos->x, (float)GameConfig::SCREEN_WIDTH - sprite->width));
            pos->y = std::max(minY, std::min(pos->y, maxY));
            hb->bounds.x = pos->x;
            hb->bounds.y = pos->y;
        }
    }
}

bool MovementSystem::CheckEntityCollision(World& world, Entity entity, int currentScene) {
    Hitbox* entityHitbox = world.GetHitbox(entity);
    if (!entityHitbox) return false;
    
    for (Entity other : world.GetEntities()) {
        if (other == entity) continue;
        
        Scene* scene = world.GetScene(other);
        if (!scene || scene->sceneId != currentScene) continue;
        
        Hitbox* otherHitbox = world.GetHitbox(other);
        if (!otherHitbox) continue;
        
        if (CheckCollisionRecs(entityHitbox->bounds, otherHitbox->bounds)) {
            return true;
        }
    }
    return false;
}