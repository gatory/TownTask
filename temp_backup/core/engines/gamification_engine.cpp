#include "gamification_engine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

// Achievement implementation
GamificationEngine::Achievement::Achievement(const std::string& id, const std::string& name, 
                                           const std::string& desc, AchievementType type, int target)
    : id(id), name(name), description(desc), type(type), targetValue(target), 
      currentProgress(0), unlocked(false) {}

float GamificationEngine::Achievement::getProgressPercentage() const {
    if (targetValue == 0) return 100.0f;
    return std::min(100.0f, (static_cast<float>(currentProgress) / targetValue) * 100.0f);
}

// Reward implementation
GamificationEngine::Reward::Reward(RewardType type, int amount, const std::string& desc, const std::string& condition)
    : type(type), amount(amount), description(desc), unlockCondition(condition) {}

// BuildingUpgrade implementation
GamificationEngine::BuildingUpgrade::BuildingUpgrade(BuildingType building, int level, int cost, 
                                                   const std::string& name, const std::string& desc)
    : building(building), level(level), cost(cost), name(name), description(desc), unlocked(false) {}

// Decoration implementation
GamificationEngine::Decoration::Decoration(const std::string& id, const std::string& name, 
                                          const std::string& desc, BuildingType building, int cost)
    : id(id), name(name), description(desc), building(building), cost(cost), unlocked(false), purchased(false) {}

// Statistics implementation
GamificationEngine::Statistics::Statistics()
    : totalTasksCompleted(0), totalHabitsCompleted(0), totalPomodorosCompleted(0),
      longestHabitStreak(0), totalExperienceEarned(0), totalCoffeeTokensEarned(0),
      totalCoffeeTokensSpent(0), achievementsUnlocked(0), buildingUpgradesPurchased(0),
      decorationsPurchased(0), consecutiveLoginDays(0) {
    auto now = std::chrono::system_clock::now();
    firstLoginDate = now;
    lastLoginDate = now;
}

// GamificationEngine implementation
GamificationEngine::GamificationEngine()
    : currentExperience(0), currentLevel(1), coffeeTokens(0), levelUpFlag(false), dailyRewardClaimed(false) {
    
    // Initialize building levels
    buildingLevels[BuildingType::COFFEE_SHOP] = 1;
    buildingLevels[BuildingType::BULLETIN_BOARD] = 1;
    buildingLevels[BuildingType::LIBRARY] = 1;
    buildingLevels[BuildingType::GYM] = 1;
    buildingLevels[BuildingType::HOME] = 1;
    
    initializeAchievements();
    initializeBuildingUpgrades();
    initializeDecorations();
}

// Experience and leveling
void GamificationEngine::awardExperience(int points, const std::string& reason) {
    if (points <= 0) return;
    
    int oldLevel = currentLevel;
    currentExperience += points;
    statistics.totalExperienceEarned += points;
    
    checkLevelUp();
    
    if (currentLevel > oldLevel) {
        triggerLevelUpCallback(oldLevel, currentLevel);
    }
    
    Reward reward(RewardType::EXPERIENCE_POINTS, points, 
                 reason.empty() ? "Experience gained" : reason);
    triggerRewardCallback(reward);
    
    checkAchievements();
}

int GamificationEngine::getCurrentLevel() const {
    return currentLevel;
}

int GamificationEngine::getCurrentExperience() const {
    return currentExperience;
}

int GamificationEngine::getExperienceForLevel(int level) const {
    if (level <= 1) return 0;
    // Exponential growth: level^2 * 100
    return static_cast<int>(std::pow(level - 1, 2) * 100);
}

int GamificationEngine::getExperienceToNextLevel() const {
    int nextLevelExp = getTotalExperienceRequired(currentLevel + 1);
    return nextLevelExp - currentExperience;
}

int GamificationEngine::getTotalExperienceRequired(int level) const {
    if (level <= 1) return 0;
    
    int totalExp = 0;
    for (int i = 2; i <= level; i++) {
        totalExp += getExperienceForLevel(i);
    }
    return totalExp;
}

