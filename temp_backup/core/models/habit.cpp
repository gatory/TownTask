#include "habit.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

// Static member initialization
uint32_t Habit::nextId = 1;

Habit::Habit(const std::string& name, Frequency frequency)
    : id(generateId()), name(name), frequency(frequency), currentStreak(0), longestStreak(0),
      createdAt(std::chrono::system_clock::now()) {
    // Initialize lastCompleted to epoch (never completed)
    lastCompleted = std::chrono::system_clock::time_point{};
}

Habit::Habit(uint32_t id, const std::string& name, Frequency frequency)
    : id(id), name(name), frequency(frequency), currentStreak(0), longestStreak(0),
      createdAt(std::chrono::system_clock::now()) {
    // Update nextId to ensure no conflicts
    if (id >= nextId) {
        nextId = id + 1;
    }
    lastCompleted = std::chrono::system_clock::time_point{};
}

uint32_t Habit::generateId() {
    return nextId++;
}

bool Habit::checkIn() {
    return checkIn(std::chrono::system_clock::now());
}

bool Habit::checkIn(const std::chrono::system_clock::time_point& date) {
    // Check if already completed for this period
    if (isCompletedOn(date)) {
        return false; // Already checked in for this period
    }
    
    // Add to completion history
    completionHistory.push_back(date);
    
    // Sort completion history to maintain chronological order
    std::sort(completionHistory.begin(), completionHistory.end());
    
    // Update last completed
    lastCompleted = date;
    
    // Update streak
    updateStreak();
    
    return true;
}

bool Habit::isCompletedToday() const {
    return isCompletedOn(std::chrono::system_clock::now());
}

bool Habit::isCompletedOn(const std::chrono::system_clock::time_point& date) const {
    for (const auto& completion : completionHistory) {
        switch (frequency) {
            case DAILY:
                if (isSameDay(completion, date)) return true;
                break;
            case WEEKLY:
                if (isSameWeek(completion, date)) return true;
                break;
            case MONTHLY:
                if (isSameMonth(completion, date)) return true;
                break;
        }
    }
    return false;
}

void Habit::updateStreak() {
    if (completionHistory.empty()) {
        currentStreak = 0;
        return;
    }
    
    // Sort completion history
    auto sortedHistory = completionHistory;
    std::sort(sortedHistory.begin(), sortedHistory.end(), std::greater<>());
    
    currentStreak = 1; // At least one completion
    
    // Count consecutive periods backwards from most recent
    for (size_t i = 1; i < sortedHistory.size(); ++i) {
        if (isConsecutivePeriod(sortedHistory[i], sortedHistory[i-1])) {
            currentStreak++;
        } else {
            break;
        }
    }
    
    // Update longest streak if necessary
    if (currentStreak > longestStreak) {
        longestStreak = currentStreak;
    }
}

void Habit::resetStreak() {
    currentStreak = 0;
}

bool Habit::isStreakActive() const {
    if (completionHistory.empty()) return false;
    
    auto daysSince = getDaysSinceLastCompletion();
    
    switch (frequency) {
        case DAILY:
            return daysSince <= 1; // Allow for today or yesterday
        case WEEKLY:
            return daysSince <= 7;
        case MONTHLY:
            return daysSince <= 31;
    }
    return false;
}

int Habit::getDaysSinceLastCompletion() const {
    if (completionHistory.empty()) {
        auto now = std::chrono::system_clock::now();
        auto daysSinceCreation = std::chrono::duration_cast<std::chrono::hours>(now - createdAt).count() / 24;
        return static_cast<int>(daysSinceCreation);
    }
    
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(now - lastCompleted);
    return static_cast<int>(duration.count() / 24);
}

float Habit::getCompletionRate(int days) const {
    if (days <= 0) return 0.0f;
    
    auto now = std::chrono::system_clock::now();
    auto startDate = now - std::chrono::hours(24 * days);
    
    int completionsInPeriod = 0;
    for (const auto& completion : completionHistory) {
        if (completion >= startDate && completion <= now) {
            completionsInPeriod++;
        }
    }
    
    int expectedCompletions;
    switch (frequency) {
        case DAILY:
            expectedCompletions = days;
            break;
        case WEEKLY:
            expectedCompletions = std::max(1, days / 7);
            break;
        case MONTHLY:
            expectedCompletions = std::max(1, days / 30);
            break;
        default:
            expectedCompletions = days;
    }
    
    return static_cast<float>(completionsInPeriod) / static_cast<float>(expectedCompletions);
}

