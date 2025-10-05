#include "habit_tracker.h"
#include <algorithm>
#include <ctime>

HabitTracker::HabitTracker() = default;

// Habit management
uint32_t HabitTracker::createHabit(const std::string& name, Habit::Frequency frequency) {
    Habit newHabit(name, frequency);
    uint32_t habitId = newHabit.getId();
    
    auto [it, inserted] = habits.emplace(habitId, std::move(newHabit));
    triggerHabitCreationCallback(it->second);
    
    return habitId;
}

bool HabitTracker::updateHabit(uint32_t habitId, const Habit& updatedHabit) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    it->second = updatedHabit;
    triggerHabitUpdateCallback(it->second);
    return true;
}

bool HabitTracker::deleteHabit(uint32_t habitId) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    habits.erase(it);
    triggerHabitDeletionCallback(habitId);
    return true;
}

// Habit modification
bool HabitTracker::setHabitName(uint32_t habitId, const std::string& name) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    it->second.setName(name);
    triggerHabitUpdateCallback(it->second);
    return true;
}

bool HabitTracker::setHabitFrequency(uint32_t habitId, Habit::Frequency frequency) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    it->second.setFrequency(frequency);
    triggerHabitUpdateCallback(it->second);
    return true;
}

// Check-in functionality
bool HabitTracker::checkInHabit(uint32_t habitId) {
    return checkInHabit(habitId, std::chrono::system_clock::now());
}

bool HabitTracker::checkInHabit(uint32_t habitId, const std::chrono::system_clock::time_point& date) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    int oldStreak = it->second.getCurrentStreak();
    bool success = it->second.checkIn(date);
    
    if (success) {
        int newStreak = it->second.getCurrentStreak();
        triggerCheckInCallback(habitId, date);
        
        // Check for milestone achievements
        checkAndTriggerMilestones(habitId, oldStreak, newStreak);
        
        triggerHabitUpdateCallback(it->second);
    }
    
    return success;
}

bool HabitTracker::undoCheckIn(uint32_t habitId) {
    return undoCheckIn(habitId, std::chrono::system_clock::now());
}

bool HabitTracker::undoCheckIn(uint32_t habitId, const std::chrono::system_clock::time_point& /* date */) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    // This would require implementing an undo mechanism in the Habit class
    // For now, we'll return false as this feature isn't implemented in the Habit model
    return false; // TODO: Implement undo functionality in Habit class
}

// Streak management
bool HabitTracker::resetHabitStreak(uint32_t habitId) {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    int oldStreak = it->second.getCurrentStreak();
    it->second.resetStreak();
    
    if (oldStreak > 0) {
        triggerStreakBrokenCallback(habitId, oldStreak);
    }
    
    triggerHabitUpdateCallback(it->second);
    return true;
}

int HabitTracker::getHabitStreak(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.getCurrentStreak();
    }
    return 0;
}

bool HabitTracker::isStreakActive(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.isStreakActive();
    }
    return false;
}

// Querying
std::vector<Habit> HabitTracker::getAllHabits() const {
    std::vector<Habit> result;
    result.reserve(habits.size());
    
    for (const auto& [id, habit] : habits) {
        result.push_back(habit);
    }
    
    return result;
}

std::optional<Habit> HabitTracker::getHabit(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Habit> HabitTracker::getHabitsByFrequency(Habit::Frequency frequency) const {
    std::vector<Habit> result;
    
    for (const auto& [id, habit] : habits) {
        if (habit.getFrequency() == frequency) {
            result.push_back(habit);
        }
    }
    
    return result;
}

std::vector<Habit> HabitTracker::getActiveHabits() const {
    std::vector<Habit> result;
    
    for (const auto& [id, habit] : habits) {
        if (habit.isStreakActive()) {
            result.push_back(habit);
        }
    }
    
    return result;
}

std::vector<Habit> HabitTracker::getInactiveHabits() const {
    std::vector<Habit> result;
    
    for (const auto& [id, habit] : habits) {
        if (!habit.isStreakActive()) {
            result.push_back(habit);
        }
    }
    
    return result;
}

// Progress tracking
float HabitTracker::getHabitCompletionRate(uint32_t habitId, int days) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.getCompletionRate(days);
    }
    return 0.0f;
}

