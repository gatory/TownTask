#include "town_state.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <iostream>

// Building implementation
Building::Building(const std::string& id, const std::string& name, BuildingType type,
                  const Vector2& pos, const Vector2& size)
    : id(id), name(name), type(type), position(pos), size(size), unlocked(true), level(1) {
    
    // Set entrance position to the bottom center of the building
    entrancePosition = {pos.x + size.x / 2, pos.y + size.y};
}

bool Building::isNearEntrance(const Vector2& characterPos, float interactionDistance) const {
    return Vector2Distance(characterPos, entrancePosition) <= interactionDistance;
}

bool Building::containsPoint(const Vector2& point) const {
    return point.x >= position.x && point.x <= position.x + size.x &&
           point.y >= position.y && point.y <= position.y + size.y;
}

Vector2 Building::getCenter() const {
    return {position.x + size.x / 2, position.y + size.y / 2};
}

nlohmann::json Building::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["name"] = name;
    json["type"] = static_cast<int>(type);
    json["position"] = {{"x", position.x}, {"y", position.y}};
    json["size"] = {{"x", size.x}, {"y", size.y}};
    json["entrancePosition"] = {{"x", entrancePosition.x}, {"y", entrancePosition.y}};
    json["unlocked"] = unlocked;
    json["level"] = level;
    json["decorations"] = decorations;
    return json;
}

Building Building::fromJson(const nlohmann::json& json) {
    Building building;
    building.id = json.value("id", "");
    building.name = json.value("name", "");
    building.type = static_cast<BuildingType>(json.value("type", 0));
    
    if (json.contains("position")) {
        building.position.x = json["position"].value("x", 0.0f);
        building.position.y = json["position"].value("y", 0.0f);
    }
    
    if (json.contains("size")) {
        building.size.x = json["size"].value("x", 64.0f);
        building.size.y = json["size"].value("y", 64.0f);
    }
    
    if (json.contains("entrancePosition")) {
        building.entrancePosition.x = json["entrancePosition"].value("x", building.position.x + building.size.x / 2);
        building.entrancePosition.y = json["entrancePosition"].value("y", building.position.y + building.size.y);
    }
    
    building.unlocked = json.value("unlocked", true);
    building.level = json.value("level", 1);
    
    if (json.contains("decorations")) {
        building.decorations = json["decorations"].get<std::vector<std::string>>();
    }
    
    return building;
}

// TownMap implementation
void TownMap::addBuilding(const Building& building) {
    buildings.push_back(building);
}

void TownMap::removeBuilding(const std::string& buildingId) {
    buildings.erase(
        std::remove_if(buildings.begin(), buildings.end(),
                      [&buildingId](const Building& b) { return b.id == buildingId; }),
        buildings.end()
    );
}

Building* TownMap::findBuilding(const std::string& buildingId) {
    auto it = std::find_if(buildings.begin(), buildings.end(),
                          [&buildingId](const Building& b) { return b.id == buildingId; });
    return it != buildings.end() ? &(*it) : nullptr;
}

const Building* TownMap::findBuilding(const std::string& buildingId) const {
    auto it = std::find_if(buildings.begin(), buildings.end(),
                          [&buildingId](const Building& b) { return b.id == buildingId; });
    return it != buildings.end() ? &(*it) : nullptr;
}

Building* TownMap::findBuildingAt(const Vector2& position) {
    for (auto& building : buildings) {
        if (building.containsPoint(position)) {
            return &building;
        }
    }
    return nullptr;
}

std::vector<Building*> TownMap::getBuildingsNearPosition(const Vector2& position, float radius) {
    std::vector<Building*> nearbyBuildings;
    
    for (auto& building : buildings) {
        if (Vector2Distance(building.getCenter(), position) <= radius) {
            nearbyBuildings.push_back(&building);
        }
    }
    
    return nearbyBuildings;
}

bool TownMap::isPositionValid(const Vector2& position, const Vector2& characterSize) const {
    // Check map boundaries
    if (position.x < 0 || position.y < 0 ||
        position.x + characterSize.x > size.x ||
        position.y + characterSize.y > size.y) {
        return false;
    }
    
    // Check building collisions
    Vector2 characterCenter = {position.x + characterSize.x / 2, position.y + characterSize.y / 2};
    
    for (const auto& building : buildings) {
        if (building.containsPoint(characterCenter)) {
            return false;
        }
    }
    
    return true;
}

