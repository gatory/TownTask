#include <raylib.h>
#include <iostream>
#include <memory>

// Core systems
#include "core/simple_integrated_app.h"
#include "assets/asset_manager.h"

// Screen dimensions
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int TARGET_FPS = 60;

int main() {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TaskTown - Integrated Version");
    SetTargetFPS(TARGET_FPS);
    
    // Disable ESC key from closing window - we'll handle it manually
    SetExitKey(KEY_NULL);
    
    std::cout << "TaskTown Integrated Version Starting..." << std::endl;
    std::cout << "Raylib version: " << RAYLIB_VERSION << std::endl;
    
    try {
        // Initialize asset manager first
        AssetManager& assetManager = AssetManager::getInstance();
        if (!assetManager.initialize()) {
            std::cerr << "Failed to initialize asset manager!" << std::endl;
            CloseWindow();
            return -1;
        }
        
        // Create and initialize simple integrated application
        auto app = std::make_unique<SimpleIntegratedApp>();
        if (!app->initialize()) {
            std::cerr << "Failed to initialize simple integrated application!" << std::endl;
            assetManager.cleanup();
            CloseWindow();
            return -1;
        }
        
        std::cout << "All systems initialized successfully!" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move character" << std::endl;
        std::cout << "  E - Enter buildings" << std::endl;
        std::cout << "  ESC - Exit buildings" << std::endl;
        std::cout << "  F1 - Toggle debug mode" << std::endl;
        std::cout << "  F5 - Save game" << std::endl;
        std::cout << "  F9 - Load game" << std::endl;
        
        // Main game loop
        while (!WindowShouldClose() && !app->shouldClose()) {
            float deltaTime = GetFrameTime();
            
            // Update application
            app->update(deltaTime);
            
            // Render application
            BeginDrawing();
            app->render();
            EndDrawing();
        }
        
        // Cleanup
        std::cout << "Shutting down..." << std::endl;
        app.reset();
        assetManager.cleanup();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        CloseWindow();
        return -1;
    }
    
    CloseWindow();
    std::cout << "TaskTown shutdown complete." << std::endl;
    return 0;
}