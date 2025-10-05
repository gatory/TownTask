#include <gtest/gtest.h>
#include "../src/ui/screens/gym_screen.h"
#include "../src/core/engines/habit_tracker.h"
#include "../src/core/engines/gamification_engine.h"
#include "../src/input/input_manager.h"
#include "../src/ui/animations/animation_manager.h"

class GymScreenTest : public ::testing::Test {
protected:
    void SetUp() override {
        inputManager.initialize();
        gymScreen = std::make_unique<GymScreen>(
            habitTracker, gamificationEngine, inputManager, animationManager
        );
    }
    
    void TearDown() override {
        inputManager.shutdown();
    }
    
    HabitTracker habitTracker;
    GamificationEngine gamificationEngine;
    InputManager inputManager;
    AnimationManager animationManager;
    std::unique_ptr<GymScreen> gymScreen;
};

TEST_F(GymScreenTest, InitializationTest) {
    EXPECT_FALSE(gymScreen->isActive());
    EXPECT_EQ(gymScreen->getName(), "GymScreen");
    EXPECT_FALSE(gymScreen->isShowingHabitCreationDialog());
    EXPECT_FALSE(gymScreen->isShowingHabitEditDialog());
    EXPECT_FALSE(gymScreen->isShowingCalendarView());
    EXPECT_EQ(gymScreen->getSelectedHabitId(), 0);
    EXPECT_EQ(gymScreen->getViewMode(), "grid");
    EXPECT_EQ(gymScreen->getGymLevel(), 1);
}

TEST_F(GymScreenTest, ActivationTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    EXPECT_TRUE(gymScreen->isActive());
    
    gymScreen->onExit();
    gymScreen->setActive(false);
    
    EXPECT_FALSE(gymScreen->isActive());
}

