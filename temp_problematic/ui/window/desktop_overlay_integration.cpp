#include "desktop_overlay_integration.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

DesktopOverlayIntegration::DesktopOverlayIntegration(std::shared_ptr<WindowManager> windowManager, TownState& townState)
    : windowManager(windowManager), townState(townState) {
    
    // Load settings
    loadDesktopSettings();
    
    // Set up window manager callbacks
    windowManager->setOnModeChanged([this](WindowMode from, WindowMode to) {
        bool isDesktop = (to == WindowMode::DESKTOP_OVERLAY);
        desktopModeEnabled = isDesktop;
        
        if (onDesktopModeChanged) {
            onDesktopModeChanged(isDesktop);
        }
        
        if (isDesktop) {
            setupDesktopMode();
        } else {
            setupWindowedMode();
        }
    });
}

// Desktop mode management
bool DesktopOverlayIntegration::enableDesktopMode() {
    if (desktopModeEnabled) {
        return true;
    }
    
    bool success = windowManager->setWindowMode(WindowMode::DESKTOP_OVERLAY);
    if (success) {
        desktopModeEnabled = true;
        setupDesktopMode();
        
        std::cout << "DesktopOverlayIntegration: Desktop mode enabled" << std::endl;
    }
    
    return success;
}

bool DesktopOverlayIntegration::disableDesktopMode() {
    if (!desktopModeEnabled) {
        return true;
    }
    
    bool success = windowManager->setWindowMode(WindowMode::WINDOWED);
    if (success) {
        desktopModeEnabled = false;
        setupWindowedMode();
        
        std::cout << "DesktopOverlayIntegration: Desktop mode disabled" << std::endl;
    }
    
    return success;
}

bool DesktopOverlayIntegration::toggleDesktopMode() {
    if (desktopModeEnabled) {
        return disableDesktopMode();
    } else {
        return enableDesktopMode();
    }
}

bool DesktopOverlayIntegration::isDesktopModeEnabled() const {
    return desktopModeEnabled;
}

// Character position management in desktop mode
void DesktopOverlayIntegration::updateCharacterPosition(const Vector2& gamePosition) {
    lastCharacterPosition = gamePosition;
    
    if (desktopModeEnabled) {
        // Convert game position to screen position
        characterScreenPosition = gamePositionToScreenPosition(gamePosition);
        
        // Clamp to screen if enabled
        if (desktopSettings.clampCharacterToScreen) {
            characterScreenPosition = clampCharacterToScreen(characterScreenPosition);
        }
        
        if (onCharacterPositionChanged) {
            onCharacterPositionChanged(characterScreenPosition);
        }
    }
}

Vector2 DesktopOverlayIntegration::getCharacterScreenPosition() const {
    return characterScreenPosition;
}

Vector2 DesktopOverlayIntegration::clampCharacterToScreen(const Vector2& position) const {
    Vector2 screenSize = windowManager->getScreenSize();
    float padding = (float)desktopSettings.screenPadding;
    float characterSize = 32.0f * characterScale; // Assuming 32x32 character sprite
    
    Vector2 clampedPos = position;
    clampedPos.x = std::clamp(clampedPos.x, padding, screenSize.x - characterSize - padding);
    clampedPos.y = std::clamp(clampedPos.y, padding, screenSize.y - characterSize - padding);
    
    return clampedPos;
}

// Building overlay management
void DesktopOverlayIntegration::setBuildingOverlayAlpha(float alpha) {
    buildingOverlayAlpha = std::clamp(alpha, MIN_OVERLAY_ALPHA, MAX_OVERLAY_ALPHA);
    desktopSettings.buildingOverlayAlpha = buildingOverlayAlpha;
}

void DesktopOverlayIntegration::enableBuildingOverlays(bool enabled) {
    buildingOverlaysEnabled = enabled;
    desktopSettings.showBuildingOverlays = enabled;
}

// Desktop interaction
bool DesktopOverlayIntegration::isMouseOverGameElement(const Vector2& mousePos) const {
    if (!desktopModeEnabled) {
        return true; // In windowed mode, all clicks are for the game
    }
    
    // Check if mouse is over character
    Vector2 characterPos = getCharacterScreenPosition();
    float characterSize = 32.0f * characterScale;
    
    if (mousePos.x >= characterPos.x && mousePos.x <= characterPos.x + characterSize &&
        mousePos.y >= characterPos.y && mousePos.y <= characterPos.y + characterSize) {
        return true;
    }
    
    // Check if mouse is over any building overlays
    if (buildingOverlaysEnabled) {
        const TownMap& townMap = townState.getTownMap();
        for (const auto& building : townMap.buildings) {
            Vector2 buildingScreenPos = gamePositionToScreenPosition(building.position);
            Vector2 buildingSize = {building.size.x * characterScale, building.size.y * characterScale};
            
            if (mousePos.x >= buildingScreenPos.x && mousePos.x <= buildingScreenPos.x + buildingSize.x &&
                mousePos.y >= buildingScreenPos.y && mousePos.y <= buildingScreenPos.y + buildingSize.y) {
                return true;
            }
        }
    }
    
    return false;
}

