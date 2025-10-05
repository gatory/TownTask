#pragma once

#include "screen.h"
#include "../window/window_manager.h"
#include "../window/desktop_overlay_integration.h"
#include "../../audio/audio_manager.h"
#include "../../input/input_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>

enum class SettingsCategory {
    GENERAL,
    DESKTOP_MODE,
    AUDIO,
    CONTROLS,
    GRAPHICS
};

struct SettingsButton {
    float x, y, width, height;
    std::string text;
    bool enabled = true;
    bool hovered = false;
    std::function<void()> onClick;
    
    SettingsButton(float x, float y, float width, float height, const std::string& text)
        : x(x), y(y), width(width), height(height), text(text) {}
};

struct SettingsSlider {
    float x, y, width, height;
    std::string label;
    float value;
    float minValue, maxValue;
    bool dragging = false;
    std::function<void(float)> onValueChanged;
    
    SettingsSlider(float x, float y, float width, float height, const std::string& label, 
                   float value, float minValue, float maxValue)
        : x(x), y(y), width(width), height(height), label(label), 
          value(value), minValue(minValue), maxValue(maxValue) {}
};

struct SettingsToggle {
    float x, y, width, height;
    std::string label;
    bool value;
    bool hovered = false;
    std::function<void(bool)> onValueChanged;
    
    SettingsToggle(float x, float y, float width, float height, const std::string& label, bool value)
        : x(x), y(y), width(width), height(height), label(label), value(value) {}
};

class SettingsScreen : public Screen {
public:
    SettingsScreen(InputManager& inputManager, 
                   std::shared_ptr<WindowManager> windowManager = nullptr,
                   std::shared_ptr<DesktopOverlayIntegration> desktopOverlay = nullptr,
                   std::shared_ptr<AudioManager> audioManager = nullptr);
    ~SettingsScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Settings management
    void setCurrentCategory(SettingsCategory category);
    SettingsCategory getCurrentCategory() const { return currentCategory; }
    
    // Desktop mode specific
    void showDesktopModeSettings();
    void toggleDesktopMode();
    void adjustDesktopAlpha(float delta);
    void toggleClickThrough();
    void toggleBuildingOverlays();
    void resetDesktopSettings();
    
    // Audio settings
    void showAudioSettings();
    void setMasterVolume(float volume);
    void setSoundEffectVolume(float volume);
    void setMusicVolume(float volume);
    void toggleSoundEffects();
    void toggleMusic();
    
    // General settings
    void showGeneralSettings();
    void toggleFullscreen();
    void setTargetFPS(int fps);
    void toggleVSync();
    
    // Event callbacks
    void setOnExitRequested(std::function<void()> callback);
    void setOnSettingsChanged(std::function<void()> callback);
    
    // Settings persistence
    void saveAllSettings();
    void loadAllSettings();
    void resetAllSettings();
    
private:
    // Core references
    InputManager& inputManager;
    std::shared_ptr<WindowManager> windowManager;
    std::shared_ptr<DesktopOverlayIntegration> desktopOverlay;
    std::shared_ptr<AudioManager> audioManager;
    
    // UI state
    SettingsCategory currentCategory = SettingsCategory::GENERAL;
    std::vector<SettingsButton> buttons;
    std::vector<SettingsSlider> sliders;
    std::vector<SettingsToggle> toggles;
    
    // Category tabs
    std::vector<SettingsButton> categoryTabs;
    
    // Desktop mode preview
    bool showDesktopPreview = false;
    float previewAlpha = 0.8f;
    
    // UI layout constants
    static constexpr float PANEL_WIDTH = 800.0f;
    static constexpr float PANEL_HEIGHT = 600.0f;
    static constexpr float TAB_HEIGHT = 40.0f;
    static constexpr float ITEM_HEIGHT = 35.0f;
    static constexpr float ITEM_SPACING = 10.0f;
    static constexpr float SLIDER_WIDTH = 200.0f;
    static constexpr float BUTTON_WIDTH = 120.0f;
    static constexpr float TOGGLE_WIDTH = 60.0f;
    
    // Event callbacks
    std::function<void()> onExitRequested;
    std::function<void()> onSettingsChanged;
    
    // Internal methods
    void setupUI();
    void setupCategoryTabs();
    void setupGeneralSettings();
    void setupDesktopModeSettings();
    void setupAudioSettings();
    void setupControlsSettings();
    void setupGraphicsSettings();
    
    void updateUI(float deltaTime);
    void updateButtons(const InputState& input);
    void updateSliders(const InputState& input);
    void updateToggles(const InputState& input);
    void updateCategoryTabs(const InputState& input);
    
    void renderUI();
    void renderBackground();
    void renderCategoryTabs();
    void renderCurrentCategory();
    void renderDesktopModePreview();
    void renderButtons();
    void renderSliders();
    void renderToggles();
    void renderDesktopModeStatus();
    
    // Desktop mode UI helpers
    void renderDesktopModeToggle();
    void renderDesktopAlphaSlider();
    void renderClickThroughToggle();
    void renderBuildingOverlaysToggle();
    void renderDesktopModeIndicator();
    void renderDesktopModeHelp();
    
    // Audio UI helpers
    void renderVolumeSliders();
    void renderAudioToggles();
    
    // Utility methods
    Vector2 getPanelPosition() const;
    Vector2 getPanelSize() const;
    bool isPointInRect(Vector2 point, float x, float y, float width, float height) const;
    void clearUIElements();
    
    // Settings validation
    void validateSettings();
    void applySettings();
    
    // Help and tooltips
    void renderTooltip(const std::string& text, Vector2 position);
    void renderHelpText(const std::string& text, Vector2 position);
};