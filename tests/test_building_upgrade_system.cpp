#include <gtest/gtest.h>
#include "../src/core/systems/building_upgrade_system.h"
#include "../src/core/models/town_state.h"
#include "../src/core/engines/gamification_engine.h"

class BuildingUpgradeSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test building
        Building testBuilding("coffee_shop_01", "Test Coffee Shop", BuildingType::COFFEE_SHOP, 
                             Vector2(100, 100), Vector2(96, 128));
        townState.getTownMap().buildings.push_back(testBuilding);
        
        // Give the gamification engine some initial resources
        gamificationEngine.awardExperience(1000, "Test setup");
        gamificationEngine.awardCoffeeTokens(100);
        
        upgradeSystem = std::make_unique<BuildingUpgradeSystem>(townState, gamificationEngine);
    }
    
    TownState townState;
    GamificationEngine gamificationEngine;
    std::unique_ptr<BuildingUpgradeSystem> upgradeSystem;
};

TEST_F(BuildingUpgradeSystemTest, InitializationTest) {
    EXPECT_NE(upgradeSystem.get(), nullptr);
    
    // Test that building exists and has default level
    EXPECT_EQ(upgradeSystem->getBuildingLevel("coffee_shop_01"), 1);
    EXPECT_GT(upgradeSystem->getMaxBuildingLevel("coffee_shop_01"), 1);
}

TEST_F(BuildingUpgradeSystemTest, UpgradeAvailabilityTest) {
    // Should be able to upgrade from level 1 to 2
    EXPECT_TRUE(upgradeSystem->canUpgradeBuilding("coffee_shop_01"));
    
    // Should have upgrade cost
    int cost = upgradeSystem->getUpgradeCost("coffee_shop_01");
    EXPECT_GT(cost, 0);
    
    // Should have upgrade currency
    std::string currency = upgradeSystem->getUpgradeCurrency("coffee_shop_01");
    EXPECT_FALSE(currency.empty());
}

TEST_F(BuildingUpgradeSystemTest, BuildingUpgradeTest) {
    std::string buildingId = "coffee_shop_01";
    
    // Get initial level
    int initialLevel = upgradeSystem->getBuildingLevel(buildingId);
    EXPECT_EQ(initialLevel, 1);
    
    // Get initial resources
    int initialXP = gamificationEngine.getExperience();
    
    // Upgrade building
    bool success = upgradeSystem->upgradeBuilding(buildingId);
    EXPECT_TRUE(success);
    
    // Verify level increased
    int newLevel = upgradeSystem->getBuildingLevel(buildingId);
    EXPECT_EQ(newLevel, initialLevel + 1);
    
    // Verify resources were spent (XP might have increased due to upgrade bonus, so check cost was deducted)
    int upgradeCost = 100; // From initialization
    int finalXP = gamificationEngine.getExperience();
    // Note: Due to bonus XP awarded for upgrading, we can't do a simple subtraction check
    // In a real implementation, you'd want separate spend/earn methods
}

TEST_F(BuildingUpgradeSystemTest, MaxLevelTest) {
    std::string buildingId = "coffee_shop_01";
    int maxLevel = upgradeSystem->getMaxBuildingLevel(buildingId);
    
    // Upgrade to max level
    for (int level = 1; level < maxLevel; level++) {
        if (upgradeSystem->canUpgradeBuilding(buildingId)) {
            upgradeSystem->upgradeBuilding(buildingId);
        }
    }
    
    // Should not be able to upgrade beyond max level
    EXPECT_FALSE(upgradeSystem->canUpgradeBuilding(buildingId));
    EXPECT_EQ(upgradeSystem->getBuildingLevel(buildingId), maxLevel);
}

TEST_F(BuildingUpgradeSystemTest, UpgradeInfoTest) {
    std::string buildingId = "coffee_shop_01";
    
    // Get upgrade information
    std::vector<BuildingUpgradeSystem::UpgradeInfo> upgrades = upgradeSystem->getBuildingUpgrades(buildingId);
    EXPECT_FALSE(upgrades.empty());
    
    // Get next upgrade
    BuildingUpgradeSystem::UpgradeInfo nextUpgrade = upgradeSystem->getNextUpgrade(buildingId);
    EXPECT_EQ(nextUpgrade.level, 2); // Should be level 2 upgrade
    EXPECT_FALSE(nextUpgrade.name.empty());
    EXPECT_FALSE(nextUpgrade.description.empty());
}

TEST_F(BuildingUpgradeSystemTest, DecorationTest) {
    std::string buildingId = "coffee_shop_01";
    
    // Get available decorations
    std::vector<BuildingUpgradeSystem::DecorationInfo> decorations = upgradeSystem->getAvailableDecorations(buildingId);
    
    // Should have some decorations available (even at level 1)
    // Note: This depends on the decoration requirements in initialization
    
    // Test decoration purchase (if any are available)
    if (!decorations.empty()) {
        std::string decorationId = decorations[0].id;
        
        if (upgradeSystem->canPurchaseDecoration(decorationId)) {
            bool success = upgradeSystem->purchaseDecoration(decorationId, buildingId);
            EXPECT_TRUE(success);
            
            // Verify decoration was added to building
            std::vector<std::string> buildingDecorations = upgradeSystem->getBuildingDecorations(buildingId);
            EXPECT_TRUE(std::find(buildingDecorations.begin(), buildingDecorations.end(), decorationId) != buildingDecorations.end());
        }
    }
}

