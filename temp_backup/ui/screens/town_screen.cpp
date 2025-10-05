#include "town_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>

TownScreen::TownScreen(TownState& townState, InputManager& inputManager, AnimationManager& animationManager, 
                       BuildingUpgradeSystem& upgradeSystem)
    : Screen("TownScreen"), townState(townState), inputManager(inputManager), animationManager(animationManager),
      upgradeSystem(upgradeSystem), movementSystem(std::make_unique<MovementSystem>(townState)),
      buildingRenderer(std::make_unique<BuildingRenderer>(upgradeSystem)),
      cameraPosition(0, 0), targetCameraPosition(0, 0), cameraFollowCharacter(true), 
      cameraLerpSpeed(DEFAULT_CAMERA_LERP_SPEED), nearestBuilding(nullptr),
      interactionDistance(DEFAULT_INTERACTION_DISTANCE), showInteractionPrompts(true),
      showBuildingNames(false), showDebugInfo(false), desktopMode(false), 
      desktopAlpha(DEFAULT_DESKTOP_ALPHA), characterVelocity(0, 0), 
      characterSpeed(DEFAULT_CHARACTER_SPEED) {
    
    // Initialize character animator
    characterAnimator = animationManager.createAnimator("character", "character_sheet");
    if (characterAnimator) {
        animationManager.createCharacterAnimations("character", "character_sheet", 32, 32);
        currentCharacterAnimation = "idle";
        characterAnimator->playAnimation("idle");
    }
    
    // Initialize camera position
    Vector2 characterPos = Vector2(townState.getCharacter().getPosition().x, townState.getCharacter().getPosition().y);
    cameraPosition = getCharacterCameraTarget();
    targetCameraPosition = cameraPosition;
    lastCharacterPosition = characterPos;
}

TownScreen::~TownScreen() {
    // Cleanup is handled by smart pointers
}

// Screen interface
void TownScreen::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update character movement
    const InputState& input = inputManager.getCurrentState();
    updateCharacterMovement(input, deltaTime);
    
    // Update character animation
    updateCharacterAnimation();
    
    // Update camera
    updateCamera(deltaTime);
    
    // Update building interactions
    updateBuildingInteractions();
    
    // Update town state
    townState.update(deltaTime);
    
    // Update character animator
    if (characterAnimator) {
        characterAnimator->update(deltaTime);
    }
}

void TownScreen::render() {
    if (!isActive() && !shouldRenderWhenPaused()) return;
    
    // Render in order: background, buildings, character, UI
    renderBackground();
    renderBuildings();
    renderCharacter();
    renderUI();
    
    if (desktopMode) {
        renderDesktopOverlay();
    }
}

void TownScreen::handleInput(const InputState& input) {
    if (!isActive()) return;
    
    // Handle building interaction
    if (input.isActionJustPressed(InputAction::INTERACT)) {
        if (nearestBuilding && canInteractWithNearestBuilding()) {
            interactWithBuilding(nearestBuilding->id);
        }
    }
    
    // Handle debug toggle
    if (input.isActionJustPressed(InputAction::TOGGLE_DEBUG_INFO)) {
        setShowDebugInfo(!showDebugInfo);
    }
    
    // Handle desktop mode toggle
    if (input.isActionJustPressed(InputAction::TOGGLE_DESKTOP_MODE)) {
        setDesktopMode(!desktopMode);
    }
}

void TownScreen::onEnter() {
    Screen::onEnter();
    
    // Reset character position to spawn point if needed
    Vector2 spawnPoint = townState.getTownMap().characterSpawnPoint;
    Character& character = townState.getCharacter();
    
    if (character.getPosition().x == 0 && character.getPosition().y == 0) {
        character.setPosition(Character::Position(spawnPoint.x, spawnPoint.y));
    }
    
    // Update camera to character position
    Vector2 characterPos = Vector2(character.getPosition().x, character.getPosition().y);
    cameraPosition = getCharacterCameraTarget();
    targetCameraPosition = cameraPosition;
    
    std::cout << "TownScreen: Entered town screen" << std::endl;
}