TEST_F(GymScreenTest, HabitCreationTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit
    std::string habitName = "Daily Exercise";
    int targetFrequency = 1;
    std::string unit = "times";
    
    gymScreen->createHabit(habitName, targetFrequency, unit);
    
    // Verify habit was created in the tracker
    std::vector<Habit> habits = habitTracker.getAllHabits();
    EXPECT_EQ(habits.size(), 1);
    EXPECT_EQ(habits[0].getName(), habitName);
    EXPECT_EQ(habits[0].getTargetFrequency(), targetFrequency);
    EXPECT_EQ(habits[0].getUnit(), unit);
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, HabitCheckInTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit first
    gymScreen->createHabit("Daily Exercise", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Check initial streak
    EXPECT_EQ(gymScreen->getHabitStreak(habitId), 0);
    
    // Check in the habit
    gymScreen->checkInHabit(habitId);
    
    // Verify streak increased
    EXPECT_EQ(gymScreen->getHabitStreak(habitId), 1);
    
    // Verify habit was completed today
    Habit updatedHabit = habitTracker.getHabit(habitId);
    EXPECT_TRUE(updatedHabit.wasCompletedToday());
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, HabitDeletionTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit first
    gymScreen->createHabit("Daily Exercise", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Delete the habit
    gymScreen->deleteHabit(habitId);
    
    // Verify habit was deleted
    habits = habitTracker.getAllHabits();
    EXPECT_EQ(habits.size(), 0);
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, HabitPauseResumeTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit first
    gymScreen->createHabit("Daily Exercise", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Verify habit is initially active
    Habit habit = habitTracker.getHabit(habitId);
    EXPECT_TRUE(habit.isActive());
    
    // Pause the habit
    gymScreen->pauseHabit(habitId);
    
    // Verify habit is paused
    habit = habitTracker.getHabit(habitId);
    EXPECT_FALSE(habit.isActive());
    
    // Resume the habit
    gymScreen->resumeHabit(habitId);
    
    // Verify habit is active again
    habit = habitTracker.getHabit(habitId);
    EXPECT_TRUE(habit.isActive());
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, ViewModeTest) {
    // Test different view modes
    EXPECT_EQ(gymScreen->getViewMode(), "grid"); // Default
    
    gymScreen->setViewMode("list");
    EXPECT_EQ(gymScreen->getViewMode(), "list");
    
    gymScreen->setViewMode("grid");
    EXPECT_EQ(gymScreen->getViewMode(), "grid");
}

TEST_F(GymScreenTest, HabitSelectionTest) {
    // Test habit selection
    EXPECT_EQ(gymScreen->getSelectedHabitId(), 0); // No selection
    
    gymScreen->setSelectedHabitId(123);
    EXPECT_EQ(gymScreen->getSelectedHabitId(), 123);
    
    gymScreen->setSelectedHabitId(0);
    EXPECT_EQ(gymScreen->getSelectedHabitId(), 0);
}

TEST_F(GymScreenTest, CalendarViewTest) {
    // Test calendar view toggle
    EXPECT_FALSE(gymScreen->isShowingCalendarView()); // Default
    
    gymScreen->setShowCalendarView(true);
    EXPECT_TRUE(gymScreen->isShowingCalendarView());
    
    gymScreen->setShowCalendarView(false);
    EXPECT_FALSE(gymScreen->isShowingCalendarView());
}

TEST_F(GymScreenTest, VisualSettingsTest) {
    // Test streaks visibility
    EXPECT_TRUE(gymScreen->isShowingStreaks()); // Default
    gymScreen->setShowStreaks(false);
    EXPECT_FALSE(gymScreen->isShowingStreaks());
    
    // Test progress visibility
    EXPECT_TRUE(gymScreen->isShowingProgress()); // Default
    gymScreen->setShowProgress(false);
    EXPECT_FALSE(gymScreen->isShowingProgress());
    
    // Test milestones visibility
    EXPECT_TRUE(gymScreen->isShowingMilestones()); // Default
    gymScreen->setShowMilestones(false);
    EXPECT_FALSE(gymScreen->isShowingMilestones());
    
    // Test gym level
    EXPECT_EQ(gymScreen->getGymLevel(), 1); // Default
    gymScreen->setGymLevel(3);
    EXPECT_EQ(gymScreen->getGymLevel(), 3);
    
    // Test gym level clamping
    gymScreen->setGymLevel(0); // Too low
    EXPECT_EQ(gymScreen->getGymLevel(), 1); // Should clamp to minimum
    
    gymScreen->setGymLevel(10); // Too high
    EXPECT_EQ(gymScreen->getGymLevel(), 5); // Should clamp to maximum
}

TEST_F(GymScreenTest, HabitCreationDialogTest) {
    // Test habit creation dialog state
    EXPECT_FALSE(gymScreen->isShowingHabitCreationDialog());
    
    gymScreen->setShowHabitCreationDialog(true);
    EXPECT_TRUE(gymScreen->isShowingHabitCreationDialog());
    
    gymScreen->setShowHabitCreationDialog(false);
    EXPECT_FALSE(gymScreen->isShowingHabitCreationDialog());
}

TEST_F(GymScreenTest, HabitEditDialogTest) {
    // Test habit edit dialog state
    EXPECT_FALSE(gymScreen->isShowingHabitEditDialog());
    
    gymScreen->setShowHabitEditDialog(true);
    EXPECT_TRUE(gymScreen->isShowingHabitEditDialog());
    
    gymScreen->setShowHabitEditDialog(false);
    EXPECT_FALSE(gymScreen->isShowingHabitEditDialog());
}

TEST_F(GymScreenTest, HabitCompletionRateTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit
    gymScreen->createHabit("Daily Exercise", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Initially, completion rate should be 0
    float completionRate = gymScreen->getHabitCompletionRate(habitId, 7);
    EXPECT_EQ(completionRate, 0.0f);
    
    // After checking in, completion rate should increase
    gymScreen->checkInHabit(habitId);
    completionRate = gymScreen->getHabitCompletionRate(habitId, 1); // Check for 1 day
    EXPECT_GT(completionRate, 0.0f);
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, HabitCalendarDataTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit
    gymScreen->createHabit("Daily Exercise", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Get calendar data
    std::vector<bool> calendarData = gymScreen->getHabitCalendarData(habitId, 7);
    EXPECT_EQ(calendarData.size(), 7);
    
    // Initially, all days should be false (not completed)
    for (bool completed : calendarData) {
        EXPECT_FALSE(completed);
    }
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, CallbackTest) {
    bool habitCheckedInCalled = false;
    bool habitCreatedCalled = false;
    bool milestoneAchievedCalled = false;
    bool exitRequestedCalled = false;
    
    uint32_t checkedInHabitId = 0;
    int newStreak = 0;
    uint32_t createdHabitId = 0;
    uint32_t milestoneHabitId = 0;
    int milestoneValue = 0;
    
    // Set up callbacks
    gymScreen->setOnHabitCheckedIn([&](uint32_t habitId, int streak) {
        habitCheckedInCalled = true;
        checkedInHabitId = habitId;
        newStreak = streak;
    });
    
    gymScreen->setOnHabitCreated([&](uint32_t habitId) {
        habitCreatedCalled = true;
        createdHabitId = habitId;
    });
    
    gymScreen->setOnMilestoneAchieved([&](uint32_t habitId, int milestone) {
        milestoneAchievedCalled = true;
        milestoneHabitId = habitId;
        milestoneValue = milestone;
    });
    
    gymScreen->setOnExitRequested([&]() {
        exitRequestedCalled = true;
    });
    
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Test habit creation callback
    gymScreen->createHabit("Daily Exercise", 1, "times");
    EXPECT_TRUE(habitCreatedCalled);
    EXPECT_NE(createdHabitId, 0);
    
    // Test habit check-in callback
    gymScreen->checkInHabit(createdHabitId);
    EXPECT_TRUE(habitCheckedInCalled);
    EXPECT_EQ(checkedInHabitId, createdHabitId);
    EXPECT_EQ(newStreak, 1);
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, UpdateTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Test that update doesn't crash
    float deltaTime = 0.016f; // ~60 FPS
    
    for (int i = 0; i < 10; i++) {
        gymScreen->update(deltaTime);
    }
    
    // If we get here without crashing, the test passes
    EXPECT_TRUE(true);
    
    gymScreen->onExit();
}

TEST_F(GymScreenTest, HabitEditTest) {
    gymScreen->setActive(true);
    gymScreen->onEnter();
    
    // Create a habit first
    gymScreen->createHabit("Original Habit", 1, "times");
    
    std::vector<Habit> habits = habitTracker.getAllHabits();
    ASSERT_EQ(habits.size(), 1);
    uint32_t habitId = habits[0].getId();
    
    // Edit the habit (this should open the edit dialog)
    gymScreen->editHabit(habitId);
    
    // Verify edit dialog is shown and habit is selected
    EXPECT_TRUE(gymScreen->isShowingHabitEditDialog());
    EXPECT_EQ(gymScreen->getSelectedHabitId(), habitId);
    
    gymScreen->onExit();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}