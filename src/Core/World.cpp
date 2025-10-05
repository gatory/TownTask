#include "../include/Core/World.h"
#include <stdexcept>

World::World()
    : nextEntityId(1)
{
}

Entity World::CreateEntity() {
    Entity e = nextEntityId++;
    entities.push_back(e);
    return e;
}

const std::vector<Entity>& World::GetEntities() const {
    return entities;
}

// -------------------- Add methods --------------------
void World::AddPosition(Entity e, float x, float y) {
    positions[e] = Position{ x, y };
}

void World::AddSprite(Entity e, const std::string& path, int width, int height, Color tint) {
    sprites[e] = Sprite{ path, {}, width, height, tint, false };
}

void World::AddHitbox(Entity e, float width, float height) {
    // Initialize bounds; if the entity already has a position, use it so bounds are correct immediately
    float bx = 0.0f;
    float by = 0.0f;
    auto pit = positions.find(e);
    if (pit != positions.end()) {
        bx = pit->second.x;
        by = pit->second.y;
    }
    hitboxes[e] = Hitbox{ width, height, {bx, by, width, height}, false };
}

void World::AddAnimation(Entity e, int frameCount, float frameTime, int frameWidth, int frameHeight) {
    animations[e] = Animation{ frameCount, 0, frameTime, frameTime, frameWidth, frameHeight };
}

void World::AddPlayerInput(Entity e, float speed) {
    playerInputs[e] = PlayerInput{ speed, true, false };
}

void World::AddBuilding(Entity e, const std::string& displayName, int interiorSceneId) {
    buildings[e] = Building{ displayName, interiorSceneId };
}

void World::AddSpeechBubble(Entity e, const std::string& text, float offsetX, float offsetY) {
    speechBubbles[e] = SpeechBubble{ text, false, offsetX, offsetY, 0.0f, 0.0f, 1.0f, 0.08f, 0.08f };
}

void World::AddScene(Entity e, int sceneId) {
    scenes[e] = Scene{ sceneId };
}

void World::AddPlayer(Entity e) {
    players[e] = Player{true};
}

void World::AddInteractionZone(Entity e, float width, float height, float offsetX, float offsetY) {
    float ix = 0.0f;
    float iy = 0.0f;
    auto pit = positions.find(e);
    if (pit != positions.end()) {
        ix = pit->second.x + offsetX;
        iy = pit->second.y + offsetY;
    }
    interactionZones[e] = InteractionZone{ width, height, offsetX, offsetY, {ix, iy, width, height}};
}

void World::AddBuildingInterior(Entity e, GUIType guiType, const std::string& guiBg, Color guiColor,
                                const std::string& gameBg, Texture2D bgTex, Color gameColor) {
    buildingInteriors[e] = BuildingInterior{ guiType, guiBg, guiColor, gameBg, gameColor, bgTex, bgTex, false };
}

void World::AddAIWander(Entity e, float speed, float minTime, float maxTime) {
    // Start with no movement; direction will be picked by MovementSystem
    aiWanders[e] = AIWander{ speed, 0, 0, minTime, maxTime, 0.0f, 0.0f, false };
}

void World::AddExitDoor(Entity e, const std::string& displayText) {
    exitDoors[e] = ExitDoor{ displayText, false };
}

void World::AddInteractable(Entity e, const std::string& name, const std::string& prompt, const std::string& type) {
    interactables[e] = Interactable{ name, prompt, type, false };
}

void World::AddLibraryData(Entity e, const LibraryData& data) {
    libraryData[e] = data;
}

void World::AddTodoListData(Entity e, const TodoListData& data) {
    todoListData[e] = data;
}
// void World::AddFaceDetection(Entity entity) {
//     FaceDetection fd;
//     faceDetections[entity] = fd;
// }

// void World::AddFaceDetection(Entity entity, float awayThreshold, float reactionCooldown) {
//     FaceDetection fd;
//     fd.awayThreshold = awayThreshold;
//     fd.reactionCooldown = reactionCooldown;
//     faceDetections[entity] = fd;
// }

// FaceDetection* World::GetFaceDetection(Entity entity) {
//     auto it = faceDetections.find(entity);
//     return (it != faceDetections.end()) ? &it->second : nullptr;
// }

// bool World::HasFaceDetection(Entity entity) const {
//     return faceDetections.find(entity) != faceDetections.end();
// }

// void World::RemoveFaceDetection(Entity entity) {
//     faceDetections.erase(entity);
// }

// // Character State component implementation
// void World::AddCharacterState(Entity entity) {
//     CharacterStateComponent csc;
//     characterStates[entity] = csc;
// }

// void World::AddCharacterState(Entity entity, CharacterState initialState) {
//     CharacterStateComponent csc;
//     csc.currentState = initialState;
//     characterStates[entity] = csc;
// }

