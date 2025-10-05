#include "simple_integrated_app.h"
#include "../assets/asset_manager.h"
#include "../ui/feedback/notification_system.h"
#include "performance/performance_monitor.h"
#include <iostream>
#include <functional>
#include <chrono>

SimpleIntegratedApp::SimpleIntegratedApp() {
    // Constructor
}

SimpleIntegratedApp::~SimpleIntegratedApp() {
    shutdown();
}

bool SimpleIntegratedApp::initialize() {
    if (initialized) {
        return true;
    }
    
    std::cout << "SimpleIntegratedApp: Initializing..." << std::endl;
    
    // Get asset manager instance
    assetManager = &AssetManager::getInstance();
    
    // Initialize notification system
    notificationSystem = std::make_unique<NotificationSystem>();
    notificationSystem->setNotificationPosition({10, 10});
    
    // Initialize performance monitor
    performanceMonitor = std::make_unique<PerformanceMonitor>();
    performanceMonitor->setTargetFPS(60.0f);
    performanceMonitor->setDisplayPosition({static_cast<float>(GetScreenWidth() - 320), 10.0f});
    g_performanceMonitor = performanceMonitor.get(); // Set global reference
    
    initialized = true;
    std::cout << "SimpleIntegratedApp: Initialization complete" << std::endl;
    
    // Show welcome notification
    showSuccess("TaskTown initialized successfully!");
    
    return true;
}

void SimpleIntegratedApp::shutdown() {
    if (!initialized) {
        return;
    }
    
    std::cout << "SimpleIntegratedApp: Shutting down..." << std::endl;
    
    // Cleanup systems
    g_performanceMonitor = nullptr; // Clear global reference
    performanceMonitor.reset();
    notificationSystem.reset();
    
    initialized = false;
    std::cout << "SimpleIntegratedApp: Shutdown complete" << std::endl;
}

void SimpleIntegratedApp::update(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Performance monitoring - start update timer
    auto updateTimer = performanceMonitor ? performanceMonitor->createTimer("Update") : nullptr;
    
    // Update notification system first (handles input)
    if (notificationSystem) {
        notificationSystem->update(deltaTime);
        if (performanceMonitor) {
            performanceMonitor->recordNotificationCount(
                notificationSystem->hasActiveNotifications() ? 1 : 0
            );
        }
    }
    
    // Handle input (only if no dialogs are active)
    auto inputTimer = performanceMonitor ? performanceMonitor->createTimer("Input") : nullptr;
    bool inputHandled = false;
    if (notificationSystem && notificationSystem->hasActiveDialogs()) {
        inputHandled = notificationSystem->handleInput();
    }
    
    if (!inputHandled) {
        handleInput();
    }
    inputTimer.reset(); // Stop input timer
    
    if (!inBuilding) {
        // Update character movement
        updateCharacterMovement(deltaTime);
        
        // Check building interactions
        checkBuildingInteractions();
    }
    
    // Auto-save timer
    autoSaveTimer += deltaTime;
    if (autoSaveTimer >= AUTO_SAVE_INTERVAL) {
        saveGameWithFeedback();
        autoSaveTimer = 0.0f;
    }
    
    // Update performance monitor
    if (performanceMonitor) {
        performanceMonitor->update(deltaTime);
        performanceMonitor->recordFrameTime(deltaTime);
        
        // Record texture count from asset manager
        if (assetManager) {
            // This would need to be implemented in AssetManager
            performanceMonitor->recordTextureCount(5); // Placeholder
        }
    }
    
    updateTimer.reset(); // Stop update timer
}

void SimpleIntegratedApp::render() {
    if (!initialized) {
        return;
    }
    
    // Performance monitoring - start render timer
    auto renderTimer = performanceMonitor ? performanceMonitor->createTimer("Render") : nullptr;
    
    if (inBuilding) {
        renderBuildingScreen();
    } else {
        renderTownScreen();
    }
    
    renderUI();
    
    // Render notification system on top
    if (notificationSystem) {
        notificationSystem->render();
    }
    
    // Render performance monitor last
    if (performanceMonitor) {
        performanceMonitor->render();
    }
    
    renderTimer.reset(); // Stop render timer
}

bool SimpleIntegratedApp::handleInput() {
    // Global input handling
    if (IsKeyPressed(KEY_F1)) {
        toggleDebugMode();
        return true;
    }
    
    if (IsKeyPressed(KEY_F5)) {
        saveGameWithFeedback();
        return true;
    }
    
    if (IsKeyPressed(KEY_F9)) {
        loadGameWithFeedback();
        return true;
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (inBuilding) {
            exitCurrentBuilding();
        } else {
            // ESC in town - ask for confirmation to exit
            showConfirmation("Exit Game", "Are you sure you want to exit TaskTown?", 
                           [this]() { requestExit(); });
        }
        return true;
    }
    
    // Performance monitor toggle (F11)
    if (IsKeyPressed(KEY_F11)) {
        togglePerformanceMonitor();
        return true;
    }
    
    // Test error dialog (F12)
    if (IsKeyPressed(KEY_F12)) {
        showError("Test error occurred", "This is a detailed error message for testing purposes. The error includes technical details that might help with debugging.");
        return true;
    }
    
    return false;
}

