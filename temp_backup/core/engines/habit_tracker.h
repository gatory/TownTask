#pragma once

#include "../models/habit.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>

class HabitTracker {
public:
    // Constructor
    HabitTracker();
    
    // Habit management
    uint32_t createHabit(const std::string& name, Habit::Frequency frequency = Habit::Frequency::DAILY);
    bool updateHabit(uint32_t habitId, const Habit& updatedHabit);
    bool deleteHabit(uint32_t habitId);
    
    // Habit modification
    bool setHabitName(uint32_t habitId, const std::string& name);
    bool setHabitFrequency(uint32_t habitId, Habit::Frequency frequency);
    
    // Check-in functionality
    bool checkInHabit(uint32_t habitId);
    bool checkInHabit(uint32_t habitId, const std::chrono::system_clock::time_point& date);
    bool undoCheckIn(uint32_t habitId);
    bool undoCheckIn(uint32_t habitId, const std::chrono::system_clock::time_point& date);
    
    // Streak management
    bool resetHabitStreak(uint32_t habitId);
    int getHabitStreak(uint32_t habitId) const;
    bool isStreakActive(uint32_t habitId) const;
    
    // Querying
    std::vector<Habit> getAllHabits() const;
    std::optional<Habit> getHabit(uint32_t habitId) const;
    std::vector<Habit> getHabitsByFrequency(Habit::Frequency frequency) const;
    std::vector<Habit> getActiveHabits() const; // Habits with recent activity
    std::vector<Habit> getInactiveHabits() const; // Habits without recent activity
    
    // Progress tracking
    float getHabitCompletionRate(uint32_t habitId, int days = 30) const;
    std::vector<std::chrono::system_clock::time_point> getHabitCompletions(uint32_t habitId, int days = 30) const;
    int getDaysSinceLastCompletion(uint32_t habitId) const;
    
    // Milestone tracking
    struct Milestone {
        int streakTarget;
        std::string name;
        std::string description;
        bool isAchieved;
    };
    std::vector<Milestone> getHabitMilestones(uint32_t habitId) const;
    std::vector<Milestone> getAchievedMilestones(uint32_t habitId) const;
    std::vector<Milestone> getUpcomingMilestones(uint32_t habitId) const;
    bool hasReachedMilestone(uint32_t habitId, int streakTarget) const;
    
    // Statistics and analytics
    struct HabitStats {
        uint32_t habitId;
        std::string name;
        int currentStreak;
        int longestStreak;
        int totalCompletions;
        float completionRate30Days;
        float completionRate7Days;
        int daysSinceLastCompletion;
        bool isActive;
    };
    HabitStats getHabitStatistics(uint32_t habitId) const;
    std::vector<HabitStats> getAllHabitStatistics() const;
    
    // Filtering and sorting
    enum class SortBy {
        NAME_ASC,
        NAME_DESC,
        STREAK_ASC,
        STREAK_DESC,
        COMPLETION_RATE_ASC,
        COMPLETION_RATE_DESC,
        CREATED_DATE_ASC,
        CREATED_DATE_DESC,
        LAST_COMPLETION_ASC,
        LAST_COMPLETION_DESC
    };
    std::vector<Habit> getSortedHabits(SortBy sortBy) const;
    
    // Calendar view support
    struct DayStatus {
        std::chrono::system_clock::time_point date;
        bool completed;
        bool required; // Based on habit frequency
    };
    std::vector<DayStatus> getHabitCalendar(uint32_t habitId, int days = 30) const;
    std::vector<DayStatus> getHabitCalendarForMonth(uint32_t habitId, int year, int month) const;
    
    // Bulk operations
    std::vector<uint32_t> createMultipleHabits(const std::vector<std::pair<std::string, Habit::Frequency>>& habitData);
    bool deleteMultipleHabits(const std::vector<uint32_t>& habitIds);
    bool checkInMultipleHabits(const std::vector<uint32_t>& habitIds);
    bool resetMultipleStreaks(const std::vector<uint32_t>& habitIds);
    
    // Global statistics
    size_t getTotalHabitCount() const;
    size_t getActiveHabitCount() const;
    size_t getInactiveHabitCount() const;
    int getTotalCompletionsToday() const;
    int getTotalCompletionsThisWeek() const;
    int getTotalCompletionsThisMonth() const;
    float getOverallCompletionRate(int days = 30) const;
    
    // Streaks and achievements
    std::vector<uint32_t> getHabitsWithActiveStreaks() const;
    std::vector<uint32_t> getHabitsWithBrokenStreaks() const;
    int getLongestCurrentStreak() const;
    int getLongestAllTimeStreak() const;
    std::vector<uint32_t> getHabitsAtMilestone(int streakTarget) const;
    
    // Data management
    void loadHabits(const std::vector<Habit>& habitList);
    std::vector<Habit> exportHabits() const;
    void clearAllHabits();
    
    // Events and callbacks
    void setHabitCreationCallback(std::function<void(const Habit&)> callback);
    void setHabitUpdateCallback(std::function<void(const Habit&)> callback);
    void setHabitDeletionCallback(std::function<void(uint32_t)> callback);
    void setCheckInCallback(std::function<void(uint32_t, const std::chrono::system_clock::time_point&)> callback);
    void setStreakMilestoneCallback(std::function<void(uint32_t, int)> callback); // habitId, streakReached
    void setStreakBrokenCallback(std::function<void(uint32_t, int)> callback); // habitId, previousStreak
    
    // Validation
    bool habitExists(uint32_t habitId) const;
    bool isValidHabitId(uint32_t habitId) const;
    bool canCheckInToday(uint32_t habitId) const;
    bool hasCheckedInToday(uint32_t habitId) const;
    
    // Reminders and notifications
    std::vector<uint32_t> getHabitsNeedingAttention() const; // Habits that should be done today
    std::vector<uint32_t> getOverdueHabits() const; // Habits that are overdue based on frequency
    std::vector<uint32_t> getHabitsAtRisk() const; // Habits with streaks at risk of breaking

private:
    // Storage
    std::unordered_map<uint32_t, Habit> habits;
    
    // Callbacks
    std::function<void(const Habit&)> onHabitCreated;
    std::function<void(const Habit&)> onHabitUpdated;
    std::function<void(uint32_t)> onHabitDeleted;
    std::function<void(uint32_t, const std::chrono::system_clock::time_point&)> onCheckIn;
    std::function<void(uint32_t, int)> onStreakMilestone;
    std::function<void(uint32_t, int)> onStreakBroken;
    
    // Helper methods
    void triggerHabitCreationCallback(const Habit& habit);
    void triggerHabitUpdateCallback(const Habit& habit);
    void triggerHabitDeletionCallback(uint32_t habitId);
    void triggerCheckInCallback(uint32_t habitId, const std::chrono::system_clock::time_point& date);
    void triggerStreakMilestoneCallback(uint32_t habitId, int streak);
    void triggerStreakBrokenCallback(uint32_t habitId, int previousStreak);
    
    // Date and time helpers
    bool isSameDay(const std::chrono::system_clock::time_point& date1, 
                   const std::chrono::system_clock::time_point& date2) const;
    std::chrono::system_clock::time_point getStartOfDay(const std::chrono::system_clock::time_point& date) const;
    std::chrono::system_clock::time_point getTodayStart() const;
    int getDaysBetween(const std::chrono::system_clock::time_point& start, 
                       const std::chrono::system_clock::time_point& end) const;
    
    // Milestone helpers
    std::vector<int> getDefaultMilestoneTargets() const;
    bool checkAndTriggerMilestones(uint32_t habitId, int oldStreak, int newStreak);
};