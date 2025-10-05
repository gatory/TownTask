#pragma once
#include "World.h"
#include "SceneManager.h"
#include "GameState.h"
#include "raylib.h"
#include "raygui.h"

// Animation System - updates animation frames over time
class AnimationSystem {
public:
    static void Update(World& world, float deltaTime) {
        for (Entity e : world.GetEntities()) {
            Animation* anim = world.GetAnimation(e);
            if (!anim) continue;

            anim->timer -= deltaTime;
            if (anim->timer <= 0) {
                anim->timer = anim->frameTime;
                anim->currentFrame = (anim->currentFrame + 1) % anim->frameCount;
            }
        }
    }
};

// AI Wander System - makes entities wander randomly
class AIWanderSystem {
public:
    static void Update(World& world, float deltaTime, int screenWidth, int gameHeight) {
        for (Entity e : world.GetEntities()) {
            AIWander* wander = world.GetAIWander(e);
            if (!wander) continue;

            Position* pos = world.GetPosition(e);
            Sprite* sprite = world.GetSprite(e);
            if (!pos || !sprite) continue;

            // Update timer
            wander->moveTimer += deltaTime;

            if (wander->moveTimer >= wander->currentMoveTime) {
                // Time to change state
                wander->moveTimer = 0;

                if (wander->isMoving) {
                    // Stop moving
                    wander->isMoving = false;
                    wander->currentMoveTime = ((float)rand() / RAND_MAX) * (wander->maxMoveTime - wander->minMoveTime) + wander->minMoveTime;
                } else {
                    // Start moving
                    wander->isMoving = true;
                    wander->currentMoveTime = ((float)rand() / RAND_MAX) * (wander->maxMoveTime - wander->minMoveTime) + wander->minMoveTime;

                    // Random direction
                    wander->direction = (rand() % 2 == 0) ? -1 : 1;
                }
            }

            // Move if active
            if (wander->isMoving) {
                pos->x += wander->direction * wander->speed * deltaTime;

                // Check bounds and reverse direction
                if (pos->x < 0) {
                    pos->x = 0;
                    wander->direction = 1;
                }
                if (pos->x > screenWidth - sprite->width) {
                    pos->x = screenWidth - sprite->width;
                    wander->direction = -1;
                }
            }
        }
    }
};

// Input System - handles player input and movement
class InputSystem {
public:
    static void Update(World& world, float deltaTime, int screenWidth, int screenHeight, int currentSceneId) {
        for (Entity e : world.GetEntities()) {
            PlayerInput* input = world.GetPlayerInput(e);
            if (!input) continue;

            // Skip if in a building interior (AI wander takes over)
            if (currentSceneId != SCENE_MAIN && world.HasAIWander(e)) continue;

            Position* pos = world.GetPosition(e);
            Sprite* sprite = world.GetSprite(e);
            Hitbox* playerHitbox = world.GetHitbox(e);
            if (!pos || !sprite || !playerHitbox) continue;

            // Store original position
            float originalX = pos->x;
            float originalY = pos->y;

            // Handle arrow key input
            if (IsKeyDown(KEY_RIGHT)) {
                pos->x += input->speed * deltaTime;
            }
            if (IsKeyDown(KEY_LEFT)) {
                pos->x -= input->speed * deltaTime;
            }
            if (IsKeyDown(KEY_DOWN)) {
                pos->y += input->speed * deltaTime;
            }
            if (IsKeyDown(KEY_UP)) {
                pos->y -= input->speed * deltaTime;
            }

            // Update player hitbox for collision check
            playerHitbox->bounds.x = pos->x;
            playerHitbox->bounds.y = pos->y;

            // Check collision with buildings in current scene
            bool collided = false;
            for (Entity other : world.GetEntities()) {
                Scene* scene = world.GetScene(other);
                if (!scene || scene->sceneId != currentSceneId) continue;

                Hitbox* otherHitbox = world.GetHitbox(other);
                if (!otherHitbox || other == e) continue;

                if (CheckCollisionRecs(playerHitbox->bounds, otherHitbox->bounds)) {
                    collided = true;
                    break;
                }
            }

            // If collided, revert to original position
            if (collided) {
                pos->x = originalX;
                pos->y = originalY;
            }

            // Keep within screen boundaries
            if (pos->x < 0) pos->x = 0;
            if (pos->y < 0) pos->y = 0;
            if (pos->x > screenWidth - sprite->width) pos->x = screenWidth - sprite->width;
            if (pos->y > screenHeight - sprite->height) pos->y = screenHeight - sprite->height;
        }
    }
};

