#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include "../src/core/engines/gamification_engine.h"

void testBasicExperienceAndLeveling() {
    std::cout << "Testing basic experience and leveling..." << std::endl;
    
    GamificationEngine engine;
    
    // Test initial state
    assert(engine.getCurrentLevel() == 1);
    assert(engine.getCurrentExperience() == 0);
    assert(!engine.hasLeveledUp());
    
    // Test awarding experience
    engine.awardExperience(50, "Test reward");
    assert(engine.getCurrentExperience() == 50);
    assert(engine.getCurrentLevel() == 1); // Should still be level 1
    
    // Award enough experience to level up
    engine.awardExperience(100, "Level up test");
    assert(engine.getCurrentExperience() == 150);
    assert(engine.getCurrentLevel() == 2); // Should be level 2 now
    assert(engine.hasLeveledUp());
    
    // Clear level up flag
    engine.clearLevelUpFlag();
    assert(!engine.hasLeveledUp());
    
    std::cout << "Basic experience and leveling test passed!" << std::endl;
}

void testTaskCompletionRewards() {
    std::cout << "Testing task completion rewards..." << std::endl;
    
    GamificationEngine engine;
    
    // Test low priority task
    int lowPriorityReward = engine.calculateTaskReward(0, false, true);
    assert(lowPriorityReward > 0);
    
    // Test high priority task should give more reward
    int highPriorityReward = engine.calculateTaskReward(2, false, true);
    assert(highPriorityReward > lowPriorityReward);
    
    // Test task with subtasks should give bonus
    int subtaskReward = engine.calculateTaskReward(1, true, true);
    int normalReward = engine.calculateTaskReward(1, false, true);
    assert(subtaskReward > normalReward);
    
    // Test on-time completion bonus
    int onTimeReward = engine.calculateTaskReward(1, false, true);
    int lateReward = engine.calculateTaskReward(1, false, false);
    assert(onTimeReward > lateReward);
    
    // Test task completion callback
    int initialExp = engine.getCurrentExperience();
    engine.onTaskCompleted(1, false, true);
    assert(engine.getCurrentExperience() > initialExp);
    
    std::cout << "Task completion rewards test passed!" << std::endl;
}

void testHabitStreakRewards() {
    std::cout << "Testing habit streak rewards..." << std::endl;
    
    GamificationEngine engine;
    
    // Test basic habit reward
    int basicReward = engine.calculateHabitReward(1, false);
    assert(basicReward > 0);
    
    // Test streak multiplier
    int streakReward = engine.calculateHabitReward(10, false);
    assert(streakReward >= basicReward);
    
    // Test milestone bonus
    int milestoneReward = engine.calculateHabitReward(7, true);
    int normalReward = engine.calculateHabitReward(7, false);
    assert(milestoneReward > normalReward);
    
    // Test habit completion callback
    int initialExp = engine.getCurrentExperience();
    engine.onHabitCompleted(5);
    assert(engine.getCurrentExperience() > initialExp);
    
    std::cout << "Habit streak rewards test passed!" << std::endl;
}

void testPomodoroRewards() {
    std::cout << "Testing Pomodoro rewards..." << std::endl;
    
    GamificationEngine engine;
    
    // Test basic Pomodoro reward
    int reward25min = engine.calculatePomodoroReward(25, true);
    assert(reward25min > 0);
    
    // Test longer session gives more reward
    int reward45min = engine.calculatePomodoroReward(45, true);
    assert(reward45min > reward25min);
    
    // Test incomplete session gives less reward
    int incompleteReward = engine.calculatePomodoroReward(25, false);
    int completeReward = engine.calculatePomodoroReward(25, true);
    assert(incompleteReward < completeReward);
    
    // Test Pomodoro completion callback
    int initialExp = engine.getCurrentExperience();
    int initialTokens = engine.getCoffeeTokens();
    
    engine.onPomodoroCompleted(25, true);
    
    assert(engine.getCurrentExperience() > initialExp);
    assert(engine.getCoffeeTokens() > initialTokens); // Should award coffee tokens
    
    std::cout << "Pomodoro rewards test passed!" << std::endl;
}