void SimpleIntegratedApp::updateCharacterMovement(float deltaTime) {
    Vector2 movement = {0, 0};
    
    // Handle movement input
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) movement.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) movement.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movement.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movement.x += 1;
    
    // Normalize diagonal movement
    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.707f;
        movement.y *= 0.707f;
    }
    
    // Update position
    Vector2 newPos = {
        characterPosition.x + movement.x * characterSpeed * deltaTime,
        characterPosition.y + movement.y * characterSpeed * deltaTime
    };
    
    // Boundary checking
    if (newPos.x >= 0 && newPos.x <= 1024 - 32) {
        characterPosition.x = newPos.x;
    }
    if (newPos.y >= 0 && newPos.y <= 768 - 32) {
        characterPosition.y = newPos.y;
    }
}

void SimpleIntegratedApp::checkBuildingInteractions() {
    if (!assetManager) return;
    
    const auto& buildings = assetManager->getBuildings();
    
    for (const auto& building : buildings) {
        Vector2 buildingCenter = {
            building.position.x + building.size.x / 2,
            building.position.y + building.size.y / 2
        };
        
        Vector2 characterCenter = {
            characterPosition.x + 16,
            characterPosition.y + 16
        };
        
        float distance = sqrt(pow(characterCenter.x - buildingCenter.x, 2) + 
                            pow(characterCenter.y - buildingCenter.y, 2));
        
        if (distance <= building.interactionRadius) {
            if (IsKeyPressed(KEY_E)) {
                enterBuilding(building.id);
                return;
            }
        }
    }
}

void SimpleIntegratedApp::renderTownScreen() {
    ClearBackground(DARKGREEN);
    
    if (!assetManager) return;
    
    // Draw grass pattern background
    const int tileSize = assetManager->getTileSize();
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color grassColor = ((x + y) / tileSize) % 2 == 0 ? DARKGREEN : GREEN;
            DrawRectangle(x, y, tileSize, tileSize, grassColor);
        }
    }
    
    // Draw buildings
    const auto& buildings = assetManager->getBuildings();
    
    for (const auto& building : buildings) {
        // Check if character is near
        Vector2 buildingCenter = {
            building.position.x + building.size.x / 2,
            building.position.y + building.size.y / 2
        };
        
        Vector2 characterCenter = {
            characterPosition.x + 16,
            characterPosition.y + 16
        };
        
        float distance = sqrt(pow(characterCenter.x - buildingCenter.x, 2) + 
                            pow(characterCenter.y - buildingCenter.y, 2));
        bool isNear = distance <= building.interactionRadius;
        
        // Draw building
        if (building.textureLoaded) {
            Color tint = isNear ? Fade(WHITE, 0.8f) : WHITE;
            DrawTexture(building.texture, 
                       (int)building.position.x, 
                       (int)building.position.y, 
                       tint);
        } else {
            Color buildingColor = isNear ? Fade(WHITE, 0.8f) : building.color;
            DrawRectangle((int)building.position.x, (int)building.position.y, 
                         (int)building.size.x, (int)building.size.y, buildingColor);
        }
        
        // Draw outline
        Color outlineColor = isNear ? YELLOW : BLACK;
        DrawRectangleLines((int)building.position.x, (int)building.position.y, 
                          (int)building.size.x, (int)building.size.y, outlineColor);
        
        // Draw name
        int textWidth = MeasureText(building.name.c_str(), 12);
        DrawText(building.name.c_str(), 
                (int)(building.position.x + building.size.x/2 - textWidth/2), 
                (int)(building.position.y - 16), 
                12, WHITE);
        
        // Draw interaction prompt
        if (isNear) {
            const char* promptText = "Press E to enter";
            int promptWidth = MeasureText(promptText, 16);
            int promptX = (int)(characterPosition.x + 16 - promptWidth/2);
            int promptY = (int)(characterPosition.y - 20);
            
            DrawRectangle(promptX - 4, promptY - 2, promptWidth + 8, 20, Fade(BLACK, 0.7f));
            DrawText(promptText, promptX, promptY, 16, WHITE);
        }
    }
    
    // Draw character
    DrawCircle((int)(characterPosition.x + 16), (int)(characterPosition.y + 16), 12, BLUE);
    DrawCircleLines((int)(characterPosition.x + 16), (int)(characterPosition.y + 16), 12, BLACK);
}

