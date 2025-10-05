#pragma once

#include "../models/character.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <memory>

class GamificationEngine {
public:
    // Achievement types
    enum class AchievementType {
        TASK_COMPLETION,
        HABIT_STREAK,
        POMODORO_SESSION,
        LEVEL_MILESTONE,
        BUILDING_UPGRADE,
        DAILY_LOGIN
    };
    
    // Reward types
    enum class RewardType {
        EXPERIENCE_POINTS,
        COFFEE_TOKENS,
        BUILDING_UPGRADE,
        DECORATION_UNLOCK,
        CHARACTER_CUSTOMIZATION
    };
    
    // Building types for upgrades
    enum class BuildingType {
        COFFEE_SHOP,
        BULLETIN_BOARD,
        LIBRARY,
        GYM,
        HOME
    };
    
    // Achievement structure
    struct Achievement {
        std::string id;
        std::string name;
        std::string description;
        AchievementType type;
        int targetValue;
        int currentProgress;
        bool unlocked;
        std::vector<RewardType> rewards;
        std::chrono::system_clock::time_point unlockedAt;
        
        Achievement() = default;
        Achievement(const std::string& id, const std::string& name, const std::string& desc, 
                   AchievementType type, int target);
        bool isComplete() const { return currentProgress >= targetValue; }
        float getProgressPercentage() const;
    };
    
    // Reward structure
    struct Reward {
        RewardType type;
        int amount;
        std::string description;
        std::string unlockCondition;
        
        Reward() = default;
        Reward(RewardType type, int amount, const std::string& desc, const std::string& condition = "");
    };
    
    // Building upgrade structure
    struct BuildingUpgrade {
        BuildingType building;
        int level;
        int cost;
        std::string name;
        std::string description;
        std::vector<std::string> visualChanges;
        bool unlocked;
        
        BuildingUpgrade() = default;
        BuildingUpgrade(BuildingType building, int level, int cost, const std::string& name, 
                       const std::string& desc);
    };
    
    // Constructor
    GamificationEngine();
    
    // Experience and leveling
    void awardExperience(int points, const std::string& reason = "");
    int getCurrentLevel() const;
    int getCurrentExperience() const;
    int getExperienceForLevel(int level) const;
    int getExperienceToNextLevel() const;
    int getTotalExperienceRequired(int level) const;
    bool hasLeveledUp() const;
    void clearLevelUpFlag();
    
    // Task completion rewards
    int calculateTaskReward(int priority, bool hasSubtasks, bool completedOnTime) const;
    void onTaskCompleted(int priority, bool hasSubtasks, bool completedOnTime);
    
    // Habit streak rewards
    int calculateHabitReward(int streakLength, bool isMilestone) const;
    void onHabitCompleted(int streakLength);
    
    // Pomodoro session rewards
    int calculatePomodoroReward(int sessionLengthMinutes, bool completedFully) const;
    void onPomodoroCompleted(int sessionLengthMinutes, bool completedFully);
    
    // Coffee token system
    void awardCoffeeTokens(int amount);
    int getCoffeeTokens() const;
    bool spendCoffeeTokens(int amount);
    
    // Achievement system
    void checkAchievements();
    std::vector<Achievement> getUnlockedAchievements() const;
    std::vector<Achievement> getLockedAchievements() const;
    std::vector<Achievement> getAllAchievements() const;
    Achievement* getAchievement(const std::string& id);
    bool isAchievementUnlocked(const std::string& id) const;
    void updateAchievementProgress(const std::string& id, int progress);
    void unlockAchievement(const std::string& id);
    
    // Building upgrade system
    std::vector<BuildingUpgrade> getAvailableUpgrades(BuildingType building) const;
    std::vector<BuildingUpgrade> getUnlockedUpgrades(BuildingType building) const;
    bool canAffordUpgrade(const BuildingUpgrade& upgrade) const;
    bool purchaseUpgrade(const std::string& upgradeId);
    int getBuildingLevel(BuildingType building) const;
    bool isBuildingUpgradeUnlocked(BuildingType building, int level) const;
    
