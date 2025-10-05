#include "building_upgrade_system.h"
#include <iostream>
#include <algorithm>
#include <cmath>

BuildingUpgradeSystem::BuildingUpgradeSystem(TownState& townState, GamificationEngine& gamificationEngine)
    : townState(townState), gamificationEngine(gamificationEngine) {
    
    // Initialize all buildings at level 1
    buildingLevels[BuildingType::HOME] = 1;
    buildingLevels[BuildingType::COFFEE_SHOP] = 1;
    buildingLevels[BuildingType::BULLETIN_BOARD] = 1;
    buildingLevels[BuildingType::LIBRARY] = 1;
    buildingLevels[BuildingType::GYM] = 1;
    
    // Initialize XP to 0
    buildingXP[BuildingType::HOME] = 0;
    buildingXP[BuildingType::COFFEE_SHOP] = 0;
    buildingXP[BuildingType::BULLETIN_BOARD] = 0;
    buildingXP[BuildingType::LIBRARY] = 0;
    buildingXP[BuildingType::GYM] = 0;
    
    // Initialize upgrade definitions and decorations
    initializeUpgrades();
    initializeDecorations();
    
    std::cout << "BuildingUpgradeSystem: Initialized successfully" << std::endl;
}

BuildingUpgradeSystem::~BuildingUpgradeSystem() {
    // Cleanup handled by containers
}

// Building upgrade management
bool BuildingUpgradeSystem::canUpgradeBuilding(BuildingType buildingType) const {
    int currentLevel = getBuildingLevel(buildingType);
    if (currentLevel >= MAX_BUILDING_LEVEL) {
        return false; // Already at max level
    }
    
    int currentXP = getBuildingXP(buildingType);
    int requiredXP = getRequiredXPForNextLevel(buildingType);
    int upgradeCost = getUpgradeCost(buildingType);
    
    return currentXP >= requiredXP && gamificationEngine.getCoffeeTokens() >= upgradeCost;
}

bool BuildingUpgradeSystem::upgradeBuilding(BuildingType buildingType) {
    if (!canUpgradeBuilding(buildingType)) {
        return false;
    }
    
    int upgradeCost = getUpgradeCost(buildingType);
    
    // Spend coffee tokens
    if (!gamificationEngine.spendCoffeeTokens(upgradeCost)) {
        return false;
    }
    
    // Increase building level
    buildingLevels[buildingType]++;
    int newLevel = buildingLevels[buildingType];
    
    // Unlock new decorations
    unlockDecorations();
    
    // Trigger callback
    if (onBuildingUpgraded) {
        onBuildingUpgraded(buildingType, newLevel);
    }
    
    std::cout << "BuildingUpgradeSystem: Upgraded " << (int)buildingType 
              << " to level " << newLevel << std::endl;
    
    return true;
}

int BuildingUpgradeSystem::getBuildingLevel(BuildingType buildingType) const {
    auto it = buildingLevels.find(buildingType);
    return (it != buildingLevels.end()) ? it->second : 1;
}

int BuildingUpgradeSystem::getBuildingXP(BuildingType buildingType) const {
    auto it = buildingXP.find(buildingType);
    return (it != buildingXP.end()) ? it->second : 0;
}

void BuildingUpgradeSystem::addBuildingXP(BuildingType buildingType, int xp) {
    buildingXP[buildingType] += xp;
    
    // Check if building can be upgraded
    checkForUpgrades();
}

// Decoration management
std::vector<Decoration> BuildingUpgradeSystem::getAvailableDecorations(BuildingType buildingType) const {
    std::vector<Decoration> available;
    
    int buildingLevel = getBuildingLevel(buildingType);
    
    for (const auto& pair : decorationCatalog) {
        const Decoration& decoration = pair.second;
        if (decoration.compatibleBuilding == buildingType && 
            decoration.requiredLevel <= buildingLevel) {
            available.push_back(decoration);
        }
    }
    
    return available;
}

std::vector<Decoration> BuildingUpgradeSystem::getPurchasedDecorations(BuildingType buildingType) const {
    std::vector<Decoration> purchased;
    
    for (const std::string& decorationId : purchasedDecorations) {
        auto it = decorationCatalog.find(decorationId);
        if (it != decorationCatalog.end() && it->second.compatibleBuilding == buildingType) {
            purchased.push_back(it->second);
        }
    }
    
    return purchased;
}

