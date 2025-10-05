#include <iostream>
#include "../src/core/systems/movement_system.h"
#include "../src/core/models/town_state.h"

int main() {
    std::cout << "=== Simple Movement System Test ===" << std::endl;
    
    // Create a basic town state
    TownState townState;
    
    // Create movement system
    MovementSystem movementSystem(townState);
    
    // Test basic functionality
    Vector2 testPosition(100.0f, 100.0f);
    movementSystem.setPosition(testPosition);
    
    Vector2 retrievedPosition = movementSystem.getPosition();
    std::cout << "Set position: (" << testPosition.x << ", " << testPosition.y << ")" << std::endl;
    std::cout << "Retrieved position: (" << retrievedPosition.x << ", " << retrievedPosition.y << ")" << std::endl;
    
    // Test collision detection
    Vector2 characterSize(32.0f, 32.0f);
    CollisionInfo collision = movementSystem.checkCollision(testPosition, characterSize);
    std::cout << "Collision detected: " << (collision.hasCollision ? "Yes" : "No") << std::endl;
    
    // Test position validation
    bool isValid = movementSystem.isValidPosition(testPosition, characterSize);
    std::cout << "Position is valid: " << (isValid ? "Yes" : "No") << std::endl;
    
    std::cout << "Movement system test completed successfully!" << std::endl;
    return 0;
}