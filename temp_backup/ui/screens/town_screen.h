#pragma once

#include "screen.h"
#include "../../core/models/town_state.h"
#include "../../core/models/character.h"
#include "../../core/systems/movement_system.h"
#include "../../core/systems/building_upgrade_system.h"
#include "../renderers/building_renderer.h"
#include "../../input/input_manager.h"
#include "../animations/animation_manager.h"
#include <memory>
#include <string>
#include <functional>

// Forward declarations
class ScreenManager;

class TownScreen : public Screen {
public:
    TownScreen(TownState& townState, InputManager& inputManager, AnimationManager& animationManager, 
               BuildingUpgradeSystem& upgradeSystem);
    ~TownScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Town-specific functionality
    void setCharacterPosition(const Vector2& position);
    Vector2 getCharacterPosition() const;
    
    // Building interaction
    Building* getNearestInteractableBuilding() const;
    bool canInteractWithNearestBuilding() const;
    void interactWithBuilding(const std::string& buildingId);
    
    // Camera and viewport
    void setCameraPosition(const Vector2& position);
    Vector2 getCameraPosition() const;
    void setCameraFollowCharacter(bool follow);
    bool isCameraFollowingCharacter() const;
    
    // Visual settings
    void setShowInteractionPrompts(bool show);
    bool isShowingInteractionPrompts() const;
    void setShowBuildingNames(bool show);
    bool isShowingBuildingNames() const;
    void setShowDebugInfo(bool show);
    bool isShowingDebugInfo() const;
    
    // Event callbacks
    void setOnBuildingInteraction(std::function<void(const std::string&)> callback);
    void setOnCharacterMoved(std::function<void(const Vector2&)> callback);
    void setOnCameraChanged(std::function<void(const Vector2&)> callback);
    
    // Desktop mode support
    void setDesktopMode(bool enabled);
    bool isDesktopMode() const;
    void setDesktopAlpha(float alpha);
    float getDesktopAlpha() const;
    
private:
    // Core references
    TownState& townState;
    InputManager& inputManager;
    AnimationManager& animationManager;
    BuildingUpgradeSystem& upgradeSystem;
    std::unique_ptr<MovementSystem> movementSystem;
    std::unique_ptr<BuildingRenderer> buildingRenderer;
    
    // Character animation
    std::shared_ptr<SpriteAnimator> characterAnimator;
    std::string currentCharacterAnimation;
    
    // Camera system
    Vector2 cameraPosition;
    Vector2 targetCameraPosition;
    bool cameraFollowCharacter;
    float cameraLerpSpeed;
    
    // Interaction system
    Building* nearestBuilding;
    float interactionDistance;
    bool showInteractionPrompts;
    bool showBuildingNames;
    bool showDebugInfo;
    
    // Desktop mode
    bool desktopMode;
    float desktopAlpha;
    
    // Movement system
    Vector2 characterVelocity;
    float characterSpeed;
    Vector2 lastCharacterPosition;
    
    // Event callbacks
    std::function<void(const std::string&)> onBuildingInteraction;
    std::function<void(const Vector2&)> onCharacterMoved;
    std::function<void(const Vector2&)> onCameraChanged;
    
    // Internal update methods
    void updateCharacterMovement(const InputState& input, float deltaTime);
    void updateCharacterAnimation();
    void updateCamera(float deltaTime);
    void updateBuildingInteractions();
    
    // Rendering methods
    void renderBackground();
    void renderBuildings();
    void renderCharacter();
    void renderUI();
    void renderInteractionPrompts();
    void renderBuildingNames();
    void renderDebugInfo();
    void renderDesktopOverlay();
    
    // Character movement helpers
    Vector2 calculateMovementInput(const InputState& input) const;
    bool isValidCharacterPosition(const Vector2& position) const;
    Vector2 resolveCollisions(const Vector2& desiredPosition) const;
    
    // Animation helpers
    void playCharacterAnimation(const std::string& animationName);
    std::string getMovementAnimationName(const Vector2& movement) const;
    std::string getIdleAnimationName() const;
    
    // Camera helpers
    Vector2 getCharacterCameraTarget() const;
    Vector2 clampCameraToMap(const Vector2& position) const;
    
    // Interaction helpers
    void updateNearestBuilding();
    bool isNearBuilding(const Building& building) const;
    Vector2 getInteractionPromptPosition(const Building& building) const;
    
    // Rendering helpers
    Vector2 worldToScreen(const Vector2& worldPos) const;
    Vector2 screenToWorld(const Vector2& screenPos) const;
    bool isOnScreen(const Vector2& worldPos, const Vector2& size) const;
    
    // Debug helpers
    void renderCharacterDebugInfo();
    void renderBuildingDebugInfo();
    void renderCameraDebugInfo();
    
    // Utility methods
    float lerp(float a, float b, float t) const;
    Vector2 lerp(const Vector2& a, const Vector2& b, float t) const;
    
    // Constants
    static constexpr float DEFAULT_CHARACTER_SPEED = 150.0f;
    static constexpr float DEFAULT_INTERACTION_DISTANCE = 48.0f;
    static constexpr float DEFAULT_CAMERA_LERP_SPEED = 5.0f;
    static constexpr float DEFAULT_DESKTOP_ALPHA = 0.8f;
};