Vector2 TownMap::clampToMap(const Vector2& position, const Vector2& characterSize) const {
    Vector2 clampedPos = position;
    
    clampedPos.x = std::max(0.0f, std::min(clampedPos.x, size.x - characterSize.x));
    clampedPos.y = std::max(0.0f, std::min(clampedPos.y, size.y - characterSize.y));
    
    return clampedPos;
}

nlohmann::json TownMap::toJson() const {
    nlohmann::json json;
    json["size"] = {{"x", size.x}, {"y", size.y}};
    json["characterSpawnPoint"] = {{"x", characterSpawnPoint.x}, {"y", characterSpawnPoint.y}};
    json["backgroundTexture"] = backgroundTexture;
    
    nlohmann::json buildingsArray = nlohmann::json::array();
    for (const auto& building : buildings) {
        buildingsArray.push_back(building.toJson());
    }
    json["buildings"] = buildingsArray;
    
    return json;
}

TownMap TownMap::fromJson(const nlohmann::json& json) {
    TownMap townMap;
    
    if (json.contains("size")) {
        townMap.size.x = json["size"].value("x", 800.0f);
        townMap.size.y = json["size"].value("y", 600.0f);
    }
    
    if (json.contains("characterSpawnPoint")) {
        townMap.characterSpawnPoint.x = json["characterSpawnPoint"].value("x", 400.0f);
        townMap.characterSpawnPoint.y = json["characterSpawnPoint"].value("y", 300.0f);
    }
    
    townMap.backgroundTexture = json.value("backgroundTexture", "");
    
    if (json.contains("buildings")) {
        for (const auto& buildingData : json["buildings"]) {
            townMap.buildings.push_back(Building::fromJson(buildingData));
        }
    }
    
    return townMap;
}

// TownState implementation
TownState::TownState() 
    : character("Player"), currentTime(TimeOfDay::MORNING), 
      currentWeather(Weather::SUNNY), debugMode(false) {
    initializeDefaultTown();
}

void TownState::setCharacter(const Character& character) {
    this->character = character;
}

Building* TownState::findNearestBuilding(const Vector2& position, float maxDistance) {
    Building* nearest = nullptr;
    float nearestDistance = maxDistance;
    
    for (auto& building : townMap.buildings) {
        float distance = Vector2Distance(building.entrancePosition, position);
        if (distance < nearestDistance) {
            nearest = &building;
            nearestDistance = distance;
        }
    }
    
    return nearest;
}

std::vector<Building*> TownState::getBuildingsInRange(const Vector2& position, float range) {
    return townMap.getBuildingsNearPosition(position, range);
}

bool TownState::canInteractWithBuilding(const std::string& buildingId) const {
    const Building* building = townMap.findBuilding(buildingId);
    return building && building->unlocked;
}

void TownState::unlockBuilding(const std::string& buildingId) {
    Building* building = townMap.findBuilding(buildingId);
    if (building) {
        building->unlocked = true;
        
        if (debugMode) {
            std::cout << "TownState: Unlocked building '" << buildingId << "'" << std::endl;
        }
    }
}

void TownState::upgradeBuilding(const std::string& buildingId) {
    Building* building = townMap.findBuilding(buildingId);
    if (building && building->unlocked) {
        building->level++;
        
        if (debugMode) {
            std::cout << "TownState: Upgraded building '" << buildingId 
                      << "' to level " << building->level << std::endl;
        }
    }
}

void TownState::addDecorationToBuilding(const std::string& buildingId, const std::string& decorationId) {
    Building* building = townMap.findBuilding(buildingId);
    if (building && building->unlocked) {
        auto it = std::find(building->decorations.begin(), building->decorations.end(), decorationId);
        if (it == building->decorations.end()) {
            building->decorations.push_back(decorationId);
            
            if (debugMode) {
                std::cout << "TownState: Added decoration '" << decorationId 
                          << "' to building '" << buildingId << "'" << std::endl;
            }
        }
    }
}