bool BuildingUpgradeSystem::canPurchaseDecoration(const std::string& decorationId) const {
    auto it = decorationCatalog.find(decorationId);
    if (it == decorationCatalog.end()) {
        return false;
    }
    
    const Decoration& decoration = it->second;
    
    // Check if already purchased
    if (std::find(purchasedDecorations.begin(), purchasedDecorations.end(), decorationId) != purchasedDecorations.end()) {
        return false;
    }
    
    // Check building level requirement
    int buildingLevel = getBuildingLevel(decoration.compatibleBuilding);
    if (buildingLevel < decoration.requiredLevel) {
        return false;
    }
    
    // Check cost
    return gamificationEngine.getCoffeeTokens() >= decoration.cost;
}

bool BuildingUpgradeSystem::purchaseDecoration(const std::string& decorationId) {
    if (!canPurchaseDecoration(decorationId)) {
        return false;
    }
    
    auto it = decorationCatalog.find(decorationId);
    const Decoration& decoration = it->second;
    
    // Spend coffee tokens
    if (!gamificationEngine.spendCoffeeTokens(decoration.cost)) {
        return false;
    }
    
    // Add to purchased decorations
    purchasedDecorations.push_back(decorationId);
    
    // Trigger callback
    if (onDecorationPurchased) {
        onDecorationPurchased(decorationId);
    }
    
    std::cout << "BuildingUpgradeSystem: Purchased decoration " << decorationId << std::endl;
    
    return true;
}

// Decoration placement
bool BuildingUpgradeSystem::placeDecoration(BuildingType buildingType, const std::string& decorationId, float x, float y) {
    // Check if decoration is purchased
    if (std::find(purchasedDecorations.begin(), purchasedDecorations.end(), decorationId) == purchasedDecorations.end()) {
        return false;
    }
    
    // Check if placement is valid
    if (!isDecorationPlacementValid(buildingType, decorationId, x, y)) {
        return false;
    }
    
    // Check instance limits
    auto it = decorationCatalog.find(decorationId);
    if (it != decorationCatalog.end()) {
        const Decoration& decoration = it->second;
        if (!decoration.canPlaceMultiple) {
            int currentCount = getDecorationInstanceCount(buildingType, decorationId);
            if (currentCount >= decoration.maxInstances) {
                return false;
            }
        }
    }
    
    // Place the decoration
    placedDecorations[buildingType].emplace_back(decorationId, x, y);
    
    // Trigger callback
    if (onDecorationPlaced) {
        onDecorationPlaced(buildingType, decorationId);
    }
    
    std::cout << "BuildingUpgradeSystem: Placed decoration " << decorationId 
              << " at (" << x << ", " << y << ")" << std::endl;
    
    return true;
}

bool BuildingUpgradeSystem::removeDecoration(BuildingType buildingType, const std::string& decorationId, float x, float y) {
    auto& decorations = placedDecorations[buildingType];
    
    auto it = std::find_if(decorations.begin(), decorations.end(),
        [&](const PlacedDecoration& placed) {
            return placed.decorationId == decorationId && 
                   std::abs(placed.x - x) < 5.0f && std::abs(placed.y - y) < 5.0f;
        });
    
    if (it != decorations.end()) {
        decorations.erase(it);
        return true;
    }
    
    return false;
}

bool BuildingUpgradeSystem::moveDecoration(BuildingType buildingType, const std::string& decorationId, 
                                          float oldX, float oldY, float newX, float newY) {
    auto& decorations = placedDecorations[buildingType];
    
    auto it = std::find_if(decorations.begin(), decorations.end(),
        [&](const PlacedDecoration& placed) {
            return placed.decorationId == decorationId && 
                   std::abs(placed.x - oldX) < 5.0f && std::abs(placed.y - oldY) < 5.0f;
        });
    
    if (it != decorations.end() && isDecorationPlacementValid(buildingType, decorationId, newX, newY)) {
        it->x = newX;
        it->y = newY;
        return true;
    }
    
    return false;
}

std::vector<PlacedDecoration> BuildingUpgradeSystem::getPlacedDecorations(BuildingType buildingType) const {
    auto it = placedDecorations.find(buildingType);
    return (it != placedDecorations.end()) ? it->second : std::vector<PlacedDecoration>();
}

