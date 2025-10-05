#include "character.h"
#include <cmath>
#include <algorithm>

// Position utility methods
float Character::Position::distanceTo(const Position& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}

Character::Position Character::Position::operator+(const Position& other) const {
    return Position(x + other.x, y + other.y);
}

Character::Position Character::Position::operator-(const Position& other) const {
    return Position(x - other.x, y - other.y);
}

Character::Position& Character::Position::operator+=(const Position& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Character::Position& Character::Position::operator-=(const Position& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

bool Character::Position::operator==(const Position& other) const {
    const float epsilon = 0.001f;
    return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
}

bool Character::Position::operator!=(const Position& other) const {
    return !(*this == other);
}

// Character implementation
Character::Character(const std::string& name, Position startPos)
    : name(name), position(startPos), facingDirection(DOWN), currentState(IDLE),
      level(1), experience(0), movementSpeed(100.0f) {
}

void Character::move(Direction direction, float deltaTime) {
    facingDirection = direction;
    currentState = WALKING;
    
    float distance = movementSpeed * deltaTime;
    
    switch (direction) {
        case UP:
            position.y -= distance;
            break;
        case DOWN:
            position.y += distance;
            break;
        case LEFT:
            position.x -= distance;
            break;
        case RIGHT:
            position.x += distance;
            break;
    }
}

void Character::addExperience(int points) {
    if (points <= 0) return;
    
    experience += points;
    
    // Check for level up
    while (canLevelUp()) {
        levelUp();
    }
}

int Character::getExperienceToNextLevel() const {
    int nextLevelExp = getExperienceForLevel(level + 1);
    return std::max(0, nextLevelExp - experience);
}

int Character::getExperienceForLevel(int targetLevel) const {
    return calculateExperienceForLevel(targetLevel);
}

bool Character::canLevelUp() const {
    return experience >= getExperienceForLevel(level + 1);
}

void Character::levelUp() {
    if (canLevelUp()) {
        level++;
        // Could add level-up effects here (sound, particles, etc.)
    }
}

std::string Character::getStateString() const {
    return stateToString(currentState);
}

std::string Character::getDirectionString() const {
    return directionToString(facingDirection);
}

int Character::calculateExperienceForLevel(int targetLevel) const {
    if (targetLevel <= 1) return 0;
    
    int totalExp = 0;
    for (int i = 2; i <= targetLevel; i++) {
        totalExp += static_cast<int>(BASE_EXPERIENCE_PER_LEVEL * std::pow(EXPERIENCE_MULTIPLIER, i - 2));
    }
    return totalExp;
}

std::string Character::stateToString(State state) {
    switch (state) {
        case IDLE: return "IDLE";
        case WALKING: return "WALKING";
        case INTERACTING: return "INTERACTING";
        case FOCUSED: return "FOCUSED";
        default: return "IDLE";
    }
}

Character::State Character::stringToState(const std::string& stateStr) {
    if (stateStr == "WALKING") return WALKING;
    if (stateStr == "INTERACTING") return INTERACTING;
    if (stateStr == "FOCUSED") return FOCUSED;
    return IDLE; // Default
}

std::string Character::directionToString(Direction direction) {
    switch (direction) {
        case UP: return "UP";
        case DOWN: return "DOWN";
        case LEFT: return "LEFT";
        case RIGHT: return "RIGHT";
        default: return "DOWN";
    }
}

Character::Direction Character::stringToDirection(const std::string& directionStr) {
    if (directionStr == "UP") return UP;
    if (directionStr == "LEFT") return LEFT;
    if (directionStr == "RIGHT") return RIGHT;
    return DOWN; // Default
}

nlohmann::json Character::toJson() const {
    nlohmann::json j;
    
    j["name"] = name;
    j["position"] = {
        {"x", position.x},
        {"y", position.y}
    };
    j["facingDirection"] = directionToString(facingDirection);
    j["currentState"] = stateToString(currentState);
    j["level"] = level;
    j["experience"] = experience;
    j["movementSpeed"] = movementSpeed;
    
    return j;
}

Character Character::fromJson(const nlohmann::json& json) {
    std::string name = json.at("name").get<std::string>();
    
    Position pos(0, 0);
    if (json.contains("position")) {
        auto posJson = json.at("position");
        pos.x = posJson.at("x").get<float>();
        pos.y = posJson.at("y").get<float>();
    }
    
    Character character(name, pos);
    
    // Set facing direction
    if (json.contains("facingDirection")) {
        character.setFacingDirection(
            stringToDirection(json.at("facingDirection").get<std::string>())
        );
    }
    
    // Set current state
    if (json.contains("currentState")) {
        character.setState(
            stringToState(json.at("currentState").get<std::string>())
        );
    }
    
    // Set level and experience
    if (json.contains("level")) {
        character.level = json.at("level").get<int>();
    }
    
    if (json.contains("experience")) {
        character.experience = json.at("experience").get<int>();
    }
    
    // Set movement speed
    if (json.contains("movementSpeed")) {
        character.setMovementSpeed(json.at("movementSpeed").get<float>());
    }
    
    return character;
}