std::vector<std::chrono::system_clock::time_point> HabitTracker::getHabitCompletions(uint32_t habitId, int days) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.getRecentCompletions(days);
    }
    return {};
}

int HabitTracker::getDaysSinceLastCompletion(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.getDaysSinceLastCompletion();
    }
    return -1; // Habit not found
}

// Milestone tracking
std::vector<HabitTracker::Milestone> HabitTracker::getHabitMilestones(uint32_t habitId) const {
    std::vector<Milestone> milestones;
    auto targets = getDefaultMilestoneTargets();
    int currentStreak = getHabitStreak(habitId);
    
    for (int target : targets) {
        Milestone milestone;
        milestone.streakTarget = target;
        milestone.name = std::to_string(target) + " Day Streak";
        milestone.description = "Complete the habit for " + std::to_string(target) + " consecutive days";
        milestone.isAchieved = currentStreak >= target;
        milestones.push_back(milestone);
    }
    
    return milestones;
}

std::vector<HabitTracker::Milestone> HabitTracker::getAchievedMilestones(uint32_t habitId) const {
    auto allMilestones = getHabitMilestones(habitId);
    std::vector<Milestone> achieved;
    
    for (const auto& milestone : allMilestones) {
        if (milestone.isAchieved) {
            achieved.push_back(milestone);
        }
    }
    
    return achieved;
}

std::vector<HabitTracker::Milestone> HabitTracker::getUpcomingMilestones(uint32_t habitId) const {
    auto allMilestones = getHabitMilestones(habitId);
    std::vector<Milestone> upcoming;
    
    for (const auto& milestone : allMilestones) {
        if (!milestone.isAchieved) {
            upcoming.push_back(milestone);
        }
    }
    
    return upcoming;
}

bool HabitTracker::hasReachedMilestone(uint32_t habitId, int streakTarget) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        return it->second.hasReachedMilestone(streakTarget);
    }
    return false;
}

// Statistics and analytics
HabitTracker::HabitStats HabitTracker::getHabitStatistics(uint32_t habitId) const {
    HabitStats stats = {};
    
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return stats;
    }
    
    const auto& habit = it->second;
    stats.habitId = habitId;
    stats.name = habit.getName();
    stats.currentStreak = habit.getCurrentStreak();
    stats.longestStreak = habit.getLongestStreak();
    stats.totalCompletions = habit.getTotalCompletions();
    stats.completionRate30Days = habit.getCompletionRate(30);
    stats.completionRate7Days = habit.getCompletionRate(7);
    stats.daysSinceLastCompletion = habit.getDaysSinceLastCompletion();
    stats.isActive = habit.isStreakActive();
    
    return stats;
}

std::vector<HabitTracker::HabitStats> HabitTracker::getAllHabitStatistics() const {
    std::vector<HabitStats> allStats;
    allStats.reserve(habits.size());
    
    for (const auto& [id, habit] : habits) {
        allStats.push_back(getHabitStatistics(id));
    }
    
    return allStats;
}

// Filtering and sorting
std::vector<Habit> HabitTracker::getSortedHabits(SortBy sortBy) const {
    std::vector<Habit> result = getAllHabits();
    
    switch (sortBy) {
        case SortBy::NAME_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getName() < b.getName(); });
            break;
        case SortBy::NAME_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getName() > b.getName(); });
            break;
        case SortBy::STREAK_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCurrentStreak() < b.getCurrentStreak(); });
            break;
        case SortBy::STREAK_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCurrentStreak() > b.getCurrentStreak(); });
            break;
        case SortBy::COMPLETION_RATE_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCompletionRate(30) < b.getCompletionRate(30); });
            break;
        case SortBy::COMPLETION_RATE_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCompletionRate(30) > b.getCompletionRate(30); });
            break;
        case SortBy::CREATED_DATE_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCreatedAt() < b.getCreatedAt(); });
            break;
        case SortBy::CREATED_DATE_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { return a.getCreatedAt() > b.getCreatedAt(); });
            break;
        case SortBy::LAST_COMPLETION_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { 
                         auto aLast = a.getLastCompleted();
                         auto bLast = b.getLastCompleted();
                         // Compare time points directly (habits with no completions will have epoch time)
                         return aLast < bLast;
                     });
            break;
        case SortBy::LAST_COMPLETION_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Habit& a, const Habit& b) { 
                         auto aLast = a.getLastCompleted();
                         auto bLast = b.getLastCompleted();
                         // Compare time points directly (habits with no completions will have epoch time)
                         return aLast > bLast;
                     });
            break;
    }
    
    return result;
}