void testCoffeeTokenSystem() {
    std::cout << "Testing coffee token system..." << std::endl;
    
    GamificationEngine engine;
    
    // Test initial state
    assert(engine.getCoffeeTokens() == 0);
    
    // Test awarding tokens
    engine.awardCoffeeTokens(10);
    assert(engine.getCoffeeTokens() == 10);
    
    // Test spending tokens
    bool success = engine.spendCoffeeTokens(5);
    assert(success);
    assert(engine.getCoffeeTokens() == 5);
    
    // Test spending more than available
    bool failure = engine.spendCoffeeTokens(10);
    assert(!failure);
    assert(engine.getCoffeeTokens() == 5); // Should remain unchanged
    
    // Test spending exact amount
    bool exactSpend = engine.spendCoffeeTokens(5);
    assert(exactSpend);
    assert(engine.getCoffeeTokens() == 0);
    
    std::cout << "Coffee token system test passed!" << std::endl;
}

void testAchievementSystem() {
    std::cout << "Testing achievement system..." << std::endl;
    
    GamificationEngine engine;
    
    // Test initial state
    auto allAchievements = engine.getAllAchievements();
    assert(!allAchievements.empty());
    
    auto unlockedAchievements = engine.getUnlockedAchievements();
    assert(unlockedAchievements.empty()); // Should start with no unlocked achievements
    
    // Test getting specific achievement
    auto* achievement = engine.getAchievement("task_completer_1");
    assert(achievement != nullptr);
    assert(!achievement->unlocked);
    assert(achievement->currentProgress == 0);
    
    // Test updating achievement progress
    engine.updateAchievementProgress("task_completer_1", 1);
    assert(achievement->currentProgress == 1);
    assert(achievement->unlocked); // Should auto-unlock when target is reached
    
    // Test achievement is now in unlocked list
    unlockedAchievements = engine.getUnlockedAchievements();
    assert(unlockedAchievements.size() == 1);
    
    // Test achievement unlock status
    assert(engine.isAchievementUnlocked("task_completer_1"));
    assert(!engine.isAchievementUnlocked("task_completer_10"));
    
    std::cout << "Achievement system test passed!" << std::endl;
}

void testBuildingUpgradeSystem() {
    std::cout << "Testing building upgrade system..." << std::endl;
    
    GamificationEngine engine;
    
    // Test initial building levels
    assert(engine.getBuildingLevel(GamificationEngine::BuildingType::COFFEE_SHOP) == 1);
    assert(engine.getBuildingLevel(GamificationEngine::BuildingType::LIBRARY) == 1);
    
    // Test getting available upgrades
    auto coffeeUpgrades = engine.getAvailableUpgrades(GamificationEngine::BuildingType::COFFEE_SHOP);
    assert(!coffeeUpgrades.empty());
    
    // Test upgrade affordability without tokens
    if (!coffeeUpgrades.empty()) {
        assert(!engine.canAffordUpgrade(coffeeUpgrades[0]));
    }
    
    // Award enough tokens and test purchase
    engine.awardCoffeeTokens(50);
    if (!coffeeUpgrades.empty()) {
        assert(engine.canAffordUpgrade(coffeeUpgrades[0]));
        
        // Test purchasing upgrade
        std::string upgradeId = "coffee_shop_2"; // Assuming this exists
        bool purchased = engine.purchaseUpgrade(upgradeId);
        if (purchased) {
            assert(engine.getBuildingLevel(GamificationEngine::BuildingType::COFFEE_SHOP) == 2);
        }
    }
    
    std::cout << "Building upgrade system test passed!" << std::endl;
}

void testDecorationSystem() {
    std::cout << "Testing decoration system..." << std::endl;
    
    GamificationEngine engine;
    
    // Test getting available decorations
    auto coffeeDecorations = engine.getAvailableDecorations(GamificationEngine::BuildingType::COFFEE_SHOP);
    // Should have some decorations available
    
    // Test purchasing decoration
    engine.awardCoffeeTokens(20);
    if (!coffeeDecorations.empty()) {
        std::string decorationId = coffeeDecorations[0].id;
        bool purchased = engine.purchaseDecoration(decorationId);
        assert(purchased);
        
        // Test decoration is now in purchased list
        auto purchasedDecorations = engine.getPurchasedDecorations(GamificationEngine::BuildingType::COFFEE_SHOP);
        bool found = false;
        for (const auto& decoration : purchasedDecorations) {
            if (decoration.id == decorationId) {
                found = true;
                break;
            }
        }
        assert(found);
    }
    
    std::cout << "Decoration system test passed!" << std::endl;
}