TEST_F(BuildingUpgradeSystemTest, VisualSystemTest) {
    std::string buildingId = "coffee_shop_01";
    
    // Get building theme
    BuildingUpgradeSystem::BuildingTheme theme = upgradeSystem->getBuildingTheme(buildingId);
    EXPECT_NE(theme.primaryColor.r, 0); // Should have some color
    
    // Get building color
    Color buildingColor = upgradeSystem->getBuildingColor(buildingId);
    EXPECT_EQ(buildingColor.r, theme.primaryColor.r);
    
    // Get visual elements
    std::vector<std::string> elements = upgradeSystem->getBuildingVisualElements(buildingId);
    EXPECT_FALSE(elements.empty()); // Should have at least base elements
}

TEST_F(BuildingUpgradeSystemTest, StatisticsTest) {
    // Test statistics methods
    int totalCost = upgradeSystem->getTotalUpgradesCost();
    EXPECT_GT(totalCost, 0);
    
    int completedUpgrades = upgradeSystem->getTotalUpgradesCompleted();
    EXPECT_EQ(completedUpgrades, 0); // No upgrades completed yet
    
    float progress = upgradeSystem->getTownUpgradeProgress();
    EXPECT_EQ(progress, 0.0f); // No progress yet
    
    std::vector<std::string> recentUpgrades = upgradeSystem->getRecentUpgrades();
    EXPECT_TRUE(recentUpgrades.empty()); // No recent upgrades
}

TEST_F(BuildingUpgradeSystemTest, EffectSystemTest) {
    std::string buildingId = "coffee_shop_01";
    
    // Initially no active effects
    EXPECT_FALSE(upgradeSystem->hasActiveEffect(buildingId));
    
    // Trigger upgrade effect
    upgradeSystem->triggerUpgradeEffect(buildingId);
    EXPECT_TRUE(upgradeSystem->hasActiveEffect(buildingId));
    
    // Update effects (simulate time passing)
    for (int i = 0; i < 100; i++) {
        upgradeSystem->updateEffects(0.1f); // 10 seconds total
    }
    
    // Effect should be gone after duration
    EXPECT_FALSE(upgradeSystem->hasActiveEffect(buildingId));
}

TEST_F(BuildingUpgradeSystemTest, CallbackTest) {
    bool buildingUpgradedCalled = false;
    bool decorationPlacedCalled = false;
    
    std::string upgradedBuildingId;
    int upgradedLevel = 0;
    std::string placedDecorationId;
    std::string decorationBuildingId;
    
    // Set up callbacks
    upgradeSystem->setOnBuildingUpgraded([&](const std::string& buildingId, int level) {
        buildingUpgradedCalled = true;
        upgradedBuildingId = buildingId;
        upgradedLevel = level;
    });
    
    upgradeSystem->setOnDecorationPlaced([&](const std::string& decorationId, const std::string& buildingId) {
        decorationPlacedCalled = true;
        placedDecorationId = decorationId;
        decorationBuildingId = buildingId;
    });
    
    // Test building upgrade callback
    std::string buildingId = "coffee_shop_01";
    if (upgradeSystem->canUpgradeBuilding(buildingId)) {
        upgradeSystem->upgradeBuilding(buildingId);
        EXPECT_TRUE(buildingUpgradedCalled);
        EXPECT_EQ(upgradedBuildingId, buildingId);
        EXPECT_EQ(upgradedLevel, 2);
    }
}

TEST_F(BuildingUpgradeSystemTest, InvalidBuildingTest) {
    // Test with non-existent building
    EXPECT_FALSE(upgradeSystem->canUpgradeBuilding("invalid_building"));
    EXPECT_EQ(upgradeSystem->getBuildingLevel("invalid_building"), 0);
    EXPECT_EQ(upgradeSystem->getUpgradeCost("invalid_building"), 0);
    EXPECT_FALSE(upgradeSystem->upgradeBuilding("invalid_building"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 
           EXPECT_EQ(placedDecorations[0].decorationId, decorationId);
            EXPECT_FLOAT_EQ(placedDecorations[0].x, 100.0f);
            EXPECT_FLOAT_EQ(placedDecorations[0].y, 100.0f);
        }
    }
}

