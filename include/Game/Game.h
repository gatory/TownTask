#pragma once
#include "Core/World.h"
#include "Core/Constants.h"
#include "SceneManager.h"
#include "GameState.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/CollisionSystem.h"
#include "Systems/InteractionSystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/GUISystem.h"
#include "Systems/LibraryUISystem.h"
#include "Systems/TodoUISystem.h"
#include "raylib.h"
#include <vector>
#include <string>

class Game {
private:
    World world;
    SceneManager sceneManager;
    PomodoroTimer pomodoroTimer;
    
    AnimationSystem animationSystem;
    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    InteractionSystem interactionSystem;
    RenderSystem renderSystem;
    GUISystem guiSystem;
    LibraryUISystem libraryUISystem;
    TodoUISystem todoUISystem;
    
    Texture2D speechBubbleTexture;
    bool showDebug;
    std::vector<std::string> debugOverlapInfo;
    // Sound effects
    Sound sfxOpenDoor;
    Sound sfxCloseDoor;
    Sound sfxCoffeeAmbience;
    Sound sfxLevelComplete;
    // Background music
    Music mainMusic;
    bool musicPlaying = false;
    bool mainMusicLoaded = false;
    // Volume controls (0.0 - 1.0)
    float musicVolume = 0.5f;
    float sfxVolume = 0.7f;
    

    void PlaySfx(const Sound& s);
    
    void InitializeEntities();
    void Update(float deltaTime);
    void Render();
    int previousScene = SceneID::MAIN;

public:
    Game();
    ~Game();
    void Run();
};
