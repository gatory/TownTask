#include "desktop_mode_widget.h"
#include <iostream>
#include <algorithm>
#include <cmath>

DesktopModeWidget::DesktopModeWidget(std::shared_ptr<WindowManager> windowManager,
                                     std::shared_ptr<DesktopOverlayIntegration> desktopOverlay)
    : windowManager(windowManager), desktopOverlay(desktopOverlay) {
    
    // Set up initial quick settings position
    quickSettings.position = {position.x, position.y + size.y + 5};
    
    // Initialize alpha slider value from current desktop settings
    if (desktopOverlay) {
        quickSettings.alphaSliderValue = desktopOverlay->getDesktopModeAlpha();
    }
}

// Widget management
void DesktopModeWidget::setPosition(Vector2 newPosition) {
    position = newPosition;
    quickSettings.position = {position.x, position.y + size.y + 5};
}

void DesktopModeWidget::setSize(Vector2 newSize) {
    size = newSize;
    quickSettings.position = {position.x, position.y + size.y + 5};
}

void DesktopModeWidget::setVisible(bool newVisible) {
    visible = newVisible;
    if (!visible) {
        hideQuickSettings();
    }
}

void DesktopModeWidget::setEnabled(bool newEnabled) {
    enabled = newEnabled;
}

// Update and rendering
void DesktopModeWidget::update(float deltaTime) {
    if (!visible) return;
    
    updateAnimations(deltaTime);
    
    // Update quick settings alpha slider value
    if (desktopOverlay) {
        quickSettings.alphaSliderValue = desktopOverlay->getDesktopModeAlpha();
    }
}

void DesktopModeWidget::render() {
    if (!visible) return;
    
    renderWidget();
    
    if (showingQuickSettings) {
        renderQuickSettings();
    }
}

void DesktopModeWidget::handleInput(Vector2 mousePos, bool mousePressed) {
    if (!visible || !enabled) return;
    
    // Update hover state
    bool wasHovered = hovered;
    hovered = isPointInWidget(mousePos);
    
    // Handle widget click
    if (hovered && mousePressed && !pressed) {
        pressed = true;
        
        if (onWidgetClicked) {
            onWidgetClicked();
        }
        
        // Toggle desktop mode on click
        toggleDesktopMode();
    }
    
    if (!mousePressed) {
        pressed = false;
    }
    
    // Handle quick settings
    if (showingQuickSettings) {
        updateQuickSettings(mousePos, mousePressed);
        
        // Hide quick settings if clicking outside
        if (mousePressed && !isPointInQuickSettings(mousePos) && !hovered) {
            hideQuickSettings();
        }
    } else {
        // Show quick settings on right-click
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            showQuickSettings();
        }
    }
}

// Desktop mode control
void DesktopModeWidget::toggleDesktopMode() {
    if (!desktopOverlay) return;
    
    bool wasActive = isDesktopModeActive();
    bool success = desktopOverlay->toggleDesktopMode();
    
    if (success) {
        isTransitioning = true;
        animationTime = 0.0f;
        
        if (onDesktopModeChanged) {
            onDesktopModeChanged(!wasActive);
        }
        
        std::cout << "DesktopModeWidget: Desktop mode " 
                  << (isDesktopModeActive() ? "enabled" : "disabled") << std::endl;
    }
}

bool DesktopModeWidget::isDesktopModeActive() const {
    return desktopOverlay && desktopOverlay->isDesktopModeEnabled();
}

void DesktopModeWidget::setDesktopModeActive(bool active) {
    if (!desktopOverlay) return;
    
    bool currentState = isDesktopModeActive();
    if (currentState != active) {
        if (active) {
            desktopOverlay->enableDesktopMode();
        } else {
            desktopOverlay->disableDesktopMode();
        }
        
        if (onDesktopModeChanged) {
            onDesktopModeChanged(active);
        }
    }
}

// Widget appearance
void DesktopModeWidget::setCompactMode(bool compact) {
    compactMode = compact;
    
    if (compact) {
        size = {COMPACT_SIZE, COMPACT_SIZE};
    } else {
        size = {120, FULL_HEIGHT};
    }
    
    quickSettings.position = {position.x, position.y + size.y + 5};
}

void DesktopModeWidget::setShowLabel(bool show) {
    showLabel = show;
}

// Event callbacks
void DesktopModeWidget::setOnDesktopModeChanged(std::function<void(bool)> callback) {
    onDesktopModeChanged = callback;
}

void DesktopModeWidget::setOnWidgetClicked(std::function<void()> callback) {
    onWidgetClicked = callback;
}

// Quick access methods
void DesktopModeWidget::showQuickSettings() {
    showingQuickSettings = true;
    quickSettings.visible = true;
}

void DesktopModeWidget::hideQuickSettings() {
    showingQuickSettings = false;
    quickSettings.visible = false;
    quickSettings.alpha = 0.0f;
}

