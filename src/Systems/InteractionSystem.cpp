#include "../include/Systems/InteractionSystem.h"
#include "../include/Game/SceneManager.h"
#include "../include/Core/Constants.h"

void InteractionSystem::Update(World& world, SceneManager& sceneManager) {
    Entity player = FindPlayerEntity(world);
    if (player == INVALID_ENTITY) return;
    
    int currentScene = sceneManager.GetCurrentScene();
    Hitbox* playerHitbox = world.GetHitbox(player);
    if (!playerHitbox) return;
    
    // Handle Z key - exit building via door
    if (IsKeyPressed(KEY_Z) && currentScene != SceneID::MAIN) {
        // Check if near exit door
        for (Entity e : world.GetEntities()) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentScene) continue;
            
            ExitDoor* exitDoor = world.GetExitDoor(e);
            InteractionZone* zone = world.GetInteractionZone(e);
            if (!exitDoor || !zone) continue;
            
            if (CheckCollisionRecs(playerHitbox->bounds, zone->bounds)) {
                sceneManager.ExitToMain(world, player);
                return;
            }
        }
    }
    
    // Handle X key - interact with buildings/objects
    if (IsKeyPressed(KEY_X)) {
        // In main scene - enter buildings
        if (currentScene == SceneID::MAIN) {
            HandleBuildingEntry(world, sceneManager, player);
        } 
        // In interiors - interact with objects
        else {
            HandleInteriorInteraction(world, currentScene);
        }
    }
}
Entity InteractionSystem::FindPlayerEntity(World& world) {
    for (Entity e : world.GetEntities()) {
        if (world.HasPlayer(e)) return e;
    }
    return INVALID_ENTITY;
}


void InteractionSystem::HandleBuildingEntry(World& world, SceneManager& sceneManager, Entity player) {
    Hitbox* playerHitbox = world.GetHitbox(player);
    if (!playerHitbox) return;

    // Find a building in the main scene that the player is near and enter it
    for (Entity e : world.GetEntities()) {
        Scene* scene = world.GetScene(e);
        if (!scene || scene->sceneId != SceneID::MAIN) continue;

        Building* building = world.GetBuilding(e);
        InteractionZone* zone = world.GetInteractionZone(e);
        if (!building || !zone) continue;

        if (CheckCollisionRecs(playerHitbox->bounds, zone->bounds)) {
            // Use a sensible interior spawn point (center-ish within the local interior game area)
            float spawnX = GameConfig::SCREEN_WIDTH / 2.0f;
            float spawnY = GameConfig::GAME_HEIGHT / 2.0f - 50.0f;
            sceneManager.EnterBuilding(world, player, building->interiorSceneId, spawnX, spawnY);
            return;
        }
    }
}


void InteractionSystem::HandleInteriorInteraction(World& world, int currentScene) {
    Entity player = FindPlayerEntity(world);
    if (player == INVALID_ENTITY) return;
    
    Hitbox* playerHitbox = world.GetHitbox(player);
    if (!playerHitbox) return;
    
    // Check for interactable objects
    for (Entity e : world.GetEntities()) {
        Scene* scene = world.GetScene(e);
        if (!scene || scene->sceneId != currentScene) continue;
        
        Interactable* interactable = world.GetInteractable(e);
        InteractionZone* zone = world.GetInteractionZone(e);
        if (!interactable || !zone || !interactable->isPlayerNear) continue;
        
        if (CheckCollisionRecs(playerHitbox->bounds, zone->bounds)) {
            // Handle different interaction types
            if (interactable->interactionType == "librarian") {
                // Show library UI
                for (Entity ie : world.GetEntities()) {
                    LibraryData* library = world.GetLibraryData(ie);
                    if (library) {
                        library->isShowingUI = true;
                        break;
                    }
                }
            } 
            else if (interactable->interactionType == "desk") {
                // Show todo list UI
                for (Entity ie : world.GetEntities()) {
                    TodoListData* todo = world.GetTodoListData(ie);
                    if (todo) {
                        todo->isShowingUI = true;
                        break;
                    }
                }
            }
            // For barista we only set the prompt; Game will handle actual timer start/freeze
            else if (interactable->interactionType == "barista") {
                SpeechBubble* sb = world.GetSpeechBubble(player);
                if (sb) {
                    sb->active = true;
                    sb->text = "Start Pomodoro (X)";
                }
            }
        }
    }
}