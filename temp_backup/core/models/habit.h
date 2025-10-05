#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

class Habit {
public:
    enum Frequency { DAILY, WEEKLY, MONTHLY };
    
    // Constructors
    Habit(const std::string& name, Frequency frequency = DAILY);
    Habit(uint32_t id, const std::string& name, Frequency frequency = DAILY);
    
    // Getters
    uint32_t getId() const { return id; }
    std::string getName() const { return name; }
    Frequency getFrequency() const { return frequency; }
    int getCurrentStreak() const { return currentStreak; }
    int getLongestStreak() const { return longestStreak; }
    std::chrono::system_clock::time_point getLastCompleted() const { return lastCompleted; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt; }
    
    // Setters
    void setName(const std::string& newName) { name = newName; }
    void setFrequency(Frequency newFrequency) { frequency = newFrequency; }
    
    // Completion tracking
    bool checkIn(); // Check in for today/current period
    bool checkIn(const std::chrono::system_clock::time_point& date); // Check in for specific date
    bool isCompletedToday() const;
    bool isCompletedOn(const std::chrono::system_clock::time_point& date) const;
    
    // Streak management
    void updateStreak();
    void resetStreak();
    bool isStreakActive() const;
    int getDaysSinceLastCompletion() const;
    
    // Statistics
    size_t getTotalCompletions() const { return completionHistory.size(); }
    float getCompletionRate(int days = 30) const; // Completion rate over last N days
    std::vector<std::chrono::system_clock::time_point> getCompletionHistory() const { return completionHistory; }
    std::vector<std::chrono::system_clock::time_point> getRecentCompletions(int days = 7) const;
    
    // Milestone checking
    bool hasReachedMilestone(int streakLength) const;
    std::vector<int> getAchievedMilestones() const;
    
    // Utility methods
    std::string getFrequencyString() const;
    bool shouldCheckInToday() const;
    std::chrono::system_clock::time_point getNextDueDate() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Habit fromJson(const nlohmann::json& json);
    
    // Static utility methods
    static std::string frequencyToString(Frequency frequency);
    static Frequency stringToFrequency(const std::string& frequencyStr);
    static std::vector<int> getDefaultMilestones(); // 7, 30, 100, 365 days
    
private:
    uint32_t id;
    std::string name;
    Frequency frequency;
    int currentStreak;
    int longestStreak;
    std::chrono::system_clock::time_point lastCompleted;
    std::chrono::system_clock::time_point createdAt;
    std::vector<std::chrono::system_clock::time_point> completionHistory;
    
    static uint32_t nextId;
    static uint32_t generateId();
    
    // Helper methods
    bool isSameDay(const std::chrono::system_clock::time_point& date1,
                   const std::chrono::system_clock::time_point& date2) const;
    bool isSameWeek(const std::chrono::system_clock::time_point& date1,
                    const std::chrono::system_clock::time_point& date2) const;
    bool isSameMonth(const std::chrono::system_clock::time_point& date1,
                     const std::chrono::system_clock::time_point& date2) const;
    bool isConsecutivePeriod(const std::chrono::system_clock::time_point& date1,
                            const std::chrono::system_clock::time_point& date2) const;
    std::chrono::system_clock::time_point getStartOfDay(const std::chrono::system_clock::time_point& date) const;
};