bool GamificationEngine::hasLeveledUp() const {
    return levelUpFlag;
}

void GamificationEngine::clearLevelUpFlag() {
    levelUpFlag = false;
}

// Task completion rewards
int GamificationEngine::calculateTaskReward(int priority, bool hasSubtasks, bool completedOnTime) const {
    int baseReward = 50; // Base experience for completing a task
    
    // Priority multiplier
    switch (priority) {
        case 0: baseReward *= 1; break;    // Low priority
        case 1: baseReward *= 2; break;    // Medium priority  
        case 2: baseReward *= 3; break;    // High priority
        default: baseReward *= 1; break;
    }
    
    // Subtask bonus
    if (hasSubtasks) {
        baseReward = static_cast<int>(baseReward * 1.5f);
    }
    
    // On-time completion bonus
    if (completedOnTime) {
        baseReward = static_cast<int>(baseReward * 1.2f);
    }
    
    return baseReward;
}

void GamificationEngine::onTaskCompleted(int priority, bool hasSubtasks, bool completedOnTime) {
    int expReward = calculateTaskReward(priority, hasSubtasks, completedOnTime);
    awardExperience(expReward, "Task completed");
    
    statistics.totalTasksCompleted++;
    updateAchievementProgress("task_completer_1", statistics.totalTasksCompleted);
    updateAchievementProgress("task_completer_10", statistics.totalTasksCompleted);
    updateAchievementProgress("task_completer_100", statistics.totalTasksCompleted);
    
    checkAchievements();
}

// Habit streak rewards
int GamificationEngine::calculateHabitReward(int streakLength, bool isMilestone) const {
    int baseReward = 25; // Base experience for habit completion
    
    // Streak multiplier (caps at 2x for very long streaks)
    float streakMultiplier = 1.0f + std::min(1.0f, streakLength / 30.0f);
    baseReward = static_cast<int>(baseReward * streakMultiplier);
    
    // Milestone bonus
    if (isMilestone) {
        baseReward *= 2;
    }
    
    return baseReward;
}

void GamificationEngine::onHabitCompleted(int streakLength) {
    bool isMilestone = isStreakMilestone(streakLength);
    int expReward = calculateHabitReward(streakLength, isMilestone);
    awardExperience(expReward, "Habit completed");
    
    statistics.totalHabitsCompleted++;
    statistics.longestHabitStreak = std::max(statistics.longestHabitStreak, streakLength);
    
    updateAchievementProgress("habit_starter", statistics.totalHabitsCompleted);
    updateAchievementProgress("habit_master", statistics.longestHabitStreak);
    
    if (isMilestone) {
        triggerMilestoneCallback("habit_streak", streakLength);
    }
    
    checkAchievements();
}

// Pomodoro session rewards
int GamificationEngine::calculatePomodoroReward(int sessionLengthMinutes, bool completedFully) const {
    int baseReward = sessionLengthMinutes * 2; // 2 XP per minute
    
    if (completedFully) {
        baseReward = static_cast<int>(baseReward * 1.5f); // Bonus for full completion
        return baseReward;
    }
    
    return baseReward / 2; // Reduced reward for incomplete sessions
}

void GamificationEngine::onPomodoroCompleted(int sessionLengthMinutes, bool completedFully) {
    int expReward = calculatePomodoroReward(sessionLengthMinutes, completedFully);
    awardExperience(expReward, "Pomodoro session");
    
    if (completedFully) {
        awardCoffeeTokens(1); // Award 1 coffee token for completed session
        statistics.totalPomodorosCompleted++;
        
        updateAchievementProgress("focus_master", statistics.totalPomodorosCompleted);
        updateAchievementProgress("coffee_lover", statistics.totalCoffeeTokensEarned);
    }
    
    checkAchievements();
}

// Coffee token system
void GamificationEngine::awardCoffeeTokens(int amount) {
    if (amount <= 0) return;
    
    coffeeTokens += amount;
    statistics.totalCoffeeTokensEarned += amount;
    
    Reward reward(RewardType::COFFEE_TOKENS, amount, "Coffee tokens earned");
    triggerRewardCallback(reward);
}

