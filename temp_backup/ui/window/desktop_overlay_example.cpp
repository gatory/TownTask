// Example integration of desktop overlay functionality
// This file shows how to use the WindowManager and DesktopOverlayIntegration
// in a real application

#include "window_manager.h"
#include "desktop_overlay_integration.h"
#include "../../core/models/town_state.h"
#include <iostream>
#include <memory>

class DesktopOverlayExample {
public:
    DesktopOverlayExample() {
        // Initialize core systems
        townState = std::make_unique<TownState>();
        windowManager = std::make_shared<WindowManager>();
        desktopOverlay = std::make_unique<DesktopOverlayIntegration>(windowManager, *townState);
        
        setupCallbacks();
    }
    
    bool initialize() {
        // Set up window settings
        WindowSettings settings;
        settings.width = 1024;
        settings.height = 768;
        settings.title = "TaskTown - Desktop Overlay Example";
        settings.overlayAlpha = 0.8f;
        settings.clickThrough = true;
        settings.saveWindowState = true;
        
        // Initialize window manager
        if (!windowManager->initialize(settings)) {
            std::cerr << "Failed to initialize window manager" << std::endl;
            return false;
        }
        
        std::cout << "Desktop overlay example initialized successfully" << std::endl;
        return true;
    }
    
    void run() {
        std::cout << "Starting desktop overlay example..." << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  D - Toggle desktop overlay mode" << std::endl;
        std::cout << "  A - Adjust overlay alpha" << std::endl;
        std::cout << "  C - Toggle click-through" << std::endl;
        std::cout << "  B - Toggle building overlays" << std::endl;
        std::cout << "  ESC - Exit" << std::endl;
        
        while (!windowManager->shouldClose()) {
            handleInput();
            update();
            render();
        }
        
        shutdown();
    }
    
private:
    std::unique_ptr<TownState> townState;
    std::shared_ptr<WindowManager> windowManager;
    std::unique_ptr<DesktopOverlayIntegration> desktopOverlay;
    
    // Example state
    Vector2 characterPosition = {400, 300};
    float characterSpeed = 200.0f;
    
    void setupCallbacks() {
        // Set up desktop mode change callback
        desktopOverlay->setOnDesktopModeChanged([this](bool enabled) {
            std::cout << "Desktop mode " << (enabled ? "enabled" : "disabled") << std::endl;
            
            if (enabled) {
                std::cout << "Character is now on your desktop!" << std::endl;
                std::cout << "You can interact with other applications underneath." << std::endl;
            } else {
                std::cout << "Returned to windowed mode." << std::endl;
            }
        });
        
        // Set up character position change callback
        desktopOverlay->setOnCharacterPositionChanged([this](Vector2 screenPos) {
            // Character position changed on screen
            // Could trigger desktop pet behaviors here
        });
    }
    
    void handleInput() {
        // Toggle desktop overlay mode
        if (IsKeyPressed(KEY_D)) {
            desktopOverlay->toggleDesktopMode();
        }
        
        // Adjust overlay alpha
        if (IsKeyPressed(KEY_A)) {
            float currentAlpha = desktopOverlay->getDesktopModeAlpha();
            float newAlpha = (currentAlpha >= 1.0f) ? 0.3f : currentAlpha + 0.2f;
            desktopOverlay->setDesktopModeAlpha(newAlpha);
            std::cout << "Overlay alpha: " << newAlpha << std::endl;
        }
        
        // Toggle click-through
        if (IsKeyPressed(KEY_C)) {
            bool clickThrough = !windowManager->isClickThroughEnabled();
            windowManager->setClickThrough(clickThrough);
            std::cout << "Click-through " << (clickThrough ? "enabled" : "disabled") << std::endl;
        }
        
        // Toggle building overlays
        if (IsKeyPressed(KEY_B)) {
            bool overlays = !desktopOverlay->areBuildingOverlaysEnabled();
            desktopOverlay->enableBuildingOverlays(overlays);
            std::cout << "Building overlays " << (overlays ? "enabled" : "disabled") << std::endl;
        }
        
        // Character movement (WASD)
        Vector2 movement = {0, 0};
        if (IsKeyDown(KEY_W)) movement.y -= 1;
        if (IsKeyDown(KEY_S)) movement.y += 1;
        if (IsKeyDown(KEY_A)) movement.x -= 1;
        if (IsKeyDown(KEY_D)) movement.x += 1;
        
        // Normalize diagonal movement
        if (movement.x != 0 && movement.y != 0) {
            movement.x *= 0.707f;
            movement.y *= 0.707f;
        }
        
        // Update character position
        float deltaTime = GetFrameTime();
        characterPosition.x += movement.x * characterSpeed * deltaTime;
        characterPosition.y += movement.y * characterSpeed * deltaTime;
        
        // Keep character in bounds (simple boundary checking)
        Vector2 screenSize = windowManager->getScreenSize();
        characterPosition.x = std::max(0.0f, std::min(characterPosition.x, screenSize.x - 32));
        characterPosition.y = std::max(0.0f, std::min(characterPosition.y, screenSize.y - 32));
    }
    
