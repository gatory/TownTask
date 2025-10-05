#include <gtest/gtest.h>
#include "../src/core/engines/pomodoro_timer.h"
#include <thread>
#include <chrono>

class PomodoroTimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        timer = std::make_unique<PomodoroTimer>();
    }
    
    void TearDown() override {
        timer.reset();
    }
    
    std::unique_ptr<PomodoroTimer> timer;
    
    // Helper method to wait for a short duration
    void waitMs(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
};

// Constructor and Initial State Tests
TEST_F(PomodoroTimerTest, Constructor_DefaultValues_SetsCorrectInitialState) {
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::STOPPED);
    EXPECT_EQ(timer->getCurrentSessionType(), PomodoroTimer::SessionType::WORK);
    EXPECT_EQ(timer->getRemainingTimeSeconds(), 0);
    EXPECT_EQ(timer->getTotalDurationSeconds(), 0);
    EXPECT_EQ(timer->getProgress(), 0.0f);
    EXPECT_EQ(timer->getCompletedPomodoros(), 0);
    EXPECT_EQ(timer->getCoffeeRewards(), 0);
    EXPECT_FALSE(timer->shouldSuggestLongBreak());
}

TEST_F(PomodoroTimerTest, Constructor_DefaultDurations_SetsCorrectValues) {
    EXPECT_EQ(timer->getWorkDuration(), 25);
    EXPECT_EQ(timer->getShortBreakDuration(), 5);
    EXPECT_EQ(timer->getLongBreakDuration(), 15);
}

// Timer Control Tests
TEST_F(PomodoroTimerTest, Start_WorkSession_SetsCorrectState) {
    timer->start(PomodoroTimer::SessionType::WORK);
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::RUNNING);
    EXPECT_EQ(timer->getCurrentSessionType(), PomodoroTimer::SessionType::WORK);
    EXPECT_EQ(timer->getTotalDurationSeconds(), 25 * 60); // 25 minutes
    EXPECT_GT(timer->getRemainingTimeSeconds(), 0);
}

TEST_F(PomodoroTimerTest, Start_CustomDuration_UsesProvidedDuration) {
    timer->start(PomodoroTimer::SessionType::WORK, 45);
    
    EXPECT_EQ(timer->getTotalDurationSeconds(), 45 * 60); // 45 minutes
    EXPECT_EQ(timer->getRemainingTimeSeconds(), 45 * 60);
}

TEST_F(PomodoroTimerTest, Start_ShortBreak_SetsCorrectDuration) {
    timer->start(PomodoroTimer::SessionType::SHORT_BREAK);
    
    EXPECT_EQ(timer->getCurrentSessionType(), PomodoroTimer::SessionType::SHORT_BREAK);
    EXPECT_EQ(timer->getTotalDurationSeconds(), 5 * 60); // 5 minutes
}

TEST_F(PomodoroTimerTest, Start_LongBreak_SetsCorrectDuration) {
    timer->start(PomodoroTimer::SessionType::LONG_BREAK);
    
    EXPECT_EQ(timer->getCurrentSessionType(), PomodoroTimer::SessionType::LONG_BREAK);
    EXPECT_EQ(timer->getTotalDurationSeconds(), 15 * 60); // 15 minutes
}

TEST_F(PomodoroTimerTest, Pause_RunningTimer_ChangeStateTopaused) {
    timer->start(PomodoroTimer::SessionType::WORK);
    timer->pause();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::PAUSED);
}

TEST_F(PomodoroTimerTest, Pause_StoppedTimer_DoesNotChangeState) {
    timer->pause();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::STOPPED);
}

TEST_F(PomodoroTimerTest, Resume_PausedTimer_ChangesStateToRunning) {
    timer->start(PomodoroTimer::SessionType::WORK);
    timer->pause();
    timer->resume();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::RUNNING);
}

TEST_F(PomodoroTimerTest, Resume_StoppedTimer_DoesNotChangeState) {
    timer->resume();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::STOPPED);
}

