#pragma once
#include <ctime>

enum class PomodoroState {
    Idle,
    Study,
    Break
};

class PomodoroTimer {
private:
    int cycleCount;
    int currentCycle;
    PomodoroState state;
    time_t startTime;
    time_t elapsedTime;
    bool isRunning;

public:
    static const int STUDY_DURATION = 25 * 60;
    static const int BREAK_DURATION = 5 * 60;
    static const int MAX_CYCLES = 20;

    PomodoroTimer();
    
    void Start();
    void Reset();
    void Update();
    
    int GetCurrentPhaseRemaining() const;
    int GetTotalElapsed() const;
    
    int GetCycleCount() const { return cycleCount; }
    int GetCurrentCycle() const { return currentCycle; }
    PomodoroState GetState() const { return state; }
    bool IsRunning() const { return isRunning; }
    
    void SetCycleCount(int count) { cycleCount = count; }
};