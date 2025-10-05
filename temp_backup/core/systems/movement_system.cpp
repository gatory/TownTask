#include "movement_system.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// Static constant definitions
const Vector2 MovementSystem::DEFAULT_CHARACTER_SIZE = Vector2(32.0f, 32.0f);

MovementSystem::MovementSystem(TownState& townState, const MovementConfig& config)
    : townState(townState), config(config), characterSize(DEFAULT_CHARACTER_SIZE), debugMode(false) {
    
    // Initialize movement state with character's current position
    const Character& character = townState.getCharacter();
    movementState.position = Vector2(character.getPosition().x, character.getPosition().y);
    movementState.facingDirection = character.getFacingDirection();
    movementState.characterState = character.getState();
    
    previousState = movementState;
}

// Core movement update
void MovementSystem::update(float deltaTime, const InputState& input) {
    previousState = movementState;
    
    updateInput(input);
    updateMovement(deltaTime);
    updateCollisions();
    updateCharacterState();
    
    // Update the character in town state
    Character& character = townState.getCharacter();
    character.setPosition(Character::Position(movementState.position.x, movementState.position.y));
    character.setFacingDirection(movementState.facingDirection);
    character.setState(movementState.characterState);
    
    triggerCallbacks();
    
    if (debugMode) {
        logMovementState();
    }
}

// Movement control
void MovementSystem::setPosition(const Vector2& position) {
    Vector2 validPosition = clampToBoundaries(position, characterSize);
    
    if (isValidPosition(validPosition, characterSize)) {
        updateMovementState(validPosition, movementState.velocity);
    }
}

void MovementSystem::setVelocity(const Vector2& velocity) {
    Vector2 clampedVelocity = velocity;
    float speed = calculateSpeed(clampedVelocity);
    
    if (speed > config.maxVelocity) {
        clampedVelocity = clampedVelocity.normalized() * config.maxVelocity;
    }
    
    movementState.velocity = clampedVelocity;
    movementState.currentSpeed = calculateSpeed(clampedVelocity);
}

void MovementSystem::addForce(const Vector2& force) {
    movementState.acceleration = movementState.acceleration + force;
}

void MovementSystem::stop() {
    movementState.velocity = Vector2(0, 0);
    movementState.acceleration = Vector2(0, 0);
    movementState.currentSpeed = 0.0f;
    movementState.isMoving = false;
    movementState.isRunning = false;
    movementState.characterState = Character::State::IDLE;
}

void MovementSystem::teleport(const Vector2& position) {
    Vector2 validPosition = clampToBoundaries(position, characterSize);
    
    if (isValidPosition(validPosition, characterSize)) {
        movementState.position = validPosition;
        stop(); // Reset movement when teleporting
    }
}

// Configuration
void MovementSystem::setMovementConfig(const MovementConfig& newConfig) {
    config = newConfig;
}

void MovementSystem::setBaseSpeed(float speed) {
    config.baseSpeed = std::max(0.0f, speed);
}

void MovementSystem::enableRunning(bool enable) {
    config.enableRunning = enable;
    if (!enable) {
        movementState.isRunning = false;
    }
}

void MovementSystem::enableSmoothing(bool enable) {
    config.enableSmoothing = enable;
}

void MovementSystem::enableCollisionSliding(bool enable) {
    config.enableCollisionSliding = enable;
}

// Collision system
CollisionInfo MovementSystem::checkCollision(const Vector2& position, const Vector2& size) const {
    // Check building collisions first
    CollisionInfo buildingCollision = checkCollisionWithBuildings(position, size);
    if (buildingCollision.hasCollision) {
        return buildingCollision;
    }
    
    // Check boundary collisions
    return checkCollisionWithBoundaries(position, size);
}

CollisionInfo MovementSystem::checkCollisionWithBuildings(const Vector2& position, const Vector2& size) const {
    const TownMap& townMap = townState.getTownMap();
    
    // Character bounding box
    Vector2 characterCenter = position + size * 0.5f;
    
    for (const auto& building : townMap.buildings) {
        if (isPointInBuilding(characterCenter, building)) {
            Vector2 buildingCenter = building.getCenter();
            Vector2 collisionNormal = (characterCenter - buildingCenter).normalized();
            
            return CollisionInfo(true, characterCenter, collisionNormal, 
                                building.id, CollisionInfo::CollisionType::BUILDING);
        }
    }
    
    return CollisionInfo();
}

