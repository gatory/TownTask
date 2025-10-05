#pragma once

#include <string>
#include <nlohmann/json.hpp>

class Character {
public:
    struct Position {
        float x, y;
        Position(float x = 0, float y = 0) : x(x), y(y) {}
        
        // Utility methods
        float distanceTo(const Position& other) const;
        Position operator+(const Position& other) const;
        Position operator-(const Position& other) const;
        Position& operator+=(const Position& other);
        Position& operator-=(const Position& other);
        bool operator==(const Position& other) const;
        bool operator!=(const Position& other) const;
    };
    
    enum Direction { UP, DOWN, LEFT, RIGHT };
    enum State { IDLE, WALKING, INTERACTING, FOCUSED };
    
    // Constructors
    Character(const std::string& name, Position startPos = Position(0, 0));
    
    // Movement
    void move(Direction direction, float deltaTime);
    void setPosition(const Position& pos) { position = pos; }
    Position getPosition() const { return position; }
    void setMovementSpeed(float speed) { movementSpeed = speed; }
    float getMovementSpeed() const { return movementSpeed; }
    
    // State management
    void setState(State state) { currentState = state; }
    State getState() const { return currentState; }
    void setFacingDirection(Direction direction) { facingDirection = direction; }
    Direction getFacingDirection() const { return facingDirection; }
    
    // Character properties
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    
    // Gamification
    void addExperience(int points);
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNextLevel() const;
    int getExperienceForLevel(int targetLevel) const;
    bool canLevelUp() const;
    void levelUp();
    
    // Utility methods
    std::string getStateString() const;
    std::string getDirectionString() const;
    bool isMoving() const { return currentState == WALKING; }
    bool isIdle() const { return currentState == IDLE; }
    
    // Serialization
    nlohmann::json toJson() const;
    static Character fromJson(const nlohmann::json& json);
    
    // Static utility methods
    static std::string stateToString(State state);
    static State stringToState(const std::string& stateStr);
    static std::string directionToString(Direction direction);
    static Direction stringToDirection(const std::string& directionStr);
    
private:
    std::string name;
    Position position;
    Direction facingDirection;
    State currentState;
    int level;
    int experience;
    float movementSpeed; // pixels per second
    
    // Experience calculation constants
    static constexpr int BASE_EXPERIENCE_PER_LEVEL = 100;
    static constexpr float EXPERIENCE_MULTIPLIER = 1.5f;
    
    int calculateExperienceForLevel(int targetLevel) const;
};