    // Decoration system
    struct Decoration {
        std::string id;
        std::string name;
        std::string description;
        BuildingType building;
        int cost;
        bool unlocked;
        bool purchased;
        
        Decoration() = default;
        Decoration(const std::string& id, const std::string& name, const std::string& desc,
                  BuildingType building, int cost);
    };
    
    std::vector<Decoration> getAvailableDecorations(BuildingType building) const;
    std::vector<Decoration> getPurchasedDecorations(BuildingType building) const;
    bool purchaseDecoration(const std::string& decorationId);
    bool isDecorationUnlocked(const std::string& decorationId) const;
    
    // Statistics and tracking
    struct Statistics {
        int totalTasksCompleted;
        int totalHabitsCompleted;
        int totalPomodorosCompleted;
        int longestHabitStreak;
        int totalExperienceEarned;
        int totalCoffeeTokensEarned;
        int totalCoffeeTokensSpent;
        int achievementsUnlocked;
        int buildingUpgradesPurchased;
        int decorationsPurchased;
        std::chrono::system_clock::time_point firstLoginDate;
        std::chrono::system_clock::time_point lastLoginDate;
        int consecutiveLoginDays;
        
        Statistics();
    };
    
    const Statistics& getStatistics() const;
    void updateStatistics();
    void recordLogin();
    
    // Milestone detection
    bool isLevelMilestone(int level) const;
    bool isStreakMilestone(int streak) const;
    std::vector<std::string> getMilestoneRewards(int level) const;
    
    // Daily rewards and bonuses
    void processDailyLogin();
    bool hasClaimedDailyReward() const;
    Reward getDailyReward() const;
    void claimDailyReward();
    
    // Event callbacks
    void setOnLevelUp(std::function<void(int, int)> callback); // oldLevel, newLevel
    void setOnAchievementUnlocked(std::function<void(const Achievement&)> callback);
    void setOnRewardEarned(std::function<void(const Reward&)> callback);
    void setOnMilestoneReached(std::function<void(const std::string&, int)> callback);
    
    // Data management
    void loadData(const nlohmann::json& data);
    nlohmann::json saveData() const;
    void resetProgress();
    
private:
    // Core data
    int currentExperience;
    int currentLevel;
    int coffeeTokens;
    bool levelUpFlag;
    Statistics statistics;
    
    // Achievement and upgrade data
    std::vector<Achievement> achievements;
    std::unordered_map<std::string, BuildingUpgrade> buildingUpgrades;
    std::unordered_map<BuildingType, int> buildingLevels;
    std::vector<Decoration> decorations;
    
    // Daily reward tracking
    std::chrono::system_clock::time_point lastDailyRewardClaim;
    bool dailyRewardClaimed;
    
    // Event callbacks
    std::function<void(int, int)> onLevelUp;
    std::function<void(const Achievement&)> onAchievementUnlocked;
    std::function<void(const Reward&)> onRewardEarned;
    std::function<void(const std::string&, int)> onMilestoneReached;
    
    // Helper methods
    void initializeAchievements();
    void initializeBuildingUpgrades();
    void initializeDecorations();
    void checkLevelUp();
    void triggerLevelUpCallback(int oldLevel, int newLevel);
    void triggerAchievementCallback(const Achievement& achievement);
    void triggerRewardCallback(const Reward& reward);
    void triggerMilestoneCallback(const std::string& type, int value);
    
    // Experience calculation helpers
    int getBaseExperienceForLevel(int level) const;
    bool isValidLevel(int level) const;
    
    // Achievement checking helpers
    void checkTaskAchievements();
    void checkHabitAchievements();
    void checkPomodoroAchievements();
    void checkLevelAchievements();
    void checkLoginAchievements();
    
    // Utility methods
    std::string getCurrentDateString() const;
    bool isSameDay(const std::chrono::system_clock::time_point& date1, 
                   const std::chrono::system_clock::time_point& date2) const;
    int daysBetween(const std::chrono::system_clock::time_point& date1,
                    const std::chrono::system_clock::time_point& date2) const;
};