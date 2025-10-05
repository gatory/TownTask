#pragma once

#include <raylib.h>
#include <nlohmann/json.hpp>
#include "../models/town_state.h"
#include "../engines/gamification_engine.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

struct Decoration {
    std::string id;
    std::string name;
    std::string description;
    int cost;
    int requiredLevel;
    BuildingType compatibleBuilding;
    std::string category; // "furniture", "plants", "equipment", "art", "lighting"
    bool isUnlocked;
    bool isPurchased;
    
    // Visual properties
    float width, height;
    std::string spriteId;
    Color tintColor;
    
    // Placement properties
    bool canPlaceMultiple;
    int maxInstances;
    
    Decoration() : cost(0), requiredLevel(0), compatibleBuilding(BuildingType::HOME),
                   category("furniture"), isUnlocked(false), isPurchased(false),
                   width(32), height(32), spriteId(""), tintColor(WHITE),
                   canPlaceMultiple(false), maxInstances(1) {}
    
    Decoration(const std::string& id, const std::string& name, int cost, int requiredLevel, BuildingType building)
        : id(id), name(name), cost(cost), requiredLevel(requiredLevel), compatibleBuilding(building),
          category("furniture"), isUnlocked(false), isPurchased(false),
          width(32), height(32), spriteId(""), tintColor(WHITE),
          canPlaceMultiple(false), maxInstances(1) {}
};

struct PlacedDecoration {
    std::string decorationId;
    float x, y;
    float rotation;
    float scale;
    bool isVisible;
    
    PlacedDecoration(const std::string& id, float x, float y)
        : decorationId(id), x(x), y(y), rotation(0.0f), scale(1.0f), isVisible(true) {}
};

struct BuildingUpgrade {
    BuildingType buildingType;
    int level;
    int requiredXP;
    int upgradeCost;
    std::string name;
    std::string description;
    
    // Visual improvements
    std::vector<std::string> newDecorations;
    std::string exteriorSpriteId;
    std::string interiorBackgroundId;
    Color ambientColor;
    
    // Functional improvements
    float efficiencyBonus; // Multiplier for XP/rewards
    int capacityIncrease; // Additional slots/features
    std::vector<std::string> newFeatures;
    
    BuildingUpgrade(BuildingType type, int level, int requiredXP, int cost)
        : buildingType(type), level(level), requiredXP(requiredXP), upgradeCost(cost),
          efficiencyBonus(1.0f), capacityIncrease(0), ambientColor(WHITE) {}
};

class BuildingUpgradeSystem {
public:
    BuildingUpgradeSystem(TownState& townState, GamificationEngine& gamificationEngine);
    ~BuildingUpgradeSystem();
    
    // Building upgrade management
    bool canUpgradeBuilding(BuildingType buildingType) const;
    bool upgradeBuilding(BuildingType buildingType);
    int getBuildingLevel(BuildingType buildingType) const;
    int getBuildingXP(BuildingType buildingType) const;
    void addBuildingXP(BuildingType buildingType, int xp);
    
    // Decoration management
    std::vector<Decoration> getAvailableDecorations(BuildingType buildingType) const;
    std::vector<Decoration> getPurchasedDecorations(BuildingType buildingType) const;
    bool canPurchaseDecoration(const std::string& decorationId) const;
    bool purchaseDecoration(const std::string& decorationId);
    
    // Decoration placement
    bool placeDecoration(BuildingType buildingType, const std::string& decorationId, float x, float y);
    bool removeDecoration(BuildingType buildingType, const std::string& decorationId, float x, float y);
    bool moveDecoration(BuildingType buildingType, const std::string& decorationId, float oldX, float oldY, float newX, float newY);
    std::vector<PlacedDecoration> getPlacedDecorations(BuildingType buildingType) const;
    
    // Visual system
    std::string getBuildingExteriorSprite(BuildingType buildingType) const;
    std::string getBuildingInteriorBackground(BuildingType buildingType) const;
    Color getBuildingAmbientColor(BuildingType buildingType) const;
    float getBuildingEfficiencyBonus(BuildingType buildingType) const;
    
    // Upgrade information
    BuildingUpgrade getNextUpgrade(BuildingType buildingType) const;
    std::vector<BuildingUpgrade> getAllUpgrades(BuildingType buildingType) const;
    int getUpgradeCost(BuildingType buildingType) const;
    int getRequiredXPForNextLevel(BuildingType buildingType) const;
    
    // Decoration catalog
    void initializeDecorations();
    Decoration getDecoration(const std::string& decorationId) const;
    std::vector<std::string> getDecorationCategories() const;
    std::vector<Decoration> getDecorationsByCategory(const std::string& category) const;
    
    // Prestige system
    bool isBuildingMaxLevel(BuildingType buildingType) const;
    bool hasBuildingPrestige(BuildingType buildingType) const;
    std::string getBuildingPrestigeEffect(BuildingType buildingType) const;
    
    // Event callbacks
    void setOnBuildingUpgraded(std::function<void(BuildingType, int)> callback);
    void setOnDecorationPurchased(std::function<void(std::string)> callback);
    void setOnDecorationPlaced(std::function<void(BuildingType, std::string)> callback);
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
    
    // Update and maintenance
    void update(float deltaTime);
    void checkForUpgrades();
    void unlockDecorations();

private:
    // Core references
    TownState& townState;
    GamificationEngine& gamificationEngine;
    
    // Building data
    std::unordered_map<BuildingType, int> buildingLevels;
    std::unordered_map<BuildingType, int> buildingXP;
    std::unordered_map<BuildingType, std::vector<PlacedDecoration>> placedDecorations;
    
    // Decoration catalog
    std::unordered_map<std::string, Decoration> decorationCatalog;
    std::vector<std::string> purchasedDecorations;
    
    // Upgrade definitions
    std::unordered_map<BuildingType, std::vector<BuildingUpgrade>> upgradeDefinitions;
    
    // Event callbacks
    std::function<void(BuildingType, int)> onBuildingUpgraded;
    std::function<void(std::string)> onDecorationPurchased;
    std::function<void(BuildingType, std::string)> onDecorationPlaced;
    
    // Internal helpers
    void initializeUpgrades();
    void initializeCoffeeShopUpgrades();
    void initializeBulletinBoardUpgrades();
    void initializeLibraryUpgrades();
    void initializeGymUpgrades();
    void initializeHomeUpgrades();
    
    void initializeCoffeeShopDecorations();
    void initializeBulletinBoardDecorations();
    void initializeLibraryDecorations();
    void initializeGymDecorations();
    void initializeHomeDecorations();
    
    bool isDecorationPlacementValid(BuildingType buildingType, const std::string& decorationId, float x, float y) const;
    int getDecorationInstanceCount(BuildingType buildingType, const std::string& decorationId) const;
    
    // Constants
    static constexpr int MAX_BUILDING_LEVEL = 5;
    static constexpr int BASE_UPGRADE_COST = 100;
    static constexpr int BASE_XP_REQUIREMENT = 500;
    static constexpr float XP_SCALING_FACTOR = 1.5f;
    static constexpr float COST_SCALING_FACTOR = 2.0f;
};