#include "settings_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>

SettingsScreen::SettingsScreen(InputManager& inputManager,
                               std::shared_ptr<WindowManager> windowManager,
                               std::shared_ptr<DesktopOverlayIntegration> desktopOverlay,
                               std::shared_ptr<AudioManager> audioManager)
    : Screen("SettingsScreen")
    , inputManager(inputManager)
    , windowManager(windowManager)
    , desktopOverlay(desktopOverlay)
    , audioManager(audioManager) {
    
    setupUI();
}

SettingsScreen::~SettingsScreen() {
    // Cleanup handled by containers
}

void SettingsScreen::update(float deltaTime) {
    updateUI(deltaTime);
}

void SettingsScreen::render() {
    renderBackground();
    renderUI();
}

void SettingsScreen::handleInput(const InputState& input) {
    updateCategoryTabs(input);
    updateButtons(input);
    updateSliders(input);
    updateToggles(input);
    
    // ESC to exit
    if (input.isKeyJustPressed(KEY_ESCAPE)) {
        if (onExitRequested) {
            onExitRequested();
        }
    }
    
    // Desktop mode hotkeys
    if (desktopOverlay) {
        // Ctrl+D to toggle desktop mode
        if (input.isKeyDown(KEY_LEFT_CONTROL) && input.isKeyJustPressed(KEY_D)) {
            toggleDesktopMode();
        }
        
        // Ctrl+T to toggle click-through
        if (input.isKeyDown(KEY_LEFT_CONTROL) && input.isKeyJustPressed(KEY_T)) {
            toggleClickThrough();
        }
    }
}

void SettingsScreen::onEnter() {
    Screen::onEnter();
    
    // Load current settings
    loadAllSettings();
    
    std::cout << "SettingsScreen: Entered settings" << std::endl;
}

void SettingsScreen::onExit() {
    // Save settings when exiting
    saveAllSettings();
    
    Screen::onExit();
    std::cout << "SettingsScreen: Exited settings" << std::endl;
}

// Settings management
void SettingsScreen::setCurrentCategory(SettingsCategory category) {
    if (currentCategory != category) {
        currentCategory = category;
        setupUI(); // Rebuild UI for new category
    }
}

// Desktop mode specific
void SettingsScreen::showDesktopModeSettings() {
    setCurrentCategory(SettingsCategory::DESKTOP_MODE);
}

void SettingsScreen::toggleDesktopMode() {
    if (!desktopOverlay) return;
    
    bool success = desktopOverlay->toggleDesktopMode();
    if (success) {
        std::cout << "SettingsScreen: Desktop mode " 
                  << (desktopOverlay->isDesktopModeEnabled() ? "enabled" : "disabled") << std::endl;
        
        if (onSettingsChanged) {
            onSettingsChanged();
        }
    }
}