int GamificationEngine::getCoffeeTokens() const {
    return coffeeTokens;
}

bool GamificationEngine::spendCoffeeTokens(int amount) {
    if (amount <= 0 || coffeeTokens < amount) {
        return false;
    }
    
    coffeeTokens -= amount;
    statistics.totalCoffeeTokensSpent += amount;
    return true;
}

// Achievement system
void GamificationEngine::checkAchievements() {
    checkTaskAchievements();
    checkHabitAchievements();
    checkPomodoroAchievements();
    checkLevelAchievements();
    checkLoginAchievements();
}

std::vector<GamificationEngine::Achievement> GamificationEngine::getUnlockedAchievements() const {
    std::vector<Achievement> unlocked;
    for (const auto& achievement : achievements) {
        if (achievement.unlocked) {
            unlocked.push_back(achievement);
        }
    }
    return unlocked;
}

std::vector<GamificationEngine::Achievement> GamificationEngine::getLockedAchievements() const {
    std::vector<Achievement> locked;
    for (const auto& achievement : achievements) {
        if (!achievement.unlocked) {
            locked.push_back(achievement);
        }
    }
    return locked;
}

std::vector<GamificationEngine::Achievement> GamificationEngine::getAllAchievements() const {
    return achievements;
}

GamificationEngine::Achievement* GamificationEngine::getAchievement(const std::string& id) {
    auto it = std::find_if(achievements.begin(), achievements.end(),
                          [&id](const Achievement& a) { return a.id == id; });
    return it != achievements.end() ? &(*it) : nullptr;
}

bool GamificationEngine::isAchievementUnlocked(const std::string& id) const {
    auto it = std::find_if(achievements.begin(), achievements.end(),
                          [&id](const Achievement& a) { return a.id == id; });
    return it != achievements.end() && it->unlocked;
}

void GamificationEngine::updateAchievementProgress(const std::string& id, int progress) {
    Achievement* achievement = getAchievement(id);
    if (achievement && !achievement->unlocked) {
        achievement->currentProgress = progress;
        if (achievement->isComplete()) {
            unlockAchievement(id);
        }
    }
}

void GamificationEngine::unlockAchievement(const std::string& id) {
    Achievement* achievement = getAchievement(id);
    if (achievement && !achievement->unlocked) {
        achievement->unlocked = true;
        achievement->unlockedAt = std::chrono::system_clock::now();
        statistics.achievementsUnlocked++;
        
        // Award rewards for the achievement
        for (const auto& rewardType : achievement->rewards) {
            Reward reward(rewardType, 100, "Achievement unlocked: " + achievement->name);
            switch (rewardType) {
                case RewardType::EXPERIENCE_POINTS:
                    awardExperience(100, "Achievement: " + achievement->name);
                    break;
                case RewardType::COFFEE_TOKENS:
                    awardCoffeeTokens(5);
                    break;
                default:
                    break;
            }
            triggerRewardCallback(reward);
        }
        
        triggerAchievementCallback(*achievement);
    }
}

// Building upgrade system
std::vector<GamificationEngine::BuildingUpgrade> GamificationEngine::getAvailableUpgrades(BuildingType building) const {
    std::vector<BuildingUpgrade> available;
    int currentLevel = getBuildingLevel(building);
    
    for (const auto& [id, upgrade] : buildingUpgrades) {
        if (upgrade.building == building && upgrade.level == currentLevel + 1) {
            available.push_back(upgrade);
        }
    }
    
    return available;
}

std::vector<GamificationEngine::BuildingUpgrade> GamificationEngine::getUnlockedUpgrades(BuildingType building) const {
    std::vector<BuildingUpgrade> unlocked;
    
    for (const auto& [id, upgrade] : buildingUpgrades) {
        if (upgrade.building == building && upgrade.unlocked) {
            unlocked.push_back(upgrade);
        }
    }
    
    return unlocked;
}