// Private methods
void DesktopModeWidget::updateAnimations(float deltaTime) {
    animationTime += deltaTime * ANIMATION_SPEED;
    pulseAnimation += deltaTime * PULSE_SPEED;
    
    if (animationTime > 1.0f) {
        isTransitioning = false;
        animationTime = 1.0f;
    }
    
    // Update quick settings fade
    if (showingQuickSettings) {
        quickSettings.alpha = std::min(quickSettings.alpha + deltaTime * QUICK_SETTINGS_FADE_SPEED, 1.0f);
    } else {
        quickSettings.alpha = std::max(quickSettings.alpha - deltaTime * QUICK_SETTINGS_FADE_SPEED, 0.0f);
    }
}

void DesktopModeWidget::updateQuickSettings(Vector2 mousePos, bool mousePressed) {
    // Handle alpha slider
    float sliderX = quickSettings.position.x + 10;
    float sliderY = quickSettings.position.y + 40;
    float sliderWidth = quickSettings.size.x - 20;
    float sliderHeight = 20;
    
    bool inSlider = mousePos.x >= sliderX && mousePos.x <= sliderX + sliderWidth &&
                    mousePos.y >= sliderY && mousePos.y <= sliderY + sliderHeight;
    
    if (inSlider && mousePressed) {
        quickSettings.alphaSliderDragging = true;
    }
    
    if (!mousePressed) {
        quickSettings.alphaSliderDragging = false;
    }
    
    if (quickSettings.alphaSliderDragging && desktopOverlay) {
        float relativeX = mousePos.x - sliderX;
        float normalizedValue = std::clamp(relativeX / sliderWidth, 0.0f, 1.0f);
        float newAlpha = 0.1f + normalizedValue * 0.9f; // Range from 0.1 to 1.0
        
        desktopOverlay->setDesktopModeAlpha(newAlpha);
        quickSettings.alphaSliderValue = newAlpha;
    }
}

void DesktopModeWidget::renderWidget() {
    if (compactMode) {
        renderCompactWidget();
    } else {
        renderFullWidget();
    }
}

void DesktopModeWidget::renderQuickSettings() {
    if (quickSettings.alpha <= 0.0f) return;
    
    Color panelColor = Fade(Color{40, 40, 50, 255}, quickSettings.alpha);
    Color borderColor = Fade(Color{100, 100, 120, 255}, quickSettings.alpha);
    Color textColor = Fade(WHITE, quickSettings.alpha);
    
    // Panel background
    DrawRectangle((int)quickSettings.position.x, (int)quickSettings.position.y,
                  (int)quickSettings.size.x, (int)quickSettings.size.y, panelColor);
    DrawRectangleLines((int)quickSettings.position.x, (int)quickSettings.position.y,
                       (int)quickSettings.size.x, (int)quickSettings.size.y, borderColor);
    
    // Title
    DrawText("Desktop Mode", (int)(quickSettings.position.x + 10), (int)(quickSettings.position.y + 10), 12, textColor);
    
    // Alpha slider
    float sliderX = quickSettings.position.x + 10;
    float sliderY = quickSettings.position.y + 40;
    float sliderWidth = quickSettings.size.x - 20;
    
    DrawText("Transparency:", (int)sliderX, (int)(sliderY - 15), 10, Fade(LIGHTGRAY, quickSettings.alpha));
    
    // Slider track
    DrawRectangle((int)sliderX, (int)(sliderY + 5), (int)sliderWidth, 4, Fade(DARKGRAY, quickSettings.alpha));
    
    // Slider handle
    float normalizedValue = (quickSettings.alphaSliderValue - 0.1f) / 0.9f;
    float handleX = sliderX + normalizedValue * sliderWidth;
    
    Color handleColor = quickSettings.alphaSliderDragging ? 
        Fade(Color{120, 160, 200, 255}, quickSettings.alpha) : 
        Fade(Color{80, 120, 160, 255}, quickSettings.alpha);
    
    DrawCircleV({handleX, sliderY + 7}, 6, handleColor);
    
    // Alpha value text
    char alphaText[16];
    snprintf(alphaText, sizeof(alphaText), "%.1f", quickSettings.alphaSliderValue);
    DrawText(alphaText, (int)(sliderX + sliderWidth + 10), (int)(sliderY + 2), 10, textColor);
    
    // Quick toggle buttons
    float buttonY = quickSettings.position.y + 80;
    
    if (windowManager) {
        const char* clickThroughText = windowManager->isClickThroughEnabled() ? "Click-through: ON" : "Click-through: OFF";
        Color buttonColor = windowManager->isClickThroughEnabled() ? 
            Fade(GREEN, quickSettings.alpha * 0.3f) : 
            Fade(RED, quickSettings.alpha * 0.3f);
        
        DrawRectangle((int)(quickSettings.position.x + 10), (int)buttonY, (int)(quickSettings.size.x - 20), 20, buttonColor);
        DrawText(clickThroughText, (int)(quickSettings.position.x + 15), (int)(buttonY + 5), 10, textColor);
        
        // Handle click-through toggle
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            mousePos.x >= quickSettings.position.x + 10 && mousePos.x <= quickSettings.position.x + quickSettings.size.x - 10 &&
            mousePos.y >= buttonY && mousePos.y <= buttonY + 20) {
            windowManager->setClickThrough(!windowManager->isClickThroughEnabled());
        }
    }
    
    // Help text
    DrawText("Right-click widget for settings", (int)(quickSettings.position.x + 10), 
             (int)(quickSettings.position.y + quickSettings.size.y - 20), 8, Fade(LIGHTGRAY, quickSettings.alpha));
}