// Visual system
std::string BuildingUpgradeSystem::getBuildingExteriorSprite(BuildingType buildingType) const {
    int level = getBuildingLevel(buildingType);
    
    // Return sprite ID based on building type and level
    std::string baseSprite;
    switch (buildingType) {
        case BuildingType::HOME: baseSprite = "home"; break;
        case BuildingType::COFFEE_SHOP: baseSprite = "coffee_shop"; break;
        case BuildingType::BULLETIN_BOARD: baseSprite = "bulletin_board"; break;
        case BuildingType::LIBRARY: baseSprite = "library"; break;
        case BuildingType::GYM: baseSprite = "gym"; break;
        default: baseSprite = "building"; break;
    }
    
    return baseSprite + "_level_" + std::to_string(level);
}

std::string BuildingUpgradeSystem::getBuildingInteriorBackground(BuildingType buildingType) const {
    int level = getBuildingLevel(buildingType);
    
    std::string baseBackground;
    switch (buildingType) {
        case BuildingType::HOME: baseBackground = "home_interior"; break;
        case BuildingType::COFFEE_SHOP: baseBackground = "coffee_interior"; break;
        case BuildingType::BULLETIN_BOARD: baseBackground = "bulletin_interior"; break;
        case BuildingType::LIBRARY: baseBackground = "library_interior"; break;
        case BuildingType::GYM: baseBackground = "gym_interior"; break;
        default: baseBackground = "interior"; break;
    }
    
    return baseBackground + "_level_" + std::to_string(level);
}

Color BuildingUpgradeSystem::getBuildingAmbientColor(BuildingType buildingType) const {
    int level = getBuildingLevel(buildingType);
    
    // Base colors for each building type
    Color baseColor;
    switch (buildingType) {
        case BuildingType::HOME: baseColor = BLUE; break;
        case BuildingType::COFFEE_SHOP: baseColor = ORANGE; break;
        case BuildingType::BULLETIN_BOARD: baseColor = YELLOW; break;
        case BuildingType::LIBRARY: baseColor = PURPLE; break;
        case BuildingType::GYM: baseColor = RED; break;
        default: baseColor = WHITE; break;
    }
    
    // Enhance color based on level
    float intensity = 0.1f + (level - 1) * 0.05f; // Gradually increase intensity
    return Fade(baseColor, intensity);
}

float BuildingUpgradeSystem::getBuildingEfficiencyBonus(BuildingType buildingType) const {
    int level = getBuildingLevel(buildingType);
    return 1.0f + (level - 1) * 0.1f; // 10% bonus per level above 1
}

// Upgrade information
BuildingUpgrade BuildingUpgradeSystem::getNextUpgrade(BuildingType buildingType) const {
    int currentLevel = getBuildingLevel(buildingType);
    int nextLevel = currentLevel + 1;
    
    auto it = upgradeDefinitions.find(buildingType);
    if (it != upgradeDefinitions.end()) {
        for (const BuildingUpgrade& upgrade : it->second) {
            if (upgrade.level == nextLevel) {
                return upgrade;
            }
        }
    }
    
    // Return default upgrade if not found
    return BuildingUpgrade(buildingType, nextLevel, 
                          BASE_XP_REQUIREMENT * (int)std::pow(XP_SCALING_FACTOR, nextLevel - 1),
                          BASE_UPGRADE_COST * (int)std::pow(COST_SCALING_FACTOR, nextLevel - 1));
}

std::vector<BuildingUpgrade> BuildingUpgradeSystem::getAllUpgrades(BuildingType buildingType) const {
    auto it = upgradeDefinitions.find(buildingType);
    return (it != upgradeDefinitions.end()) ? it->second : std::vector<BuildingUpgrade>();
}

int BuildingUpgradeSystem::getUpgradeCost(BuildingType buildingType) const {
    BuildingUpgrade nextUpgrade = getNextUpgrade(buildingType);
    return nextUpgrade.upgradeCost;
}

int BuildingUpgradeSystem::getRequiredXPForNextLevel(BuildingType buildingType) const {
    BuildingUpgrade nextUpgrade = getNextUpgrade(buildingType);
    return nextUpgrade.requiredXP;
}

// Decoration catalog
void BuildingUpgradeSystem::initializeDecorations() {
    initializeCoffeeShopDecorations();
    initializeBulletinBoardDecorations();
    initializeLibraryDecorations();
    initializeGymDecorations();
    initializeHomeDecorations();
}

