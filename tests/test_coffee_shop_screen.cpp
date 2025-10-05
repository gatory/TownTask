#include <gtest/gtest.h>
#include "../src/ui/screens/coffee_shop_screen.h"
#include "../src/core/engines/pomodoro_timer.h"
#include "../src/core/engines/gamification_engine.h"
#include "../src/input/input_manager.h"
#include "../src/ui/animations/animation_manager.h"

class CoffeeShopScreenTest : public ::testing::Test {
protected:
    void SetUp() override {
        inputManager.initialize();
        coffeeShopScreen = std::make_unique<CoffeeShopScreen>(
            pomodoroTimer, gamificationEngine, inputManager, animationManager
        );
    }
    
    void TearDown() override {
        inputManager.shutdown();
    }
    
    PomodoroTimer pomodoroTimer;
    GamificationEngine gamificationEngine;
    InputManager inputManager;
    AnimationManager animationManager;
    std::unique_ptr<CoffeeShopScreen> coffeeShopScreen;
};

TEST_F(CoffeeShopScreenTest, InitializationTest) {
    EXPECT_FALSE(coffeeShopScreen->isActive());
    EXPECT_EQ(coffeeShopScreen->getName(), "CoffeeShopScreen");
    EXPECT_FALSE(coffeeShopScreen->isShowingCustomDurationDialog());
    EXPECT_EQ(coffeeShopScreen->getCustomDuration(), 25); // Default work duration
    EXPECT_EQ(coffeeShopScreen->getCoffeeShopLevel(), 1); // Default level
}

TEST_F(CoffeeShopScreenTest, ActivationTest) {
    coffeeShopScreen->setActive(true);
    coffeeShopScreen->onEnter();
    
    EXPECT_TRUE(coffeeShopScreen->isActive());
    
    coffeeShopScreen->onExit();
    coffeeShopScreen->setActive(false);
    
    EXPECT_FALSE(coffeeShopScreen->isActive());
}

TEST_F(CoffeeShopScreenTest, CustomDurationTest) {
    // Test setting custom duration
    coffeeShopScreen->setCustomDuration(30);
    EXPECT_EQ(coffeeShopScreen->getCustomDuration(), 30);
    
    // Test clamping
    coffeeShopScreen->setCustomDuration(0);
    EXPECT_EQ(coffeeShopScreen->getCustomDuration(), 1); // Should clamp to minimum
    
    coffeeShopScreen->setCustomDuration(150);
    EXPECT_EQ(coffeeShopScreen->getCustomDuration(), 120); // Should clamp to maximum
}

TEST_F(CoffeeShopScreenTest, CustomDurationDialogTest) {
    EXPECT_FALSE(coffeeShopScreen->isShowingCustomDurationDialog());
    
    coffeeShopScreen->setShowCustomDurationDialog(true);
    EXPECT_TRUE(coffeeShopScreen->isShowingCustomDurationDialog());
    
    coffeeShopScreen->setShowCustomDurationDialog(false);
    EXPECT_FALSE(coffeeShopScreen->isShowingCustomDurationDialog());
}

TEST_F(CoffeeShopScreenTest, VisualSettingsTest) {
    // Test timer details visibility
    EXPECT_TRUE(coffeeShopScreen->isShowingTimerDetails()); // Default should be true
    coffeeShopScreen->setShowTimerDetails(false);
    EXPECT_FALSE(coffeeShopScreen->isShowingTimerDetails());
    
    // Test coffee rewards visibility
    EXPECT_TRUE(coffeeShopScreen->isShowingCoffeeRewards()); // Default should be true
    coffeeShopScreen->setShowCoffeeRewards(false);
    EXPECT_FALSE(coffeeShopScreen->isShowingCoffeeRewards());
    
    // Test session history visibility
    EXPECT_FALSE(coffeeShopScreen->isShowingSessionHistory()); // Default should be false
    coffeeShopScreen->setShowSessionHistory(true);
    EXPECT_TRUE(coffeeShopScreen->isShowingSessionHistory());
}

TEST_F(CoffeeShopScreenTest, CoffeeShopLevelTest) {
    EXPECT_EQ(coffeeShopScreen->getCoffeeShopLevel(), 1); // Default level
    
    coffeeShopScreen->setCoffeeShopLevel(3);
    EXPECT_EQ(coffeeShopScreen->getCoffeeShopLevel(), 3);
    
    // Test clamping
    coffeeShopScreen->setCoffeeShopLevel(0);
    EXPECT_EQ(coffeeShopScreen->getCoffeeShopLevel(), 1); // Should clamp to minimum
    
    coffeeShopScreen->setCoffeeShopLevel(10);
    EXPECT_EQ(coffeeShopScreen->getCoffeeShopLevel(), 5); // Should clamp to maximum
}

