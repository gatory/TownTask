#include <raylib.h>
#include <iostream>

// Screen dimensions
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int TARGET_FPS = 60;

// Simple character structure
struct Character {
    Vector2 position;
    Vector2 velocity;
    float speed;
    Color color;
    
    Character() : position({400, 300}), velocity({0, 0}), speed(200.0f), color(BLUE) {}
};

// Simple building structure
struct Building {
    Vector2 position;
    Vector2 size;
    Color color;
    const char* name;
    bool isNear;
    
    Building(Vector2 pos, Vector2 sz, Color col, const char* nm) 
        : position(pos), size(sz), color(col), name(nm), isNear(false) {}
};

// Check if character is near a building
bool isCharacterNearBuilding(const Character& character, const Building& building, float threshold = 50.0f) {
    Vector2 buildingCenter = {
        building.position.x + building.size.x / 2,
        building.position.y + building.size.y / 2
    };
    
    Vector2 characterCenter = {
        character.position.x + 16,
        character.position.y + 16
    };
    
    float distance = sqrt(pow(characterCenter.x - buildingCenter.x, 2) + 
                         pow(characterCenter.y - buildingCenter.y, 2));
    
    return distance <= threshold;
}

int main() {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TaskTown - Gamified Productivity");
    SetTargetFPS(TARGET_FPS);
    
    std::cout << "TaskTown initialized successfully!" << std::endl;
    std::cout << "Raylib version: " << RAYLIB_VERSION << std::endl;
    
    // Create character
    Character player;
    
    // Create some buildings
    Building buildings[] = {
        {{200, 200}, {96, 128}, ORANGE, "Coffee Shop"},
        {{450, 350}, {128, 96}, BLUE, "Home"},
        {{600, 150}, {80, 96}, YELLOW, "Bulletin Board"},
        {{750, 300}, {120, 100}, PURPLE, "Library"},
        {{150, 500}, {110, 90}, RED, "Gym"}
    };
    int buildingCount = 5;
    
    // Game state
    bool showInteractionPrompt = false;
    const char* nearestBuildingName = "";
    
    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Handle input
        Vector2 movement = {0, 0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) movement.y -= 1;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) movement.y += 1;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movement.x -= 1;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movement.x += 1;
        
        // Normalize diagonal movement
        if (movement.x != 0 && movement.y != 0) {
            movement.x *= 0.707f; // 1/sqrt(2)
            movement.y *= 0.707f;
        }
        
        // Update character position
        player.velocity.x = movement.x * player.speed;
        player.velocity.y = movement.y * player.speed;
        
        Vector2 newPosition = {
            player.position.x + player.velocity.x * deltaTime,
            player.position.y + player.velocity.y * deltaTime
        };
        
        // Simple boundary checking
        if (newPosition.x >= 0 && newPosition.x <= SCREEN_WIDTH - 32) {
            player.position.x = newPosition.x;
        }
        if (newPosition.y >= 0 && newPosition.y <= SCREEN_HEIGHT - 32) {
            player.position.y = newPosition.y;
        }
        
        // Change character color based on movement
        if (movement.x != 0 || movement.y != 0) {
            player.color = SKYBLUE;
        } else {
            player.color = BLUE;
        }
        
        // Check for building interactions
        showInteractionPrompt = false;
        for (int i = 0; i < buildingCount; i++) {
            buildings[i].isNear = isCharacterNearBuilding(player, buildings[i]);
            if (buildings[i].isNear) {
                showInteractionPrompt = true;
                nearestBuildingName = buildings[i].name;
                break;
            }
        }
        
        // Handle interaction
        if (showInteractionPrompt && IsKeyPressed(KEY_E)) {
            std::cout << "Entering " << nearestBuildingName << "!" << std::endl;
            // Here we would switch to the appropriate screen
            // For now, just show a message
        }
        
        // Draw
        BeginDrawing();
        ClearBackground(DARKGREEN);
        
        // Draw grass pattern background
        const int tileSize = 64;
        for (int x = 0; x < SCREEN_WIDTH; x += tileSize) {
            for (int y = 0; y < SCREEN_HEIGHT; y += tileSize) {
                Color grassColor = ((x + y) / tileSize) % 2 == 0 ? DARKGREEN : GREEN;
                DrawRectangle(x, y, tileSize, tileSize, grassColor);
            }
        }
        
        // Draw buildings
        for (int i = 0; i < buildingCount; i++) {
            Building& building = buildings[i];
            
            // Highlight building if character is near
            Color buildingColor = building.isNear ? Fade(WHITE, 0.8f) : building.color;
            
            // Draw building
            DrawRectangle((int)building.position.x, (int)building.position.y, 
                         (int)building.size.x, (int)building.size.y, buildingColor);
            
            // Draw building outline
            Color outlineColor = building.isNear ? YELLOW : BLACK;
            DrawRectangleLines((int)building.position.x, (int)building.position.y, 
                              (int)building.size.x, (int)building.size.y, outlineColor);
            
            // Draw building name
            int textWidth = MeasureText(building.name, 12);
            DrawText(building.name, 
                    (int)(building.position.x + building.size.x/2 - textWidth/2), 
                    (int)(building.position.y - 16), 
                    12, WHITE);
        }
        
        // Draw character
        DrawCircle((int)(player.position.x + 16), (int)(player.position.y + 16), 12, player.color);
        DrawCircleLines((int)(player.position.x + 16), (int)(player.position.y + 16), 12, BLACK);
        
        // Draw interaction prompt
        if (showInteractionPrompt) {
            const char* promptText = "Press E to enter";
            int promptWidth = MeasureText(promptText, 16);
            int promptX = (int)(player.position.x + 16 - promptWidth/2);
            int promptY = (int)(player.position.y - 20);
            
            // Background for prompt
            DrawRectangle(promptX - 4, promptY - 2, promptWidth + 8, 20, Fade(BLACK, 0.7f));
            DrawText(promptText, promptX, promptY, 16, WHITE);
        }
        
        // Draw UI
        DrawText("TaskTown - Use WASD or Arrow Keys to Move", 10, 10, 20, WHITE);
        DrawText("Press E to enter buildings when nearby", 10, 35, 16, LIGHTGRAY);
        DrawText("Press ESC to exit", 10, SCREEN_HEIGHT - 30, 16, LIGHTGRAY);
        
        // Draw character position
        char posText[100];
        snprintf(posText, sizeof(posText), "Character Position: (%.0f, %.0f)", player.position.x, player.position.y);
        DrawText(posText, 10, 60, 16, WHITE);
        
        // Draw current building info
        if (showInteractionPrompt) {
            char buildingText[100];
            snprintf(buildingText, sizeof(buildingText), "Near: %s", nearestBuildingName);
            DrawText(buildingText, 10, 85, 16, YELLOW);
        }
        
        EndDrawing();
    }
    
    // Cleanup
    CloseWindow();
    
    return 0;
}