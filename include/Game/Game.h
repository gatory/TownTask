#pragma once
#include "Core/World.h"
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
    
    void InitializeEntities();
    void Update(float deltaTime);
    void Render();

public:
    Game();
    ~Game();
    void Run();
};