void testMilestoneDetection() {
    std::cout << "Testing milestone detection..." << std::endl;
    
    GamificationEngine engine;
    
    // Test level milestones
    assert(engine.isLevelMilestone(5));
    assert(engine.isLevelMilestone(10));
    assert(engine.isLevelMilestone(25));
    assert(!engine.isLevelMilestone(3));
    assert(!engine.isLevelMilestone(7));
    
    // Test streak milestones
    assert(engine.isStreakMilestone(7));
    assert(engine.isStreakMilestone(30));
    assert(engine.isStreakMilestone(100));
    assert(!engine.isStreakMilestone(5));
    assert(!engine.isStreakMilestone(15));
    
    // Test milestone rewards
    auto rewards = engine.getMilestoneRewards(10);
    assert(!rewards.empty());
    
    std::cout << "Milestone detection test passed!" << std::endl;
}

void testStatisticsTracking() {
    std::cout << "Testing statistics tracking..." << std::endl;
    
    GamificationEngine engine;
    
    // Test initial statistics
    const auto& stats = engine.getStatistics();
    assert(stats.totalTasksCompleted == 0);
    assert(stats.totalHabitsCompleted == 0);
    assert(stats.totalPomodorosCompleted == 0);
    
    // Test statistics update through actions
    engine.onTaskCompleted(1, false, true);
    assert(engine.getStatistics().totalTasksCompleted == 1);
    
    engine.onHabitCompleted(5);
    assert(engine.getStatistics().totalHabitsCompleted == 1);
    
    engine.onPomodoroCompleted(25, true);
    assert(engine.getStatistics().totalPomodorosCompleted == 1);
    
    std::cout << "Statistics tracking test passed!" << std::endl;
}

void testCallbackSystem() {
    std::cout << "Testing callback system..." << std::endl;
    
    GamificationEngine engine;
    
    // Test level up callback
    bool levelUpCalled = false;
    int oldLevel = 0, newLevel = 0;
    
    engine.setOnLevelUp([&](int old, int new_level) {
        levelUpCalled = true;
        oldLevel = old;
        newLevel = new_level;
    });
    
    // Award enough experience to level up
    engine.awardExperience(200);
    assert(levelUpCalled);
    assert(oldLevel == 1);
    assert(newLevel > 1);
    
    // Test achievement unlock callback
    bool achievementCalled = false;
    std::string achievementName;
    
    engine.setOnAchievementUnlocked([&](const GamificationEngine::Achievement& achievement) {
        achievementCalled = true;
        achievementName = achievement.name;
    });
    
    // Manually unlock an achievement to test callback
    engine.unlockAchievement("task_completer_10");
    assert(achievementCalled);
    assert(!achievementName.empty());
    
    std::cout << "Callback system test passed!" << std::endl;
}

void testDataPersistence() {
    std::cout << "Testing data persistence..." << std::endl;
    
    GamificationEngine engine1;
    
    // Set up some state
    engine1.awardExperience(150);
    engine1.awardCoffeeTokens(25);
    engine1.onTaskCompleted(2, true, true);
    
    // Save data
    auto saveData = engine1.saveData();
    assert(!saveData.empty());
    
    // Create new engine and load data
    GamificationEngine engine2;
    engine2.loadData(saveData);
    
    // Verify data was loaded correctly
    assert(engine2.getCurrentExperience() == engine1.getCurrentExperience());
    assert(engine2.getCurrentLevel() == engine1.getCurrentLevel());
    assert(engine2.getCoffeeTokens() == engine1.getCoffeeTokens());
    
    std::cout << "Data persistence test passed!" << std::endl;
}

int main() {
    try {
        testBasicExperienceAndLeveling();
        testTaskCompletionRewards();
        testHabitStreakRewards();
        testPomodoroRewards();
        testCoffeeTokenSystem();
        testAchievementSystem();
        testBuildingUpgradeSystem();
        testDecorationSystem();
        testMilestoneDetection();
        testStatisticsTracking();
        testCallbackSystem();
        testDataPersistence();
        
        std::cout << "\nAll tests passed! GamificationEngine implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}