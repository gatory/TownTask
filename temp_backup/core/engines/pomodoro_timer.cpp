#include "pomodoro_timer.h"
#include <sstream>
#include <iomanip>
#include <ctime>

PomodoroTimer::PomodoroTimer()
    : state(TimerState::STOPPED)
    , currentSessionType(SessionType::WORK)
    , totalDurationSeconds(0)
    , remainingSeconds(0)
    , completedPomodoros(0)
    , coffeeRewards(0)
    , workDurationMinutes(25)
    , shortBreakDurationMinutes(5)
    , longBreakDurationMinutes(15)
    , totalWorkMinutesToday(0)
    , totalWorkMinutesAllTime(0)
    , lastResetDate(std::chrono::system_clock::now())
{
}

void PomodoroTimer::start(SessionType sessionType, int durationMinutes) {
    checkAndResetDailyStats();
    
    currentSessionType = sessionType;
    
    // Use provided duration or default
    int duration = (durationMinutes > 0) ? durationMinutes : getDefaultDuration(sessionType);
    totalDurationSeconds = duration * 60;
    remainingSeconds = totalDurationSeconds;
    
    startTime = std::chrono::steady_clock::now();
    setState(TimerState::RUNNING);
}

void PomodoroTimer::pause() {
    if (state == TimerState::RUNNING) {
        pauseTime = std::chrono::steady_clock::now();
        setState(TimerState::PAUSED);
    }
}

void PomodoroTimer::resume() {
    if (state == TimerState::PAUSED) {
        // Adjust start time to account for pause duration
        auto pauseDuration = std::chrono::steady_clock::now() - pauseTime;
        startTime += pauseDuration;
        setState(TimerState::RUNNING);
    }
}

void PomodoroTimer::stop() {
    if (state == TimerState::RUNNING || state == TimerState::PAUSED) {
        interruptSession();
    }
    setState(TimerState::STOPPED);
    remainingSeconds = 0;
}

void PomodoroTimer::reset() {
    setState(TimerState::STOPPED);
    remainingSeconds = 0;
    totalDurationSeconds = 0;
}

PomodoroTimer::TimerState PomodoroTimer::getState() const {
    return state;
}

PomodoroTimer::SessionType PomodoroTimer::getCurrentSessionType() const {
    return currentSessionType;
}

int PomodoroTimer::getRemainingTimeSeconds() const {
    if (state == TimerState::RUNNING) {
        // Calculate remaining time on-the-fly for running timers
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        int remaining = totalDurationSeconds - static_cast<int>(elapsed.count());
        return (remaining < 0) ? 0 : remaining;
    }
    return remainingSeconds;
}

int PomodoroTimer::getTotalDurationSeconds() const {
    return totalDurationSeconds;
}

float PomodoroTimer::getProgress() const {
    if (totalDurationSeconds == 0) {
        return 0.0f;
    }
    
    int currentRemaining = getRemainingTimeSeconds();
    return 1.0f - (static_cast<float>(currentRemaining) / static_cast<float>(totalDurationSeconds));
}

int PomodoroTimer::getCompletedPomodoros() const {
    return completedPomodoros;
}

int PomodoroTimer::getCoffeeRewards() const {
    return coffeeRewards;
}

bool PomodoroTimer::shouldSuggestLongBreak() const {
    return completedPomodoros > 0 && completedPomodoros % 4 == 0;
}

void PomodoroTimer::update() {
    if (state == TimerState::RUNNING) {
        updateRemainingTime();
        
        if (remainingSeconds <= 0) {
            completeSession();
        } else {
            // Trigger tick callback
            if (onTick) {
                onTick(remainingSeconds);
            }
        }
    }
}

void PomodoroTimer::setOnSessionCompleted(std::function<void(SessionType)> callback) {
    onSessionCompleted = std::move(callback);
}

void PomodoroTimer::setOnSessionInterrupted(std::function<void(SessionType)> callback) {
    onSessionInterrupted = std::move(callback);
}

void PomodoroTimer::setOnTick(std::function<void(int)> callback) {
    onTick = std::move(callback);
}

