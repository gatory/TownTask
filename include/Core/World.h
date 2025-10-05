#pragma once
#include "Types.h"
#include "Components.h"
#include <vector>
#include <unordered_map>

template<typename T>
using ComponentMap = std::unordered_map<Entity, T>;

class World {
private:
    Entity nextEntityId;
    std::vector<Entity> entities;

    ComponentMap<Position> positions;
    ComponentMap<Sprite> sprites;
    ComponentMap<Hitbox> hitboxes;
    ComponentMap<Animation> animations;
    ComponentMap<PlayerInput> playerInputs;
    ComponentMap<Building> buildings;
    ComponentMap<SpeechBubble> speechBubbles;
    ComponentMap<Scene> scenes;
    ComponentMap<Player> players;
    ComponentMap<InteractionZone> interactionZones;
    ComponentMap<BuildingInterior> buildingInteriors;
    ComponentMap<AIWander> aiWanders;
    ComponentMap<ExitDoor> exitDoors;
    ComponentMap<Interactable> interactables;
    ComponentMap<LibraryData> libraryData;
    ComponentMap<TodoListData> todoListData;
    ComponentMap<FaceDetection> faceDetections; 
    ComponentMap<CharacterStateComponent> characterStates;

public:
    World();
    
    Entity CreateEntity();
    const std::vector<Entity>& GetEntities() const;

    // Add methods
    void AddPosition(Entity e, float x, float y);
    void AddSprite(Entity e, const std::string& path, int width, int height, Color tint = WHITE);
    void AddHitbox(Entity e, float width, float height);
    void AddAnimation(Entity e, int frameCount, float frameTime, int frameWidth, int frameHeight);
    void AddPlayerInput(Entity e, float speed);
    void AddBuilding(Entity e, const std::string& displayName, int interiorSceneId);
    void AddSpeechBubble(Entity e, const std::string& text, float offsetX = 0, float offsetY = -32);
    void AddScene(Entity e, int sceneId);
    void AddPlayer(Entity e);
    void AddInteractionZone(Entity e, float width, float height, float offsetX = 0, float offsetY = 0);
    void AddBuildingInterior(Entity e, GUIType guiType, const std::string& guiBg, Color guiColor, 
                            const std::string& gameBg, Texture2D bgTex, Color gameColor);
    void AddAIWander(Entity e, float speed, float minTime, float maxTime);
    void AddExitDoor(Entity e, const std::string& displayText);
    void AddInteractable(Entity e, const std::string& name, const std::string& prompt, 
                        const std::string& type);
    void AddLibraryData(Entity e, const LibraryData& data);
    void AddTodoListData(Entity e, const TodoListData& data);
    void AddFaceDetection(Entity entity);
    void AddFaceDetection(Entity entity, float awayThreshold, float reactionCooldown);
    FaceDetection* GetFaceDetection(Entity entity);
    bool HasFaceDetection(Entity entity) const;
    void RemoveFaceDetection(Entity entity);
    
    // Character State component methods
    void AddCharacterState(Entity entity);
    void AddCharacterState(Entity entity, CharacterState initialState);
    CharacterStateComponent* GetCharacterState(Entity entity);
    bool HasCharacterState(Entity entity) const;
    void RemoveCharacterState(Entity entity);

    // Get methods
    Position* GetPosition(Entity e);
    Sprite* GetSprite(Entity e);
    Hitbox* GetHitbox(Entity e);
    Animation* GetAnimation(Entity e);
    PlayerInput* GetPlayerInput(Entity e);
    Building* GetBuilding(Entity e);
    SpeechBubble* GetSpeechBubble(Entity e);
    Scene* GetScene(Entity e);
    Player* GetPlayer(Entity e);
    InteractionZone* GetInteractionZone(Entity e);
    BuildingInterior* GetBuildingInterior(Entity e);
    AIWander* GetAIWander(Entity e);
    ExitDoor* GetExitDoor(Entity e);
    Interactable* GetInteractable(Entity e);
    LibraryData* GetLibraryData(Entity e);
    TodoListData* GetTodoListData(Entity e);

    // Has methods
    bool HasPosition(Entity e) const;
    bool HasSprite(Entity e) const;
    bool HasHitbox(Entity e) const;
    bool HasAnimation(Entity e) const;
    bool HasPlayerInput(Entity e) const;
    bool HasBuilding(Entity e) const;
    bool HasSpeechBubble(Entity e) const;
    bool HasScene(Entity e) const;
    bool HasPlayer(Entity e) const;
    bool HasInteractionZone(Entity e) const;
    bool HasBuildingInterior(Entity e) const;
    bool HasAIWander(Entity e) const;
    bool HasExitDoor(Entity e) const;
    bool HasInteractable(Entity e) const;
    bool HasLibraryData(Entity e) const;
    bool HasTodoListData(Entity e) const;
};