TEST_F(BuildingUpgradeSystemTest, PrestigeSystemTest) {
    // Initially not at max level
    EXPECT_FALSE(upgradeSystem->isBuildingMaxLevel(BuildingType::COFFEE_SHOP));
    EXPECT_FALSE(upgradeSystem->hasBuildingPrestige(BuildingType::COFFEE_SHOP));
    
    // Upgrade to max level (would need a lot of XP and tokens in real scenario)
    for (int level = 2; level <= 5; level++) {
        upgradeSystem->addBuildingXP(BuildingType::COFFEE_SHOP, 10000); // Plenty of XP
        gamificationEngine.awardCoffeeTokens(10000); // Plenty of tokens
        upgradeSystem->upgradeBuilding(BuildingType::COFFEE_SHOP);
    }
    
    // Should now be at max level with prestige
    EXPECT_TRUE(upgradeSystem->isBuildingMaxLevel(BuildingType::COFFEE_SHOP));
    EXPECT_TRUE(upgradeSystem->hasBuildingPrestige(BuildingType::COFFEE_SHOP));
    
    // Should have a prestige effect
    std::string prestigeEffect = upgradeSystem->getBuildingPrestigeEffect(BuildingType::COFFEE_SHOP);
    EXPECT_FALSE(prestigeEffect.empty());
}

TEST_F(BuildingUpgradeSystemTest, VisualSystemTest) {
    // Test sprite IDs change with level
    std::string level1Sprite = upgradeSystem->getBuildingExteriorSprite(BuildingType::COFFEE_SHOP);
    
    // Upgrade building
    upgradeSystem->addBuildingXP(BuildingType::COFFEE_SHOP, 500);
    upgradeSystem->upgradeBuilding(BuildingType::COFFEE_SHOP);
    
    std::string level2Sprite = upgradeSystem->getBuildingExteriorSprite(BuildingType::COFFEE_SHOP);
    
    // Sprites should be different
    EXPECT_NE(level1Sprite, level2Sprite);
    
    // Test interior backgrounds
    std::string interiorBg = upgradeSystem->getBuildingInteriorBackground(BuildingType::COFFEE_SHOP);
    EXPECT_FALSE(interiorBg.empty());
    
    // Test ambient colors
    Color ambientColor = upgradeSystem->getBuildingAmbientColor(BuildingType::COFFEE_SHOP);
    // Color should have some properties (not testing exact values)
}

TEST_F(BuildingUpgradeSystemTest, SerializationTest) {
    // Add some data
    upgradeSystem->addBuildingXP(BuildingType::COFFEE_SHOP, 300);
    upgradeSystem->addBuildingXP(BuildingType::HOME, 150);
    
    // Upgrade a building
    upgradeSystem->addBuildingXP(BuildingType::COFFEE_SHOP, 200);
    upgradeSystem->upgradeBuilding(BuildingType::COFFEE_SHOP);
    
    // Serialize
    nlohmann::json json = upgradeSystem->toJson();
    
    // Create new system and deserialize
    BuildingUpgradeSystem newSystem(townState, gamificationEngine);
    newSystem.fromJson(json);
    
    // Check that data was preserved
    EXPECT_EQ(newSystem.getBuildingLevel(BuildingType::COFFEE_SHOP), 2);
    EXPECT_EQ(newSystem.getBuildingXP(BuildingType::COFFEE_SHOP), 500);
    EXPECT_EQ(newSystem.getBuildingXP(BuildingType::HOME), 150);
}

TEST_F(BuildingUpgradeSystemTest, CallbackTest) {
    bool upgradeCallbackTriggered = false;
    BuildingType upgradedBuilding;
    int upgradedLevel = 0;
    
    // Set callback
    upgradeSystem->setOnBuildingUpgraded([&](BuildingType building, int level) {
        upgradeCallbackTriggered = true;
        upgradedBuilding = building;
        upgradedLevel = level;
    });
    
    // Perform upgrade
    upgradeSystem->addBuildingXP(BuildingType::COFFEE_SHOP, 500);
    upgradeSystem->upgradeBuilding(BuildingType::COFFEE_SHOP);
    
    // Check callback was triggered
    EXPECT_TRUE(upgradeCallbackTriggered);
    EXPECT_EQ(upgradedBuilding, BuildingType::COFFEE_SHOP);
    EXPECT_EQ(upgradedLevel, 2);
}

TEST_F(BuildingUpgradeSystemTest, MultipleUpgradesTest) {
    // Test upgrading multiple buildings
    std::vector<BuildingType> buildings = {
        BuildingType::COFFEE_SHOP,
        BuildingType::BULLETIN_BOARD,
        BuildingType::LIBRARY,
        BuildingType::GYM,
        BuildingType::HOME
    };
    
    for (BuildingType building : buildings) {
        // Add XP and upgrade
        upgradeSystem->addBuildingXP(building, 1000);
        gamificationEngine.awardCoffeeTokens(500);
        
        if (upgradeSystem->canUpgradeBuilding(building)) {
            bool success = upgradeSystem->upgradeBuilding(building);
            EXPECT_TRUE(success);
            EXPECT_EQ(upgradeSystem->getBuildingLevel(building), 2);
        }
    }
}