TEST_F(PomodoroTimerTest, Stop_RunningTimer_ChangesStateToStopped) {
    timer->start(PomodoroTimer::SessionType::WORK);
    timer->stop();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::STOPPED);
    EXPECT_EQ(timer->getRemainingTimeSeconds(), 0);
}

TEST_F(PomodoroTimerTest, Reset_Timer_ResetsAllValues) {
    timer->start(PomodoroTimer::SessionType::WORK);
    timer->reset();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::STOPPED);
    EXPECT_EQ(timer->getRemainingTimeSeconds(), 0);
    EXPECT_EQ(timer->getTotalDurationSeconds(), 0);
}

// Progress and Time Tests
TEST_F(PomodoroTimerTest, GetProgress_NewTimer_ReturnsZero) {
    EXPECT_EQ(timer->getProgress(), 0.0f);
}

TEST_F(PomodoroTimerTest, GetProgress_RunningTimer_ReturnsCorrectProgress) {
    timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute for quick test
    
    int initialRemaining = timer->getRemainingTimeSeconds();
    int totalDuration = timer->getTotalDurationSeconds();
    
    // Wait longer to ensure measurable progress
    waitMs(1100); // Wait just over 1 second
    timer->update();
    
    int finalRemaining = timer->getRemainingTimeSeconds();
    float progress = timer->getProgress();
    
    EXPECT_EQ(totalDuration, 60); // Should be 60 seconds for 1 minute
    EXPECT_LT(finalRemaining, initialRemaining); // Time should have decreased
    EXPECT_GT(progress, 0.0f);
    EXPECT_LT(progress, 1.0f);
    EXPECT_GT(progress, 0.01f); // At least 1% progress after 1+ second
}

// Update and Time Progression Tests
TEST_F(PomodoroTimerTest, Update_RunningTimer_DecreasesRemainingTime) {
    timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute
    int initialTime = timer->getRemainingTimeSeconds();
    
    waitMs(1100); // Wait just over 1 second
    timer->update();
    
    EXPECT_LT(timer->getRemainingTimeSeconds(), initialTime);
}

TEST_F(PomodoroTimerTest, Update_PausedTimer_DoesNotDecreaseTime) {
    timer->start(PomodoroTimer::SessionType::WORK, 1);
    timer->pause();
    int timeWhenPaused = timer->getRemainingTimeSeconds();
    
    waitMs(1100);
    timer->update();
    
    EXPECT_EQ(timer->getRemainingTimeSeconds(), timeWhenPaused);
}

// Session Completion Tests
TEST_F(PomodoroTimerTest, SessionCompletion_WorkSession_AwardsRewards) {
    // Start a work session
    timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute
    
    // Simulate time passage by setting the start time to 61 seconds ago
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    // Update the timer to process the completion
    timer->update();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::COMPLETED);
    EXPECT_EQ(timer->getCompletedPomodoros(), 1);
    EXPECT_EQ(timer->getCoffeeRewards(), 1);
}

TEST_F(PomodoroTimerTest, SessionCompletion_BreakSession_DoesNotAwardRewards) {
    timer->start(PomodoroTimer::SessionType::SHORT_BREAK, 1); // 1 minute break
    
    // Simulate time passage by setting the start time to 61 seconds ago
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    // Update the timer to process the completion
    timer->update();
    
    EXPECT_EQ(timer->getState(), PomodoroTimer::TimerState::COMPLETED);
    EXPECT_EQ(timer->getCompletedPomodoros(), 0); // No pomodoro count for breaks
    EXPECT_EQ(timer->getCoffeeRewards(), 0); // No rewards for breaks
}

// Long Break Suggestion Tests
TEST_F(PomodoroTimerTest, ShouldSuggestLongBreak_FourCompletedPomodoros_ReturnsTrue) {
    // Simulate completing 4 work sessions
    for (int i = 0; i < 4; i++) {
        timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute
        
        // Simulate completion by setting start time to past
        auto now = std::chrono::steady_clock::now();
        auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
        timer->setStartTimeForTesting(pastStartTime);
        
        timer->update(); // Process completion
    }
    
    EXPECT_TRUE(timer->shouldSuggestLongBreak());
    EXPECT_EQ(timer->getCompletedPomodoros(), 4);
}