void PomodoroTimer::setOnStateChanged(std::function<void(TimerState, TimerState)> callback) {
    onStateChanged = std::move(callback);
}

void PomodoroTimer::setWorkDuration(int minutes) {
    if (minutes > 0) {
        workDurationMinutes = minutes;
    }
}

void PomodoroTimer::setShortBreakDuration(int minutes) {
    if (minutes > 0) {
        shortBreakDurationMinutes = minutes;
    }
}

void PomodoroTimer::setLongBreakDuration(int minutes) {
    if (minutes > 0) {
        longBreakDurationMinutes = minutes;
    }
}

int PomodoroTimer::getWorkDuration() const {
    return workDurationMinutes;
}

int PomodoroTimer::getShortBreakDuration() const {
    return shortBreakDurationMinutes;
}

int PomodoroTimer::getLongBreakDuration() const {
    return longBreakDurationMinutes;
}

int PomodoroTimer::getTotalWorkMinutesToday() const {
    return totalWorkMinutesToday;
}

int PomodoroTimer::getTotalWorkMinutesAllTime() const {
    return totalWorkMinutesAllTime;
}

void PomodoroTimer::resetDailyStats() {
    totalWorkMinutesToday = 0;
    lastResetDate = std::chrono::system_clock::now();
}

std::string PomodoroTimer::getStateAsString() const {
    std::ostringstream oss;
    oss << "state:" << timerStateToString(state) << ";";
    oss << "sessionType:" << sessionTypeToString(currentSessionType) << ";";
    oss << "remainingSeconds:" << remainingSeconds << ";";
    oss << "totalDurationSeconds:" << totalDurationSeconds << ";";
    oss << "completedPomodoros:" << completedPomodoros << ";";
    oss << "coffeeRewards:" << coffeeRewards << ";";
    oss << "workDuration:" << workDurationMinutes << ";";
    oss << "shortBreakDuration:" << shortBreakDurationMinutes << ";";
    oss << "longBreakDuration:" << longBreakDurationMinutes << ";";
    oss << "totalWorkToday:" << totalWorkMinutesToday << ";";
    oss << "totalWorkAllTime:" << totalWorkMinutesAllTime << ";";
    
    // Add timestamp for last reset
    auto time_t = std::chrono::system_clock::to_time_t(lastResetDate);
    oss << "lastReset:" << time_t << ";";
    
    return oss.str();
}

void PomodoroTimer::loadStateFromString(const std::string& stateStr) {
    std::istringstream iss(stateStr);
    std::string pair;
    
    while (std::getline(iss, pair, ';')) {
        if (pair.empty()) continue;
        
        size_t colonPos = pair.find(':');
        if (colonPos == std::string::npos) continue;
        
        std::string key = pair.substr(0, colonPos);
        std::string value = pair.substr(colonPos + 1);
        
        if (key == "state") {
            state = stringToTimerState(value);
        } else if (key == "sessionType") {
            currentSessionType = stringToSessionType(value);
        } else if (key == "remainingSeconds") {
            remainingSeconds = std::stoi(value);
        } else if (key == "totalDurationSeconds") {
            totalDurationSeconds = std::stoi(value);
        } else if (key == "completedPomodoros") {
            completedPomodoros = std::stoi(value);
        } else if (key == "coffeeRewards") {
            coffeeRewards = std::stoi(value);
        } else if (key == "workDuration") {
            workDurationMinutes = std::stoi(value);
        } else if (key == "shortBreakDuration") {
            shortBreakDurationMinutes = std::stoi(value);
        } else if (key == "longBreakDuration") {
            longBreakDurationMinutes = std::stoi(value);
        } else if (key == "totalWorkToday") {
            totalWorkMinutesToday = std::stoi(value);
        } else if (key == "totalWorkAllTime") {
            totalWorkMinutesAllTime = std::stoi(value);
        } else if (key == "lastReset") {
            std::time_t time_t = std::stoll(value);
            lastResetDate = std::chrono::system_clock::from_time_t(time_t);
        }
    }
    
    // Reset timer state if it was running (can't restore running state)
    if (state == TimerState::RUNNING) {
        state = TimerState::STOPPED;
        remainingSeconds = 0;
    }
}