// CharacterStateComponent* World::GetCharacterState(Entity entity) {
//     auto it = characterStates.find(entity);
//     return (it != characterStates.end()) ? &it->second : nullptr;
// }

// bool World::HasCharacterState(Entity entity) const {
//     return characterStates.find(entity) != characterStates.end();
// }

// void World::RemoveCharacterState(Entity entity) {
//     characterStates.erase(entity);
// }

// -------------------- Get methods --------------------
Position* World::GetPosition(Entity e) {
    auto it = positions.find(e);
    if (it == positions.end()) return nullptr;
    return &it->second;
}

Sprite* World::GetSprite(Entity e) {
    auto it = sprites.find(e);
    if (it == sprites.end()) return nullptr;
    return &it->second;
}

Hitbox* World::GetHitbox(Entity e) {
    auto it = hitboxes.find(e);
    if (it == hitboxes.end()) return nullptr;
    return &it->second;
}

Animation* World::GetAnimation(Entity e) {
    auto it = animations.find(e);
    if (it == animations.end()) return nullptr;
    return &it->second;
}

PlayerInput* World::GetPlayerInput(Entity e) {
    auto it = playerInputs.find(e);
    if (it == playerInputs.end()) return nullptr;
    return &it->second;
}

Building* World::GetBuilding(Entity e) {
    auto it = buildings.find(e);
    if (it == buildings.end()) return nullptr;
    return &it->second;
}

SpeechBubble* World::GetSpeechBubble(Entity e) {
    auto it = speechBubbles.find(e);
    if (it == speechBubbles.end()) return nullptr;
    return &it->second;
}

Scene* World::GetScene(Entity e) {
    auto it = scenes.find(e);
    if (it == scenes.end()) return nullptr;
    return &it->second;
}

Player* World::GetPlayer(Entity e) {
    auto it = players.find(e);
    if (it == players.end()) return nullptr;
    return &it->second;
}

InteractionZone* World::GetInteractionZone(Entity e) {
    auto it = interactionZones.find(e);
    if (it == interactionZones.end()) return nullptr;
    return &it->second;
}

BuildingInterior* World::GetBuildingInterior(Entity e) {
    auto it = buildingInteriors.find(e);
    if (it == buildingInteriors.end()) return nullptr;
    return &it->second;
}

AIWander* World::GetAIWander(Entity e) {
    auto it = aiWanders.find(e);
    if (it == aiWanders.end()) return nullptr;
    return &it->second;
}

ExitDoor* World::GetExitDoor(Entity e) {
    auto it = exitDoors.find(e);
    if (it == exitDoors.end()) return nullptr;
    return &it->second;
}

Interactable* World::GetInteractable(Entity e) {
    auto it = interactables.find(e);
    if (it == interactables.end()) return nullptr;
    return &it->second;
}

LibraryData* World::GetLibraryData(Entity e) {
    auto it = libraryData.find(e);
    if (it == libraryData.end()) return nullptr;
    return &it->second;
}

TodoListData* World::GetTodoListData(Entity e) {
    auto it = todoListData.find(e);
    if (it == todoListData.end()) return nullptr;
    return &it->second;
}

// -------------------- Has methods --------------------
bool World::HasPosition(Entity e) const { return positions.find(e) != positions.end(); }
bool World::HasSprite(Entity e) const { return sprites.find(e) != sprites.end(); }
bool World::HasHitbox(Entity e) const { return hitboxes.find(e) != hitboxes.end(); }
bool World::HasAnimation(Entity e) const { return animations.find(e) != animations.end(); }
bool World::HasPlayerInput(Entity e) const { return playerInputs.find(e) != playerInputs.end(); }
bool World::HasBuilding(Entity e) const { return buildings.find(e) != buildings.end(); }
bool World::HasSpeechBubble(Entity e) const { return speechBubbles.find(e) != speechBubbles.end(); }
bool World::HasScene(Entity e) const { return scenes.find(e) != scenes.end(); }
bool World::HasPlayer(Entity e) const { return players.find(e) != players.end(); }
bool World::HasInteractionZone(Entity e) const { return interactionZones.find(e) != interactionZones.end(); }
bool World::HasBuildingInterior(Entity e) const { return buildingInteriors.find(e) != buildingInteriors.end(); }
bool World::HasAIWander(Entity e) const { return aiWanders.find(e) != aiWanders.end(); }
bool World::HasExitDoor(Entity e) const { return exitDoors.find(e) != exitDoors.end(); }
bool World::HasInteractable(Entity e) const { return interactables.find(e) != interactables.end(); }
bool World::HasLibraryData(Entity e) const { return libraryData.find(e) != libraryData.end(); }
bool World::HasTodoListData(Entity e) const { return todoListData.find(e) != todoListData.end(); }