void TownScreen::onExit() {
    Screen::onExit();
    std::cout << "TownScreen: Exited town screen" << std::endl;
}

// Town-specific functionality
void TownScreen::setCharacterPosition(const Vector2& position) {
    Character& character = townState.getCharacter();
    character.setPosition(Character::Position(position.x, position.y));
    
    if (onCharacterMoved) {
        onCharacterMoved(position);
    }
}

Vector2 TownScreen::getCharacterPosition() const {
    const Character& character = townState.getCharacter();
    return Vector2(character.getPosition().x, character.getPosition().y);
}

// Building interaction
Building* TownScreen::getNearestInteractableBuilding() const {
    return nearestBuilding;
}

bool TownScreen::canInteractWithNearestBuilding() const {
    return nearestBuilding && nearestBuilding->unlocked && 
           isNearBuilding(*nearestBuilding);
}

void TownScreen::interactWithBuilding(const std::string& buildingId) {
    if (townState.canInteractWithBuilding(buildingId)) {
        if (onBuildingInteraction) {
            onBuildingInteraction(buildingId);
        }
        
        std::cout << "TownScreen: Interacting with building '" << buildingId << "'" << std::endl;
    }
}

// Camera and viewport
void TownScreen::setCameraPosition(const Vector2& position) {
    cameraPosition = clampCameraToMap(position);
    targetCameraPosition = cameraPosition;
    
    if (onCameraChanged) {
        onCameraChanged(cameraPosition);
    }
}

Vector2 TownScreen::getCameraPosition() const {
    return cameraPosition;
}

void TownScreen::setCameraFollowCharacter(bool follow) {
    cameraFollowCharacter = follow;
}

bool TownScreen::isCameraFollowingCharacter() const {
    return cameraFollowCharacter;
}

// Visual settings
void TownScreen::setShowInteractionPrompts(bool show) {
    showInteractionPrompts = show;
}

bool TownScreen::isShowingInteractionPrompts() const {
    return showInteractionPrompts;
}

void TownScreen::setShowBuildingNames(bool show) {
    showBuildingNames = show;
}

bool TownScreen::isShowingBuildingNames() const {
    return showBuildingNames;
}

void TownScreen::setShowDebugInfo(bool show) {
    showDebugInfo = show;
}

bool TownScreen::isShowingDebugInfo() const {
    return showDebugInfo;
}

// Event callbacks
void TownScreen::setOnBuildingInteraction(std::function<void(const std::string&)> callback) {
    onBuildingInteraction = std::move(callback);
}

void TownScreen::setOnCharacterMoved(std::function<void(const Vector2&)> callback) {
    onCharacterMoved = std::move(callback);
}

void TownScreen::setOnCameraChanged(std::function<void(const Vector2&)> callback) {
    onCameraChanged = std::move(callback);
}

