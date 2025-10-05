#pragma once
#include <ctime>

// Pomodoro timer state
enum class PomodoroState {
    Idle,
    Study,
    Break
};

struct PomodoroTimer {
    int cycleCount;
    int currentCycle;
    PomodoroState state;
    time_t startTime;
    time_t elapsedTime;
    bool isRunning;

    // Constants
    static const int STUDY_DURATION = 25 * 60;  // 25 minutes in seconds
    static const int BREAK_DURATION = 5 * 60;   // 5 minutes in seconds
    static const int MAX_CYCLES = 20;

    PomodoroTimer() :
        cycleCount(1),
        currentCycle(0),
        state(PomodoroState::Idle),
        startTime(0),
        elapsedTime(0),
        isRunning(false) {}

    void Start() {
        if (cycleCount <= 0) return;
        isRunning = true;
        state = PomodoroState::Study;
        currentCycle = 1;
        startTime = time(nullptr);
        elapsedTime = 0;
    }

    void Reset() {
        isRunning = false;
        state = PomodoroState::Idle;
        currentCycle = 0;
        startTime = 0;
        elapsedTime = 0;
    }

    void Update() {
        if (!isRunning) return;

        time_t now = time(nullptr);
        elapsedTime = now - startTime;

        int cycleDuration = (state == PomodoroState::Study) ? STUDY_DURATION : BREAK_DURATION;

        if (elapsedTime >= cycleDuration) {
            // Transition to next state
            if (state == PomodoroState::Study) {
                state = PomodoroState::Break;
                startTime = now;
                elapsedTime = 0;
            } else if (state == PomodoroState::Break) {
                currentCycle++;
                if (currentCycle > cycleCount) {
                    // All cycles complete
                    isRunning = false;
                    state = PomodoroState::Idle;
                } else {
                    state = PomodoroState::Study;
                    startTime = now;
                    elapsedTime = 0;
                }
            }
        }
    }

    int GetCurrentPhaseRemaining() const {
        if (!isRunning) return 0;
        int cycleDuration = (state == PomodoroState::Study) ? STUDY_DURATION : BREAK_DURATION;
        return cycleDuration - (int)elapsedTime;
    }

    int GetTotalElapsed() const {
        if (!isRunning && state == PomodoroState::Idle) return 0;

        // Calculate total time including completed cycles
        int completedStudyTime = (currentCycle - 1) * STUDY_DURATION;
        int completedBreakTime = (currentCycle - 1) * BREAK_DURATION;

        return completedStudyTime + completedBreakTime + (int)elapsedTime;
    }
};