// Collision System - updates hitboxes and checks for collisions
class CollisionSystem {
public:
    static void Update(World& world, int currentSceneId) {
        // First, update all hitbox and interaction zone positions
        for (Entity e : world.GetEntities()) {
            Position* pos = world.GetPosition(e);

            Hitbox* hitbox = world.GetHitbox(e);
            if (pos && hitbox) {
                hitbox->bounds.x = pos->x;
                hitbox->bounds.y = pos->y;
                hitbox->bounds.width = hitbox->width;
                hitbox->bounds.height = hitbox->height;
            }

            InteractionZone* zone = world.GetInteractionZone(e);
            if (pos && zone) {
                zone->bounds.x = pos->x + zone->offsetX;
                zone->bounds.y = pos->y + zone->offsetY;
                zone->bounds.width = zone->width;
                zone->bounds.height = zone->height;
            }
        }

        // Reset all collision states
        for (Entity e : world.GetEntities()) {
            ColorChange* colorChange = world.GetColorChange(e);
            if (colorChange) {
                colorChange->isColliding = false;
            }

            SpeechBubble* bubble = world.GetSpeechBubble(e);
            if (bubble) {
                bubble->active = false;
            }
        }

        // Check collisions between player and buildings (only in current scene)
        const auto& entities = world.GetEntities();
        Entity playerEntity = -1;

        // Find player
        for (Entity e : entities) {
            if (world.HasPlayer(e)) {
                playerEntity = e;
                break;
            }
        }

        if (playerEntity == -1) return;

        Hitbox* playerHitbox = world.GetHitbox(playerEntity);
        if (!playerHitbox) return;

        // Check collisions with building interaction zones in current scene
        for (Entity e : entities) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentSceneId) continue;

            Building* building = world.GetBuilding(e);
            InteractionZone* interactionZone = world.GetInteractionZone(e);
            if (!building || !interactionZone) continue;

            if (CheckCollisionRecs(playerHitbox->bounds, interactionZone->bounds)) {
                // Activate speech bubble on player
                SpeechBubble* bubble = world.GetSpeechBubble(playerEntity);
                if (bubble) {
                    bubble->active = true;
                    bubble->text = "Enter \n" + building->displayName + "? (x)";
                }
            }
        }
    }
};

// Render System - draws all sprites and entities
class RenderSystem {
public:
    // Load all textures that haven't been loaded yet
    static void LoadTextures(World& world) {
        for (Entity e : world.GetEntities()) {
            Sprite* sprite = world.GetSprite(e);
            if (sprite && !sprite->loaded && !sprite->texturePath.empty()) {
                sprite->texture = LoadTexture(sprite->texturePath.c_str());
                sprite->loaded = true;
            }

            BuildingInterior* interior = world.GetBuildingInterior(e);
            if (interior && !interior->texturesLoaded) {
                if (!interior->guiBackgroundPath.empty()) {
                    interior->guiBackgroundTexture = LoadTexture(interior->guiBackgroundPath.c_str());
                }
                if (!interior->gameBackgroundPath.empty()) {
                    interior->gameBackgroundTexture = LoadTexture(interior->gameBackgroundPath.c_str());
                }
                interior->texturesLoaded = true;
            }
        }
    }

    static void RenderBuildingInteriorBackgrounds(World& world, int currentSceneId, int guiHeight, int gameHeight) {
        // Find building interior for current scene
        for (Entity e : world.GetEntities()) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentSceneId) continue;

            BuildingInterior* interior = world.GetBuildingInterior(e);
            if (!interior) continue;

            // Draw GUI background (top half)
            if (!interior->guiBackgroundPath.empty() && interior->texturesLoaded) {
                Rectangle source = {0, 0, (float)interior->guiBackgroundTexture.width, (float)interior->guiBackgroundTexture.height};
                Rectangle dest = {0, 0, 800, (float)guiHeight};
                DrawTexturePro(interior->guiBackgroundTexture, source, dest, Vector2{0, 0}, 0, WHITE);
            } else {
                DrawRectangle(0, 0, 800, guiHeight, interior->guiBackgroundColor);
            }

            // Draw game background (bottom half)
            if (!interior->gameBackgroundPath.empty() && interior->texturesLoaded) {
                Rectangle source = {0, 0, (float)interior->gameBackgroundTexture.width, (float)interior->gameBackgroundTexture.height};
                Rectangle dest = {0, (float)guiHeight, 800, (float)gameHeight};
                DrawTexturePro(interior->gameBackgroundTexture, source, dest, Vector2{0, 0}, 0, WHITE);
            } else {
                DrawRectangle(0, guiHeight, 800, gameHeight, interior->gameBackgroundColor);
            }

