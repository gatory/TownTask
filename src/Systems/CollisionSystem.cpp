#include "../include/Systems/CollisionSystem.h"
#include "../include/Core/Constants.h"

void CollisionSystem::Update(World& world, int currentScene)
{
    ResetCollisionStates(world);

    // Update hitboxes and interaction zones positions
    UpdateHitboxPositions(world);
    UpdateInteractionZones(world);

    // Check interactions between player and buildings
    CheckPlayerBuildingInteractions(world, currentScene);
}

void CollisionSystem::CheckPlayerBuildingInteractions(World& world, int currentScene) {
    Entity player = FindPlayerEntity(world);
    if (player == INVALID_ENTITY) return;
    
    Hitbox* playerHitbox = world.GetHitbox(player);
    if (!playerHitbox) return;
    SpeechBubble* playerBubble = world.GetSpeechBubble(player);
    
    // Check buildings in main scene
    if (currentScene == SceneID::MAIN) {
        for (Entity e : world.GetEntities()) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentScene) continue;
            
            Building* building = world.GetBuilding(e);
            InteractionZone* zone = world.GetInteractionZone(e);
            if (!building || !zone) continue;
            
            if (CheckCollisionRecs(playerHitbox->bounds, zone->bounds)) {
                if (playerBubble) {
                    playerBubble->active = true;
                    playerBubble->text = "Enter\n" + building->displayName + "? (X)";
                }
            }
        }
    }
    // Check exit doors and interactables in interiors
    else {
        for (Entity e : world.GetEntities()) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentScene) continue;
            
            InteractionZone* zone = world.GetInteractionZone(e);
            if (!zone) continue;
            
            bool isNear = CheckCollisionRecs(playerHitbox->bounds, zone->bounds);
            
            // Exit doors
            ExitDoor* exitDoor = world.GetExitDoor(e);
            if (exitDoor) {
                exitDoor->isPlayerNear = isNear;
                if (isNear && playerBubble) {
                    // Place the player's speech bubble closer to the player for exit prompts
                    playerBubble->active = true;
                    playerBubble->text = exitDoor->displayText;
                    // Move bubble closer vertically (less negative offset)
                    playerBubble->offsetY = -24.0f;
                }
            }
            
            // Interactable objects: activate the object's speech bubble only. Player bubble is used
            // for higher-level scene prompts (like Exit or Enter building) to avoid duplicates.
            Interactable* interactable = world.GetInteractable(e);
            SpeechBubble* objectBubble = world.GetSpeechBubble(e);
            if (interactable) {
                interactable->isPlayerNear = isNear;
                if (isNear) {
                    if (objectBubble) {
                        objectBubble->active = true;
                        objectBubble->text = interactable->interactionPrompt;
                    }
                }
            }
        }
    }
}
void CollisionSystem::UpdateHitboxPositions(World& world) {
    for (Entity e : world.GetEntities()) {
        if (world.HasPosition(e) && world.HasHitbox(e)) {
            Position* pos = world.GetPosition(e);
            Hitbox* hb = world.GetHitbox(e);
            hb->bounds.x = pos->x;
            hb->bounds.y = pos->y;
        }
    }
}
void CollisionSystem::UpdateInteractionZones(World& world) {
    for (Entity e : world.GetEntities()) {
        if (world.HasPosition(e) && world.HasInteractionZone(e)) {
            Position* pos = world.GetPosition(e);
            InteractionZone* zone = world.GetInteractionZone(e);
            zone->bounds.x = pos->x + zone->offsetX;
            zone->bounds.y = pos->y + zone->offsetY;
        }
    }
}

void CollisionSystem::ResetCollisionStates(World& world)
{
    for (Entity e : world.GetEntities()) {
        // Reset hitbox colliding
        if (world.HasHitbox(e)) {
            Hitbox* hb = world.GetHitbox(e);
            hb->colliding = false;
        }

        // Reset speech bubbles
        if (world.HasSpeechBubble(e)) {
            SpeechBubble* bubble = world.GetSpeechBubble(e);
            bubble->active = false;
            bubble->text.clear();
        }

        // Reset proximity flags
        if (world.HasExitDoor(e)) {
            ExitDoor* door = world.GetExitDoor(e);
            door->isPlayerNear = false;
        }
        if (world.HasInteractable(e)) {
            Interactable* interactable = world.GetInteractable(e);
            interactable->isPlayerNear = false;
        }
    }
}

Entity CollisionSystem::FindPlayerEntity(World& world) {
    for (Entity e : world.GetEntities()) {
        if (world.HasPlayer(e)) return e;
    }
    return INVALID_ENTITY;
}