TEST_F(PomodoroTimerTest, ShouldSuggestLongBreak_ThreeCompletedPomodoros_ReturnsFalse) {
    // Simulate completing 3 work sessions
    for (int i = 0; i < 3; i++) {
        timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute
        
        // Simulate completion by setting start time to past
        auto now = std::chrono::steady_clock::now();
        auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
        timer->setStartTimeForTesting(pastStartTime);
        
        timer->update(); // Process completion
    }
    
    EXPECT_FALSE(timer->shouldSuggestLongBreak());
}

// Configuration Tests
TEST_F(PomodoroTimerTest, SetWorkDuration_ValidValue_UpdatesDuration) {
    timer->setWorkDuration(30);
    EXPECT_EQ(timer->getWorkDuration(), 30);
}

TEST_F(PomodoroTimerTest, SetWorkDuration_InvalidValue_DoesNotUpdate) {
    int originalDuration = timer->getWorkDuration();
    timer->setWorkDuration(0);
    EXPECT_EQ(timer->getWorkDuration(), originalDuration);
    
    timer->setWorkDuration(-5);
    EXPECT_EQ(timer->getWorkDuration(), originalDuration);
}

TEST_F(PomodoroTimerTest, SetShortBreakDuration_ValidValue_UpdatesDuration) {
    timer->setShortBreakDuration(10);
    EXPECT_EQ(timer->getShortBreakDuration(), 10);
}

TEST_F(PomodoroTimerTest, SetLongBreakDuration_ValidValue_UpdatesDuration) {
    timer->setLongBreakDuration(20);
    EXPECT_EQ(timer->getLongBreakDuration(), 20);
}

// Statistics Tests
TEST_F(PomodoroTimerTest, GetTotalWorkMinutesToday_InitialValue_ReturnsZero) {
    EXPECT_EQ(timer->getTotalWorkMinutesToday(), 0);
}

TEST_F(PomodoroTimerTest, GetTotalWorkMinutesAllTime_InitialValue_ReturnsZero) {
    EXPECT_EQ(timer->getTotalWorkMinutesAllTime(), 0);
}

TEST_F(PomodoroTimerTest, WorkTimeTracking_CompletedSession_UpdatesStatistics) {
    timer->start(PomodoroTimer::SessionType::WORK, 1); // 1 minute session
    
    // Simulate completion by setting start time to past
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    timer->update(); // Process completion
    
    EXPECT_EQ(timer->getTotalWorkMinutesToday(), 1);
    EXPECT_EQ(timer->getTotalWorkMinutesAllTime(), 1);
}

TEST_F(PomodoroTimerTest, ResetDailyStats_ResetsOnlyDailyStats) {
    // Complete a work session to get some stats
    timer->start(PomodoroTimer::SessionType::WORK, 1);
    
    // Simulate completion by setting start time to past
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    timer->update(); // Process completion
    
    int allTimeMinutes = timer->getTotalWorkMinutesAllTime();
    timer->resetDailyStats();
    
    EXPECT_EQ(timer->getTotalWorkMinutesToday(), 0);
    EXPECT_EQ(timer->getTotalWorkMinutesAllTime(), allTimeMinutes); // Should not change
}

// Callback Tests
TEST_F(PomodoroTimerTest, OnSessionCompleted_WorkSessionCompletes_CallsCallback) {
    bool callbackCalled = false;
    PomodoroTimer::SessionType completedType = PomodoroTimer::SessionType::SHORT_BREAK;
    
    timer->setOnSessionCompleted([&](PomodoroTimer::SessionType type) {
        callbackCalled = true;
        completedType = type;
    });
    
    timer->start(PomodoroTimer::SessionType::WORK, 1);
    
    // Simulate completion by setting start time to past
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    timer->update(); // Process completion
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(completedType, PomodoroTimer::SessionType::WORK);
}