            break;
        }
    }

    static void Render(World& world, int currentSceneId, int guiHeight) {
        for (Entity e : world.GetEntities()) {
            // Check if entity belongs to current scene (or has no scene component = always render)
            Scene* scene = world.GetScene(e);
            bool isPlayer = world.HasPlayer(e);

            // Render if: no scene component (backgrounds), is player, or is in current scene
            if (scene && !isPlayer && scene->sceneId != currentSceneId) continue;

            Position* pos = world.GetPosition(e);
            Sprite* sprite = world.GetSprite(e);
            if (!pos || !sprite || !sprite->loaded) continue;

            // Skip BuildingInterior entities (they're backgrounds, handled separately)
            if (world.HasBuildingInterior(e)) continue;

            // Determine the color to use
            Color color = sprite->tint;
            ColorChange* colorChange = world.GetColorChange(e);
            if (colorChange) {
                color = colorChange->isColliding ? colorChange->collisionColor : colorChange->normalColor;
            }

            // Offset player position in building interiors (bottom half only)
            float renderY = pos->y;
            if (isPlayer) {
                BuildingInterior* interior = nullptr;
                for (Entity ie : world.GetEntities()) {
                    Scene* iscene = world.GetScene(ie);
                    if (iscene && iscene->sceneId == currentSceneId && world.HasBuildingInterior(ie)) {
                        interior = world.GetBuildingInterior(ie);
                        break;
                    }
                }
                if (interior) {
                    renderY += guiHeight;
                }
            }

            Animation* anim = world.GetAnimation(e);
            if (anim) {
                // Draw animated sprite
                Rectangle source = {
                    (float)(anim->currentFrame * anim->frameWidth),
                    0,
                    (float)anim->frameWidth,
                    (float)anim->frameHeight
                };
                Rectangle dest = {
                    pos->x,
                    renderY,
                    (float)sprite->width,
                    (float)sprite->height
                };
                DrawTexturePro(sprite->texture, source, dest, Vector2{0, 0}, 0, color);
            } else {
                // Draw static sprite/rectangle
                Hitbox* hitbox = world.GetHitbox(e);
                if (hitbox && sprite->texturePath.empty()) {
                    // If no texture path and has hitbox, draw as rectangle
                    DrawRectangle((int)pos->x, (int)renderY, (int)hitbox->width, (int)hitbox->height, color);
                } else {
                    // Draw texture
                    DrawTexture(sprite->texture, (int)pos->x, (int)renderY, color);
                }
            }
        }
    }

    static void UnloadTextures(World& world) {
        for (Entity e : world.GetEntities()) {
            Sprite* sprite = world.GetSprite(e);
            if (sprite && sprite->loaded) {
                UnloadTexture(sprite->texture);
                sprite->loaded = false;
            }

            BuildingInterior* interior = world.GetBuildingInterior(e);
            if (interior && interior->texturesLoaded) {
                if (!interior->guiBackgroundPath.empty()) {
                    UnloadTexture(interior->guiBackgroundTexture);
                }
                if (!interior->gameBackgroundPath.empty()) {
                    UnloadTexture(interior->gameBackgroundTexture);
                }
                interior->texturesLoaded = false;
            }
        }
    }
};

// Interaction System - handles building entry/exit and speech bubbles
class InteractionSystem {
public:
    static void Update(World& world, SceneManager& sceneManager, int screenWidth, int screenHeight) {
        // Find player
        Entity playerEntity = -1;
        for (Entity e : world.GetEntities()) {
            if (world.HasPlayer(e)) {
                playerEntity = e;
                break;
            }
        }

        if (playerEntity == -1) return;

        int currentScene = sceneManager.GetCurrentScene();

        // Handle Z key - exit building
        if (IsKeyPressed(KEY_Z) && currentScene != SCENE_MAIN) {
            sceneManager.ExitToMain(world, playerEntity);
            return;
        }

        // Handle X key - enter building (only if speech bubble is active)
        if (IsKeyPressed(KEY_X)) {
            SpeechBubble* bubble = world.GetSpeechBubble(playerEntity);
            if (!bubble || !bubble->active) return;

            Hitbox* playerHitbox = world.GetHitbox(playerEntity);
            if (!playerHitbox) return;

            // Find which building the player is near (check interaction zones)
            for (Entity e : world.GetEntities()) {
                Scene* scene = world.GetScene(e);
                if (!scene || scene->sceneId != currentScene) continue;

                Building* building = world.GetBuilding(e);
                InteractionZone* interactionZone = world.GetInteractionZone(e);
                if (!building || !interactionZone) continue;

                if (CheckCollisionRecs(playerHitbox->bounds, interactionZone->bounds)) {
                    // Enter this building
                    float spawnX = screenWidth / 2.0f - 32.0f;  // Center horizontally
                    float spawnY = screenHeight * 0.75f - 32.0f; // 1/4 from bottom
                    sceneManager.EnterBuilding(world, playerEntity, building->interiorSceneId, spawnX, spawnY);
                    break;
                }
            }
        }
    }
};

