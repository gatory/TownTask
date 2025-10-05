#include "../include/Game/Game.h"  
#include "../include/Core/Constants.h"

PomodoroTimer::PomodoroTimer() :
    cycleCount(1),
    currentCycle(0),
    state(PomodoroState::Idle),
    startTime(0),
    elapsedTime(0),
    isRunning(false) {}

void PomodoroTimer::Start() {
    if (cycleCount <= 0) return;
    isRunning = true;
    state = PomodoroState::Study;
    currentCycle = 1;
    startTime = time(nullptr);
    elapsedTime = 0;
}

void PomodoroTimer::Reset() {
    isRunning = false;
    state = PomodoroState::Idle;
    currentCycle = 0;
    startTime = 0;
    elapsedTime = 0;
}

void PomodoroTimer::Update() {
    if (!isRunning) return;

    time_t now = time(nullptr);
    elapsedTime = now - startTime;

    int cycleDuration = (state == PomodoroState::Study) ? STUDY_DURATION : BREAK_DURATION;

    if (elapsedTime >= cycleDuration) {
        if (state == PomodoroState::Study) {
            state = PomodoroState::Break;
            startTime = now;
            elapsedTime = 0;
        } else if (state == PomodoroState::Break) {
            currentCycle++;
            if (currentCycle > cycleCount) {
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

int PomodoroTimer::GetCurrentPhaseRemaining() const {
    if (!isRunning) return 0;
    int cycleDuration = (state == PomodoroState::Study) ? STUDY_DURATION : BREAK_DURATION;
    return cycleDuration - (int)elapsedTime;
}

int PomodoroTimer::GetTotalElapsed() const {
    if (!isRunning && state == PomodoroState::Idle) return 0;

    int completedStudyTime = (currentCycle - 1) * STUDY_DURATION;
    int completedBreakTime = (currentCycle - 1) * BREAK_DURATION;

    return completedStudyTime + completedBreakTime + (int)elapsedTime;
}