    void update() {
        float deltaTime = GetFrameTime();
        
        // Update window manager
        windowManager->update();
        
        // Update desktop overlay integration
        desktopOverlay->update(deltaTime);
        
        // Update character position in desktop overlay
        desktopOverlay->updateCharacterPosition(characterPosition);
        
        // Update town state character position
        townState->getCharacter().setPosition({characterPosition.x, characterPosition.y});
    }
    
    void render() {
        windowManager->beginDrawing();
        windowManager->clearBackground(DARKGREEN);
        
        if (desktopOverlay->isDesktopModeEnabled()) {
            // Desktop overlay mode rendering
            renderDesktopMode();
        } else {
            // Normal windowed mode rendering
            renderWindowedMode();
        }
        
        // Render UI and overlays
        renderUI();
        
        windowManager->endDrawing();
    }
    
    void renderDesktopMode() {
        // In desktop mode, render minimal elements
        
        // Render building overlays
        desktopOverlay->renderBuildingOverlays();
        
        // Render character on desktop
        desktopOverlay->renderCharacterOnDesktop(characterPosition);
        
        // Render desktop mode indicator
        desktopOverlay->renderDesktopModeIndicator();
    }
    
    void renderWindowedMode() {
        // In windowed mode, render full game
        
        // Draw background
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGREEN);
        
        // Draw simple town buildings
        DrawRectangle(200, 200, 96, 128, ORANGE);  // Coffee shop
        DrawText("Coffee Shop", 210, 180, 12, WHITE);
        
        DrawRectangle(450, 350, 128, 96, BLUE);    // Home
        DrawText("Home", 490, 330, 12, WHITE);
        
        DrawRectangle(600, 150, 80, 96, YELLOW);   // Bulletin board
        DrawText("Tasks", 620, 130, 12, WHITE);
        
        // Draw character
        DrawCircleV({characterPosition.x + 16, characterPosition.y + 16}, 12, SKYBLUE);
        DrawCircleLinesV({characterPosition.x + 16, characterPosition.y + 16}, 12, WHITE);
    }
    
    void renderUI() {
        // Draw controls help
        const char* mode = desktopOverlay->isDesktopModeEnabled() ? "Desktop Overlay" : "Windowed";
        DrawText(TextFormat("Mode: %s", mode), 10, 10, 16, WHITE);
        
        if (desktopOverlay->isDesktopModeEnabled()) {
            DrawText("Desktop Mode Active - Character is on your desktop!", 10, 30, 14, YELLOW);
            DrawText(TextFormat("Alpha: %.1f", desktopOverlay->getDesktopModeAlpha()), 10, 50, 12, WHITE);
            DrawText(TextFormat("Click-through: %s", windowManager->isClickThroughEnabled() ? "ON" : "OFF"), 10, 70, 12, WHITE);
        } else {
            DrawText("Use WASD to move character", 10, 30, 12, LIGHTGRAY);
            DrawText("Press D to enable desktop overlay mode", 10, 50, 12, LIGHTGRAY);
        }
        
        // Draw character position
        DrawText(TextFormat("Character: (%.0f, %.0f)", characterPosition.x, characterPosition.y), 10, GetScreenHeight() - 30, 12, WHITE);
    }
    
    void shutdown() {
        std::cout << "Shutting down desktop overlay example..." << std::endl;
        
        // Save settings
        desktopOverlay->saveDesktopSettings();
        
        // Cleanup is handled by destructors
    }
};

// Example main function (not used in actual game)
/*
int main() {
    DesktopOverlayExample example;
    
    if (!example.initialize()) {
        return -1;
    }
    
    example.run();
    
    return 0;
}
*/