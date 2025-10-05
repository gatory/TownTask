#pragma once

#include "window_manager.h"
#include "../../core/models/town_state.h"
#include <memory>
#include <functional>

// Helper class to integrate desktop overlay functionality with the game
class DesktopOverlayIntegration {
public:
    DesktopOverlayIntegration(std::shared_ptr<WindowManager> windowManager, TownState& townState);
    
    // Desktop mode management
    bool enableDesktopMode();
    bool disableDesktopMode();
    bool toggleDesktopMode();
    bool isDesktopModeEnabled() const;
    
    // Character position management in desktop mode
    void updateCharacterPosition(const Vector2& gamePosition);
    Vector2 getCharacterScreenPosition() const;
    Vector2 clampCharacterToScreen(const Vector2& position) const;
    
    // Building overlay management
    void setBuildingOverlayAlpha(float alpha);
    float getBuildingOverlayAlpha() const { return buildingOverlayAlpha; }
    void enableBuildingOverlays(bool enabled);
    bool areBuildingOverlaysEnabled() const { return buildingOverlaysEnabled; }
    
    // Desktop interaction
    bool isMouseOverGameElement(const Vector2& mousePos) const;
    bool shouldPassThroughClick(const Vector2& mousePos) const;
    void updateClickThroughRegions();
    
    // Visual settings for desktop mode
    void setDesktopModeAlpha(float alpha);
    float getDesktopModeAlpha() const { return desktopModeAlpha; }
    void setCharacterScale(float scale);
    float getCharacterScale() const { return characterScale; }
    
    // Desktop mode UI
    void renderDesktopModeIndicator();
    void renderBuildingOverlays();
    void renderCharacterOnDesktop(const Vector2& position);
    
    // Settings and preferences
    struct DesktopSettings {
        float characterScale = 1.0f;
        float buildingOverlayAlpha = 0.3f;
        float desktopModeAlpha = 0.8f;
        bool showBuildingOverlays = true;
        bool enableClickThrough = true;
        bool clampCharacterToScreen = true;
        int screenPadding = 20;
        bool showDesktopModeIndicator = true;
        bool animateTransitions = true;
    };
    
    void applyDesktopSettings(const DesktopSettings& settings);
    const DesktopSettings& getDesktopSettings() const { return desktopSettings; }
    void saveDesktopSettings() const;
    bool loadDesktopSettings();
    
    // Event callbacks
    void setOnDesktopModeChanged(std::function<void(bool)> callback);
    void setOnCharacterPositionChanged(std::function<void(Vector2)> callback);
    
    // Update and maintenance
    void update(float deltaTime);
    
private:
    std::shared_ptr<WindowManager> windowManager;
    TownState& townState;
    
    // Desktop mode state
    bool desktopModeEnabled = false;
    float desktopModeAlpha = 0.8f;
    float characterScale = 1.0f;
    float buildingOverlayAlpha = 0.3f;
    bool buildingOverlaysEnabled = true;
    
    // Character tracking
    Vector2 lastCharacterPosition = {0, 0};
    Vector2 characterScreenPosition = {0, 0};
    
    // Click-through regions (areas where clicks should pass through)
    struct ClickThroughRegion {
        Vector2 position;
        Vector2 size;
        bool active;
    };
    std::vector<ClickThroughRegion> clickThroughRegions;
    
    // Settings
    DesktopSettings desktopSettings;
    
    // Event callbacks
    std::function<void(bool)> onDesktopModeChanged;
    std::function<void(Vector2)> onCharacterPositionChanged;
    
    // Transition animation
    float transitionProgress = 0.0f;
    bool isTransitioning = false;
    float transitionDuration = 0.5f;
    
    // Internal helpers
    void setupDesktopMode();
    void setupWindowedMode();
    void updateTransition(float deltaTime);
    void updateClickThroughRegions();
    Vector2 gamePositionToScreenPosition(const Vector2& gamePos) const;
    Vector2 screenPositionToGamePosition(const Vector2& screenPos) const;
    
    // Rendering helpers
    void renderTransitionEffect();
    void renderBuildingOverlay(const Building& building);
    void renderDesktopCharacter();
    
    // Settings persistence
    void saveSettingsToFile() const;
    bool loadSettingsFromFile();
    
    // Constants
    static constexpr float MIN_CHARACTER_SCALE = 0.5f;
    static constexpr float MAX_CHARACTER_SCALE = 2.0f;
    static constexpr float MIN_OVERLAY_ALPHA = 0.1f;
    static constexpr float MAX_OVERLAY_ALPHA = 1.0f;
    static constexpr const char* SETTINGS_FILE = "desktop_overlay_settings.json";
};