bool GamificationEngine::canAffordUpgrade(const BuildingUpgrade& upgrade) const {
    return coffeeTokens >= upgrade.cost;
}

bool GamificationEngine::purchaseUpgrade(const std::string& upgradeId) {
    auto it = buildingUpgrades.find(upgradeId);
    if (it == buildingUpgrades.end() || it->second.unlocked) {
        return false;
    }
    
    const BuildingUpgrade& upgrade = it->second;
    if (!canAffordUpgrade(upgrade)) {
        return false;
    }
    
    if (spendCoffeeTokens(upgrade.cost)) {
        it->second.unlocked = true;
        buildingLevels[upgrade.building] = upgrade.level;
        statistics.buildingUpgradesPurchased++;
        
        Reward reward(RewardType::BUILDING_UPGRADE, 1, 
                     "Building upgraded: " + upgrade.name);
        triggerRewardCallback(reward);
        
        return true;
    }
    
    return false;
}

int GamificationEngine::getBuildingLevel(BuildingType building) const {
    auto it = buildingLevels.find(building);
    return it != buildingLevels.end() ? it->second : 1;
}

bool GamificationEngine::isBuildingUpgradeUnlocked(BuildingType building, int level) const {
    for (const auto& [id, upgrade] : buildingUpgrades) {
        if (upgrade.building == building && upgrade.level == level) {
            return upgrade.unlocked;
        }
    }
    return false;
}

// Decoration system
std::vector<GamificationEngine::Decoration> GamificationEngine::getAvailableDecorations(BuildingType building) const {
    std::vector<Decoration> available;
    
    for (const auto& decoration : decorations) {
        if (decoration.building == building && decoration.unlocked && !decoration.purchased) {
            available.push_back(decoration);
        }
    }
    
    return available;
}

std::vector<GamificationEngine::Decoration> GamificationEngine::getPurchasedDecorations(BuildingType building) const {
    std::vector<Decoration> purchased;
    
    for (const auto& decoration : decorations) {
        if (decoration.building == building && decoration.purchased) {
            purchased.push_back(decoration);
        }
    }
    
    return purchased;
}

bool GamificationEngine::purchaseDecoration(const std::string& decorationId) {
    auto it = std::find_if(decorations.begin(), decorations.end(),
                          [&decorationId](const Decoration& d) { return d.id == decorationId; });
    
    if (it == decorations.end() || it->purchased || !it->unlocked) {
        return false;
    }
    
    if (spendCoffeeTokens(it->cost)) {
        it->purchased = true;
        statistics.decorationsPurchased++;
        
        Reward reward(RewardType::DECORATION_UNLOCK, 1, 
                     "Decoration purchased: " + it->name);
        triggerRewardCallback(reward);
        
        return true;
    }
    
    return false;
}

bool GamificationEngine::isDecorationUnlocked(const std::string& decorationId) const {
    auto it = std::find_if(decorations.begin(), decorations.end(),
                          [&decorationId](const Decoration& d) { return d.id == decorationId; });
    return it != decorations.end() && it->unlocked;
}

// Statistics and tracking
const GamificationEngine::Statistics& GamificationEngine::getStatistics() const {
    return statistics;
}

void GamificationEngine::updateStatistics() {
    // This method can be called periodically to update derived statistics
    // Most statistics are updated in real-time when events occur
}

void GamificationEngine::recordLogin() {
    auto now = std::chrono::system_clock::now();
    
    if (isSameDay(now, statistics.lastLoginDate)) {
        return; // Already logged in today
    }
    
    int daysSinceLastLogin = daysBetween(statistics.lastLoginDate, now);
    
    if (daysSinceLastLogin == 1) {
        // Consecutive day
        statistics.consecutiveLoginDays++;
    } else {
        // Streak broken
        statistics.consecutiveLoginDays = 1;
    }
    
    statistics.lastLoginDate = now;
    
    // Check for login achievements
    updateAchievementProgress("daily_user", statistics.consecutiveLoginDays);
    
    checkAchievements();
}

