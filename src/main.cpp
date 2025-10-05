#include <stdlib.h>
#include <time.h>

#include "raylib.h"
#include "World.h"
#include "Systems.h"
#include "SceneManager.h"
#include "GameState.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GUI_HEIGHT 300
#define GAME_HEIGHT 300

// Speech bubble texture (loaded once, shared)
Texture2D speechBubbleTexture;

int main() {
	srand(time(0));

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Town Game");

	// Create the ECS world and scene manager
	World world;
	SceneManager sceneManager;
	PomodoroTimer pomodoroTimer;

	// Load speech bubble texture
	speechBubbleTexture = LoadTexture("assets/speech_v2_32x24.png");

	// Create main scene background (green)
	Entity mainBackground = world.CreateEntity();
	world.AddPosition(mainBackground, 0, 0);
	world.AddSprite(mainBackground, "", SCREEN_WIDTH, SCREEN_HEIGHT, GREEN);
	world.AddScene(mainBackground, SCENE_MAIN);

	// Create Pomodoro building interior (holds the interior configuration)
	Entity pomodoroInterior = world.CreateEntity();
	world.AddBuildingInterior(pomodoroInterior, GUIType::Pomodoro, "", LIGHTGRAY, "assets/shop-800x449.png", WHITE);
	world.AddScene(pomodoroInterior, SCENE_SHOP_INTERIOR);

	// Create cat entity (player)
	Entity cat = world.CreateEntity();
	world.AddPosition(cat, 0, 0);
	world.AddSprite(cat, "assets/Idle.png", 64, 64, WHITE);
	world.AddHitbox(cat, 64, 64);
	world.AddAnimation(cat, 10, 0.2f, 32, 32);
	world.AddPlayerInput(cat, 200.0f);
	world.AddPlayer(cat);
	world.AddSpeechBubble(cat, "", 0, -32); // Speech bubble appears above cat

	// Create Pomodoro building in main scene
	Entity pomodoroBuilding = world.CreateEntity();
	world.AddPosition(pomodoroBuilding, SCREEN_WIDTH - 200, 100);
	world.AddSprite(pomodoroBuilding, "", 100, 100, BROWN); // Empty path = draw as rectangle
	world.AddHitbox(pomodoroBuilding, 100, 100); // Solid barrier hitbox
	world.AddInteractionZone(pomodoroBuilding, 120, 120, -10, -10); // Larger interaction zone around building
	world.AddBuilding(pomodoroBuilding, "Pomodoro", SCENE_SHOP_INTERIOR);
	world.AddScene(pomodoroBuilding, SCENE_MAIN);

	// Load all textures
	RenderSystem::LoadTextures(world);

	// Main game loop
	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime();
		int currentScene = sceneManager.GetCurrentScene();

		// Update pomodoro timer
		pomodoroTimer.Update();

		// Update systems
		AnimationSystem::Update(world, deltaTime);

		// Use player input in main scene, AI wander in interiors
		if (currentScene == SCENE_MAIN) {
			InputSystem::Update(world, deltaTime, SCREEN_WIDTH, SCREEN_HEIGHT, currentScene);
		} else {
			AIWanderSystem::Update(world, deltaTime, SCREEN_WIDTH, GAME_HEIGHT);
		}

		CollisionSystem::Update(world, currentScene);
		InteractionSystem::Update(world, sceneManager, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Render
		BeginDrawing();
		ClearBackground(RAYWHITE);

		// Render based on current scene
		if (currentScene == SCENE_MAIN) {
			ClearBackground(GREEN);
			RenderSystem::Render(world, currentScene, GUI_HEIGHT);

			// Render speech bubbles in main scene
			SpeechBubble* bubble = world.GetSpeechBubble(cat);
			Position* catPos = world.GetPosition(cat);
			if (bubble && bubble->active && catPos) {
				float bubbleX = catPos->x + bubble->offsetX;
				float bubbleY = catPos->y + bubble->offsetY;

				Rectangle source = {0, 0, (float)speechBubbleTexture.width, (float)speechBubbleTexture.height};
				Rectangle dest = {bubbleX, bubbleY, (float)speechBubbleTexture.width * 2, (float)speechBubbleTexture.height * 2};
				DrawTexturePro(speechBubbleTexture, source, dest, Vector2{0, 0}, 0, WHITE);

				std::string text = bubble->text;
				size_t newlinePos = text.find('\n');
				if (newlinePos != std::string::npos) {
					std::string line1 = text.substr(0, newlinePos);
					std::string line2 = text.substr(newlinePos + 1);
					DrawText(line1.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 8), 8, BLACK);
					DrawText(line2.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 20), 8, BLACK);
				} else {
					DrawText(text.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 8), 8, BLACK);
				}
			}
		} else {
			// Building interior with GUI
			RenderSystem::RenderBuildingInteriorBackgrounds(world, currentScene, GUI_HEIGHT, GAME_HEIGHT);
			GUISystem::Render(world, currentScene, pomodoroTimer, GUI_HEIGHT);
			RenderSystem::Render(world, currentScene, GUI_HEIGHT);
		}

		EndDrawing();
	}

	// Cleanup
	UnloadTexture(speechBubbleTexture);
	RenderSystem::UnloadTextures(world);
	CloseWindow();

	return 0;
}
