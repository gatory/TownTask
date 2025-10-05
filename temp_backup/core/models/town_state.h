#pragma once

#include <raylib.h>
#include "character.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

// Building types in the town
enum class BuildingType {
    HOME,
    COFFEE_SHOP,
    BULLETIN_BOARD,
    LIBRARY,
    GYM,
    DECORATION
};

// Building structure
struct Building {
    std::string id;
    std::string name;
    BuildingType type;
    Vector2 position;
    Vector2 size;
    Vector2 entrancePosition;
    bool unlocked;
    int level;
    std::vector<std::string> decorations;
    
    Building() : type(BuildingType::HOME), unlocked(true), level(1) {}
    Building(const std::string& id, const std::string& name, BuildingType type, 
             const Vector2& pos, const Vector2& size);
    
    // Interaction methods
    bool isNearEntrance(const Vector2& characterPos, float interactionDistance = 32.0f) const;
    bool containsPoint(const Vector2& point) const;
    Vector2 getCenter() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Building fromJson(const nlohmann::json& json);
};

// Town map configuration
struct TownMap {
    Vector2 size;
    Vector2 characterSpawnPoint;
    std::string backgroundTexture;
    std::vector<Building> buildings;
    
    TownMap() : size({800, 600}), characterSpawnPoint({400, 300}) {}
    
    // Building management
    void addBuilding(const Building& building);
    void removeBuilding(const std::string& buildingId);
    Building* findBuilding(const std::string& buildingId);
    const Building* findBuilding(const std::string& buildingId) const;
    Building* findBuildingAt(const Vector2& position);
    std::vector<Building*> getBuildingsNearPosition(const Vector2& position, float radius);
    
    // Collision detection
    bool isPositionValid(const Vector2& position, const Vector2& characterSize) const;
    Vector2 clampToMap(const Vector2& position, const Vector2& characterSize) const;
    
    // Serialization
    nlohmann::json toJson() const;
    static TownMap fromJson(const nlohmann::json& json);
};

// Complete town state
class TownState {
public:
    TownState();
    
    // Town map access
    TownMap& getTownMap() { return townMap; }
    const TownMap& getTownMap() const { return townMap; }
    
    // Character management
    void setCharacter(const Character& character);
    Character& getCharacter() { return character; }
    const Character& getCharacter() const { return character; }
    
    // Building interactions
    Building* findNearestBuilding(const Vector2& position, float maxDistance = 50.0f);
    std::vector<Building*> getBuildingsInRange(const Vector2& position, float range);
    bool canInteractWithBuilding(const std::string& buildingId) const;
    
    // Town progression
    void unlockBuilding(const std::string& buildingId);
    void upgradeBuilding(const std::string& buildingId);
    void addDecorationToBuilding(const std::string& buildingId, const std::string& decorationId);
    
    // Time and weather (for future expansion)
    enum class TimeOfDay { MORNING, AFTERNOON, EVENING, NIGHT };
    enum class Weather { SUNNY, CLOUDY, RAINY, SNOWY };
    
    void setTimeOfDay(TimeOfDay time) { currentTime = time; }
    TimeOfDay getTimeOfDay() const { return currentTime; }
    void setWeather(Weather weather) { currentWeather = weather; }
    Weather getWeather() const { return currentWeather; }
    
    // State management
    void update(float deltaTime);
    void reset();
    
    // Serialization
    nlohmann::json toJson() const;
    void loadFromJson(const nlohmann::json& json);
    
    // Debug
    void enableDebugMode(bool enable) { debugMode = enable; }
    bool isDebugModeEnabled() const { return debugMode; }
    std::vector<std::string> getDebugInfo() const;
    
private:
    TownMap townMap;
    Character character;
    TimeOfDay currentTime;
    Weather currentWeather;
    bool debugMode;
    
    // Internal methods
    void initializeDefaultTown();
    void createDefaultBuildings();
    
    // Validation
    bool isValidBuildingId(const std::string& buildingId) const;
};