void DesktopModeWidget::renderDesktopModeIcon() {
    Vector2 iconPos = {position.x + 5, position.y + 5};
    float iconSize = compactMode ? 14 : 16;
    
    Color iconColor = getIconColor();
    
    if (isDesktopModeActive()) {
        // Desktop mode active icon (computer with overlay)
        DrawRectangle((int)iconPos.x, (int)iconPos.y, (int)iconSize, (int)(iconSize * 0.7f), iconColor);
        DrawRectangleLines((int)iconPos.x, (int)iconPos.y, (int)iconSize, (int)(iconSize * 0.7f), WHITE);
        
        // Overlay effect
        float pulseAlpha = 0.5f + 0.3f * sinf(pulseAnimation);
        DrawRectangle((int)iconPos.x, (int)iconPos.y, (int)iconSize, (int)(iconSize * 0.7f), 
                     Fade(YELLOW, pulseAlpha * 0.3f));
    } else {
        // Desktop mode inactive icon (simple computer)
        DrawRectangle((int)iconPos.x, (int)iconPos.y, (int)iconSize, (int)(iconSize * 0.7f), iconColor);
        DrawRectangleLines((int)iconPos.x, (int)iconPos.y, (int)iconSize, (int)(iconSize * 0.7f), WHITE);
    }
}

void DesktopModeWidget::renderStatusIndicator() {
    if (compactMode) return;
    
    Vector2 indicatorPos = {position.x + size.x - 15, position.y + 5};
    float indicatorSize = 8;
    
    Color indicatorColor = isDesktopModeActive() ? GREEN : RED;
    
    if (isTransitioning) {
        float alpha = 0.5f + 0.5f * sinf(animationTime * 10.0f);
        indicatorColor = Fade(indicatorColor, alpha);
    }
    
    DrawCircleV(indicatorPos, indicatorSize / 2, indicatorColor);
    DrawCircleLinesV(indicatorPos, indicatorSize / 2, WHITE);
}

void DesktopModeWidget::renderCompactWidget() {
    Color widgetColor = getWidgetColor();
    
    // Widget background
    DrawRectangle((int)position.x, (int)position.y, (int)size.x, (int)size.y, widgetColor);
    DrawRectangleLines((int)position.x, (int)position.y, (int)size.x, (int)size.y, WHITE);
    
    // Icon
    renderDesktopModeIcon();
}

void DesktopModeWidget::renderFullWidget() {
    Color widgetColor = getWidgetColor();
    
    // Widget background
    DrawRectangle((int)position.x, (int)position.y, (int)size.x, (int)size.y, widgetColor);
    DrawRectangleLines((int)position.x, (int)position.y, (int)size.x, (int)size.y, WHITE);
    
    // Icon
    renderDesktopModeIcon();
    
    // Label
    if (showLabel) {
        std::string labelText = getStatusText();
        DrawText(labelText.c_str(), (int)(position.x + 25), (int)(position.y + 8), 12, WHITE);
    }
    
    // Status indicator
    renderStatusIndicator();
}

// Utility methods
bool DesktopModeWidget::isPointInWidget(Vector2 point) const {
    return point.x >= position.x && point.x <= position.x + size.x &&
           point.y >= position.y && point.y <= position.y + size.y;
}

bool DesktopModeWidget::isPointInQuickSettings(Vector2 point) const {
    return point.x >= quickSettings.position.x && point.x <= quickSettings.position.x + quickSettings.size.x &&
           point.y >= quickSettings.position.y && point.y <= quickSettings.position.y + quickSettings.size.y;
}

Color DesktopModeWidget::getWidgetColor() const {
    if (!enabled) {
        return Color{40, 40, 50, 255};
    }
    
    if (pressed) {
        return Color{100, 140, 180, 255};
    }
    
    if (hovered) {
        return Color{80, 120, 160, 255};
    }
    
    if (isDesktopModeActive()) {
        return Color{60, 100, 60, 255}; // Green tint when active
    }
    
    return Color{60, 60, 80, 255};
}

Color DesktopModeWidget::getIconColor() const {
    if (!enabled) {
        return GRAY;
    }
    
    if (isDesktopModeActive()) {
        return GREEN;
    }
    
    return LIGHTGRAY;
}

std::string DesktopModeWidget::getStatusText() const {
    if (isDesktopModeActive()) {
        return "Desktop Mode";
    } else {
        return "Windowed Mode";
    }
}