// Text Render System - renders speech bubbles and text
class TextRenderSystem {
public:
    static void Render(World& world, int currentSceneId) {
        for (Entity e : world.GetEntities()) {
            SpeechBubble* bubble = world.GetSpeechBubble(e);
            Position* pos = world.GetPosition(e);
            if (!bubble || !bubble->active || !pos) continue;

            // Draw speech bubble sprite
            Sprite* bubbleSprite = world.GetSprite(e);
            if (bubbleSprite && bubbleSprite->loaded && !bubbleSprite->texturePath.empty()) {
                // Find speech bubble texture (we'll need to load it separately)
                // For now, draw at position + offset
                float bubbleX = pos->x + bubble->offsetX;
                float bubbleY = pos->y + bubble->offsetY;

                DrawTexture(bubbleSprite->texture, (int)bubbleX, (int)bubbleY, WHITE);

                // Draw text on top of bubble
                DrawText(bubble->text.c_str(), (int)(bubbleX + 5), (int)(bubbleY + 5), 10, BLACK);
            }
        }
    }
};

// GUI System - renders building interior GUIs
class GUISystem {
public:
    static void RenderPomodoroGUI(PomodoroTimer& timer, int guiHeight) {
        // Background is already drawn by RenderSystem

        // Center column layout
        int centerX = 400;
        int startY = 50;

        // Title
        DrawText("Pomodoro Timer", centerX - 100, startY, 30, BLACK);

        // Cycle selector
        startY += 60;
        DrawText("Cycles:", centerX - 150, startY + 8, 20, BLACK);

        if (GuiButton(Rectangle{(float)(centerX - 50), (float)startY, 30, 30}, "-")) {
            if (timer.cycleCount > 1 && !timer.isRunning) {
                timer.cycleCount--;
            }
        }

        char cycleText[16];
        sprintf(cycleText, "%d", timer.cycleCount);
        DrawText(cycleText, centerX, startY + 5, 20, BLACK);

        if (GuiButton(Rectangle{(float)(centerX + 40), (float)startY, 30, 30}, "+")) {
            if (timer.cycleCount < PomodoroTimer::MAX_CYCLES && !timer.isRunning) {
                timer.cycleCount++;
            }
        }

        // Start/Stop button
        startY += 60;
        if (!timer.isRunning) {
            if (GuiButton(Rectangle{(float)(centerX - 75), (float)startY, 150, 40}, "Start")) {
                timer.Start();
            }
        }

        // Reset button
        startY += 60;
        if (GuiButton(Rectangle{(float)(centerX - 75), (float)startY, 150, 40}, "Reset")) {
            timer.Reset();
        }

        // Timer display
        startY += 80;
        if (timer.isRunning || timer.state != PomodoroState::Idle) {
            // State display
            const char* stateText = "Idle";
            Color stateColor = GRAY;
            if (timer.state == PomodoroState::Study) {
                stateText = "STUDY TIME";
                stateColor = DARKGREEN;
            } else if (timer.state == PomodoroState::Break) {
                stateText = "BREAK TIME";
                stateColor = DARKBLUE;
            }

            DrawText(stateText, centerX - 80, startY, 24, stateColor);

            // Cycle progress
            startY += 40;
            char cycleProgress[64];
            sprintf(cycleProgress, "Cycle %d / %d", timer.currentCycle, timer.cycleCount);
            DrawText(cycleProgress, centerX - 60, startY, 18, BLACK);

            // Total elapsed time
            startY += 40;
            int totalElapsed = timer.GetTotalElapsed();
            int hours = totalElapsed / 3600;
            int minutes = (totalElapsed % 3600) / 60;
            int seconds = totalElapsed % 60;

            char timeText[32];
            sprintf(timeText, "%02d:%02d:%02d", hours, minutes, seconds);
            DrawText("Elapsed:", centerX - 100, startY, 20, BLACK);
            DrawText(timeText, centerX, startY, 20, DARKGRAY);
        }
    }

    static void Render(World& world, int currentSceneId, PomodoroTimer& pomodoroTimer, int guiHeight) {
        // Find building interior for current scene
        for (Entity e : world.GetEntities()) {
            Scene* scene = world.GetScene(e);
            if (!scene || scene->sceneId != currentSceneId) continue;

            BuildingInterior* interior = world.GetBuildingInterior(e);
            if (!interior) continue;

            // Render GUI based on type
            if (interior->guiType == GUIType::Pomodoro) {
                RenderPomodoroGUI(pomodoroTimer, guiHeight);
            }
            break;
        }
    }
};