// Milestone detection
bool GamificationEngine::isLevelMilestone(int level) const {
    return level % 5 == 0 || level == 10 || level == 25 || level == 50 || level == 100;
}

bool GamificationEngine::isStreakMilestone(int streak) const {
    return streak == 7 || streak == 30 || streak == 100 || streak == 365;
}

std::vector<std::string> GamificationEngine::getMilestoneRewards(int level) const {
    std::vector<std::string> rewards;
    
    if (level % 10 == 0) {
        rewards.push_back("Building upgrade unlocked");
        rewards.push_back("New decorations available");
    }
    
    if (level % 5 == 0) {
        rewards.push_back("Coffee token bonus");
    }
    
    return rewards;
}

// Daily rewards and bonuses
void GamificationEngine::processDailyLogin() {
    auto now = std::chrono::system_clock::now();
    
    if (!isSameDay(now, lastDailyRewardClaim)) {
        dailyRewardClaimed = false;
    }
}

bool GamificationEngine::hasClaimedDailyReward() const {
    return dailyRewardClaimed;
}

GamificationEngine::Reward GamificationEngine::getDailyReward() const {
    int bonusMultiplier = std::min(7, statistics.consecutiveLoginDays);
    int expAmount = 50 * bonusMultiplier;
    
    return Reward(RewardType::EXPERIENCE_POINTS, expAmount, 
                 "Daily login bonus (Day " + std::to_string(statistics.consecutiveLoginDays) + ")");
}

void GamificationEngine::claimDailyReward() {
    if (!dailyRewardClaimed) {
        Reward dailyReward = getDailyReward();
        awardExperience(dailyReward.amount, dailyReward.description);
        
        dailyRewardClaimed = true;
        lastDailyRewardClaim = std::chrono::system_clock::now();
    }
}

// Event callbacks
void GamificationEngine::setOnLevelUp(std::function<void(int, int)> callback) {
    onLevelUp = std::move(callback);
}

void GamificationEngine::setOnAchievementUnlocked(std::function<void(const Achievement&)> callback) {
    onAchievementUnlocked = std::move(callback);
}

void GamificationEngine::setOnRewardEarned(std::function<void(const Reward&)> callback) {
    onRewardEarned = std::move(callback);
}

void GamificationEngine::setOnMilestoneReached(std::function<void(const std::string&, int)> callback) {
    onMilestoneReached = std::move(callback);
}

// Data management
void GamificationEngine::loadData(const nlohmann::json& data) {
    if (data.contains("currentExperience")) {
        currentExperience = data["currentExperience"];
    }
    
    if (data.contains("currentLevel")) {
        currentLevel = data["currentLevel"];
    }
    
    if (data.contains("coffeeTokens")) {
        coffeeTokens = data["coffeeTokens"];
    }
    
    // Load statistics
    if (data.contains("statistics")) {
        const auto& stats = data["statistics"];
        statistics.totalTasksCompleted = stats.value("totalTasksCompleted", 0);
        statistics.totalHabitsCompleted = stats.value("totalHabitsCompleted", 0);
        statistics.totalPomodorosCompleted = stats.value("totalPomodorosCompleted", 0);
        statistics.longestHabitStreak = stats.value("longestHabitStreak", 0);
        statistics.totalExperienceEarned = stats.value("totalExperienceEarned", 0);
        statistics.totalCoffeeTokensEarned = stats.value("totalCoffeeTokensEarned", 0);
        statistics.totalCoffeeTokensSpent = stats.value("totalCoffeeTokensSpent", 0);
        statistics.achievementsUnlocked = stats.value("achievementsUnlocked", 0);
        statistics.buildingUpgradesPurchased = stats.value("buildingUpgradesPurchased", 0);
        statistics.decorationsPurchased = stats.value("decorationsPurchased", 0);
        statistics.consecutiveLoginDays = stats.value("consecutiveLoginDays", 0);
    }
    
    // Load achievements
    if (data.contains("achievements")) {
        for (const auto& achData : data["achievements"]) {
            std::string id = achData["id"];
            Achievement* achievement = getAchievement(id);
            if (achievement) {
                achievement->currentProgress = achData.value("currentProgress", 0);
                achievement->unlocked = achData.value("unlocked", false);
            }
        }
    }
    
    // Load building levels
    if (data.contains("buildingLevels")) {
        for (const auto& [buildingStr, level] : data["buildingLevels"].items()) {
            // Convert string to BuildingType enum
            if (buildingStr == "coffee_shop") buildingLevels[BuildingType::COFFEE_SHOP] = level;
            else if (buildingStr == "bulletin_board") buildingLevels[BuildingType::BULLETIN_BOARD] = level;
            else if (buildingStr == "library") buildingLevels[BuildingType::LIBRARY] = level;
            else if (buildingStr == "gym") buildingLevels[BuildingType::GYM] = level;
            else if (buildingStr == "home") buildingLevels[BuildingType::HOME] = level;
        }
    }
}

