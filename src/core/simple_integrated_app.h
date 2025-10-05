#pragma once

#include <raylib.h>
#include <memory>
#include <string>
#include <functional>

// Forward declarations
class AssetManager;
class NotificationSystem;
class PerformanceMonitor;

// Simple integrated application that demonstrates system integration
class SimpleIntegratedApp {
public:
    SimpleIntegratedApp();
    ~SimpleIntegratedApp();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void render();
    
    // Input handling
    bool handleInput();
    
    // Building interactions
    void enterBuilding(const std::string& buildingId);
    void exitCurrentBuilding();
    
    // Debug and Performance
    void toggleDebugMode();
    bool isDebugMode() const { return debugMode; }
    void togglePerformanceMonitor();
    void optimizePerformance();
    
    // Application control
    bool shouldClose() const { return shouldExit; }
    void requestExit() { shouldExit = true; }
    
    // Error handling and feedback
    void showError(const std::string& message, const std::string& details = "");
    void showSuccess(const std::string& message);
    void showWarning(const std::string& message);
    void showAchievement(const std::string& message);
    void showProgress(const std::string& message, float progress = 0.0f);
    void hideProgress();
    void showConfirmation(const std::string& title, const std::string& message, 
                         std::function<void()> onConfirm, std::function<void()> onCancel = std::function<void()>());
    
    // Save/Load with error handling
    bool saveGameWithFeedback();
    bool loadGameWithFeedback();

private:
    // State
    bool initialized = false;
    bool debugMode = false;
    bool inBuilding = false;
    bool shouldExit = false;
    std::string currentBuildingId;
    
    // Character state
    Vector2 characterPosition = {400, 300};
    float characterSpeed = 200.0f;
    
    // Asset manager reference
    AssetManager* assetManager = nullptr;
    
    // Notification system
    std::unique_ptr<NotificationSystem> notificationSystem;
    
    // Performance monitoring
    std::unique_ptr<PerformanceMonitor> performanceMonitor;
    
    // Auto-save
    float autoSaveTimer = 0.0f;
    static constexpr float AUTO_SAVE_INTERVAL = 30.0f;
    
    // Helper methods
    void updateCharacterMovement(float deltaTime);
    void checkBuildingInteractions();
    void renderTownScreen();
    void renderBuildingScreen();
    void renderUI();
};