Decoration BuildingUpgradeSystem::getDecoration(const std::string& decorationId) const {
    auto it = decorationCatalog.find(decorationId);
    return (it != decorationCatalog.end()) ? it->second : Decoration("", "", 0, 0, BuildingType::HOME);
}

std::vector<std::string> BuildingUpgradeSystem::getDecorationCategories() const {
    return {"furniture", "plants", "equipment", "art", "lighting"};
}

std::vector<Decoration> BuildingUpgradeSystem::getDecorationsByCategory(const std::string& category) const {
    std::vector<Decoration> decorations;
    
    for (const auto& pair : decorationCatalog) {
        if (pair.second.category == category) {
            decorations.push_back(pair.second);
        }
    }
    
    return decorations;
}

// Prestige system
bool BuildingUpgradeSystem::isBuildingMaxLevel(BuildingType buildingType) const {
    return getBuildingLevel(buildingType) >= MAX_BUILDING_LEVEL;
}

bool BuildingUpgradeSystem::hasBuildingPrestige(BuildingType buildingType) const {
    return isBuildingMaxLevel(buildingType);
}

std::string BuildingUpgradeSystem::getBuildingPrestigeEffect(BuildingType buildingType) const {
    if (!hasBuildingPrestige(buildingType)) {
        return "";
    }
    
    switch (buildingType) {
        case BuildingType::COFFEE_SHOP: return "Golden Steam Effect";
        case BuildingType::BULLETIN_BOARD: return "Sparkling Notes";
        case BuildingType::LIBRARY: return "Glowing Books";
        case BuildingType::GYM: return "Power Aura";
        case BuildingType::HOME: return "Cozy Warmth";
        default: return "Prestige Glow";
    }
}

// Event callbacks
void BuildingUpgradeSystem::setOnBuildingUpgraded(std::function<void(BuildingType, int)> callback) {
    onBuildingUpgraded = callback;
}

void BuildingUpgradeSystem::setOnDecorationPurchased(std::function<void(std::string)> callback) {
    onDecorationPurchased = callback;
}

void BuildingUpgradeSystem::setOnDecorationPlaced(std::function<void(BuildingType, std::string)> callback) {
    onDecorationPlaced = callback;
}

// Serialization
nlohmann::json BuildingUpgradeSystem::toJson() const {
    nlohmann::json json;
    
    // Building levels
    nlohmann::json levelsJson;
    for (const auto& pair : buildingLevels) {
        levelsJson[std::to_string((int)pair.first)] = pair.second;
    }
    json["buildingLevels"] = levelsJson;
    
    // Building XP
    nlohmann::json xpJson;
    for (const auto& pair : buildingXP) {
        xpJson[std::to_string((int)pair.first)] = pair.second;
    }
    json["buildingXP"] = xpJson;
    
    // Purchased decorations
    json["purchasedDecorations"] = purchasedDecorations;
    
    // Placed decorations
    nlohmann::json placedJson;
    for (const auto& pair : placedDecorations) {
        nlohmann::json buildingDecorations;
        for (const PlacedDecoration& decoration : pair.second) {
            nlohmann::json decorationJson;
            decorationJson["id"] = decoration.decorationId;
            decorationJson["x"] = decoration.x;
            decorationJson["y"] = decoration.y;
            decorationJson["rotation"] = decoration.rotation;
            decorationJson["scale"] = decoration.scale;
            decorationJson["visible"] = decoration.isVisible;
            buildingDecorations.push_back(decorationJson);
        }
        placedJson[std::to_string((int)pair.first)] = buildingDecorations;
    }
    json["placedDecorations"] = placedJson;
    
    return json;
}