nlohmann::json GamificationEngine::saveData() const {
    nlohmann::json data;
    
    data["currentExperience"] = currentExperience;
    data["currentLevel"] = currentLevel;
    data["coffeeTokens"] = coffeeTokens;
    
    // Save statistics
    data["statistics"] = {
        {"totalTasksCompleted", statistics.totalTasksCompleted},
        {"totalHabitsCompleted", statistics.totalHabitsCompleted},
        {"totalPomodorosCompleted", statistics.totalPomodorosCompleted},
        {"longestHabitStreak", statistics.longestHabitStreak},
        {"totalExperienceEarned", statistics.totalExperienceEarned},
        {"totalCoffeeTokensEarned", statistics.totalCoffeeTokensEarned},
        {"totalCoffeeTokensSpent", statistics.totalCoffeeTokensSpent},
        {"achievementsUnlocked", statistics.achievementsUnlocked},
        {"buildingUpgradesPurchased", statistics.buildingUpgradesPurchased},
        {"decorationsPurchased", statistics.decorationsPurchased},
        {"consecutiveLoginDays", statistics.consecutiveLoginDays}
    };
    
    // Save achievements
    nlohmann::json achievementsData = nlohmann::json::array();
    for (const auto& achievement : achievements) {
        achievementsData.push_back({
            {"id", achievement.id},
            {"currentProgress", achievement.currentProgress},
            {"unlocked", achievement.unlocked}
        });
    }
    data["achievements"] = achievementsData;
    
    // Save building levels
    data["buildingLevels"] = {
        {"coffee_shop", buildingLevels.at(BuildingType::COFFEE_SHOP)},
        {"bulletin_board", buildingLevels.at(BuildingType::BULLETIN_BOARD)},
        {"library", buildingLevels.at(BuildingType::LIBRARY)},
        {"gym", buildingLevels.at(BuildingType::GYM)},
        {"home", buildingLevels.at(BuildingType::HOME)}
    };
    
    return data;
}

void GamificationEngine::resetProgress() {
    currentExperience = 0;
    currentLevel = 1;
    coffeeTokens = 0;
    levelUpFlag = false;
    statistics = Statistics();
    
    // Reset achievements
    for (auto& achievement : achievements) {
        achievement.currentProgress = 0;
        achievement.unlocked = false;
    }
    
    // Reset building levels
    for (auto& [building, level] : buildingLevels) {
        level = 1;
    }
    
    // Reset decorations
    for (auto& decoration : decorations) {
        decoration.purchased = false;
    }
}