// Desktop mode support
void TownScreen::setDesktopMode(bool enabled) {
    desktopMode = enabled;
    std::cout << "TownScreen: Desktop mode " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool TownScreen::isDesktopMode() const {
    return desktopMode;
}

void TownScreen::setDesktopAlpha(float alpha) {
    desktopAlpha = std::clamp(alpha, 0.0f, 1.0f);
}

float TownScreen::getDesktopAlpha() const {
    return desktopAlpha;
}

// Private methods
void TownScreen::updateCharacterMovement(const InputState& input, float deltaTime) {
    Vector2 movementInput = calculateMovementInput(input);
    
    if (movementInput.x != 0.0f || movementInput.y != 0.0f) {
        // Calculate desired velocity
        characterVelocity = movementInput.normalized() * characterSpeed;
        
        // Calculate new position
        Vector2 currentPos = getCharacterPosition();
        Vector2 desiredPos = currentPos + characterVelocity * deltaTime;
        
        // Use MovementSystem for enhanced collision resolution
        Vector2 characterSize = Vector2(32.0f, 32.0f); // Character size
        Vector2 finalPos = movementSystem->resolveCollision(desiredPos, characterSize);
        
        // Update character position
        setCharacterPosition(finalPos);
        
        // Update character direction
        Character& character = townState.getCharacter();
        if (movementInput.x > 0) character.setFacingDirection(Character::Direction::RIGHT);
        else if (movementInput.x < 0) character.setFacingDirection(Character::Direction::LEFT);
        else if (movementInput.y > 0) character.setFacingDirection(Character::Direction::DOWN);
        else if (movementInput.y < 0) character.setFacingDirection(Character::Direction::UP);
        
        character.setState(Character::State::WALKING);
    } else {
        characterVelocity = Vector2(0, 0);
        townState.getCharacter().setState(Character::State::IDLE);
    }
    
    lastCharacterPosition = getCharacterPosition();
}

void TownScreen::updateCharacterAnimation() {
    if (!characterAnimator) return;
    
    Character& character = townState.getCharacter();
    std::string desiredAnimation;
    
    if (character.getState() == Character::State::WALKING) {
        desiredAnimation = getMovementAnimationName(characterVelocity);
    } else {
        desiredAnimation = getIdleAnimationName();
    }
    
    if (desiredAnimation != currentCharacterAnimation) {
        playCharacterAnimation(desiredAnimation);
    }
}

void TownScreen::updateCamera(float deltaTime) {
    if (cameraFollowCharacter) {
        targetCameraPosition = getCharacterCameraTarget();
    }
    
    // Lerp camera to target position
    cameraPosition = lerp(cameraPosition, targetCameraPosition, cameraLerpSpeed * deltaTime);
    cameraPosition = clampCameraToMap(cameraPosition);
}

void TownScreen::updateBuildingInteractions() {
    updateNearestBuilding();
}

// Rendering methods
void TownScreen::renderBackground() {
    // Draw a simple grass pattern background
    const int tileSize = 64;
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color grassColor = ((x + y) / tileSize) % 2 == 0 ? DARKGREEN : GREEN;
            DrawRectangle(x, y, tileSize, tileSize, grassColor);
        }
    }
}

void TownScreen::renderBuildings() {
    const TownMap& townMap = townState.getTownMap();
    
    for (const auto& building : townMap.buildings) {
        Vector2 screenPos = worldToScreen(building.position);
        
        if (isOnScreen(building.position, building.size)) {
            // Use the building renderer for upgraded visuals
            buildingRenderer->renderBuilding(building, screenPos, cameraPosition);
            
            // Show building names if enabled
            if (showBuildingNames) {
                const char* name = building.name.c_str();
                int textWidth = MeasureText(name, 12);
                DrawText(name, 
                        (int)(screenPos.x + building.size.x/2 - textWidth/2), 
                        (int)(screenPos.y - 16), 
                        12, WHITE);
            }
        }
    }
}

void TownScreen::renderCharacter() {
    Vector2 characterPos = getCharacterPosition();
    Vector2 screenPos = worldToScreen(characterPos);
    
    // Draw character as a simple colored circle
    const Character& character = townState.getCharacter();
    Color characterColor = BLUE;
    
    // Change color based on character state
    if (character.getState() == Character::State::WALKING) {
        characterColor = SKYBLUE;
    }
    
    // Draw character body
    DrawCircle((int)(screenPos.x + 16), (int)(screenPos.y + 16), 12, characterColor);
    
    // Draw character outline
    DrawCircleLines((int)(screenPos.x + 16), (int)(screenPos.y + 16), 12, BLACK);
    
    // Draw direction indicator
    Vector2 directionOffset(0, 0);
    switch (character.getFacingDirection()) {
        case Character::Direction::UP: directionOffset = Vector2(0, -8); break;
        case Character::Direction::DOWN: directionOffset = Vector2(0, 8); break;
        case Character::Direction::LEFT: directionOffset = Vector2(-8, 0); break;
        case Character::Direction::RIGHT: directionOffset = Vector2(8, 0); break;
    }
    
    DrawCircle((int)(screenPos.x + 16 + directionOffset.x), 
               (int)(screenPos.y + 16 + directionOffset.y), 
               3, WHITE);
}