void SimpleIntegratedApp::renderBuildingScreen() {
    ClearBackground(DARKBLUE);
    
    // Find current building info
    std::string buildingName = "Unknown Building";
    std::string buildingType = "unknown";
    
    if (assetManager) {
        const auto& buildings = assetManager->getBuildings();
        for (const auto& building : buildings) {
            if (building.id == currentBuildingId) {
                buildingName = building.name;
                buildingType = building.type;
                break;
            }
        }
    }
    
    // Draw building interior header
    DrawText(("Inside " + buildingName).c_str(), 50, 50, 32, WHITE);
    
    // Draw a separator line
    DrawLine(50, 90, GetScreenWidth() - 50, 90, LIGHTGRAY);
    
    // Building-specific content
    if (buildingType == "pomodoro") {
        DrawText("Coffee Shop - Focus & Productivity", 50, 120, 24, ORANGE);
        DrawText("This is where you'd start pomodoro sessions", 50, 160, 16, LIGHTGRAY);
        DrawText("Press SPACE to start a focus session", 50, 190, 16, YELLOW);
        
        if (IsKeyPressed(KEY_SPACE)) {
            std::cout << "Started pomodoro session!" << std::endl;
            showAchievement("Pomodoro session started! Stay focused!");
        }
    } else if (buildingType == "tasks") {
        DrawText("Bulletin Board - Task Management", 50, 120, 24, YELLOW);
        DrawText("This is where you'd manage your tasks", 50, 160, 16, LIGHTGRAY);
        DrawText("Press T to add a new task", 50, 190, 16, YELLOW);
        
        if (IsKeyPressed(KEY_T)) {
            std::cout << "Added new task!" << std::endl;
            showSuccess("New task added to your list!");
        }
    } else if (buildingType == "notes") {
        DrawText("Library - Notes & Learning", 50, 120, 24, PURPLE);
        DrawText("This is where you'd take and organize notes", 50, 160, 16, LIGHTGRAY);
        DrawText("Press N to create a new note", 50, 190, 16, YELLOW);
        
        if (IsKeyPressed(KEY_N)) {
            std::cout << "Created new note!" << std::endl;
            showSuccess("New note created in your library!");
        }
    } else if (buildingType == "habits") {
        DrawText("Gym - Habit Tracking", 50, 120, 24, RED);
        DrawText("This is where you'd track your habits", 50, 160, 16, LIGHTGRAY);
        DrawText("Press H to log a habit", 50, 190, 16, YELLOW);
        
        if (IsKeyPressed(KEY_H)) {
            std::cout << "Logged habit!" << std::endl;
            showAchievement("Habit logged! Keep up the great work!");
        }
    } else {
        DrawText("Generic Building", 50, 120, 24, WHITE);
        DrawText("This building doesn't have specific functionality yet", 50, 160, 16, LIGHTGRAY);
    }
}

void SimpleIntegratedApp::renderUI() {
    if (!inBuilding) {
        // Town UI - draw at top
        DrawText("TaskTown - Integrated Systems Demo", 10, 10, 20, WHITE);
        DrawText("Use WASD to move, E to enter buildings", 10, 35, 16, LIGHTGRAY);
        DrawText("F1 debug, F5 save, F9 load, F11 performance", 10, 60, 16, LIGHTGRAY);
        
        if (debugMode) {
            char debugText[200];
            snprintf(debugText, sizeof(debugText), 
                    "Debug: Pos(%.0f,%.0f) InBuilding:%s Building:%s", 
                    characterPosition.x, characterPosition.y,
                    inBuilding ? "Yes" : "No",
                    currentBuildingId.c_str());
            DrawText(debugText, 10, 85, 16, YELLOW);
        }
    } else {
        // Building UI - draw at bottom to avoid overlap
        int screenHeight = GetScreenHeight();
        DrawText("F1 debug, F5 save, F9 load, F11 performance", 10, screenHeight - 60, 16, LIGHTGRAY);
        DrawText("ESC to exit building", 10, screenHeight - 40, 16, YELLOW);
        
        if (debugMode) {
            char debugText[200];
            snprintf(debugText, sizeof(debugText), 
                    "Debug: InBuilding:%s Building:%s", 
                    inBuilding ? "Yes" : "No",
                    currentBuildingId.c_str());
            DrawText(debugText, 10, screenHeight - 80, 16, YELLOW);
        }
    }
}