// Private helper methods
void GamificationEngine::initializeAchievements() {
    achievements.clear();
    
    // Task completion achievements
    achievements.emplace_back("task_completer_1", "Getting Started", 
                             "Complete your first task", AchievementType::TASK_COMPLETION, 1);
    achievements.emplace_back("task_completer_10", "Task Warrior", 
                             "Complete 10 tasks", AchievementType::TASK_COMPLETION, 10);
    achievements.emplace_back("task_completer_100", "Task Master", 
                             "Complete 100 tasks", AchievementType::TASK_COMPLETION, 100);
    
    // Habit achievements
    achievements.emplace_back("habit_starter", "Building Habits", 
                             "Complete 10 habit check-ins", AchievementType::HABIT_STREAK, 10);
    achievements.emplace_back("habit_master", "Streak Champion", 
                             "Maintain a 30-day habit streak", AchievementType::HABIT_STREAK, 30);
    
    // Pomodoro achievements
    achievements.emplace_back("focus_master", "Focus Champion", 
                             "Complete 25 Pomodoro sessions", AchievementType::POMODORO_SESSION, 25);
    achievements.emplace_back("coffee_lover", "Coffee Connoisseur", 
                             "Earn 50 coffee tokens", AchievementType::POMODORO_SESSION, 50);
    
    // Level achievements
    achievements.emplace_back("level_10", "Rising Star", 
                             "Reach level 10", AchievementType::LEVEL_MILESTONE, 10);
    achievements.emplace_back("level_25", "Productivity Pro", 
                             "Reach level 25", AchievementType::LEVEL_MILESTONE, 25);
    
    // Login achievements
    achievements.emplace_back("daily_user", "Consistent User", 
                             "Log in for 7 consecutive days", AchievementType::DAILY_LOGIN, 7);
    
    // Add rewards to achievements
    for (auto& achievement : achievements) {
        achievement.rewards.push_back(RewardType::EXPERIENCE_POINTS);
        achievement.rewards.push_back(RewardType::COFFEE_TOKENS);
    }
}

void GamificationEngine::initializeBuildingUpgrades() {
    buildingUpgrades.clear();
    
    // Coffee Shop upgrades
    buildingUpgrades["coffee_shop_2"] = BuildingUpgrade(BuildingType::COFFEE_SHOP, 2, 10, 
                                                       "Espresso Machine", "Upgrade to a professional espresso machine");
    buildingUpgrades["coffee_shop_3"] = BuildingUpgrade(BuildingType::COFFEE_SHOP, 3, 25, 
                                                       "Cozy Seating", "Add comfortable seating area");
    
    // Bulletin Board upgrades
    buildingUpgrades["bulletin_board_2"] = BuildingUpgrade(BuildingType::BULLETIN_BOARD, 2, 15, 
                                                          "Digital Display", "Upgrade to a digital task board");
    buildingUpgrades["bulletin_board_3"] = BuildingUpgrade(BuildingType::BULLETIN_BOARD, 3, 30, 
                                                          "Smart Organization", "Add AI-powered task organization");
    
    // Library upgrades
    buildingUpgrades["library_2"] = BuildingUpgrade(BuildingType::LIBRARY, 2, 12, 
                                                   "Reading Nook", "Add a comfortable reading corner");
    buildingUpgrades["library_3"] = BuildingUpgrade(BuildingType::LIBRARY, 3, 28, 
                                                   "Digital Archive", "Upgrade to digital note management");
    
    // Gym upgrades
    buildingUpgrades["gym_2"] = BuildingUpgrade(BuildingType::GYM, 2, 18, 
                                               "Modern Equipment", "Upgrade to modern fitness equipment");
    buildingUpgrades["gym_3"] = BuildingUpgrade(BuildingType::GYM, 3, 35, 
                                               "Personal Trainer", "Add AI personal trainer features");
}