// Calendar view support
std::vector<HabitTracker::DayStatus> HabitTracker::getHabitCalendar(uint32_t habitId, int days) const {
    std::vector<DayStatus> calendar;
    
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return calendar;
    }
    
    const auto& habit = it->second;
    auto completions = habit.getRecentCompletions(days);
    auto today = getTodayStart();
    
    for (int i = days - 1; i >= 0; i--) {
        DayStatus status;
        status.date = today - std::chrono::hours(24 * i);
        
        // Check if this day was completed
        status.completed = false;
        for (const auto& completion : completions) {
            if (isSameDay(completion, status.date)) {
                status.completed = true;
                break;
            }
        }
        
        // Determine if this day was required based on frequency
        status.required = true; // For now, assume all days are required for daily habits
        // TODO: Implement frequency-based requirement logic
        
        calendar.push_back(status);
    }
    
    return calendar;
}

std::vector<HabitTracker::DayStatus> HabitTracker::getHabitCalendarForMonth(uint32_t habitId, int /* year */, int /* month */) const {
    // This would require more complex date calculations
    // For now, return a 30-day calendar
    return getHabitCalendar(habitId, 30);
}

// Bulk operations
std::vector<uint32_t> HabitTracker::createMultipleHabits(const std::vector<std::pair<std::string, Habit::Frequency>>& habitData) {
    std::vector<uint32_t> habitIds;
    habitIds.reserve(habitData.size());
    
    for (const auto& [name, frequency] : habitData) {
        habitIds.push_back(createHabit(name, frequency));
    }
    
    return habitIds;
}

bool HabitTracker::deleteMultipleHabits(const std::vector<uint32_t>& habitIds) {
    bool allDeleted = true;
    
    for (uint32_t habitId : habitIds) {
        if (!deleteHabit(habitId)) {
            allDeleted = false;
        }
    }
    
    return allDeleted;
}