bool DesktopOverlayIntegration::shouldPassThroughClick(const Vector2& mousePos) const {
    return desktopModeEnabled && desktopSettings.enableClickThrough && !isMouseOverGameElement(mousePos);
}

void DesktopOverlayIntegration::updateClickThroughRegions() {
    clickThroughRegions.clear();
    
    if (!desktopModeEnabled || !desktopSettings.enableClickThrough) {
        return;
    }
    
    // Add regions for game elements that should NOT be click-through
    Vector2 screenSize = windowManager->getScreenSize();
    
    // Character region
    Vector2 characterPos = getCharacterScreenPosition();
    float characterSize = 32.0f * characterScale;
    clickThroughRegions.push_back({
        characterPos,
        {characterSize, characterSize},
        true
    });
    
    // Building regions (if overlays are enabled)
    if (buildingOverlaysEnabled) {
        const TownMap& townMap = townState.getTownMap();
        for (const auto& building : townMap.buildings) {
            Vector2 buildingScreenPos = gamePositionToScreenPosition(building.position);
            Vector2 buildingSize = {building.size.x * characterScale, building.size.y * characterScale};
            
            clickThroughRegions.push_back({
                buildingScreenPos,
                buildingSize,
                true
            });
        }
    }
}

// Visual settings for desktop mode
void DesktopOverlayIntegration::setDesktopModeAlpha(float alpha) {
    desktopModeAlpha = std::clamp(alpha, MIN_OVERLAY_ALPHA, MAX_OVERLAY_ALPHA);
    desktopSettings.desktopModeAlpha = desktopModeAlpha;
    
    if (desktopModeEnabled) {
        windowManager->setOverlayAlpha(desktopModeAlpha);
    }
}

void DesktopOverlayIntegration::setCharacterScale(float scale) {
    characterScale = std::clamp(scale, MIN_CHARACTER_SCALE, MAX_CHARACTER_SCALE);
    desktopSettings.characterScale = characterScale;
}

// Desktop mode UI
void DesktopOverlayIntegration::renderDesktopModeIndicator() {
    if (!desktopModeEnabled || !desktopSettings.showDesktopModeIndicator) {
        return;
    }
    
    // Draw a small indicator in the corner
    Vector2 screenSize = windowManager->getScreenSize();
    int indicatorSize = 20;
    int margin = 10;
    
    Vector2 indicatorPos = {screenSize.x - indicatorSize - margin, margin};
    
    // Background circle
    DrawCircleV(indicatorPos, indicatorSize / 2.0f, Fade(BLACK, 0.5f));
    
    // Desktop mode icon (simplified)
    DrawCircleV(indicatorPos, (indicatorSize / 2.0f) - 2, Fade(GREEN, 0.8f));
    
    // Tooltip on hover
    Vector2 mousePos = GetMousePosition();
    float distance = Vector2Distance(mousePos, indicatorPos);
    if (distance <= indicatorSize / 2.0f) {
        const char* tooltip = "Desktop Mode Active";
        int textWidth = MeasureText(tooltip, 12);
        Vector2 tooltipPos = {indicatorPos.x - textWidth - 10, indicatorPos.y - 6};
        
        DrawRectangle((int)tooltipPos.x - 4, (int)tooltipPos.y - 2, textWidth + 8, 16, Fade(BLACK, 0.8f));
        DrawText(tooltip, (int)tooltipPos.x, (int)tooltipPos.y, 12, WHITE);
    }
}

void DesktopOverlayIntegration::renderBuildingOverlays() {
    if (!desktopModeEnabled || !buildingOverlaysEnabled) {
        return;
    }
    
    const TownMap& townMap = townState.getTownMap();
    for (const auto& building : townMap.buildings) {
        renderBuildingOverlay(building);
    }
}

void DesktopOverlayIntegration::renderCharacterOnDesktop(const Vector2& position) {
    if (!desktopModeEnabled) {
        return;
    }
    
    Vector2 screenPos = getCharacterScreenPosition();
    float size = 32.0f * characterScale;
    
    // Draw character as a simple colored circle for now
    // In a real implementation, this would draw the actual character sprite
    Color characterColor = Fade(BLUE, desktopModeAlpha);
    DrawCircleV({screenPos.x + size/2, screenPos.y + size/2}, size/2, characterColor);
    
    // Draw character outline
    DrawCircleLinesV({screenPos.x + size/2, screenPos.y + size/2}, size/2, WHITE);
}