CollisionInfo MovementSystem::checkCollisionWithBoundaries(const Vector2& position, const Vector2& size) const {
    const TownMap& townMap = townState.getTownMap();
    
    // Check left boundary
    if (position.x < 0) {
        return CollisionInfo(true, Vector2(0, position.y), Vector2(1, 0), 
                           "left_boundary", CollisionInfo::CollisionType::BOUNDARY);
    }
    
    // Check right boundary
    if (position.x + size.x > townMap.size.x) {
        return CollisionInfo(true, Vector2(townMap.size.x, position.y), Vector2(-1, 0), 
                           "right_boundary", CollisionInfo::CollisionType::BOUNDARY);
    }
    
    // Check top boundary
    if (position.y < 0) {
        return CollisionInfo(true, Vector2(position.x, 0), Vector2(0, 1), 
                           "top_boundary", CollisionInfo::CollisionType::BOUNDARY);
    }
    
    // Check bottom boundary
    if (position.y + size.y > townMap.size.y) {
        return CollisionInfo(true, Vector2(position.x, townMap.size.y), Vector2(0, -1), 
                           "bottom_boundary", CollisionInfo::CollisionType::BOUNDARY);
    }
    
    return CollisionInfo();
}

Vector2 MovementSystem::resolveCollision(const Vector2& desiredPosition, const Vector2& size) const {
    CollisionInfo collision = checkCollision(desiredPosition, size);
    
    if (!collision.hasCollision) {
        return desiredPosition;
    }
    
    Vector2 resolvedPosition = desiredPosition;
    
    if (config.enableCollisionSliding && collision.type == CollisionInfo::CollisionType::BUILDING) {
        // Try sliding along the collision surface
        Vector2 slideDirection = slideAlongSurface(movementState.velocity, collision.collisionNormal);
        Vector2 slidePosition = movementState.position + slideDirection * 0.016f; // Assuming 60 FPS
        
        if (isValidPosition(slidePosition, size)) {
            resolvedPosition = slidePosition;
        } else {
            resolvedPosition = movementState.position; // Stay in place
        }
    } else {
        // Simple boundary clamping
        resolvedPosition = clampToBoundaries(desiredPosition, size);
    }
    
    return resolvedPosition;
}

// Movement validation
bool MovementSystem::isValidPosition(const Vector2& position, const Vector2& size) const {
    return !checkCollision(position, size).hasCollision;
}

Vector2 MovementSystem::clampToBoundaries(const Vector2& position, const Vector2& size) const {
    const TownMap& townMap = townState.getTownMap();
    
    Vector2 clampedPos = position;
    clampedPos.x = std::max(0.0f, std::min(clampedPos.x, townMap.size.x - size.x));
    clampedPos.y = std::max(0.0f, std::min(clampedPos.y, townMap.size.y - size.y));
    
    return clampedPos;
}

// Movement queries
float MovementSystem::getMaxSpeed() const {
    float baseSpeed = config.baseSpeed;
    if (movementState.isRunning && config.enableRunning) {
        baseSpeed *= config.runSpeedMultiplier;
    }
    return std::min(baseSpeed, config.maxVelocity);
}

// Event callbacks
void MovementSystem::setOnPositionChanged(std::function<void(const Vector2&, const Vector2&)> callback) {
    onPositionChanged = std::move(callback);
}

void MovementSystem::setOnDirectionChanged(std::function<void(Character::Direction, Character::Direction)> callback) {
    onDirectionChanged = std::move(callback);
}

void MovementSystem::setOnStateChanged(std::function<void(Character::State, Character::State)> callback) {
    onStateChanged = std::move(callback);
}

void MovementSystem::setOnCollision(std::function<void(const CollisionInfo&)> callback) {
    onCollision = std::move(callback);
}

// Debug and diagnostics
void MovementSystem::enableDebugMode(bool enable) {
    debugMode = enable;
}