void BuildingUpgradeSystem::fromJson(const nlohmann::json& json) {
    // Building levels
    if (json.contains("buildingLevels")) {
        for (const auto& pair : json["buildingLevels"].items()) {
            BuildingType buildingType = (BuildingType)std::stoi(pair.key());
            buildingLevels[buildingType] = pair.value();
        }
    }
    
    // Building XP
    if (json.contains("buildingXP")) {
        for (const auto& pair : json["buildingXP"].items()) {
            BuildingType buildingType = (BuildingType)std::stoi(pair.key());
            buildingXP[buildingType] = pair.value();
        }
    }
    
    // Purchased decorations
    if (json.contains("purchasedDecorations")) {
        purchasedDecorations = json["purchasedDecorations"];
    }
    
    // Placed decorations
    if (json.contains("placedDecorations")) {
        for (const auto& pair : json["placedDecorations"].items()) {
            BuildingType buildingType = (BuildingType)std::stoi(pair.key());
            std::vector<PlacedDecoration> decorations;
            
            for (const auto& decorationJson : pair.value()) {
                PlacedDecoration decoration(
                    decorationJson["id"],
                    decorationJson["x"],
                    decorationJson["y"]
                );
                decoration.rotation = decorationJson.value("rotation", 0.0f);
                decoration.scale = decorationJson.value("scale", 1.0f);
                decoration.isVisible = decorationJson.value("visible", true);
                decorations.push_back(decoration);
            }
            
            placedDecorations[buildingType] = decorations;
        }
    }
    
    // Unlock decorations based on current levels
    unlockDecorations();
}

// Update and maintenance
void BuildingUpgradeSystem::update(float deltaTime) {
    // Check for automatic upgrades or time-based effects
    // For now, this is a placeholder
}

void BuildingUpgradeSystem::checkForUpgrades() {
    // Check if any buildings can be upgraded and notify
    for (const auto& pair : buildingLevels) {
        BuildingType buildingType = pair.first;
        if (canUpgradeBuilding(buildingType)) {
            // Could trigger a notification here
            std::cout << "BuildingUpgradeSystem: Building " << (int)buildingType 
                      << " can be upgraded!" << std::endl;
        }
    }
}

void BuildingUpgradeSystem::unlockDecorations() {
    // Unlock decorations based on building levels
    for (auto& pair : decorationCatalog) {
        Decoration& decoration = pair.second;
        int buildingLevel = getBuildingLevel(decoration.compatibleBuilding);
        decoration.isUnlocked = (buildingLevel >= decoration.requiredLevel);
    }
}

// Internal helpers
void BuildingUpgradeSystem::initializeUpgrades() {
    initializeCoffeeShopUpgrades();
    initializeBulletinBoardUpgrades();
    initializeLibraryUpgrades();
    initializeGymUpgrades();
    initializeHomeUpgrades();
}

void BuildingUpgradeSystem::initializeCoffeeShopUpgrades() {
    std::vector<BuildingUpgrade> upgrades;
    
    // Level 2
    BuildingUpgrade level2(BuildingType::COFFEE_SHOP, 2, 500, 100);
    level2.name = "Espresso Machine";
    level2.description = "Faster pomodoro sessions with premium coffee";
    level2.efficiencyBonus = 1.1f;
    level2.newDecorations = {"espresso_machine", "coffee_plant"};
    upgrades.push_back(level2);
    
    // Level 3
    BuildingUpgrade level3(BuildingType::COFFEE_SHOP, 3, 1200, 250);
    level3.name = "Artisan Café";
    level3.description = "Beautiful interior with specialty drinks";
    level3.efficiencyBonus = 1.2f;
    level3.newDecorations = {"art_painting", "cozy_chair", "ambient_lighting"};
    upgrades.push_back(level3);
    
    // Level 4
    BuildingUpgrade level4(BuildingType::COFFEE_SHOP, 4, 2500, 500);
    level4.name = "Premium Roastery";
    level4.description = "Professional equipment and premium atmosphere";
    level4.efficiencyBonus = 1.3f;
    level4.newDecorations = {"roasting_machine", "premium_counter", "golden_accents"};
    upgrades.push_back(level4);
    
    // Level 5 (Max)
    BuildingUpgrade level5(BuildingType::COFFEE_SHOP, 5, 5000, 1000);
    level5.name = "Master Café";
    level5.description = "The ultimate coffee experience with prestige effects";
    level5.efficiencyBonus = 1.5f;
    level5.newDecorations = {"master_machine", "golden_steam", "prestige_aura"};
    upgrades.push_back(level5);
    
    upgradeDefinitions[BuildingType::COFFEE_SHOP] = upgrades;
}

