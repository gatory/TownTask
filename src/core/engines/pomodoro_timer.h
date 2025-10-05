#pragma once

#include <chrono>
#include <functional>
#include <string>

class PomodoroTimer {
public:
    enum class SessionType {
        WORK,
        SHORT_BREAK,
        LONG_BREAK
    };
    
    enum class TimerState {
        STOPPED,
        RUNNING,
        PAUSED,
        COMPLETED
    };
    
    // Constructor
    PomodoroTimer();
    
    // Timer control methods
    void start(SessionType sessionType, int durationMinutes = 0);
    void pause();
    void resume();
    void stop();
    void reset();
    
    // State queries
    TimerState getState() const;
    SessionType getCurrentSessionType() const;
    int getRemainingTimeSeconds() const;
    int getTotalDurationSeconds() const;
    float getProgress() const; // 0.0 to 1.0
    
    // Session management
    int getCompletedPomodoros() const;
    int getCoffeeRewards() const;
    bool shouldSuggestLongBreak() const;
    
    // Update method (call this regularly to update timer)
    void update();
    
    // Callback setters
    void setOnSessionCompleted(std::function<void(SessionType)> callback);
    void setOnSessionInterrupted(std::function<void(SessionType)> callback);
    void setOnTick(std::function<void(int)> callback); // Called every second with remaining time
    void setOnStateChanged(std::function<void(TimerState, TimerState)> callback); // old state, new state
    
    // Configuration
    void setWorkDuration(int minutes);
    void setShortBreakDuration(int minutes);
    void setLongBreakDuration(int minutes);
    int getWorkDuration() const;
    int getShortBreakDuration() const;
    int getLongBreakDuration() const;
    
    // Statistics
    int getTotalWorkMinutesToday() const;
    int getTotalWorkMinutesAllTime() const;
    void resetDailyStats();
    
    // Serialization support
    std::string getStateAsString() const;
    void loadStateFromString(const std::string& state);
    
    // Testing support - allows injecting a custom start time for testing
    void setStartTimeForTesting(const std::chrono::steady_clock::time_point& testStartTime);

private:
    // Timer state
    TimerState state;
    SessionType currentSessionType;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point pauseTime;
    int totalDurationSeconds;
    int remainingSeconds;
    
    // Session tracking
    int completedPomodoros;
    int coffeeRewards;
    
    // Configuration
    int workDurationMinutes;
    int shortBreakDurationMinutes;
    int longBreakDurationMinutes;
    
    // Statistics
    int totalWorkMinutesToday;
    int totalWorkMinutesAllTime;
    std::chrono::system_clock::time_point lastResetDate;
    
    // Callbacks
    std::function<void(SessionType)> onSessionCompleted;
    std::function<void(SessionType)> onSessionInterrupted;
    std::function<void(int)> onTick;
    std::function<void(TimerState, TimerState)> onStateChanged;
    
    // Helper methods
    void setState(TimerState newState);
    void completeSession();
    void interruptSession();
    int getDefaultDuration(SessionType sessionType) const;
    void updateRemainingTime();
    bool isNewDay() const;
    void checkAndResetDailyStats();
    
    // String conversion helpers
    std::string sessionTypeToString(SessionType type) const;
    SessionType stringToSessionType(const std::string& str) const;
    std::string timerStateToString(TimerState state) const;
    TimerState stringToTimerState(const std::string& str) const;
};