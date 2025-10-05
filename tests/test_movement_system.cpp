#include <iostream>
#include <cassert>
#include <memory>
#include "../src/core/systems/movement_system.h"
#include "../src/core/models/town_state.h"
#include "../src/input/input_manager.h"

void testMovementConfig() {
    std::cout << "Testing MovementConfig..." << std::endl;
    
    MovementConfig config;
    
    // Test default values
    assert(config.baseSpeed == 150.0f);
    assert(config.runSpeedMultiplier == 1.5f);
    assert(config.acceleration == 800.0f);
    assert(config.deceleration == 1200.0f);
    assert(config.maxVelocity == 300.0f);
    assert(config.enableRunning == true);
    assert(config.enableSmoothing == true);
    assert(config.enableCollisionSliding == true);
    
    std::cout << "MovementConfig test passed!" << std::endl;
}

void testCollisionInfo() {
    std::cout << "Testing CollisionInfo..." << std::endl;
    
    // Test default collision info
    CollisionInfo info1;
    assert(!info1.hasCollision);
    assert(info1.type == CollisionInfo::CollisionType::NONE);
    
    // Test collision with building
    CollisionInfo info2(true, Vector2(100, 100), Vector2(1, 0), "building1", CollisionInfo::CollisionType::BUILDING);
    assert(info2.hasCollision);
    assert(info2.collisionPoint.x == 100 && info2.collisionPoint.y == 100);
    assert(info2.collisionNormal.x == 1 && info2.collisionNormal.y == 0);
    assert(info2.collidedObjectId == "building1");
    assert(info2.type == CollisionInfo::CollisionType::BUILDING);
    
    std::cout << "CollisionInfo test passed!" << std::endl;
}

void testMovementState() {
    std::cout << "Testing MovementState..." << std::endl;
    
    MovementState state;
    
    // Test default state
    assert(state.position.x == 0 && state.position.y == 0);
    assert(state.velocity.x == 0 && state.velocity.y == 0);
    assert(state.facingDirection == Character::Direction::DOWN);
    assert(state.characterState == Character::State::IDLE);
    assert(!state.isRunning);
    assert(!state.isMoving);
    assert(state.currentSpeed == 0.0f);
    
    // Test state with position
    MovementState state2(Vector2(100, 200));
    assert(state2.position.x == 100 && state2.position.y == 200);
    
    std::cout << "MovementState test passed!" << std::endl;
}

void testMovementSystemBasic() {
    std::cout << "Testing basic MovementSystem functionality..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    // Test initial state
    assert(movementSystem.getPosition().x >= 0 && movementSystem.getPosition().y >= 0);
    assert(movementSystem.getVelocity().x == 0 && movementSystem.getVelocity().y == 0);
    assert(!movementSystem.isMoving());
    assert(!movementSystem.isRunning());
    assert(movementSystem.getCurrentSpeed() == 0.0f);
    
    // Test configuration access
    const MovementConfig& config = movementSystem.getMovementConfig();
    assert(config.baseSpeed == 150.0f);
    
    // Test max speed calculation
    float maxSpeed = movementSystem.getMaxSpeed();
    assert(maxSpeed == config.baseSpeed);
    
    std::cout << "Basic MovementSystem test passed!" << std::endl;
}

void testMovementSystemPositionControl() {
    std::cout << "Testing MovementSystem position control..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    // Test setting position
    Vector2 testPos(300, 400);
    movementSystem.setPosition(testPos);
    
    Vector2 currentPos = movementSystem.getPosition();
    assert(std::abs(currentPos.x - testPos.x) < 0.1f);
    assert(std::abs(currentPos.y - testPos.y) < 0.1f);
    
    // Test teleport
    Vector2 teleportPos(500, 300);
    movementSystem.teleport(teleportPos);
    
    currentPos = movementSystem.getPosition();
    assert(std::abs(currentPos.x - teleportPos.x) < 0.1f);
    assert(std::abs(currentPos.y - teleportPos.y) < 0.1f);
    assert(movementSystem.getVelocity().length() == 0.0f); // Should reset velocity
    
    // Test stop
    movementSystem.setVelocity(Vector2(100, 100));
    assert(movementSystem.getVelocity().length() > 0);
    
    movementSystem.stop();
    assert(movementSystem.getVelocity().length() == 0.0f);
    assert(!movementSystem.isMoving());
    
    std::cout << "MovementSystem position control test passed!" << std::endl;
}

void testMovementSystemConfiguration() {
    std::cout << "Testing MovementSystem configuration..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    // Test setting base speed
    movementSystem.setBaseSpeed(200.0f);
    assert(movementSystem.getMovementConfig().baseSpeed == 200.0f);
    assert(movementSystem.getMaxSpeed() == 200.0f);
    
    // Test enabling/disabling running
    movementSystem.enableRunning(false);
    assert(!movementSystem.getMovementConfig().enableRunning);
    
    movementSystem.enableRunning(true);
    assert(movementSystem.getMovementConfig().enableRunning);
    
    // Test smoothing
    movementSystem.enableSmoothing(false);
    assert(!movementSystem.getMovementConfig().enableSmoothing);
    
    // Test collision sliding
    movementSystem.enableCollisionSliding(false);
    assert(!movementSystem.getMovementConfig().enableCollisionSliding);
    
    // Test custom config
    MovementConfig customConfig;
    customConfig.baseSpeed = 100.0f;
    customConfig.enableRunning = false;
    
    movementSystem.setMovementConfig(customConfig);
    assert(movementSystem.getMovementConfig().baseSpeed == 100.0f);
    assert(!movementSystem.getMovementConfig().enableRunning);
    
    std::cout << "MovementSystem configuration test passed!" << std::endl;
}