void TownState::update(float deltaTime) {
    // Update character
    // Character update would be handled elsewhere, but we could add town-specific updates here
    
    // Update time of day (simplified)
    static float timeAccumulator = 0.0f;
    timeAccumulator += deltaTime;
    
    // Change time every 5 minutes of real time (for demo purposes)
    if (timeAccumulator >= 300.0f) {
        timeAccumulator = 0.0f;
        currentTime = static_cast<TimeOfDay>((static_cast<int>(currentTime) + 1) % 4);
        
        if (debugMode) {
            std::cout << "TownState: Time changed to " << static_cast<int>(currentTime) << std::endl;
        }
    }
}

void TownState::reset() {
    character = Character("Player");
    character.setPosition(Character::Position(townMap.characterSpawnPoint.x, townMap.characterSpawnPoint.y));
    currentTime = TimeOfDay::MORNING;
    currentWeather = Weather::SUNNY;
    
    // Reset all buildings to default state
    for (auto& building : townMap.buildings) {
        building.unlocked = (building.type == BuildingType::HOME); // Only home is unlocked by default
        building.level = 1;
        building.decorations.clear();
    }
    
    if (debugMode) {
        std::cout << "TownState: Reset to default state" << std::endl;
    }
}

void TownState::loadFromJson(const nlohmann::json& json) {
    if (json.contains("townMap")) {
        townMap = TownMap::fromJson(json["townMap"]);
    }
    
    if (json.contains("character")) {
        character = Character::fromJson(json["character"]);
    }
    
    currentTime = static_cast<TimeOfDay>(json.value("currentTime", 0));
    currentWeather = static_cast<Weather>(json.value("currentWeather", 0));
    
    if (debugMode) {
        std::cout << "TownState: Loaded from JSON" << std::endl;
    }
}

std::vector<std::string> TownState::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== TownState Debug Info ===");
    info.push_back("Map Size: " + std::to_string(townMap.size.x) + "x" + std::to_string(townMap.size.y));
    info.push_back("Character Position: (" + std::to_string(character.getPosition().x) + 
                   ", " + std::to_string(character.getPosition().y) + ")");
    info.push_back("Time of Day: " + std::to_string(static_cast<int>(currentTime)));
    info.push_back("Weather: " + std::to_string(static_cast<int>(currentWeather)));
    info.push_back("Buildings: " + std::to_string(townMap.buildings.size()));
    
    info.push_back("\n=== Buildings ===");
    for (const auto& building : townMap.buildings) {
        std::string status = building.unlocked ? "Unlocked" : "Locked";
        info.push_back(building.name + " (Level " + std::to_string(building.level) + ", " + status + ")");
    }
    
    return info;
}

void TownState::initializeDefaultTown() {
    // Set up default town map
    townMap.size = {1024, 768};
    townMap.characterSpawnPoint = {512, 400};
    townMap.backgroundTexture = "town_background.png";
    
    createDefaultBuildings();
}

void TownState::createDefaultBuildings() {
    townMap.buildings.clear();
    
    // Home (always unlocked)
    Building home("home", "Home", BuildingType::HOME, {450, 350}, {128, 96});
    home.unlocked = true;
    townMap.addBuilding(home);
    
    // Coffee Shop
    Building coffeeShop("coffee_shop", "Coffee Shop", BuildingType::COFFEE_SHOP, {200, 200}, {96, 128});
    coffeeShop.unlocked = true; // Start with coffee shop unlocked
    townMap.addBuilding(coffeeShop);
    
    // Bulletin Board
    Building bulletinBoard("bulletin_board", "Bulletin Board", BuildingType::BULLETIN_BOARD, {600, 150}, {80, 96});
    townMap.addBuilding(bulletinBoard);
    
    // Library
    Building library("library", "Library", BuildingType::LIBRARY, {750, 300}, {120, 100});
    townMap.addBuilding(library);
    
    // Gym
    Building gym("gym", "Gym", BuildingType::GYM, {150, 500}, {110, 90});
    townMap.addBuilding(gym);
    
    if (debugMode) {
        std::cout << "TownState: Created " << townMap.buildings.size() << " default buildings" << std::endl;
    }
}

bool TownState::isValidBuildingId(const std::string& buildingId) const {
    return townMap.findBuilding(buildingId) != nullptr;
}