// Settings and preferences
void DesktopOverlayIntegration::applyDesktopSettings(const DesktopSettings& settings) {
    desktopSettings = settings;
    
    // Apply settings immediately
    setCharacterScale(settings.characterScale);
    setBuildingOverlayAlpha(settings.buildingOverlayAlpha);
    setDesktopModeAlpha(settings.desktopModeAlpha);
    enableBuildingOverlays(settings.showBuildingOverlays);
    
    if (desktopModeEnabled) {
        windowManager->setClickThrough(settings.enableClickThrough);
    }
}

void DesktopOverlayIntegration::saveDesktopSettings() const {
    saveSettingsToFile();
}

bool DesktopOverlayIntegration::loadDesktopSettings() {
    return loadSettingsFromFile();
}

// Event callbacks
void DesktopOverlayIntegration::setOnDesktopModeChanged(std::function<void(bool)> callback) {
    onDesktopModeChanged = callback;
}

void DesktopOverlayIntegration::setOnCharacterPositionChanged(std::function<void(Vector2)> callback) {
    onCharacterPositionChanged = callback;
}

// Update and maintenance
void DesktopOverlayIntegration::update(float deltaTime) {
    if (isTransitioning) {
        updateTransition(deltaTime);
    }
    
    if (desktopModeEnabled) {
        updateClickThroughRegions();
        
        // Update character position from town state
        Vector2 characterPos = {townState.getCharacter().getPosition().x, townState.getCharacter().getPosition().y};
        updateCharacterPosition(characterPos);
    }
}

// Private helper methods
void DesktopOverlayIntegration::setupDesktopMode() {
    std::cout << "DesktopOverlayIntegration: Setting up desktop mode" << std::endl;
    
    // Apply desktop mode settings
    windowManager->setOverlayAlpha(desktopModeAlpha);
    windowManager->setClickThrough(desktopSettings.enableClickThrough);
    
    // Start transition animation if enabled
    if (desktopSettings.animateTransitions) {
        isTransitioning = true;
        transitionProgress = 0.0f;
    }
}

void DesktopOverlayIntegration::setupWindowedMode() {
    std::cout << "DesktopOverlayIntegration: Setting up windowed mode" << std::endl;
    
    // Reset to windowed mode settings
    windowManager->setOverlayAlpha(1.0f);
    windowManager->setClickThrough(false);
    
    // Start transition animation if enabled
    if (desktopSettings.animateTransitions) {
        isTransitioning = true;
        transitionProgress = 0.0f;
    }
}

void DesktopOverlayIntegration::updateTransition(float deltaTime) {
    transitionProgress += deltaTime / transitionDuration;
    
    if (transitionProgress >= 1.0f) {
        transitionProgress = 1.0f;
        isTransitioning = false;
    }
}

Vector2 DesktopOverlayIntegration::gamePositionToScreenPosition(const Vector2& gamePos) const {
    // Simple mapping - in a real implementation, this would consider
    // the game world coordinate system and map it to screen coordinates
    Vector2 screenSize = windowManager->getScreenSize();
    const TownMap& townMap = townState.getTownMap();
    
    // Map game coordinates to screen coordinates
    float scaleX = screenSize.x / townMap.size.x;
    float scaleY = screenSize.y / townMap.size.y;
    
    return {gamePos.x * scaleX, gamePos.y * scaleY};
}

Vector2 DesktopOverlayIntegration::screenPositionToGamePosition(const Vector2& screenPos) const {
    Vector2 screenSize = windowManager->getScreenSize();
    const TownMap& townMap = townState.getTownMap();
    
    // Map screen coordinates back to game coordinates
    float scaleX = townMap.size.x / screenSize.x;
    float scaleY = townMap.size.y / screenSize.y;
    
    return {screenPos.x * scaleX, screenPos.y * scaleY};
}

void DesktopOverlayIntegration::renderTransitionEffect() {
    if (!isTransitioning) {
        return;
    }
    
    // Simple fade transition effect
    float alpha = desktopModeEnabled ? transitionProgress : (1.0f - transitionProgress);
    Color overlayColor = Fade(BLACK, 0.2f * alpha);
    
    Vector2 screenSize = windowManager->getScreenSize();
    DrawRectangle(0, 0, (int)screenSize.x, (int)screenSize.y, overlayColor);
}