void BuildingUpgradeSystem::initializeBulletinBoardUpgrades() {
    std::vector<BuildingUpgrade> upgrades;
    
    // Level 2
    BuildingUpgrade level2(BuildingType::BULLETIN_BOARD, 2, 400, 80);
    level2.name = "Digital Board";
    level2.description = "Enhanced task organization with digital features";
    level2.efficiencyBonus = 1.1f;
    level2.capacityIncrease = 5;
    upgrades.push_back(level2);
    
    // Level 3
    BuildingUpgrade level3(BuildingType::BULLETIN_BOARD, 3, 1000, 200);
    level3.name = "Smart Board";
    level3.description = "AI-powered task suggestions and automation";
    level3.efficiencyBonus = 1.2f;
    level3.capacityIncrease = 10;
    upgrades.push_back(level3);
    
    // Level 4
    BuildingUpgrade level4(BuildingType::BULLETIN_BOARD, 4, 2000, 400);
    level4.name = "Command Center";
    level4.description = "Multi-screen setup with advanced analytics";
    level4.efficiencyBonus = 1.3f;
    level4.capacityIncrease = 20;
    upgrades.push_back(level4);
    
    // Level 5 (Max)
    BuildingUpgrade level5(BuildingType::BULLETIN_BOARD, 5, 4000, 800);
    level5.name = "Mission Control";
    level5.description = "Ultimate productivity hub with holographic displays";
    level5.efficiencyBonus = 1.5f;
    level5.capacityIncrease = 50;
    upgrades.push_back(level5);
    
    upgradeDefinitions[BuildingType::BULLETIN_BOARD] = upgrades;
}

void BuildingUpgradeSystem::initializeLibraryUpgrades() {
    std::vector<BuildingUpgrade> upgrades;
    
    // Level 2
    BuildingUpgrade level2(BuildingType::LIBRARY, 2, 600, 120);
    level2.name = "Digital Archive";
    level2.description = "Digital cataloging and search capabilities";
    level2.efficiencyBonus = 1.1f;
    upgrades.push_back(level2);
    
    // Level 3
    BuildingUpgrade level3(BuildingType::LIBRARY, 3, 1500, 300);
    level3.name = "Research Center";
    level3.description = "Advanced research tools and comfortable reading areas";
    level3.efficiencyBonus = 1.2f;
    upgrades.push_back(level3);
    
    // Level 4
    BuildingUpgrade level4(BuildingType::LIBRARY, 4, 3000, 600);
    level4.name = "Knowledge Hub";
    level4.description = "AI-powered research assistant and premium collections";
    level4.efficiencyBonus = 1.3f;
    upgrades.push_back(level4);
    
    // Level 5 (Max)
    BuildingUpgrade level5(BuildingType::LIBRARY, 5, 6000, 1200);
    level5.name = "Wisdom Sanctuary";
    level5.description = "Mystical knowledge repository with ancient wisdom";
    level5.efficiencyBonus = 1.5f;
    upgrades.push_back(level5);
    
    upgradeDefinitions[BuildingType::LIBRARY] = upgrades;
}

void BuildingUpgradeSystem::initializeGymUpgrades() {
    std::vector<BuildingUpgrade> upgrades;
    
    // Level 2
    BuildingUpgrade level2(BuildingType::GYM, 2, 450, 90);
    level2.name = "Fitness Center";
    level2.description = "Modern equipment and better tracking";
    level2.efficiencyBonus = 1.1f;
    upgrades.push_back(level2);
    
    // Level 3
    BuildingUpgrade level3(BuildingType::GYM, 3, 1100, 220);
    level3.name = "Athletic Club";
    level3.description = "Premium equipment and personal training features";
    level3.efficiencyBonus = 1.2f;
    upgrades.push_back(level3);
    
    // Level 4
    BuildingUpgrade level4(BuildingType::GYM, 4, 2200, 440);
    level4.name = "Performance Lab";
    level4.description = "Scientific training methods and biometric tracking";
    level4.efficiencyBonus = 1.3f;
    upgrades.push_back(level4);
    
    // Level 5 (Max)
    BuildingUpgrade level5(BuildingType::GYM, 5, 4500, 900);
    level5.name = "Champion's Arena";
    level5.description = "Elite training facility with legendary equipment";
    level5.efficiencyBonus = 1.5f;
    upgrades.push_back(level5);
    
    upgradeDefinitions[BuildingType::GYM] = upgrades;
}