void TownScreen::renderUI() {
    if (showInteractionPrompts) {
        renderInteractionPrompts();
    }
    
    if (showBuildingNames) {
        renderBuildingNames();
    }
    
    if (showDebugInfo) {
        renderDebugInfo();
    }
}

void TownScreen::renderInteractionPrompts() {
    if (nearestBuilding && canInteractWithNearestBuilding()) {
        Vector2 promptPos = getInteractionPromptPosition(*nearestBuilding);
        Vector2 screenPos = worldToScreen(promptPos);
        
        // Draw interaction prompt
        const char* promptText = "Press E to interact";
        int textWidth = MeasureText(promptText, 16);
        
        // Draw background for prompt
        DrawRectangle((int)(screenPos.x - textWidth/2 - 4), (int)(screenPos.y - 8), 
                     textWidth + 8, 20, Fade(BLACK, 0.7f));
        
        // Draw prompt text
        DrawText(promptText, 
                (int)(screenPos.x - textWidth/2), (int)(screenPos.y - 4), 
                16, WHITE);
    }
}

void TownScreen::renderBuildingNames() {
    const TownMap& townMap = townState.getTownMap();
    
    for (const auto& building : townMap.buildings) {
        if (isOnScreen(building.position, building.size)) {
            Vector2 namePos = Vector2(building.position.x + building.size.x / 2, building.position.y - 10);
            Vector2 screenPos = worldToScreen(namePos);
            
            // Placeholder: In a real implementation, this would render text
            std::cout << "TownScreen: Rendering building name '" << building.name 
                      << "' at (" << screenPos.x << ", " << screenPos.y << ")" << std::endl;
        }
    }
}

void TownScreen::renderDebugInfo() {
    renderCharacterDebugInfo();
    renderBuildingDebugInfo();
    renderCameraDebugInfo();
}

void TownScreen::renderDesktopOverlay() {
    // Placeholder: In a real implementation, this would render semi-transparent overlays
    std::cout << "TownScreen: Rendering desktop overlay with alpha " << desktopAlpha << std::endl;
}

// Character movement helpers
Vector2 TownScreen::calculateMovementInput(const InputState& input) const {
    Vector2 movement(0, 0);
    
    if (input.isActionActive(InputAction::MOVE_LEFT)) movement.x -= 1.0f;
    if (input.isActionActive(InputAction::MOVE_RIGHT)) movement.x += 1.0f;
    if (input.isActionActive(InputAction::MOVE_UP)) movement.y -= 1.0f;
    if (input.isActionActive(InputAction::MOVE_DOWN)) movement.y += 1.0f;
    
    return movement;
}

bool TownScreen::isValidCharacterPosition(const Vector2& position) const {
    Vector2 characterSize(32, 32); // Assuming 32x32 character
    return townState.getTownMap().isPositionValid(position, characterSize);
}

Vector2 TownScreen::resolveCollisions(const Vector2& desiredPosition) const {
    Vector2 characterSize(32, 32);
    
    // First, clamp to map boundaries
    Vector2 clampedPos = townState.getTownMap().clampToMap(desiredPosition, characterSize);
    
    // Then check building collisions
    if (isValidCharacterPosition(clampedPos)) {
        return clampedPos;
    }
    
    // If there's a collision, try to slide along walls
    Vector2 currentPos = getCharacterPosition();
    
    // Try horizontal movement only
    Vector2 horizontalPos(clampedPos.x, currentPos.y);
    if (isValidCharacterPosition(horizontalPos)) {
        return horizontalPos;
    }
    
    // Try vertical movement only
    Vector2 verticalPos(currentPos.x, clampedPos.y);
    if (isValidCharacterPosition(verticalPos)) {
        return verticalPos;
    }
    
    // If all else fails, stay in current position
    return currentPos;
}

// Animation helpers
void TownScreen::playCharacterAnimation(const std::string& animationName) {
    if (characterAnimator && characterAnimator->hasAnimation(animationName)) {
        characterAnimator->playAnimation(animationName);
        currentCharacterAnimation = animationName;
    }
}