void GamificationEngine::initializeDecorations() {
    decorations.clear();
    
    // Coffee Shop decorations
    decorations.emplace_back("coffee_plants", "Coffee Plants", "Add some greenery to the coffee shop", 
                            BuildingType::COFFEE_SHOP, 5);
    decorations.emplace_back("vintage_posters", "Vintage Posters", "Decorate walls with vintage coffee posters", 
                            BuildingType::COFFEE_SHOP, 8);
    
    // Library decorations
    decorations.emplace_back("reading_lamp", "Reading Lamp", "Add a cozy reading lamp", 
                            BuildingType::LIBRARY, 6);
    decorations.emplace_back("bookshelf_expansion", "Bookshelf Expansion", "Add more bookshelves", 
                            BuildingType::LIBRARY, 12);
    
    // Gym decorations
    decorations.emplace_back("motivational_posters", "Motivational Posters", "Add inspiring workout posters", 
                            BuildingType::GYM, 7);
    decorations.emplace_back("mirror_wall", "Mirror Wall", "Install a full mirror wall", 
                            BuildingType::GYM, 15);
    
    // Unlock basic decorations
    for (auto& decoration : decorations) {
        decoration.unlocked = true;
    }
}

void GamificationEngine::checkLevelUp() {
    int newLevel = 1;
    int totalExpRequired = 0;
    
    while (totalExpRequired <= currentExperience) {
        newLevel++;
        totalExpRequired = getTotalExperienceRequired(newLevel);
    }
    
    newLevel--; // Go back to the last level we can afford
    
    if (newLevel > currentLevel) {
        currentLevel = newLevel;
        levelUpFlag = true;
        
        if (isLevelMilestone(currentLevel)) {
            triggerMilestoneCallback("level", currentLevel);
        }
    }
}

void GamificationEngine::triggerLevelUpCallback(int oldLevel, int newLevel) {
    if (onLevelUp) {
        onLevelUp(oldLevel, newLevel);
    }
}

void GamificationEngine::triggerAchievementCallback(const Achievement& achievement) {
    if (onAchievementUnlocked) {
        onAchievementUnlocked(achievement);
    }
}

void GamificationEngine::triggerRewardCallback(const Reward& reward) {
    if (onRewardEarned) {
        onRewardEarned(reward);
    }
}

void GamificationEngine::triggerMilestoneCallback(const std::string& type, int value) {
    if (onMilestoneReached) {
        onMilestoneReached(type, value);
    }
}

int GamificationEngine::getBaseExperienceForLevel(int level) const {
    return getExperienceForLevel(level);
}

bool GamificationEngine::isValidLevel(int level) const {
    return level >= 1 && level <= 100; // Max level 100
}

// Achievement checking helpers
void GamificationEngine::checkTaskAchievements() {
    updateAchievementProgress("task_completer_1", statistics.totalTasksCompleted);
    updateAchievementProgress("task_completer_10", statistics.totalTasksCompleted);
    updateAchievementProgress("task_completer_100", statistics.totalTasksCompleted);
}

void GamificationEngine::checkHabitAchievements() {
    updateAchievementProgress("habit_starter", statistics.totalHabitsCompleted);
    updateAchievementProgress("habit_master", statistics.longestHabitStreak);
}

void GamificationEngine::checkPomodoroAchievements() {
    updateAchievementProgress("focus_master", statistics.totalPomodorosCompleted);
    updateAchievementProgress("coffee_lover", statistics.totalCoffeeTokensEarned);
}

void GamificationEngine::checkLevelAchievements() {
    updateAchievementProgress("level_10", currentLevel);
    updateAchievementProgress("level_25", currentLevel);
}

void GamificationEngine::checkLoginAchievements() {
    updateAchievementProgress("daily_user", statistics.consecutiveLoginDays);
}

// Utility methods
std::string GamificationEngine::getCurrentDateString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
    return ss.str();
}

bool GamificationEngine::isSameDay(const std::chrono::system_clock::time_point& date1, 
                                  const std::chrono::system_clock::time_point& date2) const {
    auto time1 = std::chrono::system_clock::to_time_t(date1);
    auto time2 = std::chrono::system_clock::to_time_t(date2);
    
    auto tm1 = *std::localtime(&time1);
    auto tm2 = *std::localtime(&time2);
    
    return tm1.tm_year == tm2.tm_year && tm1.tm_yday == tm2.tm_yday;
}

int GamificationEngine::daysBetween(const std::chrono::system_clock::time_point& date1,
                                   const std::chrono::system_clock::time_point& date2) const {
    auto duration = date2 - date1;
    return static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24);
}