void BuildingUpgradeSystem::initializeHomeUpgrades() {
    std::vector<BuildingUpgrade> upgrades;
    
    // Level 2
    BuildingUpgrade level2(BuildingType::HOME, 2, 300, 60);
    level2.name = "Cozy Home";
    level2.description = "Comfortable living space with basic amenities";
    level2.efficiencyBonus = 1.05f;
    upgrades.push_back(level2);
    
    // Level 3
    BuildingUpgrade level3(BuildingType::HOME, 3, 800, 160);
    level3.name = "Smart Home";
    level3.description = "Automated systems and modern conveniences";
    level3.efficiencyBonus = 1.1f;
    upgrades.push_back(level3);
    
    // Level 4
    BuildingUpgrade level4(BuildingType::HOME, 4, 1600, 320);
    level4.name = "Luxury Home";
    level4.description = "Premium finishes and high-end appliances";
    level4.efficiencyBonus = 1.15f;
    upgrades.push_back(level4);
    
    // Level 5 (Max)
    BuildingUpgrade level5(BuildingType::HOME, 5, 3200, 640);
    level5.name = "Dream Home";
    level5.description = "Perfect living space with every comfort imaginable";
    level5.efficiencyBonus = 1.2f;
    upgrades.push_back(level5);
    
    upgradeDefinitions[BuildingType::HOME] = upgrades;
}void 
BuildingUpgradeSystem::initializeCoffeeShopDecorations() {
    // Plants
    Decoration plant1("coffee_plant", "Coffee Plant", 50, 2, BuildingType::COFFEE_SHOP);
    plant1.category = "plants";
    plant1.description = "A beautiful coffee plant that adds natural ambiance";
    plant1.canPlaceMultiple = true;
    plant1.maxInstances = 3;
    decorationCatalog[plant1.id] = plant1;
    
    // Equipment
    Decoration espresso("espresso_machine", "Espresso Machine", 200, 2, BuildingType::COFFEE_SHOP);
    espresso.category = "equipment";
    espresso.description = "Professional espresso machine for premium coffee";
    decorationCatalog[espresso.id] = espresso;
    
    // Furniture
    Decoration chair("cozy_chair", "Cozy Armchair", 100, 3, BuildingType::COFFEE_SHOP);
    chair.category = "furniture";
    chair.description = "Comfortable seating for customers";
    chair.canPlaceMultiple = true;
    chair.maxInstances = 4;
    decorationCatalog[chair.id] = chair;
    
    // Art
    Decoration painting("art_painting", "Coffee Art", 150, 3, BuildingType::COFFEE_SHOP);
    painting.category = "art";
    painting.description = "Beautiful coffee-themed artwork";
    painting.canPlaceMultiple = true;
    painting.maxInstances = 2;
    decorationCatalog[painting.id] = painting;
    
    // Lighting
    Decoration lighting("ambient_lighting", "Warm Lighting", 80, 3, BuildingType::COFFEE_SHOP);
    lighting.category = "lighting";
    lighting.description = "Soft, warm lighting for cozy atmosphere";
    decorationCatalog[lighting.id] = lighting;
}

void BuildingUpgradeSystem::initializeBulletinBoardDecorations() {
    // Equipment
    Decoration monitor("extra_monitor", "Additional Monitor", 120, 2, BuildingType::BULLETIN_BOARD);
    monitor.category = "equipment";
    monitor.description = "Extra screen for better task management";
    monitor.canPlaceMultiple = true;
    monitor.maxInstances = 3;
    decorationCatalog[monitor.id] = monitor;
    
    // Furniture
    Decoration desk("ergonomic_desk", "Ergonomic Desk", 180, 3, BuildingType::BULLETIN_BOARD);
    desk.category = "furniture";
    desk.description = "Comfortable and adjustable workspace";
    decorationCatalog[desk.id] = desk;
    
    // Organization
    Decoration organizer("task_organizer", "Task Organizer", 60, 2, BuildingType::BULLETIN_BOARD);
    organizer.category = "furniture";
    organizer.description = "Physical organization system for tasks";
    organizer.canPlaceMultiple = true;
    organizer.maxInstances = 2;
    decorationCatalog[organizer.id] = organizer;
}