bool HabitTracker::checkInMultipleHabits(const std::vector<uint32_t>& habitIds) {
    bool allSucceeded = true;
    
    for (uint32_t habitId : habitIds) {
        if (!checkInHabit(habitId)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

bool HabitTracker::resetMultipleStreaks(const std::vector<uint32_t>& habitIds) {
    bool allSucceeded = true;
    
    for (uint32_t habitId : habitIds) {
        if (!resetHabitStreak(habitId)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

// Global statistics
size_t HabitTracker::getTotalHabitCount() const {
    return habits.size();
}

size_t HabitTracker::getActiveHabitCount() const {
    return getActiveHabits().size();
}

size_t HabitTracker::getInactiveHabitCount() const {
    return getInactiveHabits().size();
}

int HabitTracker::getTotalCompletionsToday() const {
    int count = 0;
    auto today = getTodayStart();
    
    for (const auto& [id, habit] : habits) {
        auto completions = habit.getRecentCompletions(1);
        for (const auto& completion : completions) {
            if (isSameDay(completion, today)) {
                count++;
                break; // Only count once per habit per day
            }
        }
    }
    
    return count;
}

int HabitTracker::getTotalCompletionsThisWeek() const {
    int count = 0;
    
    for (const auto& [id, habit] : habits) {
        auto completions = habit.getRecentCompletions(7);
        count += completions.size();
    }
    
    return count;
}

int HabitTracker::getTotalCompletionsThisMonth() const {
    int count = 0;
    
    for (const auto& [id, habit] : habits) {
        auto completions = habit.getRecentCompletions(30);
        count += completions.size();
    }
    
    return count;
}

float HabitTracker::getOverallCompletionRate(int days) const {
    if (habits.empty()) {
        return 0.0f;
    }
    
    float totalRate = 0.0f;
    for (const auto& [id, habit] : habits) {
        totalRate += habit.getCompletionRate(days);
    }
    
    return totalRate / habits.size();
}

// Streaks and achievements
std::vector<uint32_t> HabitTracker::getHabitsWithActiveStreaks() const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        if (habit.isStreakActive() && habit.getCurrentStreak() > 0) {
            result.push_back(id);
        }
    }
    
    return result;
}

std::vector<uint32_t> HabitTracker::getHabitsWithBrokenStreaks() const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        if (!habit.isStreakActive() && habit.getCurrentStreak() == 0) {
            result.push_back(id);
        }
    }
    
    return result;
}

int HabitTracker::getLongestCurrentStreak() const {
    int longest = 0;
    
    for (const auto& [id, habit] : habits) {
        longest = std::max(longest, habit.getCurrentStreak());
    }
    
    return longest;
}

int HabitTracker::getLongestAllTimeStreak() const {
    int longest = 0;
    
    for (const auto& [id, habit] : habits) {
        longest = std::max(longest, habit.getLongestStreak());
    }
    
    return longest;
}

std::vector<uint32_t> HabitTracker::getHabitsAtMilestone(int streakTarget) const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        if (habit.getCurrentStreak() == streakTarget) {
            result.push_back(id);
        }
    }
    
    return result;
}

// Data management
void HabitTracker::loadHabits(const std::vector<Habit>& habitList) {
    habits.clear();
    
    for (const auto& habit : habitList) {
        habits.emplace(habit.getId(), habit);
    }
}

std::vector<Habit> HabitTracker::exportHabits() const {
    return getAllHabits();
}

void HabitTracker::clearAllHabits() {
    habits.clear();
}

// Events and callbacks
void HabitTracker::setHabitCreationCallback(std::function<void(const Habit&)> callback) {
    onHabitCreated = std::move(callback);
}

void HabitTracker::setHabitUpdateCallback(std::function<void(const Habit&)> callback) {
    onHabitUpdated = std::move(callback);
}

void HabitTracker::setHabitDeletionCallback(std::function<void(uint32_t)> callback) {
    onHabitDeleted = std::move(callback);
}

void HabitTracker::setCheckInCallback(std::function<void(uint32_t, const std::chrono::system_clock::time_point&)> callback) {
    onCheckIn = std::move(callback);
}

void HabitTracker::setStreakMilestoneCallback(std::function<void(uint32_t, int)> callback) {
    onStreakMilestone = std::move(callback);
}

void HabitTracker::setStreakBrokenCallback(std::function<void(uint32_t, int)> callback) {
    onStreakBroken = std::move(callback);
}

// Validation
bool HabitTracker::habitExists(uint32_t habitId) const {
    return habits.find(habitId) != habits.end();
}

bool HabitTracker::isValidHabitId(uint32_t habitId) const {
    return habitExists(habitId);
}

bool HabitTracker::canCheckInToday(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        // For daily habits, can always check in if not already done today
        return !hasCheckedInToday(habitId);
    }
    return false;
}

bool HabitTracker::hasCheckedInToday(uint32_t habitId) const {
    auto it = habits.find(habitId);
    if (it == habits.end()) {
        return false;
    }
    
    auto completions = it->second.getRecentCompletions(1);
    auto today = getTodayStart();
    
    for (const auto& completion : completions) {
        if (isSameDay(completion, today)) {
            return true;
        }
    }
    
    return false;
}

// Reminders and notifications
std::vector<uint32_t> HabitTracker::getHabitsNeedingAttention() const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        if (!hasCheckedInToday(id) && habit.getFrequency() == Habit::Frequency::DAILY) {
            result.push_back(id);
        }
    }
    
    return result;
}

