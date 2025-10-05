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
                // Use the player's speech bubble, but move it to the side opposite the building
                // so the building doesn't cover the text.
                Position* playerPos = world.GetPosition(player);
                Position* buildingPos = world.GetPosition(e);
                Sprite* playerSprite = world.GetSprite(player);
                Sprite* buildingSprite = world.GetSprite(e);
                float pw = (playerSprite) ? (float)playerSprite->width : 48.0f;
                float ph = (playerSprite) ? (float)playerSprite->height : 48.0f;
                float bw = (buildingSprite) ? (float)buildingSprite->width : 64.0f;
                float bh = (buildingSprite) ? (float)buildingSprite->height : 64.0f;

                // Clear building bubble (do not show both)
                SpeechBubble* buildingBubble = world.GetSpeechBubble(e);
                if (buildingBubble) { buildingBubble->active = false; }

                if (playerBubble && playerPos && buildingPos) {
                    float dx = playerPos->x - buildingPos->x;
                    float dy = playerPos->y - buildingPos->y;
                    // horizontal approach
                    if (fabsf(dx) > fabsf(dy)) {
                        if (dx > 0) {
                            // player is to the right of building -> bubble on player's right
                            playerBubble->offsetX = pw + 8.0f;
                            playerBubble->offsetY = -ph/2.0f;
                        } else {
                            // player is to the left -> bubble on player's left
                            playerBubble->offsetX = -pw - 48.0f;
                            playerBubble->offsetY = -ph/2.0f;
                        }
                    } else {
                        // vertical approach
                        if (dy > 0) {
                            // player below building -> bubble below player
                            playerBubble->offsetX = 0.0f;
                            playerBubble->offsetY = ph + 8.0f;
                        } else {
                            // player above building -> bubble above player (closer)
                            playerBubble->offsetX = 0.0f;
                            playerBubble->offsetY = -ph - 16.0f;
                        }
                    }
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
            
            // Exit doors: attach the prompt to the door's speech bubble so it appears at the door
            ExitDoor* exitDoor = world.GetExitDoor(e);
            if (exitDoor) {
                exitDoor->isPlayerNear = isNear;
                SpeechBubble* doorBubble = world.GetSpeechBubble(e);
                if (isNear && doorBubble) {
                    doorBubble->active = true;
                    doorBubble->text = exitDoor->displayText;
                    // default door bubble above the door
                    doorBubble->offsetX = 0.0f;
                    doorBubble->offsetY = -40.0f;
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
                        // Position object's bubble opposite the player's approach so it doesn't overlap
                        Position* playerPos = world.GetPosition(player);
                        Position* objPos = world.GetPosition(e);
                        Sprite* objSprite = world.GetSprite(e);
                        float ow = (objSprite) ? (float)objSprite->width : 48.0f;
                        float oh = (objSprite) ? (float)objSprite->height : 48.0f;
                        if (playerPos && objPos) {
                            float dx = playerPos->x - objPos->x;
                            float dy = playerPos->y - objPos->y;
                            if (fabsf(dx) > fabsf(dy)) {
                                // horizontal approach -> put bubble on opposite horizontal side (closer)
                                if (dx > 0) {
                                    objectBubble->offsetX = -ow - 24.0f;
                                    objectBubble->offsetY = -oh/2.0f;
                                } else {
                                    objectBubble->offsetX = ow + 8.0f;
                                    objectBubble->offsetY = -oh/2.0f;
                                }
                            } else {
                                // vertical approach -> put bubble on opposite vertical side (closer)
                                if (dy > 0) {
                                    // player below object -> put bubble above object
                                    objectBubble->offsetX = 0.0f;
                                    objectBubble->offsetY = -oh - 24.0f;
                                } else {
                                    // player above object -> put bubble below object
                                    objectBubble->offsetX = 0.0f;
                                    objectBubble->offsetY = oh + 8.0f;
                                }
                            }
                        }
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