void BuildingUpgradeSystem::initializeLibraryDecorations() {
    // Furniture
    Decoration bookshelf("premium_bookshelf", "Premium Bookshelf", 200, 2, BuildingType::LIBRARY);
    bookshelf.category = "furniture";
    bookshelf.description = "High-quality bookshelf with rare books";
    bookshelf.canPlaceMultiple = true;
    bookshelf.maxInstances = 4;
    decorationCatalog[bookshelf.id] = bookshelf;
    
    Decoration readingChair("reading_chair", "Reading Chair", 150, 3, BuildingType::LIBRARY);
    readingChair.category = "furniture";
    readingChair.description = "Comfortable chair for long reading sessions";
    readingChair.canPlaceMultiple = true;
    readingChair.maxInstances = 3;
    decorationCatalog[readingChair.id] = readingChair;
    
    // Lighting
    Decoration readingLamp("reading_lamp", "Reading Lamp", 80, 2, BuildingType::LIBRARY);
    readingLamp.category = "lighting";
    readingLamp.description = "Perfect lighting for reading and writing";
    readingLamp.canPlaceMultiple = true;
    readingLamp.maxInstances = 3;
    decorationCatalog[readingLamp.id] = readingLamp;
    
    // Art
    Decoration globe("antique_globe", "Antique Globe", 120, 4, BuildingType::LIBRARY);
    globe.category = "art";
    globe.description = "Beautiful antique globe for worldly knowledge";
    decorationCatalog[globe.id] = globe;
}

void BuildingUpgradeSystem::initializeGymDecorations() {
    // Equipment
    Decoration weights("premium_weights", "Premium Weights", 150, 2, BuildingType::GYM);
    weights.category = "equipment";
    weights.description = "High-quality weight set for strength training";
    decorationCatalog[weights.id] = weights;
    
    Decoration treadmill("advanced_treadmill", "Advanced Treadmill", 300, 3, BuildingType::GYM);
    treadmill.category = "equipment";
    treadmill.description = "State-of-the-art cardio equipment";
    decorationCatalog[treadmill.id] = treadmill;
    
    // Furniture
    Decoration bench("workout_bench", "Workout Bench", 100, 2, BuildingType::GYM);
    bench.category = "furniture";
    bench.description = "Adjustable bench for various exercises";
    bench.canPlaceMultiple = true;
    bench.maxInstances = 2;
    decorationCatalog[bench.id] = bench;
    
    // Motivation
    Decoration mirror("motivational_mirror", "Motivational Mirror", 80, 3, BuildingType::GYM);
    mirror.category = "art";
    mirror.description = "Mirror with motivational quotes";
    mirror.canPlaceMultiple = true;
    mirror.maxInstances = 3;
    decorationCatalog[mirror.id] = mirror;
}

void BuildingUpgradeSystem::initializeHomeDecorations() {
    // Furniture
    Decoration sofa("comfortable_sofa", "Comfortable Sofa", 200, 2, BuildingType::HOME);
    sofa.category = "furniture";
    sofa.description = "Cozy sofa for relaxation";
    decorationCatalog[sofa.id] = sofa;
    
    // Plants
    Decoration housePlant("house_plant", "House Plant", 40, 2, BuildingType::HOME);
    housePlant.category = "plants";
    housePlant.description = "Beautiful plant that purifies the air";
    housePlant.canPlaceMultiple = true;
    housePlant.maxInstances = 5;
    decorationCatalog[housePlant.id] = housePlant;
    
    // Art
    Decoration familyPhoto("family_photo", "Family Photo", 30, 1, BuildingType::HOME);
    familyPhoto.category = "art";
    familyPhoto.description = "Cherished family memories";
    familyPhoto.canPlaceMultiple = true;
    familyPhoto.maxInstances = 3;
    decorationCatalog[familyPhoto.id] = familyPhoto;
    
    // Lighting
    Decoration chandelier("elegant_chandelier", "Elegant Chandelier", 250, 4, BuildingType::HOME);
    chandelier.category = "lighting";
    chandelier.description = "Beautiful chandelier for elegant ambiance";
    decorationCatalog[chandelier.id] = chandelier;
}

bool BuildingUpgradeSystem::isDecorationPlacementValid(BuildingType buildingType, const std::string& decorationId, float x, float y) const {
    // Basic bounds checking (would be more sophisticated in a real implementation)
    return x >= 0 && x <= 1000 && y >= 0 && y <= 700;
}

int BuildingUpgradeSystem::getDecorationInstanceCount(BuildingType buildingType, const std::string& decorationId) const {
    auto it = placedDecorations.find(buildingType);
    if (it == placedDecorations.end()) {
        return 0;
    }
    
    int count = 0;
    for (const PlacedDecoration& decoration : it->second) {
        if (decoration.decorationId == decorationId) {
            count++;
        }
    }
    
    return count;
}