void DesktopOverlayIntegration::renderBuildingOverlay(const Building& building) {
    Vector2 screenPos = gamePositionToScreenPosition(building.position);
    Vector2 size = {building.size.x * characterScale, building.size.y * characterScale};
    
    // Choose color based on building type
    Color buildingColor;
    switch (building.type) {
        case BuildingType::HOME: buildingColor = BLUE; break;
        case BuildingType::COFFEE_SHOP: buildingColor = ORANGE; break;
        case BuildingType::LIBRARY: buildingColor = PURPLE; break;
        case BuildingType::GYM: buildingColor = RED; break;
        case BuildingType::BULLETIN_BOARD: buildingColor = YELLOW; break;
        default: buildingColor = GRAY; break;
    }
    
    // Apply overlay alpha
    buildingColor = Fade(buildingColor, buildingOverlayAlpha);
    
    // Draw building overlay
    DrawRectangleV(screenPos, size, buildingColor);
    DrawRectangleLinesV(screenPos, size, Fade(WHITE, buildingOverlayAlpha));
    
    // Draw building name if there's space
    if (size.x > 60 && size.y > 20) {
        const char* name = building.name.c_str();
        int textSize = 10;
        int textWidth = MeasureText(name, textSize);
        
        if (textWidth < size.x - 4) {
            Vector2 textPos = {screenPos.x + (size.x - textWidth) / 2, screenPos.y + (size.y - textSize) / 2};
            DrawText(name, (int)textPos.x, (int)textPos.y, textSize, Fade(WHITE, buildingOverlayAlpha));
        }
    }
}

void DesktopOverlayIntegration::renderDesktopCharacter() {
    renderCharacterOnDesktop(lastCharacterPosition);
}

// Settings persistence
void DesktopOverlayIntegration::saveSettingsToFile() const {
    std::ofstream file(SETTINGS_FILE);
    if (!file.is_open()) {
        std::cerr << "DesktopOverlayIntegration: Failed to save settings" << std::endl;
        return;
    }
    
    file << "characterScale=" << desktopSettings.characterScale << std::endl;
    file << "buildingOverlayAlpha=" << desktopSettings.buildingOverlayAlpha << std::endl;
    file << "desktopModeAlpha=" << desktopSettings.desktopModeAlpha << std::endl;
    file << "showBuildingOverlays=" << (desktopSettings.showBuildingOverlays ? 1 : 0) << std::endl;
    file << "enableClickThrough=" << (desktopSettings.enableClickThrough ? 1 : 0) << std::endl;
    file << "clampCharacterToScreen=" << (desktopSettings.clampCharacterToScreen ? 1 : 0) << std::endl;
    file << "screenPadding=" << desktopSettings.screenPadding << std::endl;
    file << "showDesktopModeIndicator=" << (desktopSettings.showDesktopModeIndicator ? 1 : 0) << std::endl;
    file << "animateTransitions=" << (desktopSettings.animateTransitions ? 1 : 0) << std::endl;
    
    file.close();
    std::cout << "DesktopOverlayIntegration: Settings saved" << std::endl;
}

bool DesktopOverlayIntegration::loadSettingsFromFile() {
    std::ifstream file(SETTINGS_FILE);
    if (!file.is_open()) {
        std::cout << "DesktopOverlayIntegration: No saved settings found, using defaults" << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "characterScale") {
            desktopSettings.characterScale = std::stof(value);
        } else if (key == "buildingOverlayAlpha") {
            desktopSettings.buildingOverlayAlpha = std::stof(value);
        } else if (key == "desktopModeAlpha") {
            desktopSettings.desktopModeAlpha = std::stof(value);
        } else if (key == "showBuildingOverlays") {
            desktopSettings.showBuildingOverlays = (std::stoi(value) != 0);
        } else if (key == "enableClickThrough") {
            desktopSettings.enableClickThrough = (std::stoi(value) != 0);
        } else if (key == "clampCharacterToScreen") {
            desktopSettings.clampCharacterToScreen = (std::stoi(value) != 0);
        } else if (key == "screenPadding") {
            desktopSettings.screenPadding = std::stoi(value);
        } else if (key == "showDesktopModeIndicator") {
            desktopSettings.showDesktopModeIndicator = (std::stoi(value) != 0);
        } else if (key == "animateTransitions") {
            desktopSettings.animateTransitions = (std::stoi(value) != 0);
        }
    }
    
    file.close();
    
    // Apply loaded settings
    characterScale = desktopSettings.characterScale;
    buildingOverlayAlpha = desktopSettings.buildingOverlayAlpha;
    desktopModeAlpha = desktopSettings.desktopModeAlpha;
    buildingOverlaysEnabled = desktopSettings.showBuildingOverlays;
    
    std::cout << "DesktopOverlayIntegration: Settings loaded" << std::endl;
    return true;
}