#pragma once

#include <raylib.h>
#include "../window/window_manager.h"
#include "../window/desktop_overlay_integration.h"
#include <memory>
#include <string>
#include <functional>

// A compact widget for toggling desktop mode that can be embedded in any screen
class DesktopModeWidget {
public:
    DesktopModeWidget(std::shared_ptr<WindowManager> windowManager,
                      std::shared_ptr<DesktopOverlayIntegration> desktopOverlay);
    
    // Widget management
    void setPosition(Vector2 position);
    void setSize(Vector2 size);
    Vector2 getPosition() const { return position; }
    Vector2 getSize() const { return size; }
    
    // Visibility and interaction
    void setVisible(bool visible);
    bool isVisible() const { return visible; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // Update and rendering
    void update(float deltaTime);
    void render();
    void handleInput(Vector2 mousePos, bool mousePressed);
    
    // Desktop mode control
    void toggleDesktopMode();
    bool isDesktopModeActive() const;
    void setDesktopModeActive(bool active);
    
    // Widget appearance
    void setCompactMode(bool compact);
    bool isCompactMode() const { return compactMode; }
    void setShowLabel(bool showLabel);
    bool isShowingLabel() const { return showLabel; }
    
    // Event callbacks
    void setOnDesktopModeChanged(std::function<void(bool)> callback);
    void setOnWidgetClicked(std::function<void()> callback);
    
    // Quick access methods
    void showQuickSettings();
    void hideQuickSettings();
    bool isShowingQuickSettings() const { return showingQuickSettings; }
    
private:
    std::shared_ptr<WindowManager> windowManager;
    std::shared_ptr<DesktopOverlayIntegration> desktopOverlay;
    
    // Widget state
    Vector2 position = {10, 10};
    Vector2 size = {120, 30};
    bool visible = true;
    bool enabled = true;
    bool hovered = false;
    bool pressed = false;
    
    // Appearance settings
    bool compactMode = false;
    bool showLabel = true;
    bool showingQuickSettings = false;
    
    // Animation state
    float animationTime = 0.0f;
    float pulseAnimation = 0.0f;
    bool isTransitioning = false;
    
    // Quick settings panel
    struct QuickSettingsPanel {
        Vector2 position;
        Vector2 size = {200, 150};
        bool visible = false;
        float alpha = 0.0f;
        
        // Quick setting controls
        bool alphaSliderDragging = false;
        float alphaSliderValue = 0.8f;
    } quickSettings;
    
    // Event callbacks
    std::function<void(bool)> onDesktopModeChanged;
    std::function<void()> onWidgetClicked;
    
    // Internal methods
    void updateAnimations(float deltaTime);
    void updateQuickSettings(Vector2 mousePos, bool mousePressed);
    void renderWidget();
    void renderQuickSettings();
    void renderDesktopModeIcon();
    void renderStatusIndicator();
    void renderCompactWidget();
    void renderFullWidget();
    
    // Utility methods
    bool isPointInWidget(Vector2 point) const;
    bool isPointInQuickSettings(Vector2 point) const;
    Color getWidgetColor() const;
    Color getIconColor() const;
    std::string getStatusText() const;
    
    // Constants
    static constexpr float ANIMATION_SPEED = 3.0f;
    static constexpr float PULSE_SPEED = 2.0f;
    static constexpr float QUICK_SETTINGS_FADE_SPEED = 5.0f;
    static constexpr int COMPACT_SIZE = 24;
    static constexpr int FULL_HEIGHT = 30;
};