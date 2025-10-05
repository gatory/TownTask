#pragma once
#include "raylib.h"
#include <string>

// Position component - where the entity is located
struct Position {
    float x;
    float y;
};

// Sprite component - visual representation
struct Sprite {
    std::string texturePath;
    Texture2D texture;
    int width;
    int height;
    Color tint;
    bool loaded;
};

// Hitbox component - collision detection
struct Hitbox {
    float width;
    float height;
    Rectangle bounds; // Calculated from position + width/height
};

// Animation component - for animated sprites
struct Animation {
    int frameCount;
    int currentFrame;
    float frameTime;
    float timer;
    int frameWidth;
    int frameHeight;
};

// PlayerInput component - marks entity as player-controlled
struct PlayerInput {
    float speed;
};

// ColorChange component - for entities that change color on collision
struct ColorChange {
    Color normalColor;
    Color collisionColor;
    bool isColliding;
};

// Building component - marks entity as an enterable building
struct Building {
    std::string displayName;
    int interiorSceneId;
};

// SpeechBubble component - displays text above entity
struct SpeechBubble {
    std::string text;
    bool active;
    float offsetX;
    float offsetY;
};

// Scene component - marks which scene an entity belongs to
struct Scene {
    int sceneId;
};

// Player component - marks the player entity (for scene transitions)
struct Player {
    bool isPlayer;
};

// InteractionZone component - larger hitbox for interaction (doesn't block movement)
struct InteractionZone {
    float width;
    float height;
    float offsetX;
    float offsetY;
    Rectangle bounds;
};

// BuildingInterior component - defines interior scene properties
enum class GUIType {
    None,
    Pomodoro
};

struct BuildingInterior {
    GUIType guiType;
    std::string guiBackgroundPath;
    Color guiBackgroundColor;
    std::string gameBackgroundPath;
    Color gameBackgroundColor;
    Texture2D guiBackgroundTexture;
    Texture2D gameBackgroundTexture;
    bool texturesLoaded;
};

// AIWander component - makes entity wander randomly
struct AIWander {
    float speed;
    float moveTimer;
    float currentMoveTime;
    float minMoveTime;
    float maxMoveTime;
    int direction; // -1 for left, 1 for right
    bool isMoving;
};
