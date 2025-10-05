#pragma once
#include "Components.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

// Simple entity ID
using Entity = int;

// Component storage - maps entity ID to component data
template<typename T>
using ComponentMap = std::unordered_map<Entity, T>;

// Simple ECS World - stores all entities and their components
class World {
private:
    Entity nextEntityId = 0;
    std::vector<Entity> entities;

    // Component storage
    ComponentMap<Position> positions;
    ComponentMap<Sprite> sprites;
    ComponentMap<Hitbox> hitboxes;
    ComponentMap<Animation> animations;
    ComponentMap<PlayerInput> playerInputs;
    ComponentMap<ColorChange> colorChanges;
    ComponentMap<Building> buildings;
    ComponentMap<SpeechBubble> speechBubbles;
    ComponentMap<Scene> scenes;
    ComponentMap<Player> players;
    ComponentMap<InteractionZone> interactionZones;
    ComponentMap<BuildingInterior> buildingInteriors;
    ComponentMap<AIWander> aiWanders;

public:
    // Create a new entity
    Entity CreateEntity() {
        Entity id = nextEntityId++;
        entities.push_back(id);
        return id;
    }

    // Add components to an entity
    void AddPosition(Entity e, float x, float y) {
        positions[e] = {x, y};
    }

    void AddSprite(Entity e, const std::string& path, int width, int height, Color tint = WHITE) {
        sprites[e] = {path, {0}, width, height, tint, false};
    }

    void AddHitbox(Entity e, float width, float height) {
        hitboxes[e] = {width, height, {0, 0, width, height}};
    }

    void AddAnimation(Entity e, int frameCount, float frameTime, int frameWidth, int frameHeight) {
        animations[e] = {frameCount, 0, frameTime, frameTime, frameWidth, frameHeight};
    }

    void AddPlayerInput(Entity e, float speed) {
        playerInputs[e] = {speed};
    }

    void AddColorChange(Entity e, Color normalColor, Color collisionColor) {
        colorChanges[e] = {normalColor, collisionColor, false};
    }

    void AddBuilding(Entity e, const std::string& displayName, int interiorSceneId) {
        buildings[e] = {displayName, interiorSceneId};
    }

    void AddSpeechBubble(Entity e, const std::string& text, float offsetX = 0, float offsetY = -32) {
        speechBubbles[e] = {text, false, offsetX, offsetY};
    }

    void AddScene(Entity e, int sceneId) {
        scenes[e] = {sceneId};
    }

    void AddPlayer(Entity e) {
        players[e] = {true};
    }

    void AddInteractionZone(Entity e, float width, float height, float offsetX = 0, float offsetY = 0) {
        interactionZones[e] = {width, height, offsetX, offsetY, {0, 0, width, height}};
    }

    void AddBuildingInterior(Entity e, GUIType guiType, const std::string& guiBg, Color guiColor, const std::string& gameBg, Color gameColor) {
        buildingInteriors[e] = {guiType, guiBg, guiColor, gameBg, gameColor, {0}, {0}, false};
    }

    void AddAIWander(Entity e, float speed, float minTime, float maxTime) {
        aiWanders[e] = {speed, 0, 0, minTime, maxTime, 1, false};
    }

    // Get components (returns pointer, nullptr if not present)
    Position* GetPosition(Entity e) {
        auto it = positions.find(e);
        return it != positions.end() ? &it->second : nullptr;
    }

    Sprite* GetSprite(Entity e) {
        auto it = sprites.find(e);
        return it != sprites.end() ? &it->second : nullptr;
    }

    Hitbox* GetHitbox(Entity e) {
        auto it = hitboxes.find(e);
        return it != hitboxes.end() ? &it->second : nullptr;
    }

    Animation* GetAnimation(Entity e) {
        auto it = animations.find(e);
        return it != animations.end() ? &it->second : nullptr;
    }

    PlayerInput* GetPlayerInput(Entity e) {
        auto it = playerInputs.find(e);
        return it != playerInputs.end() ? &it->second : nullptr;
    }

    ColorChange* GetColorChange(Entity e) {
        auto it = colorChanges.find(e);
        return it != colorChanges.end() ? &it->second : nullptr;
    }

    Building* GetBuilding(Entity e) {
        auto it = buildings.find(e);
        return it != buildings.end() ? &it->second : nullptr;
    }

    SpeechBubble* GetSpeechBubble(Entity e) {
        auto it = speechBubbles.find(e);
        return it != speechBubbles.end() ? &it->second : nullptr;
    }

    Scene* GetScene(Entity e) {
        auto it = scenes.find(e);
        return it != scenes.end() ? &it->second : nullptr;
    }

    Player* GetPlayer(Entity e) {
        auto it = players.find(e);
        return it != players.end() ? &it->second : nullptr;
    }

    InteractionZone* GetInteractionZone(Entity e) {
        auto it = interactionZones.find(e);
        return it != interactionZones.end() ? &it->second : nullptr;
    }

    BuildingInterior* GetBuildingInterior(Entity e) {
        auto it = buildingInteriors.find(e);
        return it != buildingInteriors.end() ? &it->second : nullptr;
    }

    AIWander* GetAIWander(Entity e) {
        auto it = aiWanders.find(e);
        return it != aiWanders.end() ? &it->second : nullptr;
    }

    // Get all entities
    const std::vector<Entity>& GetEntities() const {
        return entities;
    }

    // Check if entity has component
    bool HasPosition(Entity e) const { return positions.find(e) != positions.end(); }
    bool HasSprite(Entity e) const { return sprites.find(e) != sprites.end(); }
    bool HasHitbox(Entity e) const { return hitboxes.find(e) != hitboxes.end(); }
    bool HasAnimation(Entity e) const { return animations.find(e) != animations.end(); }
    bool HasPlayerInput(Entity e) const { return playerInputs.find(e) != playerInputs.end(); }
    bool HasColorChange(Entity e) const { return colorChanges.find(e) != colorChanges.end(); }
    bool HasBuilding(Entity e) const { return buildings.find(e) != buildings.end(); }
    bool HasSpeechBubble(Entity e) const { return speechBubbles.find(e) != speechBubbles.end(); }
    bool HasScene(Entity e) const { return scenes.find(e) != scenes.end(); }
    bool HasPlayer(Entity e) const { return players.find(e) != players.end(); }
    bool HasInteractionZone(Entity e) const { return interactionZones.find(e) != interactionZones.end(); }
    bool HasBuildingInterior(Entity e) const { return buildingInteriors.find(e) != buildingInteriors.end(); }
    bool HasAIWander(Entity e) const { return aiWanders.find(e) != aiWanders.end(); }
};