std::vector<uint32_t> HabitTracker::getOverdueHabits() const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        int daysSinceLastCompletion = habit.getDaysSinceLastCompletion();
        
        // Consider a habit overdue based on its frequency
        bool isOverdue = false;
        switch (habit.getFrequency()) {
            case Habit::Frequency::DAILY:
                isOverdue = daysSinceLastCompletion > 1;
                break;
            case Habit::Frequency::WEEKLY:
                isOverdue = daysSinceLastCompletion > 7;
                break;
            case Habit::Frequency::MONTHLY:
                isOverdue = daysSinceLastCompletion > 30;
                break;
        }
        
        if (isOverdue) {
            result.push_back(id);
        }
    }
    
    return result;
}

std::vector<uint32_t> HabitTracker::getHabitsAtRisk() const {
    std::vector<uint32_t> result;
    
    for (const auto& [id, habit] : habits) {
        // A habit is at risk if it has a streak but hasn't been completed today
        if (habit.getCurrentStreak() > 0 && !hasCheckedInToday(id)) {
            result.push_back(id);
        }
    }
    
    return result;
}

// Private helper methods
void HabitTracker::triggerHabitCreationCallback(const Habit& habit) {
    if (onHabitCreated) {
        onHabitCreated(habit);
    }
}

void HabitTracker::triggerHabitUpdateCallback(const Habit& habit) {
    if (onHabitUpdated) {
        onHabitUpdated(habit);
    }
}

void HabitTracker::triggerHabitDeletionCallback(uint32_t habitId) {
    if (onHabitDeleted) {
        onHabitDeleted(habitId);
    }
}

void HabitTracker::triggerCheckInCallback(uint32_t habitId, const std::chrono::system_clock::time_point& date) {
    if (onCheckIn) {
        onCheckIn(habitId, date);
    }
}

void HabitTracker::triggerStreakMilestoneCallback(uint32_t habitId, int streak) {
    if (onStreakMilestone) {
        onStreakMilestone(habitId, streak);
    }
}

void HabitTracker::triggerStreakBrokenCallback(uint32_t habitId, int previousStreak) {
    if (onStreakBroken) {
        onStreakBroken(habitId, previousStreak);
    }
}

// Date and time helpers
bool HabitTracker::isSameDay(const std::chrono::system_clock::time_point& date1, 
                            const std::chrono::system_clock::time_point& date2) const {
    auto time1 = std::chrono::system_clock::to_time_t(date1);
    auto time2 = std::chrono::system_clock::to_time_t(date2);
    
    std::tm tm1 = *std::localtime(&time1);
    std::tm tm2 = *std::localtime(&time2);
    
    return (tm1.tm_year == tm2.tm_year && 
            tm1.tm_mon == tm2.tm_mon && 
            tm1.tm_mday == tm2.tm_mday);
}

std::chrono::system_clock::time_point HabitTracker::getStartOfDay(const std::chrono::system_clock::time_point& date) const {
    auto time = std::chrono::system_clock::to_time_t(date);
    auto tm = *std::localtime(&time);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::chrono::system_clock::time_point HabitTracker::getTodayStart() const {
    return getStartOfDay(std::chrono::system_clock::now());
}

int HabitTracker::getDaysBetween(const std::chrono::system_clock::time_point& start, 
                                const std::chrono::system_clock::time_point& end) const {
    auto duration = end - start;
    return std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;
}

// Milestone helpers
std::vector<int> HabitTracker::getDefaultMilestoneTargets() const {
    return {3, 7, 14, 21, 30, 50, 75, 100, 200, 365};
}

bool HabitTracker::checkAndTriggerMilestones(uint32_t habitId, int oldStreak, int newStreak) {
    auto targets = getDefaultMilestoneTargets();
    bool triggered = false;
    
    for (int target : targets) {
        if (oldStreak < target && newStreak >= target) {
            triggerStreakMilestoneCallback(habitId, target);
            triggered = true;
        }
    }
    
    return triggered;
}