std::vector<std::chrono::system_clock::time_point> Habit::getRecentCompletions(int days) const {
    auto now = std::chrono::system_clock::now();
    auto startDate = now - std::chrono::hours(24 * days);
    
    std::vector<std::chrono::system_clock::time_point> recent;
    for (const auto& completion : completionHistory) {
        if (completion >= startDate && completion <= now) {
            recent.push_back(completion);
        }
    }
    
    return recent;
}

bool Habit::hasReachedMilestone(int streakLength) const {
    return longestStreak >= streakLength;
}

std::vector<int> Habit::getAchievedMilestones() const {
    auto milestones = getDefaultMilestones();
    std::vector<int> achieved;
    
    for (int milestone : milestones) {
        if (hasReachedMilestone(milestone)) {
            achieved.push_back(milestone);
        }
    }
    
    return achieved;
}

std::string Habit::getFrequencyString() const {
    return frequencyToString(frequency);
}

bool Habit::shouldCheckInToday() const {
    return !isCompletedToday();
}

std::chrono::system_clock::time_point Habit::getNextDueDate() const {
    if (completionHistory.empty()) {
        return std::chrono::system_clock::now();
    }
    
    auto lastCompletion = lastCompleted;
    
    switch (frequency) {
        case DAILY:
            return lastCompletion + std::chrono::hours(24);
        case WEEKLY:
            return lastCompletion + std::chrono::hours(24 * 7);
        case MONTHLY:
            return lastCompletion + std::chrono::hours(24 * 30);
    }
    
    return std::chrono::system_clock::now();
}

std::string Habit::frequencyToString(Frequency frequency) {
    switch (frequency) {
        case DAILY: return "DAILY";
        case WEEKLY: return "WEEKLY";
        case MONTHLY: return "MONTHLY";
        default: return "DAILY";
    }
}

Habit::Frequency Habit::stringToFrequency(const std::string& frequencyStr) {
    if (frequencyStr == "WEEKLY") return WEEKLY;
    if (frequencyStr == "MONTHLY") return MONTHLY;
    return DAILY; // Default
}

std::vector<int> Habit::getDefaultMilestones() {
    return {7, 30, 100, 365}; // Week, month, 100 days, year
}

bool Habit::isSameDay(const std::chrono::system_clock::time_point& date1,
                      const std::chrono::system_clock::time_point& date2) const {
    auto time1 = std::chrono::system_clock::to_time_t(date1);
    auto time2 = std::chrono::system_clock::to_time_t(date2);
    
    auto tm1 = *std::gmtime(&time1);
    auto tm2 = *std::gmtime(&time2);
    
    return tm1.tm_year == tm2.tm_year &&
           tm1.tm_mon == tm2.tm_mon &&
           tm1.tm_mday == tm2.tm_mday;
}

bool Habit::isSameWeek(const std::chrono::system_clock::time_point& date1,
                       const std::chrono::system_clock::time_point& date2) const {
    auto time1 = std::chrono::system_clock::to_time_t(date1);
    auto time2 = std::chrono::system_clock::to_time_t(date2);
    
    auto tm1 = *std::gmtime(&time1);
    auto tm2 = *std::gmtime(&time2);
    
    // Calculate week number (simplified - assumes Monday as start of week)
    int week1 = tm1.tm_yday / 7;
    int week2 = tm2.tm_yday / 7;
    
    return tm1.tm_year == tm2.tm_year && week1 == week2;
}

bool Habit::isSameMonth(const std::chrono::system_clock::time_point& date1,
                        const std::chrono::system_clock::time_point& date2) const {
    auto time1 = std::chrono::system_clock::to_time_t(date1);
    auto time2 = std::chrono::system_clock::to_time_t(date2);
    
    auto tm1 = *std::gmtime(&time1);
    auto tm2 = *std::gmtime(&time2);
    
    return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon;
}