TEST_F(CoffeeShopScreenTest, DecorationManagementTest) {
    // Initially no decorations
    EXPECT_TRUE(coffeeShopScreen->getDecorations().empty());
    
    // Add decorations
    coffeeShopScreen->addDecoration("plant_01");
    coffeeShopScreen->addDecoration("painting_01");
    
    auto decorations = coffeeShopScreen->getDecorations();
    EXPECT_EQ(decorations.size(), 2);
    EXPECT_TRUE(std::find(decorations.begin(), decorations.end(), "plant_01") != decorations.end());
    EXPECT_TRUE(std::find(decorations.begin(), decorations.end(), "painting_01") != decorations.end());
    
    // Test duplicate prevention
    coffeeShopScreen->addDecoration("plant_01");
    EXPECT_EQ(coffeeShopScreen->getDecorations().size(), 2); // Should still be 2
    
    // Remove decoration
    coffeeShopScreen->removeDecoration("plant_01");
    decorations = coffeeShopScreen->getDecorations();
    EXPECT_EQ(decorations.size(), 1);
    EXPECT_TRUE(std::find(decorations.begin(), decorations.end(), "painting_01") != decorations.end());
    EXPECT_TRUE(std::find(decorations.begin(), decorations.end(), "plant_01") == decorations.end());
}

TEST_F(CoffeeShopScreenTest, PomodoroTimerIntegrationTest) {
    coffeeShopScreen->setActive(true);
    coffeeShopScreen->onEnter();
    
    // Test starting a work session
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::STOPPED);
    
    coffeeShopScreen->startPomodoroSession(PomodoroTimer::SessionType::WORK);
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::RUNNING);
    EXPECT_EQ(pomodoroTimer.getCurrentSessionType(), PomodoroTimer::SessionType::WORK);
    
    // Test pausing
    coffeeShopScreen->pauseTimer();
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::PAUSED);
    
    // Test resuming
    coffeeShopScreen->resumeTimer();
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::RUNNING);
    
    // Test stopping
    coffeeShopScreen->stopTimer();
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::STOPPED);
    
    coffeeShopScreen->onExit();
}

TEST_F(CoffeeShopScreenTest, CustomDurationSessionTest) {
    coffeeShopScreen->setActive(true);
    coffeeShopScreen->onEnter();
    
    // Test starting a custom duration session
    int customMinutes = 45;
    coffeeShopScreen->startPomodoroSession(PomodoroTimer::SessionType::WORK, customMinutes);
    
    EXPECT_EQ(pomodoroTimer.getState(), PomodoroTimer::TimerState::RUNNING);
    EXPECT_EQ(pomodoroTimer.getCurrentSessionType(), PomodoroTimer::SessionType::WORK);
    // Note: We can't easily test the exact duration without exposing more timer internals
    
    coffeeShopScreen->onExit();
}

TEST_F(CoffeeShopScreenTest, CallbackTest) {
    bool sessionCompletedCalled = false;
    bool coffeeEarnedCalled = false;
    bool exitRequestedCalled = false;
    
    PomodoroTimer::SessionType completedSessionType;
    int completedSessionDuration = 0;
    int earnedCoffeeAmount = 0;
    
    // Set up callbacks
    coffeeShopScreen->setOnSessionCompleted([&](PomodoroTimer::SessionType sessionType, int duration) {
        sessionCompletedCalled = true;
        completedSessionType = sessionType;
        completedSessionDuration = duration;
    });
    
    coffeeShopScreen->setOnCoffeeEarned([&](int amount) {
        coffeeEarnedCalled = true;
        earnedCoffeeAmount = amount;
    });
    
    coffeeShopScreen->setOnExitRequested([&]() {
        exitRequestedCalled = true;
    });
    
    // Test that callbacks are set (we can't easily trigger them without complex timer manipulation)
    // This test mainly verifies that the callback setters work without crashing
    EXPECT_FALSE(sessionCompletedCalled);
    EXPECT_FALSE(coffeeEarnedCalled);
    EXPECT_FALSE(exitRequestedCalled);
}

TEST_F(CoffeeShopScreenTest, UpdateTest) {
    coffeeShopScreen->setActive(true);
    coffeeShopScreen->onEnter();
    
    // Test that update doesn't crash
    float deltaTime = 0.016f; // ~60 FPS
    
    for (int i = 0; i < 10; i++) {
        coffeeShopScreen->update(deltaTime);
    }
    
    // If we get here without crashing, the test passes
    EXPECT_TRUE(true);
    
    coffeeShopScreen->onExit();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}