std::vector<std::string> MovementSystem::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== MovementSystem Debug Info ===");
    info.push_back("Position: (" + std::to_string(movementState.position.x) + ", " + std::to_string(movementState.position.y) + ")");
    info.push_back("Velocity: (" + std::to_string(movementState.velocity.x) + ", " + std::to_string(movementState.velocity.y) + ")");
    info.push_back("Speed: " + std::to_string(movementState.currentSpeed) + " / " + std::to_string(getMaxSpeed()));
    info.push_back("State: " + std::to_string(static_cast<int>(movementState.characterState)));
    info.push_back("Direction: " + std::to_string(static_cast<int>(movementState.facingDirection)));
    info.push_back("Moving: " + std::string(movementState.isMoving ? "Yes" : "No"));
    info.push_back("Running: " + std::string(movementState.isRunning ? "Yes" : "No"));
    
    info.push_back("\n=== Configuration ===");
    info.push_back("Base Speed: " + std::to_string(config.baseSpeed));
    info.push_back("Run Multiplier: " + std::to_string(config.runSpeedMultiplier));
    info.push_back("Acceleration: " + std::to_string(config.acceleration));
    info.push_back("Deceleration: " + std::to_string(config.deceleration));
    info.push_back("Max Velocity: " + std::to_string(config.maxVelocity));
    info.push_back("Running Enabled: " + std::string(config.enableRunning ? "Yes" : "No"));
    info.push_back("Smoothing Enabled: " + std::string(config.enableSmoothing ? "Yes" : "No"));
    info.push_back("Collision Sliding: " + std::string(config.enableCollisionSliding ? "Yes" : "No"));
    
    return info;
}

void MovementSystem::logMovementState() const {
    if (!debugMode) return;
    
    std::cout << "MovementSystem: Pos(" << movementState.position.x << ", " << movementState.position.y 
              << ") Vel(" << movementState.velocity.x << ", " << movementState.velocity.y 
              << ") Speed(" << movementState.currentSpeed << ")" << std::endl;
}

// Private methods
void MovementSystem::updateInput(const InputState& input) {
    Vector2 inputDirection = getMovementInput(input);
    movementState.isRunning = config.enableRunning && isRunInput(input);
    
    if (inputDirection.length() > 0.0f) {
        updateFacingDirection(inputDirection);
        movementState.isMoving = true;
        movementState.characterState = Character::State::WALKING;
    } else {
        movementState.isMoving = false;
        movementState.characterState = Character::State::IDLE;
    }
}

void MovementSystem::updateMovement(float deltaTime) {
    Vector2 inputDirection = Vector2(0, 0);
    
    if (movementState.isMoving) {
        inputDirection = directionToVector(movementState.facingDirection);
    }
    
    Vector2 targetVelocity = calculateDesiredVelocity(inputDirection, deltaTime);
    
    if (config.enableSmoothing) {
        if (movementState.isMoving) {
            movementState.velocity = applyAcceleration(movementState.velocity, targetVelocity, deltaTime);
        } else {
            movementState.velocity = applyDeceleration(movementState.velocity, deltaTime);
        }
    } else {
        movementState.velocity = targetVelocity;
    }
    
    // Calculate new position
    Vector2 desiredPosition = movementState.position + movementState.velocity * deltaTime;
    Vector2 resolvedPosition = resolveCollision(desiredPosition, characterSize);
    
    updateMovementState(resolvedPosition, movementState.velocity);
}

void MovementSystem::updateCollisions() {
    CollisionInfo collision = checkCollision(movementState.position, characterSize);
    
    if (collision.hasCollision && onCollision) {
        onCollision(collision);
    }
}

void MovementSystem::updateCharacterState() {
    movementState.currentSpeed = calculateSpeed(movementState.velocity);
    
    if (movementState.currentSpeed < MIN_MOVEMENT_THRESHOLD) {
        movementState.isMoving = false;
        movementState.characterState = Character::State::IDLE;
    }
}

void MovementSystem::updateFacingDirection(const Vector2& inputDirection) {
    Character::Direction newDirection = vectorToDirection(inputDirection);
    if (newDirection != movementState.facingDirection) {
        Character::Direction oldDirection = movementState.facingDirection;
        movementState.facingDirection = newDirection;
        
        if (onDirectionChanged) {
            onDirectionChanged(oldDirection, newDirection);
        }
    }
}

