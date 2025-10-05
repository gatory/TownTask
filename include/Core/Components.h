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
    // Fade/timeout
    float lifetime; // total time to show in seconds (0 = persistent)
    float elapsed; // time since shown
    float alpha; // 0..1 current alpha for fade
    float fadeIn; // seconds to fade in
    float fadeOut; // seconds to fade out at end of lifetime
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

// struct FaceDetection {
//     bool isTracking;           // Whether face detection is active
//     bool faceDetected;         // Current face detection state
//     float timeAway;            // How long user has been away
//     float awayThreshold;       // Threshold before reaction (e.g., 5.0 seconds)
//     bool hasReacted;           // Prevent multiple reactions
//     float reactionCooldown;    // Time before can react again
//     float cooldownTimer;       // Current cooldown timer
    
//     FaceDetection() 
//         : isTracking(false), faceDetected(true), timeAway(0.0f), 
//           awayThreshold(5.0f), hasReacted(false), 
//           reactionCooldown(10.0f), cooldownTimer(0.0f) {}
// };

// // Also add these states to your existing components if needed
// enum class CharacterState {
//     Idle,
//     Walking,
//     Angry,
//     Sleeping    // For when user is away too long
// };

// // Add to existing PlayerInput or create new component
// struct CharacterStateComponent {
//     CharacterState currentState;
//     float stateTimer;          // How long in current state
//     std::string emotionText;   // Text to display in speech bubble
    
//     CharacterStateComponent() 
//         : currentState(CharacterState::Idle), stateTimer(0.0f), 
//           emotionText("") {}
// };