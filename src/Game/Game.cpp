#include "../include/Game/Game.h"
#include "../include/Core/Constants.h"
#include "../include/Data/DataManager.h"
#include <ctime>
#include <algorithm>

// Use the best available texture-filter constant for pixel-art; fall back to numeric 0 if none present

Game::Game() {
    srand(static_cast<unsigned int>(time(nullptr)));
    
    InitWindow(GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT, "Task Town");
    
    // Initialize audio device before loading sounds
    InitAudioDevice();

    speechBubbleTexture = LoadTexture(AssetPath::SPEECH_BUBBLE);
    // If the asset is missing, create a simple placeholder so bubbles still render
    if (speechBubbleTexture.width == 0 || speechBubbleTexture.height == 0) {
        // generate a small placeholder image (32x24) with a white rounded-ish rectangle
        Image img = GenImageColor(32, 24, BLANK);
        // fill with white background
        Color bg = WHITE;
        for (int y = 0; y < 24; ++y) {
            for (int x = 0; x < 32; ++x) {
                ImageDrawPixel(&img, x, y, bg);
            }
        }
        speechBubbleTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    // Set texture filter to nearest/point (0) to avoid blurring when scaling the bubble texture.
    // Use numeric 0 to remain compatible with multiple raylib versions.
    SetTextureFilter(speechBubbleTexture, 0);
    showDebug = false;

    // Load SFX (if present)
    if (IsAudioDeviceReady()) {
        sfxOpenDoor = LoadSound(AssetPath::SFX_OPEN_DOOR);
        sfxCloseDoor = LoadSound(AssetPath::SFX_CLOSE_DOOR);
        sfxCoffeeAmbience = LoadSound(AssetPath::SFX_COFFEEAMBIENCE);
        sfxLevelComplete = LoadSound(AssetPath::SFX_LEVEL_COMPLETE);
        // Load background music
        mainMusic = LoadMusicStream(AssetPath::MAIN_MUSIC);
    // Assume LoadMusicStream returned a usable Music object; mark as loaded and play
    mainMusicLoaded = true;
    PlayMusicStream(mainMusic);
    musicPlaying = true;
            // Apply initial volumes
            SetMusicVolume(mainMusic, musicVolume);
            SetSoundVolume(sfxOpenDoor, sfxVolume);
            SetSoundVolume(sfxCloseDoor, sfxVolume);
            SetSoundVolume(sfxCoffeeAmbience, sfxVolume);
            SetSoundVolume(sfxLevelComplete, sfxVolume);
    }

    InitializeEntities();
    renderSystem.LoadTextures(world);
}

Game::~Game() {
    UnloadTexture(speechBubbleTexture);
    renderSystem.UnloadTextures(world);
    // Unload sounds
    if (IsAudioDeviceReady()) {
        UnloadSound(sfxOpenDoor);
        UnloadSound(sfxCloseDoor);
        UnloadSound(sfxCoffeeAmbience);
        UnloadSound(sfxLevelComplete);
        // Unload and stop music
        if (mainMusicLoaded) {
            StopMusicStream(mainMusic);
            UnloadMusicStream(mainMusic);
        }
        CloseAudioDevice();
    }
    CloseWindow();
}

void Game::InitializeEntities() {
    using namespace GameConfig;
    using namespace BuildingConfig;
    
    // Main scene background
    Entity bg = world.CreateEntity();
    world.AddPosition(bg, 0, 0);
    // Use the main background texture if present in assets/ (use WHITE tint to preserve original colors)
    world.AddSprite(bg, AssetPath::MAIN_BACKGROUND, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    world.AddScene(bg, SceneID::MAIN);
    
    // Player (cat)
    Entity player = world.CreateEntity();
    world.AddPosition(player, 150, 150);
    world.AddSprite(player, AssetPath::PLAYER_IDLE, PLAYER_SIZE, PLAYER_SIZE, WHITE);
    world.AddHitbox(player, PLAYER_SIZE, PLAYER_SIZE);
    world.AddAnimation(player, PLAYER_ANIMATION_FRAMES, PLAYER_ANIMATION_FRAME_TIME, 
                      PLAYER_SPRITE_FRAME_WIDTH, PLAYER_SPRITE_FRAME_HEIGHT);
    world.AddPlayerInput(player, PLAYER_SPEED);
    // Add AI wander so the player can be set to autonomous
    world.AddAIWander(player, 40.0f, 1.0f, 3.0f);
    world.AddPlayer(player);
    world.AddSpeechBubble(player, AssetPath::SPEECH_BUBBLE, 0, SPEECH_BUBBLE_OFFSET_Y);
    
    // ======= POMODORO BUILDING =======
    Entity pomodoro = world.CreateEntity();
    world.AddPosition(pomodoro, SCREEN_WIDTH - 250, 100);
    // Use a pixel-art building sprite for the pomodoro shop
    world.AddSprite(pomodoro, AssetPath::HOUSE_3, POMODORO_WIDTH, POMODORO_HEIGHT, WHITE);
    world.AddHitbox(pomodoro, POMODORO_WIDTH, POMODORO_HEIGHT);
    world.AddInteractionZone(pomodoro, POMODORO_WIDTH + 20, POMODORO_HEIGHT + 20, -10, -10);
    world.AddBuilding(pomodoro, "Pomodoro", SceneID::POMODORO_INTERIOR);
    world.AddScene(pomodoro, SceneID::MAIN);
    // speech bubble for enter prompt on the building in the main scene
    world.AddSpeechBubble(pomodoro, "", 0, -40);
    
    // Pomodoro interior
    Entity pomodoroInterior = world.CreateEntity();
    world.AddBuildingInterior(pomodoroInterior, GUIType::Pomodoro, "", LIGHTGRAY, 
                             AssetPath::SHOP_BACKGROUND, WHITE);
    world.AddScene(pomodoroInterior, SceneID::POMODORO_INTERIOR);

    // Barista / coffee area in bottom half of the pomodoro interior
    Entity barista = world.CreateEntity();
    // place within the game area (y measured from top of game area)
    // y is local to the interior game area (0..GAME_HEIGHT)
    world.AddPosition(barista, GameConfig::SCREEN_WIDTH/2 - BuildingConfig::NPC_SIZE/2, 200);
    world.AddSprite(barista, AssetPath::LIBRARIAN_SPRITE, BuildingConfig::NPC_SIZE, BuildingConfig::NPC_SIZE, WHITE);
    world.AddHitbox(barista, BuildingConfig::NPC_SIZE, BuildingConfig::NPC_SIZE);
    // Make interaction zone slightly larger
    world.AddInteractionZone(barista, BuildingConfig::NPC_SIZE + 40, BuildingConfig::NPC_SIZE + 40, -20, -20);
    world.AddInteractable(barista, "Barista", "Start Pomodoro (X)", "barista");
    world.AddSpeechBubble(barista, "", 0, -40);
    world.AddScene(barista, SceneID::POMODORO_INTERIOR);
    
        Entity pomodoroDoor = world.CreateEntity();
        // place near bottom-center of the interior local game area
        world.AddPosition(pomodoroDoor, SCREEN_WIDTH/2 - DOOR_WIDTH/2, GameConfig::GAME_HEIGHT - DOOR_HEIGHT - 10);
    world.AddSprite(pomodoroDoor, "", DOOR_WIDTH, DOOR_HEIGHT, DARKBROWN);
    world.AddHitbox(pomodoroDoor, DOOR_WIDTH, DOOR_HEIGHT);
    world.AddInteractionZone(pomodoroDoor, DOOR_WIDTH + 20, DOOR_HEIGHT + 20, -10, -10);
    world.AddExitDoor(pomodoroDoor, "Exit (Z)");
    world.AddSpeechBubble(pomodoroDoor, "", 0, -40);
    world.AddScene(pomodoroDoor, SceneID::POMODORO_INTERIOR);
    
    // ======= LIBRARY BUILDING =======
    Entity library = world.CreateEntity();
    world.AddPosition(library, 50, 150);
    // Use a pixel-art building sprite for the library
    world.AddSprite(library, AssetPath::HOUSE_4, LIBRARY_WIDTH, LIBRARY_HEIGHT, WHITE);
    world.AddHitbox(library, LIBRARY_WIDTH, LIBRARY_HEIGHT);
    world.AddInteractionZone(library, LIBRARY_WIDTH + 20, LIBRARY_HEIGHT + 20, -10, -10);
    world.AddBuilding(library, "Library", SceneID::LIBRARY_INTERIOR);
    world.AddScene(library, SceneID::MAIN);
    // speech bubble for enter prompt on the building in the main scene
    world.AddSpeechBubble(library, "", 0, -40);
    
    // Library interior
    Entity libraryInterior = world.CreateEntity();
    world.AddBuildingInterior(libraryInterior, GUIType::None, "", RAYWHITE, 
                             AssetPath::LIBRARY_BACKGROUND, BEIGE);
    world.AddScene(libraryInterior, SceneID::LIBRARY_INTERIOR);
    
    // Library data
    LibraryData libData;
    libData.selectedIndex = -1;
    libData.scrollOffset = 0;
    libData.isShowingUI = false;
    memset(libData.searchBuffer, 0, sizeof(libData.searchBuffer));
    
    // Load from JSON
    DataManager::LoadLibraryData("data/library.json", libData.ebookPaths, libData.ebookTitles);
    
    // Add default books if empty
    if (libData.ebookPaths.empty()) {
        libData.ebookPaths = {"books/sample1.pdf", "books/sample2.pdf"};
        libData.ebookTitles = {"Introduction to Programming", "Game Development Basics"};
        DataManager::SaveLibraryData("data/library.json", libData.ebookPaths, libData.ebookTitles);
    }
    
    world.AddLibraryData(libraryInterior, libData);
    
        Entity libraryDoor = world.CreateEntity();
        // place near bottom-center of the library interior local game area
        world.AddPosition(libraryDoor, SCREEN_WIDTH/2 - DOOR_WIDTH/2, GameConfig::GAME_HEIGHT - DOOR_HEIGHT - 10);
    world.AddSprite(libraryDoor, "", DOOR_WIDTH, DOOR_HEIGHT, DARKBROWN);
    world.AddHitbox(libraryDoor, DOOR_WIDTH, DOOR_HEIGHT);
    world.AddInteractionZone(libraryDoor, DOOR_WIDTH + 20, DOOR_HEIGHT + 20, -10, -10);
    world.AddExitDoor(libraryDoor, "Exit (Z)");
    world.AddSpeechBubble(libraryDoor, "", 0, -40);
    world.AddScene(libraryDoor, SceneID::LIBRARY_INTERIOR);
    
    Entity librarian = world.CreateEntity();
    // y is local to interior game area
    world.AddPosition(librarian, SCREEN_WIDTH/2 - NPC_SIZE/2, 80);
    world.AddSprite(librarian, AssetPath::LIBRARIAN_SPRITE, NPC_SIZE, NPC_SIZE, WHITE);
    world.AddHitbox(librarian, NPC_SIZE, NPC_SIZE);
    world.AddAnimation(librarian, PLAYER_ANIMATION_FRAMES, PLAYER_ANIMATION_FRAME_TIME, 
                      PLAYER_SPRITE_FRAME_WIDTH, PLAYER_SPRITE_FRAME_HEIGHT);
    world.AddInteractionZone(librarian, NPC_SIZE + 30, NPC_SIZE + 30, -15, -15);
    world.AddInteractable(librarian, "Librarian", "Talk to Librarian (X)", "librarian");
    world.AddSpeechBubble(librarian, "", 0, -40);
    world.AddScene(librarian, SceneID::LIBRARY_INTERIOR);
    
    // ======= HOUSE BUILDING =======
    Entity house = world.CreateEntity();
    world.AddPosition(house, SCREEN_WIDTH - 400, 300);
    // Use a pixel-art house sprite from assets and scale it to house dimensions
    world.AddSprite(house, AssetPath::HOUSE_1, HOUSE_WIDTH, HOUSE_HEIGHT, WHITE);
    world.AddHitbox(house, HOUSE_WIDTH, HOUSE_HEIGHT);
    world.AddInteractionZone(house, HOUSE_WIDTH + 20, HOUSE_HEIGHT + 20, -10, -10);
    world.AddBuilding(house, "My House", SceneID::HOUSE_INTERIOR);
    world.AddScene(house, SceneID::MAIN);
    // speech bubble for enter prompt on the building in the main scene
    world.AddSpeechBubble(house, "", 0, -40);
    
    // House interior
    Entity houseInterior = world.CreateEntity();
    world.AddBuildingInterior(houseInterior, GUIType::None, "", SKYBLUE, 
                             AssetPath::HOUSE_BACKGROUND, LIGHTGRAY);
    world.AddScene(houseInterior, SceneID::HOUSE_INTERIOR);
    
    // Todo list data
    TodoListData todoData;
    todoData.selectedIndex = -1;
    todoData.scrollOffset = 0;
    todoData.isShowingUI = false;
    todoData.isAddingNew = false;
    memset(todoData.inputBuffer, 0, sizeof(todoData.inputBuffer));
    
    // Load from JSON
    DataManager::LoadTodoList("data/todolist.json", todoData.tasks, todoData.completed);
    
    // Add default tasks if empty
    if (todoData.tasks.empty()) {
        todoData.tasks = {"Complete project setup", "Read documentation", "Practice coding"};
        todoData.completed = {false, false, false};
        DataManager::SaveTodoList("data/todolist.json", todoData.tasks, todoData.completed);
    }
    
    world.AddTodoListData(houseInterior, todoData);
    
        Entity houseDoor = world.CreateEntity();
        // place near bottom-center of the house interior local game area
        world.AddPosition(houseDoor, SCREEN_WIDTH/2 - DOOR_WIDTH/2, GameConfig::GAME_HEIGHT - DOOR_HEIGHT - 10);
    world.AddSprite(houseDoor, "", DOOR_WIDTH, DOOR_HEIGHT, DARKBROWN);
    world.AddHitbox(houseDoor, DOOR_WIDTH, DOOR_HEIGHT);
    world.AddInteractionZone(houseDoor, DOOR_WIDTH + 20, DOOR_HEIGHT + 20, -10, -10);
    world.AddExitDoor(houseDoor, "Exit (Z)");
    world.AddSpeechBubble(houseDoor, "", 0, -40);
    world.AddScene(houseDoor, SceneID::HOUSE_INTERIOR);
    
    Entity desk = world.CreateEntity();
    // desk positioned in local interior coordinates (y relative to top of game area)
    world.AddPosition(desk, SCREEN_WIDTH/2 - DESK_WIDTH/2, 150);
    world.AddSprite(desk, AssetPath::DESK_SPRITE, DESK_WIDTH, DESK_HEIGHT, WHITE);
    world.AddHitbox(desk, DESK_WIDTH, DESK_HEIGHT);
    world.AddInteractionZone(desk, DESK_WIDTH + 30, DESK_HEIGHT + 30, -15, -15);
    world.AddInteractable(desk, "Desk", "Use Desk (X)", "desk");
    world.AddSpeechBubble(desk, "", 0, -40);
    world.AddScene(desk, SceneID::HOUSE_INTERIOR);

    // --- Development helper: ensure player isn't spawned overlapping another hitbox ---
    // Find player entity
    Entity playerEntity = INVALID_ENTITY;
    for (Entity e : world.GetEntities()) {
        if (world.HasPlayer(e)) { playerEntity = e; break; }
    }
    if (playerEntity != INVALID_ENTITY) {
        Position* ppos = world.GetPosition(playerEntity);
        Hitbox* phb = world.GetHitbox(playerEntity);
        if (ppos && phb) {
            const int maxNudge = 50;
            int attempts = 0;
            bool stillColliding = true;
            while (stillColliding && attempts < maxNudge) {
                stillColliding = false;
                for (Entity other : world.GetEntities()) {
                    if (other == playerEntity) continue;
                    Scene* scene = world.GetScene(other);
                    if (!scene || scene->sceneId != SceneID::MAIN) continue;
                    Hitbox* ohb = world.GetHitbox(other);
                    if (!ohb) continue;
                    if (CheckCollisionRecs(phb->bounds, ohb->bounds)) {
                        // Nudge player to the right
                        ppos->x += 16;
                        phb->bounds.x = ppos->x;
                        stillColliding = true;
                        break;
                    }
                }
                attempts++;
            }
        }
    }
}

void Game::Update(float deltaTime) {
    int currentScene = sceneManager.GetCurrentScene();

    // Detect scene transition and play door sounds
    if (previousScene != currentScene) {
        if (previousScene == SceneID::MAIN && currentScene != SceneID::MAIN) {
            // Entering an interior
            PlaySfx(sfxOpenDoor);
        } else if (previousScene != SceneID::MAIN && currentScene == SceneID::MAIN) {
            // Exiting to main
            PlaySfx(sfxCloseDoor);
        }
        previousScene = currentScene;
    }

    // Update systems
    animationSystem.Update(world, deltaTime);

    // Update music stream each frame
    if (IsAudioDeviceReady() && mainMusicLoaded) UpdateMusicStream(mainMusic);

    // Remember player's frozen state to detect unfreeze
    Entity playerEntity = INVALID_ENTITY;
    bool wasFrozen = false;
    for (Entity e : world.GetEntities()) {
        if (world.HasPlayer(e)) { playerEntity = e; break; }
    }
    if (playerEntity != INVALID_ENTITY) {
        PlayerInput* ppi = world.GetPlayerInput(playerEntity);
        if (ppi) wasFrozen = ppi->frozen;
    }

    movementSystem.UpdatePlayerInput(world, deltaTime, currentScene);
    movementSystem.UpdateAIWander(world, deltaTime, currentScene);
    collisionSystem.Update(world, currentScene);
    interactionSystem.Update(world, sceneManager);

    // (speech bubbles are adaptive-sized; no fade/timeout behavior here)

    // If player pressed X inside pomodoro interior and is near a barista, start pomodoro
    if (IsKeyPressed(KEY_X) && currentScene == SceneID::POMODORO_INTERIOR && playerEntity != INVALID_ENTITY) {
        Hitbox* phb = world.GetHitbox(playerEntity);
        if (phb) {
            for (Entity e : world.GetEntities()) {
                Scene* sc = world.GetScene(e);
                if (!sc || sc->sceneId != currentScene) continue;
                Interactable* it = world.GetInteractable(e);
                InteractionZone* iz = world.GetInteractionZone(e);
                if (!it || !iz) continue;
                if (it->interactionType == "barista" && CheckCollisionRecs(phb->bounds, iz->bounds)) {
                    pomodoroTimer.Start();
                    // Play start sfx
                    PlaySfx(sfxCoffeeAmbience);
                    PlayerInput* pi = world.GetPlayerInput(playerEntity);
                    if (pi) pi->frozen = true;
                    SpeechBubble* sb = world.GetSpeechBubble(playerEntity);
                    if (sb) { sb->active = true; sb->text = "I'm studying"; }
                    break;
                }
            }
        }
    }

    // Update timer and UI input handling
    pomodoroTimer.Update();
    libraryUISystem.HandleInput(world);
    todoUISystem.HandleInput(world);

    // After systems: if the player was frozen but is now unfrozen, stop the pomodoro
    if (playerEntity != INVALID_ENTITY) {
        PlayerInput* ppi2 = world.GetPlayerInput(playerEntity);
        if (ppi2 && wasFrozen && !ppi2->frozen) {
            pomodoroTimer.Reset();
            SpeechBubble* sb = world.GetSpeechBubble(playerEntity);
            if (sb) sb->active = false;
            // Play reset/complete sfx
            PlaySfx(sfxLevelComplete);
        }
    }

    // Toggle debug overlay
    if (IsKeyPressed(KEY_F1)) {
        showDebug = !showDebug;
    }

    // Dev teleport: move player to a safe location for testing
    if (IsKeyPressed(KEY_F3)) {
        // Find player
        for (Entity e : world.GetEntities()) {
            if (!world.HasPlayer(e)) continue;
            Position* ppos = world.GetPosition(e);
            Hitbox* phb = world.GetHitbox(e);
            if (!ppos || !phb) continue;
            ppos->x = GameConfig::SCREEN_WIDTH / 2.0f;
            // teleport to a safe spot in local interior coords (or main scene coords if in main)
            int cs = sceneManager.GetCurrentScene();
            if (cs == SceneID::MAIN) ppos->y = GameConfig::SCREEN_HEIGHT / 2.0f;
            else ppos->y = GameConfig::GAME_HEIGHT / 2.0f;
            phb->bounds.x = ppos->x;
            phb->bounds.y = ppos->y;
            break;
        }
    }
}

void Game::PlaySfx(const Sound& s) {
    if (IsAudioDeviceReady()) PlaySound(s);
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    int currentScene = sceneManager.GetCurrentScene();

    if (currentScene == SceneID::MAIN) {
        // Main scene: draw the main background stretched to the window, then render entities
        renderSystem.RenderMainBackground(world);
        renderSystem.RenderEntities(world, SceneID::MAIN, 0);
    } else {
        // Interior: render GUI background + game background
        renderSystem.RenderBuildingInteriorBackgrounds(world, currentScene, GameConfig::GUI_HEIGHT, GameConfig::GAME_HEIGHT);
        renderSystem.RenderEntities(world, currentScene, GameConfig::GUI_HEIGHT);

        // Render GUI overlay and specific UI systems
    guiSystem.Render(world, currentScene, pomodoroTimer, GameConfig::GUI_HEIGHT);
        libraryUISystem.Render(world, currentScene);
        todoUISystem.Render(world, currentScene);
    }

    // Speech bubbles drawn last
    renderSystem.RenderSpeechBubbles(world, speechBubbleTexture);

    // Debug overlay: show whether window has focus and arrow key states
    bool focused = IsWindowFocused();
    const char* focusText = focused ? "Window: Focused" : "Window: Not Focused";
    int focusX = 10;
    int focusY = 10;
    DrawText(focusText, focusX, focusY, 14, focused ? DARKBROWN : RED);
    DrawText(TextFormat("Keys L/R/U/D: %d %d %d %d", IsKeyDown(KEY_LEFT), IsKeyDown(KEY_RIGHT), IsKeyDown(KEY_UP), IsKeyDown(KEY_DOWN)),
             10, 30, 12, BLACK);

    // Control mode overlay (top-right): two buttons Manual / Wander
    int panelX = GameConfig::SCREEN_WIDTH - 180;
    int panelY = 8;
    int panelW = 172;
    int panelH = 36;
    Rectangle panelRec = { (float)panelX, (float)panelY, (float)panelW, (float)panelH };
    DrawRectangleRec(panelRec, Fade(RAYWHITE, 0.1f));

    int btnW = 80;
    int btnH = 28;
    Rectangle manualBtn = { (float)(panelX + 6), (float)(panelY + 4), (float)btnW, (float)btnH };
    Rectangle wanderBtn = { (float)(panelX + 6 + btnW + 6), (float)(panelY + 4), (float)btnW, (float)btnH };

    // Determine current state from player
    bool playerControlled = true;
    for (Entity e : world.GetEntities()) {
        if (!world.HasPlayer(e)) continue;
        PlayerInput* pi = world.GetPlayerInput(e);
        if (pi) { playerControlled = pi->controlled; break; }
    }

    // Draw buttons
    DrawRectangleRec(manualBtn, playerControlled ? DARKGREEN : DARKGRAY);
    DrawText("Manual (M)", panelX + 12, panelY + 10, 12, RAYWHITE);

    DrawRectangleRec(wanderBtn, !playerControlled ? DARKGREEN : DARKGRAY);
    DrawText("Wander (W)", panelX + 12 + btnW + 6, panelY + 10, 12, RAYWHITE);

    Vector2 mouse = GetMousePosition();
    bool manualClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, manualBtn);
    bool wanderClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, wanderBtn);

    // Keyboard shortcuts: M for Manual, W for Wander, T to toggle
    if (manualClicked || IsKeyPressed(KEY_M)) {
        for (Entity e : world.GetEntities()) {
            if (!world.HasPlayer(e)) continue;
            PlayerInput* pi = world.GetPlayerInput(e);
            if (pi) { pi->controlled = true; break; }
        }
    }
    if (wanderClicked || IsKeyPressed(KEY_W)) {
        for (Entity e : world.GetEntities()) {
            if (!world.HasPlayer(e)) continue;
            PlayerInput* pi = world.GetPlayerInput(e);
            if (pi) {
                pi->controlled = false;
                // ensure the player has an AIWander component so it actually wanders
                if (!world.HasAIWander(e)) {
                    world.AddAIWander(e, GameConfig::AI_WANDER_SPEED, GameConfig::AI_MIN_MOVE_TIME, GameConfig::AI_MAX_MOVE_TIME);
                }
                break;
            }
        }
    }
    if (IsKeyPressed(KEY_T)) {
        for (Entity e : world.GetEntities()) {
            if (!world.HasPlayer(e)) continue;
            PlayerInput* pi = world.GetPlayerInput(e);
            if (pi) {
                pi->controlled = !pi->controlled;
                if (!pi->controlled) {
                    if (!world.HasAIWander(e)) {
                        world.AddAIWander(e, GameConfig::AI_WANDER_SPEED, GameConfig::AI_MIN_MOVE_TIME, GameConfig::AI_MAX_MOVE_TIME);
                    }
                } else {
                    // switching to manual; unfreeze if necessary
                    pi->frozen = false;
                }
                break;
            }
        }
    }

    // Volume controls: position under the debug text (below the Keys line) to avoid overlap
    int volX = 10; // align left with other debug text
    int volY = 54; // below the keys debug line (which starts at y=30)
    int volW = 220;
    int volH = 28;
    DrawText("Music", volX, volY, 12, BLACK);
    // Slider background
    DrawRectangle(volX + 50, volY, volW - 60, volH, Fade(LIGHTGRAY, 0.8f));
    // Slider knob
    int knobX = volX + 50 + (int)((volW - 60) * musicVolume) - 6;
    DrawRectangle(knobX, volY + 6, 12, volH - 12, DARKGRAY);

    DrawText("SFX", volX, volY + 36, 12, BLACK);
    DrawRectangle(volX + 50, volY + 36, volW - 60, volH, Fade(LIGHTGRAY, 0.8f));
    int knob2X = volX + 50 + (int)((volW - 60) * sfxVolume) - 6;
    DrawRectangle(knob2X, volY + 42, 12, volH - 12, DARKGRAY);

    // Keyboard shortcuts: M/m increase/decrease music, S/s for sfx
    if (IsKeyPressed(KEY_M)) {
        musicVolume = std::min(1.0f, musicVolume + 0.1f);
        if (IsAudioDeviceReady() && mainMusicLoaded) SetMusicVolume(mainMusic, musicVolume);
    }
    if (IsKeyPressed(KEY_M | 0)) { /* placeholder to avoid duplicate define warnings */ }
    if (IsKeyPressed(KEY_S)) {
        sfxVolume = std::min(1.0f, sfxVolume + 0.1f);
        if (IsAudioDeviceReady()) {
            SetSoundVolume(sfxOpenDoor, sfxVolume);
            SetSoundVolume(sfxCloseDoor, sfxVolume);
            SetSoundVolume(sfxCoffeeAmbience, sfxVolume);
            SetSoundVolume(sfxLevelComplete, sfxVolume);
        }
    }
    // Decrease with Shift + M / Shift + S (or lowercase key handling)
    if (IsKeyPressed(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_M)) {
        musicVolume = std::max(0.0f, musicVolume - 0.1f);
        if (IsAudioDeviceReady() && mainMusicLoaded) SetMusicVolume(mainMusic, musicVolume);
    }
    if (IsKeyPressed(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) {
        sfxVolume = std::max(0.0f, sfxVolume - 0.1f);
        if (IsAudioDeviceReady()) {
            SetSoundVolume(sfxOpenDoor, sfxVolume);
            SetSoundVolume(sfxCloseDoor, sfxVolume);
            SetSoundVolume(sfxCoffeeAmbience, sfxVolume);
            SetSoundVolume(sfxLevelComplete, sfxVolume);
        }
    }

    // Mouse control: dragging sliders
    Rectangle musicSlider = { (float)(volX + 50), (float)volY, (float)(volW - 60), (float)volH };
    Rectangle sfxSlider = { (float)(volX + 50), (float)(volY + 36), (float)(volW - 60), (float)volH };
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, musicSlider)) {
            float rel = (m.x - musicSlider.x) / musicSlider.width;
            musicVolume = std::clamp(rel, 0.0f, 1.0f);
            if (IsAudioDeviceReady() && mainMusicLoaded) SetMusicVolume(mainMusic, musicVolume);
        } else if (CheckCollisionPointRec(m, sfxSlider)) {
            float rel = (m.x - sfxSlider.x) / sfxSlider.width;
            sfxVolume = std::clamp(rel, 0.0f, 1.0f);
            if (IsAudioDeviceReady()) {
                SetSoundVolume(sfxOpenDoor, sfxVolume);
                SetSoundVolume(sfxCloseDoor, sfxVolume);
                SetSoundVolume(sfxCoffeeAmbience, sfxVolume);
                SetSoundVolume(sfxLevelComplete, sfxVolume);
            }
        }
    }

    // Debug draw: hitboxes and interaction zones (only when enabled)
    if (showDebug) {
        for (Entity e : world.GetEntities()) {
            Position* pos = world.GetPosition(e);
            Hitbox* hb = world.GetHitbox(e);
            InteractionZone* iz = world.GetInteractionZone(e);
            if (hb && pos) {
                DrawRectangleLinesEx({hb->bounds.x, hb->bounds.y, hb->bounds.width, hb->bounds.height}, 2, BLUE);
            }
            if (iz && pos) {
                DrawRectangleLinesEx({iz->bounds.x, iz->bounds.y, iz->bounds.width, iz->bounds.height}, 2, ORANGE);
            }
        }

        // Show overlaps between player and other hitboxes
        // Find player
        Entity player = INVALID_ENTITY;
        for (Entity e : world.GetEntities()) {
            if (world.HasPlayer(e)) { player = e; break; }
        }
        if (player != INVALID_ENTITY) {
            Hitbox* phb = world.GetHitbox(player);
            Position* ppos = world.GetPosition(player);
            if (phb && ppos) {
                int y = 60;
                for (Entity other : world.GetEntities()) {
                    if (other == player) continue;
                    Scene* scene = world.GetScene(other);
                    if (!scene || scene->sceneId != SceneID::MAIN) continue;
                    Hitbox* ohb = world.GetHitbox(other);
                    if (!ohb) continue;
                    if (CheckCollisionRecs(phb->bounds, ohb->bounds)) {
                        // Determine label
                        std::string label;
                        Building* b = world.GetBuilding(other);
                        Interactable* it = world.GetInteractable(other);
                        ExitDoor* ed = world.GetExitDoor(other);
                        if (b) label = "Building: " + b->displayName;
                        else if (it) label = "Interactable: " + it->displayName;
                        else if (ed) label = "ExitDoor: " + ed->displayText;
                        else label = "Entity " + std::to_string(other);

                        // Highlight rectangle and print label
                        DrawRectangleLinesEx({ohb->bounds.x, ohb->bounds.y, ohb->bounds.width, ohb->bounds.height}, 3, RED);
                        DrawText(label.c_str(), (int)ohb->bounds.x, (int)(ohb->bounds.y - 16), 12, RED);
                        DrawText(label.c_str(), GameConfig::SCREEN_WIDTH - 300, y, 12, BLACK);
                        y += 18;
                    }
                }
            }
            // Also print player status (position and component presence)
            {
                std::string status = "";
                bool hasPos = (world.GetPosition(player) != nullptr);
                bool hasHB = (world.GetHitbox(player) != nullptr);
                bool hasPI = (world.GetPlayerInput(player) != nullptr);
                bool hasAnim = (world.GetAnimation(player) != nullptr);
                if (ppos) {
                    status = TextFormat("Player pos: %.1f, %.1f", ppos->x, ppos->y);
                    DrawText(status.c_str(), GameConfig::SCREEN_WIDTH - 300, 20, 12, BLACK);
                }
                status = TextFormat("HasPos:%d HasHB:%d HasPI:%d HasAnim:%d", hasPos, hasHB, hasPI, hasAnim);
                DrawText(status.c_str(), GameConfig::SCREEN_WIDTH - 300, 36, 12, BLACK);
            }
        }
    }

    EndDrawing();
}

void Game::Run() {
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Update(dt);
        Render();
    }
}