// Input processing
Vector2 MovementSystem::getMovementInput(const InputState& input) const {
    Vector2 movement(0, 0);
    
    if (input.isActionActive(InputAction::MOVE_LEFT)) movement.x -= 1.0f;
    if (input.isActionActive(InputAction::MOVE_RIGHT)) movement.x += 1.0f;
    if (input.isActionActive(InputAction::MOVE_UP)) movement.y -= 1.0f;
    if (input.isActionActive(InputAction::MOVE_DOWN)) movement.y += 1.0f;
    
    // Normalize diagonal movement
    if (movement.length() > 1.0f) {
        movement = movement.normalized();
    }
    
    return movement;
}

bool MovementSystem::isRunInput(const InputState& input) const {
    return input.shiftPressed; // Hold shift to run
}

// Movement calculations
Vector2 MovementSystem::calculateDesiredVelocity(const Vector2& inputDirection, float deltaTime) const {
    if (inputDirection.length() == 0.0f) {
        return Vector2(0, 0);
    }
    
    float targetSpeed = getMaxSpeed();
    return inputDirection * targetSpeed;
}

Vector2 MovementSystem::applyAcceleration(const Vector2& currentVelocity, const Vector2& targetVelocity, float deltaTime) const {
    Vector2 velocityDiff = targetVelocity - currentVelocity;
    float accelerationRate = config.acceleration * deltaTime;
    
    if (velocityDiff.length() <= accelerationRate) {
        return targetVelocity;
    }
    
    return currentVelocity + velocityDiff.normalized() * accelerationRate;
}

Vector2 MovementSystem::applyDeceleration(const Vector2& currentVelocity, float deltaTime) const {
    float decelerationRate = config.deceleration * deltaTime;
    
    if (currentVelocity.length() <= decelerationRate) {
        return Vector2(0, 0);
    }
    
    return currentVelocity - currentVelocity.normalized() * decelerationRate;
}

// Collision helpers
CollisionInfo MovementSystem::performCollisionCheck(const Vector2& position, const Vector2& size) const {
    return checkCollision(position, size);
}

Vector2 MovementSystem::slideAlongSurface(const Vector2& velocity, const Vector2& normal) const {
    // Project velocity onto the surface (perpendicular to normal)
    return velocity - normal * (velocity.x * normal.x + velocity.y * normal.y);
}

bool MovementSystem::isPointInBuilding(const Vector2& point, const Building& building) const {
    return building.containsPoint(point);
}

// State management
void MovementSystem::updateMovementState(const Vector2& newPosition, const Vector2& newVelocity) {
    Vector2 oldPosition = movementState.position;
    
    movementState.position = newPosition;
    movementState.velocity = newVelocity;
    movementState.currentSpeed = calculateSpeed(newVelocity);
    
    if (oldPosition.distance(newPosition) > COLLISION_EPSILON && onPositionChanged) {
        onPositionChanged(oldPosition, newPosition);
    }
}

void MovementSystem::triggerCallbacks() {
    // State change callback
    if (previousState.characterState != movementState.characterState && onStateChanged) {
        onStateChanged(previousState.characterState, movementState.characterState);
    }
}

// Utility methods
float MovementSystem::calculateSpeed(const Vector2& velocity) const {
    return velocity.length();
}

Character::Direction MovementSystem::vectorToDirection(const Vector2& direction) const {
    if (std::abs(direction.x) > std::abs(direction.y)) {
        return direction.x > 0 ? Character::Direction::RIGHT : Character::Direction::LEFT;
    } else {
        return direction.y > 0 ? Character::Direction::DOWN : Character::Direction::UP;
    }
}

Vector2 MovementSystem::directionToVector(Character::Direction direction) const {
    switch (direction) {
        case Character::Direction::UP: return Vector2(0, -1);
        case Character::Direction::DOWN: return Vector2(0, 1);
        case Character::Direction::LEFT: return Vector2(-1, 0);
        case Character::Direction::RIGHT: return Vector2(1, 0);
        default: return Vector2(0, 0);
    }
}