bool Habit::isConsecutivePeriod(const std::chrono::system_clock::time_point& date1,
                                const std::chrono::system_clock::time_point& date2) const {
    auto duration = std::chrono::duration_cast<std::chrono::hours>(date2 - date1);
    int hoursDiff = static_cast<int>(duration.count());
    
    switch (frequency) {
        case DAILY:
            return hoursDiff >= 0 && hoursDiff <= 48; // Allow for 1-2 days difference
        case WEEKLY:
            return hoursDiff >= 0 && hoursDiff <= 24 * 14; // Allow for 1-2 weeks difference
        case MONTHLY:
            return hoursDiff >= 0 && hoursDiff <= 24 * 62; // Allow for 1-2 months difference
    }
    return false;
}

std::chrono::system_clock::time_point Habit::getStartOfDay(const std::chrono::system_clock::time_point& date) const {
    auto time = std::chrono::system_clock::to_time_t(date);
    auto tm = *std::gmtime(&time);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

nlohmann::json Habit::toJson() const {
    nlohmann::json j;
    
    j["id"] = id;
    j["name"] = name;
    j["frequency"] = frequencyToString(frequency);
    j["currentStreak"] = currentStreak;
    j["longestStreak"] = longestStreak;
    
    // Convert time points to ISO 8601 strings
    auto createdTime = std::chrono::system_clock::to_time_t(createdAt);
    std::stringstream createdSs;
    createdSs << std::put_time(std::gmtime(&createdTime), "%Y-%m-%dT%H:%M:%SZ");
    j["createdAt"] = createdSs.str();
    
    // Handle lastCompleted (might be epoch if never completed)
    if (lastCompleted != std::chrono::system_clock::time_point{}) {
        auto lastCompletedTime = std::chrono::system_clock::to_time_t(lastCompleted);
        std::stringstream lastCompletedSs;
        lastCompletedSs << std::put_time(std::gmtime(&lastCompletedTime), "%Y-%m-%dT%H:%M:%SZ");
        j["lastCompleted"] = lastCompletedSs.str();
    } else {
        j["lastCompleted"] = nullptr;
    }
    
    // Serialize completion history
    nlohmann::json completionArray = nlohmann::json::array();
    for (const auto& completion : completionHistory) {
        auto completionTime = std::chrono::system_clock::to_time_t(completion);
        std::stringstream completionSs;
        completionSs << std::put_time(std::gmtime(&completionTime), "%Y-%m-%dT%H:%M:%SZ");
        completionArray.push_back(completionSs.str());
    }
    j["completionHistory"] = completionArray;
    
    return j;
}

Habit Habit::fromJson(const nlohmann::json& json) {
    uint32_t id = json.at("id").get<uint32_t>();
    std::string name = json.at("name").get<std::string>();
    Frequency frequency = stringToFrequency(json.at("frequency").get<std::string>());
    
    Habit habit(id, name, frequency);
    
    // Set streak values
    if (json.contains("currentStreak")) {
        habit.currentStreak = json.at("currentStreak").get<int>();
    }
    
    if (json.contains("longestStreak")) {
        habit.longestStreak = json.at("longestStreak").get<int>();
    }
    
    // Parse dates
    if (json.contains("createdAt")) {
        std::string createdAtStr = json.at("createdAt").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(createdAtStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            habit.createdAt = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    if (json.contains("lastCompleted") && !json.at("lastCompleted").is_null()) {
        std::string lastCompletedStr = json.at("lastCompleted").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(lastCompletedStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            habit.lastCompleted = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    // Parse completion history
    if (json.contains("completionHistory") && json.at("completionHistory").is_array()) {
        for (const auto& completionStr : json.at("completionHistory")) {
            std::tm tm = {};
            std::istringstream ss(completionStr.get<std::string>());
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            if (!ss.fail()) {
                habit.completionHistory.push_back(
                    std::chrono::system_clock::from_time_t(std::mktime(&tm))
                );
            }
        }
    }
    
    return habit;
}