void testCollisionDetection() {
    std::cout << "Testing collision detection..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    Vector2 characterSize(32, 32);
    
    // Test valid position (should be no collision)
    Vector2 validPos(50, 50);
    CollisionInfo collision = movementSystem.checkCollision(validPos, characterSize);
    assert(!collision.hasCollision);
    
    // Test boundary collision
    Vector2 invalidPos(-10, 50); // Outside left boundary
    collision = movementSystem.checkCollisionWithBoundaries(invalidPos, characterSize);
    assert(collision.hasCollision);
    assert(collision.type == CollisionInfo::CollisionType::BOUNDARY);
    assert(collision.collidedObjectId == "left_boundary");
    
    // Test position validation
    assert(movementSystem.isValidPosition(validPos, characterSize));
    assert(!movementSystem.isValidPosition(invalidPos, characterSize));
    
    // Test boundary clamping
    Vector2 clampedPos = movementSystem.clampToBoundaries(invalidPos, characterSize);
    assert(clampedPos.x == 0); // Should be clamped to 0
    assert(clampedPos.y == 50); // Y should remain unchanged
    
    std::cout << "Collision detection test passed!" << std::endl;
}

void testMovementSystemCallbacks() {
    std::cout << "Testing MovementSystem callbacks..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    // Test position changed callback
    bool positionChanged = false;
    Vector2 oldPos, newPos;
    
    movementSystem.setOnPositionChanged([&](const Vector2& old, const Vector2& new_pos) {
        positionChanged = true;
        oldPos = old;
        newPos = new_pos;
    });
    
    Vector2 initialPos = movementSystem.getPosition();
    movementSystem.setPosition(Vector2(300, 300));
    
    assert(positionChanged);
    assert(oldPos.distance(initialPos) < 0.1f);
    assert(newPos.distance(Vector2(300, 300)) < 0.1f);
    
    // Test direction changed callback
    bool directionChanged = false;
    Character::Direction oldDir, newDir;
    
    movementSystem.setOnDirectionChanged([&](Character::Direction old, Character::Direction new_dir) {
        directionChanged = true;
        oldDir = old;
        newDir = new_dir;
    });
    
    // Simulate input that would change direction
    InputState input;
    input.actionStates[InputAction::MOVE_RIGHT] = true;
    
    Character::Direction initialDir = movementSystem.getFacingDirection();
    movementSystem.update(0.016f, input);
    
    if (movementSystem.getFacingDirection() != initialDir) {
        assert(directionChanged);
    }
    
    std::cout << "MovementSystem callbacks test passed!" << std::endl;
}

void testMovementSystemDebug() {
    std::cout << "Testing MovementSystem debug functionality..." << std::endl;
    
    TownState townState;
    MovementSystem movementSystem(townState);
    
    // Test debug mode
    movementSystem.enableDebugMode(true);
    assert(movementSystem.isDebugModeEnabled());
    
    // Test debug info
    auto debugInfo = movementSystem.getDebugInfo();
    assert(!debugInfo.empty());
    
    // Should contain position and velocity information
    bool foundPositionInfo = false;
    bool foundVelocityInfo = false;
    
    for (const auto& line : debugInfo) {
        if (line.find("Position:") != std::string::npos) {
            foundPositionInfo = true;
        }
        if (line.find("Velocity:") != std::string::npos) {
            foundVelocityInfo = true;
        }
    }
    
    assert(foundPositionInfo);
    assert(foundVelocityInfo);
    
    std::cout << "MovementSystem debug test passed!" << std::endl;
}

void testMovementSystemIntegration() {
    std::cout << "Testing MovementSystem integration with input..." << std::endl;
    
    TownState townState;
    InputManager inputManager;
    MovementSystem movementSystem(townState);
    
    inputManager.initialize();
    
    // Create input state for movement
    InputState input;
    input.actionStates[InputAction::MOVE_RIGHT] = true;
    
    Vector2 initialPos = movementSystem.getPosition();
    
    // Update movement system with input
    movementSystem.update(0.016f, input);
    
    Vector2 newPos = movementSystem.getPosition();
    
    // Character should have moved right
    assert(newPos.x > initialPos.x);
    assert(movementSystem.isMoving());
    assert(movementSystem.getFacingDirection() == Character::Direction::RIGHT);
    assert(movementSystem.getCharacterState() == Character::State::WALKING);
    
    // Test stopping
    input.actionStates[InputAction::MOVE_RIGHT] = false;
    
    // Update a few frames to let deceleration take effect
    for (int i = 0; i < 10; i++) {
        movementSystem.update(0.016f, input);
    }
    
    // Should eventually stop
    assert(movementSystem.getCurrentSpeed() < 1.0f);
    
    inputManager.shutdown();
    
    std::cout << "MovementSystem integration test passed!" << std::endl;
}

int main() {
    try {
        testMovementConfig();
        testCollisionInfo();
        testMovementState();
        testMovementSystemBasic();
        testMovementSystemPositionControl();
        testMovementSystemConfiguration();
        testCollisionDetection();
        testMovementSystemCallbacks();
        testMovementSystemDebug();
        testMovementSystemIntegration();
        
        std::cout << "\nAll tests passed! Movement system implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}