void SettingsScreen::adjustDesktopAlpha(float delta) {
    if (!desktopOverlay) return;
    
    float currentAlpha = desktopOverlay->getDesktopModeAlpha();
    float newAlpha = std::clamp(currentAlpha + delta, 0.1f, 1.0f);
    desktopOverlay->setDesktopModeAlpha(newAlpha);
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::toggleClickThrough() {
    if (!windowManager) return;
    
    bool clickThrough = !windowManager->isClickThroughEnabled();
    windowManager->setClickThrough(clickThrough);
    
    std::cout << "SettingsScreen: Click-through " 
              << (clickThrough ? "enabled" : "disabled") << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::toggleBuildingOverlays() {
    if (!desktopOverlay) return;
    
    bool overlays = !desktopOverlay->areBuildingOverlaysEnabled();
    desktopOverlay->enableBuildingOverlays(overlays);
    
    std::cout << "SettingsScreen: Building overlays " 
              << (overlays ? "enabled" : "disabled") << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::resetDesktopSettings() {
    if (!desktopOverlay) return;
    
    // Reset to default desktop settings
    DesktopOverlayIntegration::DesktopSettings defaultSettings;
    desktopOverlay->applyDesktopSettings(defaultSettings);
    
    std::cout << "SettingsScreen: Desktop settings reset to defaults" << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

// Audio settings
void SettingsScreen::showAudioSettings() {
    setCurrentCategory(SettingsCategory::AUDIO);
}

void SettingsScreen::setMasterVolume(float volume) {
    if (!audioManager) return;
    
    audioManager->setMasterVolume(volume);
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::setSoundEffectVolume(float volume) {
    if (!audioManager) return;
    
    audioManager->setSoundEffectVolume(volume);
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::setMusicVolume(float volume) {
    if (!audioManager) return;
    
    audioManager->setMusicVolume(volume);
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::toggleSoundEffects() {
    if (!audioManager) return;
    
    bool enabled = !audioManager->areSoundEffectsEnabled();
    audioManager->enableSoundEffects(enabled);
    
    std::cout << "SettingsScreen: Sound effects " 
              << (enabled ? "enabled" : "disabled") << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::toggleMusic() {
    if (!audioManager) return;
    
    bool enabled = !audioManager->isMusicEnabled();
    audioManager->enableMusic(enabled);
    
    std::cout << "SettingsScreen: Music " 
              << (enabled ? "enabled" : "disabled") << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

// General settings
void SettingsScreen::showGeneralSettings() {
    setCurrentCategory(SettingsCategory::GENERAL);
}

void SettingsScreen::toggleFullscreen() {
    if (IsWindowFullscreen()) {
        ToggleFullscreen();
        std::cout << "SettingsScreen: Switched to windowed mode" << std::endl;
    } else {
        ToggleFullscreen();
        std::cout << "SettingsScreen: Switched to fullscreen mode" << std::endl;
    }
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::setTargetFPS(int fps) {
    SetTargetFPS(fps);
    std::cout << "SettingsScreen: Target FPS set to " << fps << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::toggleVSync() {
    // VSync toggle would need platform-specific implementation
    std::cout << "SettingsScreen: VSync toggle (not implemented)" << std::endl;
}

// Event callbacks
void SettingsScreen::setOnExitRequested(std::function<void()> callback) {
    onExitRequested = callback;
}

void SettingsScreen::setOnSettingsChanged(std::function<void()> callback) {
    onSettingsChanged = callback;
}

// Settings persistence
void SettingsScreen::saveAllSettings() {
    if (windowManager) {
        windowManager->saveWindowState();
    }
    
    if (desktopOverlay) {
        desktopOverlay->saveDesktopSettings();
    }
    
    if (audioManager) {
        audioManager->saveSettings("audio_settings.txt");
    }
    
    std::cout << "SettingsScreen: All settings saved" << std::endl;
}

void SettingsScreen::loadAllSettings() {
    if (windowManager) {
        windowManager->loadWindowState();
    }
    
    if (desktopOverlay) {
        desktopOverlay->loadDesktopSettings();
    }
    
    if (audioManager) {
        audioManager->loadSettings("audio_settings.txt");
    }
    
    std::cout << "SettingsScreen: All settings loaded" << std::endl;
}

void SettingsScreen::resetAllSettings() {
    if (windowManager) {
        windowManager->resetToDefaults();
    }
    
    if (desktopOverlay) {
        resetDesktopSettings();
    }
    
    if (audioManager) {
        AudioSettings defaultSettings;
        audioManager->applySettings(defaultSettings);
    }
    
    std::cout << "SettingsScreen: All settings reset to defaults" << std::endl;
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

// Private methods
void SettingsScreen::setupUI() {
    clearUIElements();
    setupCategoryTabs();
    
    switch (currentCategory) {
        case SettingsCategory::GENERAL:
            setupGeneralSettings();
            break;
        case SettingsCategory::DESKTOP_MODE:
            setupDesktopModeSettings();
            break;
        case SettingsCategory::AUDIO:
            setupAudioSettings();
            break;
        case SettingsCategory::CONTROLS:
            setupControlsSettings();
            break;
        case SettingsCategory::GRAPHICS:
            setupGraphicsSettings();
            break;
    }
}

void SettingsScreen::setupCategoryTabs() {
    categoryTabs.clear();
    
    Vector2 panelPos = getPanelPosition();
    float tabWidth = PANEL_WIDTH / 5.0f; // 5 categories
    
    // General tab
    categoryTabs.emplace_back(panelPos.x, panelPos.y, tabWidth, TAB_HEIGHT, "General");
    categoryTabs.back().onClick = [this]() { setCurrentCategory(SettingsCategory::GENERAL); };
    
    // Desktop Mode tab
    categoryTabs.emplace_back(panelPos.x + tabWidth, panelPos.y, tabWidth, TAB_HEIGHT, "Desktop");
    categoryTabs.back().onClick = [this]() { setCurrentCategory(SettingsCategory::DESKTOP_MODE); };
    
    // Audio tab
    categoryTabs.emplace_back(panelPos.x + tabWidth * 2, panelPos.y, tabWidth, TAB_HEIGHT, "Audio");
    categoryTabs.back().onClick = [this]() { setCurrentCategory(SettingsCategory::AUDIO); };
    
    // Controls tab
    categoryTabs.emplace_back(panelPos.x + tabWidth * 3, panelPos.y, tabWidth, TAB_HEIGHT, "Controls");
    categoryTabs.back().onClick = [this]() { setCurrentCategory(SettingsCategory::CONTROLS); };
    
    // Graphics tab
    categoryTabs.emplace_back(panelPos.x + tabWidth * 4, panelPos.y, tabWidth, TAB_HEIGHT, "Graphics");
    categoryTabs.back().onClick = [this]() { setCurrentCategory(SettingsCategory::GRAPHICS); };
}

void SettingsScreen::setupGeneralSettings() {
    Vector2 panelPos = getPanelPosition();
    float startY = panelPos.y + TAB_HEIGHT + 20;
    float currentY = startY;
    
    // Fullscreen toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, "Fullscreen", IsWindowFullscreen());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != IsWindowFullscreen()) {
            toggleFullscreen();
        }
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Target FPS slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, "Target FPS", 60, 30, 120);
    sliders.back().onValueChanged = [this](float value) {
        setTargetFPS((int)value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Reset button
    buttons.emplace_back(panelPos.x + 20, currentY + 20, BUTTON_WIDTH, ITEM_HEIGHT, "Reset All");
    buttons.back().onClick = [this]() {
        resetAllSettings();
    };
}

void SettingsScreen::setupDesktopModeSettings() {
    Vector2 panelPos = getPanelPosition();
    float startY = panelPos.y + TAB_HEIGHT + 20;
    float currentY = startY;
    
    if (!desktopOverlay || !windowManager) {
        // Show message if desktop overlay is not available
        return;
    }
    
    // Desktop mode toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, 
                        "Desktop Mode", desktopOverlay->isDesktopModeEnabled());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != desktopOverlay->isDesktopModeEnabled()) {
            toggleDesktopMode();
        }
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Desktop alpha slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Transparency", desktopOverlay->getDesktopModeAlpha(), 0.1f, 1.0f);
    sliders.back().onValueChanged = [this](float value) {
        desktopOverlay->setDesktopModeAlpha(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Character scale slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Character Scale", desktopOverlay->getCharacterScale(), 0.5f, 2.0f);
    sliders.back().onValueChanged = [this](float value) {
        desktopOverlay->setCharacterScale(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Click-through toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, 
                        "Click Through", windowManager->isClickThroughEnabled());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != windowManager->isClickThroughEnabled()) {
            toggleClickThrough();
        }
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Building overlays toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, 
                        "Building Overlays", desktopOverlay->areBuildingOverlaysEnabled());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != desktopOverlay->areBuildingOverlaysEnabled()) {
            toggleBuildingOverlays();
        }
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Building overlay alpha slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Building Alpha", desktopOverlay->getBuildingOverlayAlpha(), 0.1f, 1.0f);
    sliders.back().onValueChanged = [this](float value) {
        desktopOverlay->setBuildingOverlayAlpha(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Reset desktop settings button
    buttons.emplace_back(panelPos.x + 20, currentY + 20, BUTTON_WIDTH, ITEM_HEIGHT, "Reset Desktop");
    buttons.back().onClick = [this]() {
        resetDesktopSettings();
    };
}

void SettingsScreen::setupAudioSettings() {
    Vector2 panelPos = getPanelPosition();
    float startY = panelPos.y + TAB_HEIGHT + 20;
    float currentY = startY;
    
    if (!audioManager) {
        return;
    }
    
    // Master volume slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Master Volume", audioManager->getMasterVolume(), 0.0f, 1.0f);
    sliders.back().onValueChanged = [this](float value) {
        setMasterVolume(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Sound effects volume slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Sound Effects", audioManager->getSoundEffectVolume(), 0.0f, 1.0f);
    sliders.back().onValueChanged = [this](float value) {
        setSoundEffectVolume(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Music volume slider
    sliders.emplace_back(panelPos.x + 20, currentY, SLIDER_WIDTH, ITEM_HEIGHT, 
                        "Music Volume", audioManager->getMusicVolume(), 0.0f, 1.0f);
    sliders.back().onValueChanged = [this](float value) {
        setMusicVolume(value);
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Sound effects toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, 
                        "Sound Effects", audioManager->areSoundEffectsEnabled());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != audioManager->areSoundEffectsEnabled()) {
            toggleSoundEffects();
        }
    };
    currentY += ITEM_HEIGHT + ITEM_SPACING;
    
    // Music toggle
    toggles.emplace_back(panelPos.x + 20, currentY, TOGGLE_WIDTH, ITEM_HEIGHT, 
                        "Music", audioManager->isMusicEnabled());
    toggles.back().onValueChanged = [this](bool value) {
        if (value != audioManager->isMusicEnabled()) {
            toggleMusic();
        }
    };
}

void SettingsScreen::setupControlsSettings() {
    Vector2 panelPos = getPanelPosition();
    float startY = panelPos.y + TAB_HEIGHT + 20;
    
    // Placeholder for controls settings
    // This would include key binding configuration
}

void SettingsScreen::setupGraphicsSettings() {
    Vector2 panelPos = getPanelPosition();
    float startY = panelPos.y + TAB_HEIGHT + 20;
    
    // Placeholder for graphics settings
    // This would include resolution, quality settings, etc.
}void 
SettingsScreen::updateUI(float deltaTime) {
    // Update any animations or time-based effects
    if (showDesktopPreview) {
        previewAlpha = 0.5f + 0.3f * sinf(GetTime() * 2.0f); // Pulsing effect
    }
}

void SettingsScreen::updateButtons(const InputState& input) {
    Vector2 mousePos = GetMousePosition();
    
    for (auto& button : buttons) {
        bool wasHovered = button.hovered;
        button.hovered = isPointInRect(mousePos, button.x, button.y, button.width, button.height);
        
        if (button.hovered && input.mouse.leftButton && button.enabled && button.onClick) {
            button.onClick();
        }
    }
}

void SettingsScreen::updateSliders(const InputState& input) {
    Vector2 mousePos = GetMousePosition();
    
    for (auto& slider : sliders) {
        bool inSlider = isPointInRect(mousePos, slider.x, slider.y, slider.width, slider.height);
        
        if (inSlider && input.mouse.leftButton) {
            slider.dragging = true;
        }
        
        if (!input.mouse.leftButton) {
            slider.dragging = false;
        }
        
        if (slider.dragging) {
            float relativeX = mousePos.x - slider.x;
            float normalizedValue = std::clamp(relativeX / slider.width, 0.0f, 1.0f);
            float newValue = slider.minValue + normalizedValue * (slider.maxValue - slider.minValue);
            
            if (std::abs(newValue - slider.value) > 0.01f) {
                slider.value = newValue;
                if (slider.onValueChanged) {
                    slider.onValueChanged(newValue);
                }
            }
        }
    }
}

void SettingsScreen::updateToggles(const InputState& input) {
    Vector2 mousePos = GetMousePosition();
    
    for (auto& toggle : toggles) {
        bool wasHovered = toggle.hovered;
        toggle.hovered = isPointInRect(mousePos, toggle.x, toggle.y, toggle.width, toggle.height);
        
        if (toggle.hovered && input.mouse.leftButton) {
            toggle.value = !toggle.value;
            if (toggle.onValueChanged) {
                toggle.onValueChanged(toggle.value);
            }
        }
    }
}

void SettingsScreen::updateCategoryTabs(const InputState& input) {
    Vector2 mousePos = GetMousePosition();
    
    for (size_t i = 0; i < categoryTabs.size(); ++i) {
        auto& tab = categoryTabs[i];
        bool wasHovered = tab.hovered;
        tab.hovered = isPointInRect(mousePos, tab.x, tab.y, tab.width, tab.height);
        
        if (tab.hovered && input.mouse.leftButton && tab.onClick) {
            tab.onClick();
        }
    }
}

void SettingsScreen::renderUI() {
    renderCategoryTabs();
    renderCurrentCategory();
    
    if (desktopOverlay && desktopOverlay->isDesktopModeEnabled()) {
        renderDesktopModeStatus();
    }
}

void SettingsScreen::renderBackground() {
    ClearBackground(Color{30, 30, 40, 255});
    
    // Draw panel background
    Vector2 panelPos = getPanelPosition();
    Vector2 panelSize = getPanelSize();
    
    DrawRectangle((int)panelPos.x, (int)panelPos.y, (int)panelSize.x, (int)panelSize.y, Color{50, 50, 60, 255});
    DrawRectangleLines((int)panelPos.x, (int)panelPos.y, (int)panelSize.x, (int)panelSize.y, Color{100, 100, 120, 255});
}

void SettingsScreen::renderCategoryTabs() {
    for (size_t i = 0; i < categoryTabs.size(); ++i) {
        const auto& tab = categoryTabs[i];
        
        // Determine tab color
        Color tabColor;
        if ((SettingsCategory)i == currentCategory) {
            tabColor = Color{80, 80, 100, 255}; // Active tab
        } else if (tab.hovered) {
            tabColor = Color{70, 70, 90, 255}; // Hovered tab
        } else {
            tabColor = Color{60, 60, 80, 255}; // Normal tab
        }
        
        // Draw tab
        DrawRectangle((int)tab.x, (int)tab.y, (int)tab.width, (int)tab.height, tabColor);
        DrawRectangleLines((int)tab.x, (int)tab.y, (int)tab.width, (int)tab.height, Color{100, 100, 120, 255});
        
        // Draw tab text
        int textWidth = MeasureText(tab.text.c_str(), 14);
        Vector2 textPos = {
            tab.x + (tab.width - textWidth) / 2,
            tab.y + (tab.height - 14) / 2
        };
        
        Color textColor = ((SettingsCategory)i == currentCategory) ? WHITE : LIGHTGRAY;
        DrawText(tab.text.c_str(), (int)textPos.x, (int)textPos.y, 14, textColor);
    }
}

void SettingsScreen::renderCurrentCategory() {
    Vector2 panelPos = getPanelPosition();
    float contentY = panelPos.y + TAB_HEIGHT + 10;
    
    // Draw category title
    const char* categoryName = "";
    switch (currentCategory) {
        case SettingsCategory::GENERAL: categoryName = "General Settings"; break;
        case SettingsCategory::DESKTOP_MODE: categoryName = "Desktop Mode Settings"; break;
        case SettingsCategory::AUDIO: categoryName = "Audio Settings"; break;
        case SettingsCategory::CONTROLS: categoryName = "Controls Settings"; break;
        case SettingsCategory::GRAPHICS: categoryName = "Graphics Settings"; break;
    }
    
    DrawText(categoryName, (int)(panelPos.x + 20), (int)contentY, 18, WHITE);
    
    // Render UI elements
    renderButtons();
    renderSliders();
    renderToggles();
    
    // Render category-specific content
    if (currentCategory == SettingsCategory::DESKTOP_MODE) {
        renderDesktopModeHelp();
    }
}

void SettingsScreen::renderDesktopModePreview() {
    if (!showDesktopPreview || !desktopOverlay) return;
    
    // Draw a preview of what desktop mode looks like
    Vector2 panelPos = getPanelPosition();
    float previewX = panelPos.x + PANEL_WIDTH - 200;
    float previewY = panelPos.y + TAB_HEIGHT + 50;
    float previewWidth = 180;
    float previewHeight = 120;
    
    // Preview background (represents desktop)
    DrawRectangle((int)previewX, (int)previewY, (int)previewWidth, (int)previewHeight, DARKGRAY);
    DrawText("Desktop Preview", (int)previewX + 5, (int)previewY + 5, 10, WHITE);
    
    // Preview character
    Vector2 charPos = {previewX + previewWidth * 0.3f, previewY + previewHeight * 0.6f};
    DrawCircleV(charPos, 8, Fade(BLUE, previewAlpha));
    
    // Preview building overlay
    if (desktopOverlay->areBuildingOverlaysEnabled()) {
        Vector2 buildingPos = {previewX + previewWidth * 0.7f, previewY + previewHeight * 0.4f};
        DrawRectangle((int)buildingPos.x, (int)buildingPos.y, 30, 20, 
                     Fade(ORANGE, desktopOverlay->getBuildingOverlayAlpha()));
    }
    
    DrawRectangleLines((int)previewX, (int)previewY, (int)previewWidth, (int)previewHeight, WHITE);
}

void SettingsScreen::renderButtons() {
    for (const auto& button : buttons) {
        Color buttonColor = button.enabled ? 
            (button.hovered ? Color{80, 120, 160, 255} : Color{60, 100, 140, 255}) :
            Color{40, 40, 50, 255};
        
        DrawRectangle((int)button.x, (int)button.y, (int)button.width, (int)button.height, buttonColor);
        DrawRectangleLines((int)button.x, (int)button.y, (int)button.width, (int)button.height, WHITE);
        
        Color textColor = button.enabled ? WHITE : GRAY;
        int textWidth = MeasureText(button.text.c_str(), 12);
        Vector2 textPos = {
            button.x + (button.width - textWidth) / 2,
            button.y + (button.height - 12) / 2
        };
        
        DrawText(button.text.c_str(), (int)textPos.x, (int)textPos.y, 12, textColor);
    }
}

void SettingsScreen::renderSliders() {
    for (const auto& slider : sliders) {
        // Draw label
        DrawText(slider.label.c_str(), (int)slider.x, (int)(slider.y - 18), 12, WHITE);
        
        // Draw slider track
        DrawRectangle((int)slider.x, (int)(slider.y + slider.height/2 - 2), (int)slider.width, 4, DARKGRAY);
        
        // Draw slider handle
        float normalizedValue = (slider.value - slider.minValue) / (slider.maxValue - slider.minValue);
        float handleX = slider.x + normalizedValue * slider.width;
        
        Color handleColor = slider.dragging ? Color{120, 160, 200, 255} : Color{80, 120, 160, 255};
        DrawCircleV({handleX, slider.y + slider.height/2}, 8, handleColor);
        DrawCircleLinesV({handleX, slider.y + slider.height/2}, 8, WHITE);
        
        // Draw value text
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%.2f", slider.value);
        DrawText(valueText, (int)(slider.x + slider.width + 10), (int)(slider.y + slider.height/2 - 6), 12, LIGHTGRAY);
    }
}

void SettingsScreen::renderToggles() {
    for (const auto& toggle : toggles) {
        // Draw label
        DrawText(toggle.label.c_str(), (int)toggle.x, (int)(toggle.y - 18), 12, WHITE);
        
        // Draw toggle background
        Color bgColor = toggle.hovered ? Color{70, 70, 90, 255} : Color{50, 50, 70, 255};
        DrawRectangle((int)toggle.x, (int)toggle.y, (int)toggle.width, (int)toggle.height, bgColor);
        DrawRectangleLines((int)toggle.x, (int)toggle.y, (int)toggle.width, (int)toggle.height, WHITE);
        
        // Draw toggle state
        if (toggle.value) {
            DrawRectangle((int)(toggle.x + 5), (int)(toggle.y + 5), 
                         (int)(toggle.width - 10), (int)(toggle.height - 10), GREEN);
            DrawText("ON", (int)(toggle.x + toggle.width/2 - 8), (int)(toggle.y + toggle.height/2 - 6), 12, WHITE);
        } else {
            DrawText("OFF", (int)(toggle.x + toggle.width/2 - 10), (int)(toggle.y + toggle.height/2 - 6), 12, LIGHTGRAY);
        }
    }
}

void SettingsScreen::renderDesktopModeStatus() {
    if (!desktopOverlay) return;
    
    // Draw desktop mode status in corner
    Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    const char* statusText = "Desktop Mode Active";
    int textWidth = MeasureText(statusText, 14);
    
    Vector2 statusPos = {screenSize.x - textWidth - 20, 20};
    
    // Background
    DrawRectangle((int)statusPos.x - 5, (int)statusPos.y - 2, textWidth + 10, 18, Fade(BLACK, 0.7f));
    
    // Text
    DrawText(statusText, (int)statusPos.x, (int)statusPos.y, 14, GREEN);
    
    // Additional info
    char infoText[128];
    snprintf(infoText, sizeof(infoText), "Alpha: %.1f | Click-through: %s", 
             desktopOverlay->getDesktopModeAlpha(),
             windowManager && windowManager->isClickThroughEnabled() ? "ON" : "OFF");
    
    DrawText(infoText, (int)statusPos.x, (int)statusPos.y + 20, 10, LIGHTGRAY);
}

void SettingsScreen::renderDesktopModeHelp() {
    Vector2 panelPos = getPanelPosition();
    float helpY = panelPos.y + PANEL_HEIGHT - 120;
    
    DrawText("Desktop Mode Help:", (int)(panelPos.x + 20), (int)helpY, 14, YELLOW);
    helpY += 20;
    
    DrawText("• Desktop Mode makes TaskTown run as a transparent overlay", (int)(panelPos.x + 30), (int)helpY, 10, LIGHTGRAY);
    helpY += 15;
    DrawText("• Your character will appear on your actual desktop", (int)(panelPos.x + 30), (int)helpY, 10, LIGHTGRAY);
    helpY += 15;
    DrawText("• Click-through allows interaction with desktop underneath", (int)(panelPos.x + 30), (int)helpY, 10, LIGHTGRAY);
    helpY += 15;
    DrawText("• Hotkeys: Ctrl+D (toggle), Ctrl+T (click-through)", (int)(panelPos.x + 30), (int)helpY, 10, LIGHTGRAY);
}

// Utility methods
Vector2 SettingsScreen::getPanelPosition() const {
    Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    return {
        (screenSize.x - PANEL_WIDTH) / 2,
        (screenSize.y - PANEL_HEIGHT) / 2
    };
}

Vector2 SettingsScreen::getPanelSize() const {
    return {PANEL_WIDTH, PANEL_HEIGHT};
}

bool SettingsScreen::isPointInRect(Vector2 point, float x, float y, float width, float height) const {
    return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height;
}

void SettingsScreen::clearUIElements() {
    buttons.clear();
    sliders.clear();
    toggles.clear();
}

void SettingsScreen::validateSettings() {
    // Validate and clamp settings to valid ranges
    if (desktopOverlay) {
        float alpha = desktopOverlay->getDesktopModeAlpha();
        if (alpha < 0.1f || alpha > 1.0f) {
            desktopOverlay->setDesktopModeAlpha(std::clamp(alpha, 0.1f, 1.0f));
        }
    }
}

void SettingsScreen::applySettings() {
    // Apply any pending settings changes
    validateSettings();
    
    if (onSettingsChanged) {
        onSettingsChanged();
    }
}

void SettingsScreen::renderTooltip(const std::string& text, Vector2 position) {
    int textWidth = MeasureText(text.c_str(), 10);
    
    // Background
    DrawRectangle((int)position.x - 2, (int)position.y - 2, textWidth + 4, 14, Fade(BLACK, 0.8f));
    
    // Text
    DrawText(text.c_str(), (int)position.x, (int)position.y, 10, WHITE);
}

void SettingsScreen::renderHelpText(const std::string& text, Vector2 position) {
    DrawText(text.c_str(), (int)position.x, (int)position.y, 10, LIGHTGRAY);
}