void SimpleIntegratedApp::enterBuilding(const std::string& buildingId) {
    currentBuildingId = buildingId;
    inBuilding = true;
    
    std::cout << "Entered building: " << buildingId << std::endl;
    
    // Show building-specific welcome message
    if (assetManager) {
        const auto& buildings = assetManager->getBuildings();
        for (const auto& building : buildings) {
            if (building.id == buildingId) {
                showSuccess("Entered " + building.name);
                
                // Show building-specific tips
                if (building.type == "pomodoro") {
                    showSuccess("Focus time! Press SPACE to start a pomodoro session");
                } else if (building.type == "tasks") {
                    showSuccess("Time to organize! Press T to add a new task");
                } else if (building.type == "notes") {
                    showSuccess("Knowledge awaits! Press N to create a new note");
                } else if (building.type == "habits") {
                    showSuccess("Build good habits! Press H to log progress");
                }
                break;
            }
        }
    }
}

void SimpleIntegratedApp::exitCurrentBuilding() {
    if (inBuilding) {
        std::cout << "Exited building: " << currentBuildingId << std::endl;
        showSuccess("Returned to town");
        currentBuildingId.clear();
        inBuilding = false;
    }
}

void SimpleIntegratedApp::toggleDebugMode() {
    debugMode = !debugMode;
    std::cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << std::endl;
    
    if (notificationSystem) {
        if (debugMode) {
            showSuccess("Debug mode enabled");
        } else {
            showSuccess("Debug mode disabled");
        }
    }
}

// Error handling and feedback methods
void SimpleIntegratedApp::showError(const std::string& message, const std::string& details) {
    if (notificationSystem) {
        if (details.empty()) {
            notificationSystem->showError(message);
        } else {
            notificationSystem->showErrorDialog("Error", message, details);
        }
    }
    std::cerr << "Error: " << message << std::endl;
    if (!details.empty()) {
        std::cerr << "Details: " << details << std::endl;
    }
}

void SimpleIntegratedApp::showSuccess(const std::string& message) {
    if (notificationSystem) {
        notificationSystem->showSuccess(message);
    }
    std::cout << "Success: " << message << std::endl;
}

void SimpleIntegratedApp::showWarning(const std::string& message) {
    if (notificationSystem) {
        notificationSystem->showWarning(message);
    }
    std::cout << "Warning: " << message << std::endl;
}

void SimpleIntegratedApp::showAchievement(const std::string& message) {
    if (notificationSystem) {
        notificationSystem->showAchievement(message);
    }
    std::cout << "Achievement: " << message << std::endl;
}

void SimpleIntegratedApp::showProgress(const std::string& message, float progress) {
    if (notificationSystem) {
        notificationSystem->showProgress(message, progress);
    }
}

void SimpleIntegratedApp::hideProgress() {
    if (notificationSystem) {
        notificationSystem->hideProgress();
    }
}

void SimpleIntegratedApp::showConfirmation(const std::string& title, const std::string& message, 
                                         std::function<void()> onConfirm, std::function<void()> onCancel) {
    if (notificationSystem) {
        notificationSystem->showConfirmation(title, message, onConfirm, onCancel);
    }
}

bool SimpleIntegratedApp::saveGameWithFeedback() {
    try {
        showProgress("Saving game...", 0.0f);
        
        // Simulate save process with progress updates
        for (int i = 0; i <= 100; i += 25) {
            showProgress("Saving game...", i / 100.0f);
            // In a real implementation, this would be actual save steps
        }
        
        hideProgress();
        showSuccess("Game saved successfully!");
        return true;
        
    } catch (const std::exception& e) {
        hideProgress();
        showError("Failed to save game", e.what());
        return false;
    }
}

bool SimpleIntegratedApp::loadGameWithFeedback() {
    try {
        showProgress("Loading game...", 0.0f);
        
        // Simulate load process with progress updates
        for (int i = 0; i <= 100; i += 33) {
            showProgress("Loading game...", i / 100.0f);
            // In a real implementation, this would be actual load steps
        }
        
        hideProgress();
        showSuccess("Game loaded successfully!");
        return true;
        
    } catch (const std::exception& e) {
        hideProgress();
        showError("Failed to load game", e.what());
        return false;
    }
}

void SimpleIntegratedApp::togglePerformanceMonitor() {
    if (performanceMonitor) {
        performanceMonitor->toggleDisplay();
        bool isVisible = performanceMonitor->isDisplayVisible();
        showSuccess(isVisible ? "Performance monitor enabled" : "Performance monitor disabled");
    }
}

void SimpleIntegratedApp::optimizePerformance() {
    if (performanceMonitor) {
        showSuccess("Running performance optimizations...");
        
        performanceMonitor->optimizeTextureUsage();
        performanceMonitor->optimizeMemoryUsage();
        performanceMonitor->optimizeRenderingPipeline();
        
        // Clear notification history to free memory
        if (notificationSystem) {
            notificationSystem->clearAllNotifications();
        }
        
        showSuccess("Performance optimization complete!");
    }
}