// Private helper methods

void PomodoroTimer::setState(TimerState newState) {
    TimerState oldState = state;
    state = newState;
    
    if (onStateChanged && oldState != newState) {
        onStateChanged(oldState, newState);
    }
}

void PomodoroTimer::completeSession() {
    setState(TimerState::COMPLETED);
    remainingSeconds = 0;
    
    // Award rewards for work sessions
    if (currentSessionType == SessionType::WORK) {
        completedPomodoros++;
        coffeeRewards++;
        
        // Update work time statistics
        int sessionMinutes = totalDurationSeconds / 60;
        totalWorkMinutesToday += sessionMinutes;
        totalWorkMinutesAllTime += sessionMinutes;
    }
    
    // Trigger completion callback
    if (onSessionCompleted) {
        onSessionCompleted(currentSessionType);
    }
}

void PomodoroTimer::interruptSession() {
    // Trigger interruption callback
    if (onSessionInterrupted) {
        onSessionInterrupted(currentSessionType);
    }
}

int PomodoroTimer::getDefaultDuration(SessionType sessionType) const {
    switch (sessionType) {
        case SessionType::WORK:
            return workDurationMinutes;
        case SessionType::SHORT_BREAK:
            return shortBreakDurationMinutes;
        case SessionType::LONG_BREAK:
            return longBreakDurationMinutes;
        default:
            return workDurationMinutes;
    }
}

void PomodoroTimer::updateRemainingTime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    remainingSeconds = totalDurationSeconds - static_cast<int>(elapsed.count());
    
    if (remainingSeconds < 0) {
        remainingSeconds = 0;
    }
}

bool PomodoroTimer::isNewDay() const {
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    auto lastResetTime = std::chrono::system_clock::to_time_t(lastResetDate);
    
    std::tm nowTm = *std::localtime(&nowTime);
    std::tm lastResetTm = *std::localtime(&lastResetTime);
    
    return (nowTm.tm_year != lastResetTm.tm_year ||
            nowTm.tm_mon != lastResetTm.tm_mon ||
            nowTm.tm_mday != lastResetTm.tm_mday);
}

void PomodoroTimer::checkAndResetDailyStats() {
    if (isNewDay()) {
        resetDailyStats();
    }
}

// String conversion helpers

std::string PomodoroTimer::sessionTypeToString(SessionType type) const {
    switch (type) {
        case SessionType::WORK: return "WORK";
        case SessionType::SHORT_BREAK: return "SHORT_BREAK";
        case SessionType::LONG_BREAK: return "LONG_BREAK";
        default: return "WORK";
    }
}

PomodoroTimer::SessionType PomodoroTimer::stringToSessionType(const std::string& str) const {
    if (str == "WORK") return SessionType::WORK;
    if (str == "SHORT_BREAK") return SessionType::SHORT_BREAK;
    if (str == "LONG_BREAK") return SessionType::LONG_BREAK;
    return SessionType::WORK;
}

std::string PomodoroTimer::timerStateToString(TimerState state) const {
    switch (state) {
        case TimerState::STOPPED: return "STOPPED";
        case TimerState::RUNNING: return "RUNNING";
        case TimerState::PAUSED: return "PAUSED";
        case TimerState::COMPLETED: return "COMPLETED";
        default: return "STOPPED";
    }
}

PomodoroTimer::TimerState PomodoroTimer::stringToTimerState(const std::string& str) const {
    if (str == "STOPPED") return TimerState::STOPPED;
    if (str == "RUNNING") return TimerState::RUNNING;
    if (str == "PAUSED") return TimerState::PAUSED;
    if (str == "COMPLETED") return TimerState::COMPLETED;
    return TimerState::STOPPED;
}

void PomodoroTimer::setStartTimeForTesting(const std::chrono::steady_clock::time_point& testStartTime) {
    startTime = testStartTime;
}