TEST_F(PomodoroTimerTest, OnSessionInterrupted_TimerStopped_CallsCallback) {
    bool callbackCalled = false;
    PomodoroTimer::SessionType interruptedType = PomodoroTimer::SessionType::SHORT_BREAK;
    
    timer->setOnSessionInterrupted([&](PomodoroTimer::SessionType type) {
        callbackCalled = true;
        interruptedType = type;
    });
    
    timer->start(PomodoroTimer::SessionType::WORK);
    timer->stop();
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(interruptedType, PomodoroTimer::SessionType::WORK);
}

TEST_F(PomodoroTimerTest, OnStateChanged_StateChanges_CallsCallback) {
    bool callbackCalled = false;
    PomodoroTimer::TimerState oldState = PomodoroTimer::TimerState::COMPLETED;
    PomodoroTimer::TimerState newState = PomodoroTimer::TimerState::COMPLETED;
    
    timer->setOnStateChanged([&](PomodoroTimer::TimerState old, PomodoroTimer::TimerState current) {
        callbackCalled = true;
        oldState = old;
        newState = current;
    });
    
    timer->start(PomodoroTimer::SessionType::WORK);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(oldState, PomodoroTimer::TimerState::STOPPED);
    EXPECT_EQ(newState, PomodoroTimer::TimerState::RUNNING);
}

// Serialization Tests
TEST_F(PomodoroTimerTest, GetStateAsString_DefaultState_ReturnsValidString) {
    std::string state = timer->getStateAsString();
    EXPECT_FALSE(state.empty());
    EXPECT_NE(state.find("state:STOPPED"), std::string::npos);
    EXPECT_NE(state.find("sessionType:WORK"), std::string::npos);
}

TEST_F(PomodoroTimerTest, LoadStateFromString_ValidState_RestoresCorrectly) {
    // Set up some state
    timer->setWorkDuration(30);
    timer->setShortBreakDuration(10);
    timer->setLongBreakDuration(20);
    
    // Complete a session to get rewards
    timer->start(PomodoroTimer::SessionType::WORK, 1);
    
    // Simulate completion by setting start time to past
    auto now = std::chrono::steady_clock::now();
    auto pastStartTime = now - std::chrono::seconds(61); // 61 seconds ago
    timer->setStartTimeForTesting(pastStartTime);
    
    timer->update(); // Process completion
    
    std::string savedState = timer->getStateAsString();
    
    // Create new timer and load state
    auto newTimer = std::make_unique<PomodoroTimer>();
    newTimer->loadStateFromString(savedState);
    
    EXPECT_EQ(newTimer->getWorkDuration(), 30);
    EXPECT_EQ(newTimer->getShortBreakDuration(), 10);
    EXPECT_EQ(newTimer->getLongBreakDuration(), 20);
    EXPECT_EQ(newTimer->getCompletedPomodoros(), 1);
    EXPECT_EQ(newTimer->getCoffeeRewards(), 1);
}

TEST_F(PomodoroTimerTest, LoadStateFromString_RunningState_ResetsToStopped) {
    timer->start(PomodoroTimer::SessionType::WORK);
    std::string state = timer->getStateAsString();
    
    auto newTimer = std::make_unique<PomodoroTimer>();
    newTimer->loadStateFromString(state);
    
    // Running state should be reset to stopped for safety
    EXPECT_EQ(newTimer->getState(), PomodoroTimer::TimerState::STOPPED);
}

TEST_F(PomodoroTimerTest, LoadStateFromString_InvalidString_DoesNotCrash) {
    EXPECT_NO_THROW(timer->loadStateFromString("invalid:data;malformed"));
    EXPECT_NO_THROW(timer->loadStateFromString(""));
    EXPECT_NO_THROW(timer->loadStateFromString("key_without_value"));
}