#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct Position {
    float x;
    float y;
};

struct Sprite {
    std::string texturePath;
    Texture2D texture;
    int width;
    int height;
    Color tint;
    bool loaded;
};

struct Hitbox {
    float width;
    float height;
    Rectangle bounds;
    bool colliding;
};

struct Animation {
    int frameCount;
    int currentFrame;
    float frameTime;
    float timer;
    int frameWidth;
    int frameHeight;
};

struct PlayerInput {
    float speed;
    bool controlled;
    bool frozen;
};

struct Building {
    std::string displayName;
    int interiorSceneId;
};

struct SpeechBubble {
    std::string text;
    bool active;
    float offsetX;
    float offsetY;
};

struct Scene {
    int sceneId;
};

struct Player {
    bool isPlayer;
};

struct InteractionZone {
    float width;
    float height;
    float offsetX;
    float offsetY;
    Rectangle bounds;
};

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

struct AIWander {
    float speed;
    float moveTimer;
    float currentMoveTime;
    float minMoveTime;
    float maxMoveTime;
    float dirX;
    float dirY;
    bool isMoving;
};

struct ExitDoor {
    std::string displayText;
    bool isPlayerNear;
};

struct Interactable {
    std::string displayName;
    std::string interactionPrompt;
    std::string interactionType;
    bool isPlayerNear;
};

struct LibraryData {
    std::vector<std::string> ebookPaths;
    std::vector<std::string> ebookTitles;
    int selectedIndex;
    int scrollOffset;
    bool isShowingUI;
    char searchBuffer[128];
};

struct TodoListData {
    std::vector<std::string> tasks;
    std::vector<bool> completed;
    int selectedIndex;
    int scrollOffset;
    bool isShowingUI;
    bool isAddingNew;
    char inputBuffer[256];
};