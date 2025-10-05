#pragma once

#include "../models/character.h"
#include "../models/town_state.h"
#include "../../input/input_manager.h"
#include <functional>

// Movement configuration
struct MovementConfig {
    float baseSpeed = 150.0f;
    float runSpeedMultiplier = 1.5f;
    float acceleration = 800.0f;
    float deceleration = 1200.0f;
    float maxVelocity = 300.0f;
    bool enableRunning = true;
    bool enableSmoothing = true;
    bool enableCollisionSliding = true;
    
    MovementConfig() = default;
};

// Collision information
struct CollisionInfo {
    bool hasCollision = false;
    Vector2 collisionPoint;
    Vector2 collisionNormal;
    std::string collidedObjectId;
    enum class CollisionType { BUILDING, BOUNDARY, NONE } type = CollisionType::NONE;
    
    CollisionInfo() = default;
    CollisionInfo(bool collision, const Vector2& point, const Vector2& normal, 
                 const std::string& objectId, CollisionType collisionType)
        : hasCollision(collision), collisionPoint(point), collisionNormal(normal),
          collidedObjectId(objectId), type(collisionType) {}
};

// Movement state
struct MovementState {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Character::Direction facingDirection = Character::Direction::DOWN;
    Character::State characterState = Character::State::IDLE;
    bool isRunning = false;
    bool isMoving = false;
    float currentSpeed = 0.0f;
    
    MovementState() : position(0, 0), velocity(0, 0), acceleration(0, 0) {}
    MovementState(const Vector2& pos) : position(pos), velocity(0, 0), acceleration(0, 0) {}
};

class MovementSystem {
public:
    MovementSystem(TownState& townState, const MovementConfig& config = MovementConfig());
    
    // Core movement update
    void update(float deltaTime, const InputState& input);
    
    // Movement state access
    const MovementState& getMovementState() const { return movementState; }
    Vector2 getPosition() const { return movementState.position; }
    Vector2 getVelocity() const { return movementState.velocity; }
    Character::Direction getFacingDirection() const { return movementState.facingDirection; }
    Character::State getCharacterState() const { return movementState.characterState; }
    
    // Movement control
    void setPosition(const Vector2& position);
    void setVelocity(const Vector2& velocity);
    void addForce(const Vector2& force);
    void stop();
    void teleport(const Vector2& position);
    
    // Configuration
    void setMovementConfig(const MovementConfig& config);
    const MovementConfig& getMovementConfig() const { return config; }
    void setBaseSpeed(float speed);
    void enableRunning(bool enable);
    void enableSmoothing(bool enable);
    void enableCollisionSliding(bool enable);
    
    // Collision system
    CollisionInfo checkCollision(const Vector2& position, const Vector2& size) const;
    CollisionInfo checkCollisionWithBuildings(const Vector2& position, const Vector2& size) const;
    CollisionInfo checkCollisionWithBoundaries(const Vector2& position, const Vector2& size) const;
    Vector2 resolveCollision(const Vector2& desiredPosition, const Vector2& size) const;
    
    // Movement validation
    bool isValidPosition(const Vector2& position, const Vector2& size) const;
    Vector2 clampToBoundaries(const Vector2& position, const Vector2& size) const;
    
    // Movement queries
    bool isMoving() const { return movementState.isMoving; }
    bool isRunning() const { return movementState.isRunning; }
    float getCurrentSpeed() const { return movementState.currentSpeed; }
    float getMaxSpeed() const;
    
    // Event callbacks
    void setOnPositionChanged(std::function<void(const Vector2&, const Vector2&)> callback);
    void setOnDirectionChanged(std::function<void(Character::Direction, Character::Direction)> callback);
    void setOnStateChanged(std::function<void(Character::State, Character::State)> callback);
    void setOnCollision(std::function<void(const CollisionInfo&)> callback);
    
    // Debug and diagnostics
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const { return debugMode; }
    std::vector<std::string> getDebugInfo() const;
    void logMovementState() const;
    
private:
    TownState& townState;
    MovementConfig config;
    MovementState movementState;
    MovementState previousState;
    
    // Character size (for collision detection)
    Vector2 characterSize;
    
    // Debug
    bool debugMode;
    
    // Event callbacks
    std::function<void(const Vector2&, const Vector2&)> onPositionChanged;
    std::function<void(Character::Direction, Character::Direction)> onDirectionChanged;
    std::function<void(Character::State, Character::State)> onStateChanged;
    std::function<void(const CollisionInfo&)> onCollision;
    
    // Internal update methods
    void updateInput(const InputState& input);
    void updateMovement(float deltaTime);
    void updateCollisions();
    void updateCharacterState();
    void updateFacingDirection(const Vector2& inputDirection);
    
    // Input processing
    Vector2 getMovementInput(const InputState& input) const;
    bool isRunInput(const InputState& input) const;
    
    // Movement calculations
    Vector2 calculateDesiredVelocity(const Vector2& inputDirection, float deltaTime) const;
    Vector2 applyAcceleration(const Vector2& currentVelocity, const Vector2& targetVelocity, float deltaTime) const;
    Vector2 applyDeceleration(const Vector2& currentVelocity, float deltaTime) const;
    
    // Collision helpers
    CollisionInfo performCollisionCheck(const Vector2& position, const Vector2& size) const;
    Vector2 slideAlongSurface(const Vector2& velocity, const Vector2& normal) const;
    bool isPointInBuilding(const Vector2& point, const Building& building) const;
    
    // State management
    void updateMovementState(const Vector2& newPosition, const Vector2& newVelocity);
    void triggerCallbacks();
    
    // Utility methods
    float calculateSpeed(const Vector2& velocity) const;
    Character::Direction vectorToDirection(const Vector2& direction) const;
    Vector2 directionToVector(Character::Direction direction) const;
    
    // Constants
    static constexpr float MIN_MOVEMENT_THRESHOLD = 0.1f;
    static constexpr float COLLISION_EPSILON = 0.1f;
    static const Vector2 DEFAULT_CHARACTER_SIZE;
};