std::string TownScreen::getMovementAnimationName(const Vector2& movement) const {
    if (std::abs(movement.x) > std::abs(movement.y)) {
        return movement.x > 0 ? "walk_right" : "walk_left";
    } else {
        return movement.y > 0 ? "walk_down" : "walk_up";
    }
}

std::string TownScreen::getIdleAnimationName() const {
    return "idle";
}

// Camera helpers
Vector2 TownScreen::getCharacterCameraTarget() const {
    Vector2 characterPos = getCharacterPosition();
    // Center camera on character (assuming 800x600 viewport)
    return Vector2(characterPos.x - 400, characterPos.y - 300);
}

Vector2 TownScreen::clampCameraToMap(const Vector2& position) const {
    const TownMap& townMap = townState.getTownMap();
    Vector2 viewportSize(800, 600); // Assuming 800x600 viewport
    
    Vector2 clampedPos = position;
    clampedPos.x = std::max(0.0f, std::min(clampedPos.x, townMap.size.x - viewportSize.x));
    clampedPos.y = std::max(0.0f, std::min(clampedPos.y, townMap.size.y - viewportSize.y));
    
    return clampedPos;
}

// Interaction helpers
void TownScreen::updateNearestBuilding() {
    Vector2 characterPos = getCharacterPosition();
    nearestBuilding = townState.findNearestBuilding(characterPos, interactionDistance);
}

bool TownScreen::isNearBuilding(const Building& building) const {
    Vector2 characterPos = getCharacterPosition();
    return building.isNearEntrance(characterPos, interactionDistance);
}

Vector2 TownScreen::getInteractionPromptPosition(const Building& building) const {
    return Vector2(building.entrancePosition.x, building.entrancePosition.y - 20);
}

// Rendering helpers
Vector2 TownScreen::worldToScreen(const Vector2& worldPos) const {
    return Vector2(worldPos.x - cameraPosition.x, worldPos.y - cameraPosition.y);
}

Vector2 TownScreen::screenToWorld(const Vector2& screenPos) const {
    return Vector2(screenPos.x + cameraPosition.x, screenPos.y + cameraPosition.y);
}

bool TownScreen::isOnScreen(const Vector2& worldPos, const Vector2& size) const {
    Vector2 screenPos = worldToScreen(worldPos);
    Vector2 viewportSize(800, 600); // Assuming 800x600 viewport
    
    return screenPos.x + size.x >= 0 && screenPos.x <= viewportSize.x &&
           screenPos.y + size.y >= 0 && screenPos.y <= viewportSize.y;
}

// Debug helpers
void TownScreen::renderCharacterDebugInfo() {
    Vector2 characterPos = getCharacterPosition();
    const Character& character = townState.getCharacter();
    
    std::cout << "Character Debug: Position(" << characterPos.x << ", " << characterPos.y 
              << ") State(" << static_cast<int>(character.getState()) 
              << ") Animation(" << currentCharacterAnimation << ")" << std::endl;
}

void TownScreen::renderBuildingDebugInfo() {
    if (nearestBuilding) {
        std::cout << "Building Debug: Nearest='" << nearestBuilding->name 
                  << "' Distance=" << nearestBuilding->entrancePosition.distance(getCharacterPosition())
                  << " CanInteract=" << (canInteractWithNearestBuilding() ? "Yes" : "No") << std::endl;
    }
}

void TownScreen::renderCameraDebugInfo() {
    std::cout << "Camera Debug: Position(" << cameraPosition.x << ", " << cameraPosition.y 
              << ") Target(" << targetCameraPosition.x << ", " << targetCameraPosition.y 
              << ") Following=" << (cameraFollowCharacter ? "Yes" : "No") << std::endl;
}

// Utility methods
float TownScreen::lerp(float a, float b, float t) const {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

Vector2 TownScreen::lerp(const Vector2& a, const Vector2& b, float t) const {
    return Vector2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}