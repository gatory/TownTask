#include <gtest/gtest.h>
#include "../src/core/engines/habit_tracker.h"
#include <chrono>
#include <thread>

class HabitTrackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        tracker = std::make_unique<HabitTracker>();
    }
    
    void TearDown() override {
        tracker.reset();
    }
    
    std::unique_ptr<HabitTracker> tracker;
    
    // Helper method to create a habit with check-ins
    uint32_t createHabitWithCheckIns(const std::string& name, int checkInCount) {
        uint32_t habitId = tracker->createHabit(name);
        
        auto now = std::chrono::system_clock::now();
        for (int i = checkInCount - 1; i >= 0; i--) {
            auto checkInDate = now - std::chrono::hours(24 * i);
            tracker->checkInHabit(habitId, checkInDate);
        }
        
        return habitId;
    }
};

// Constructor and Initial State Tests
TEST_F(HabitTrackerTest, Constructor_DefaultState_HasNoHabits) {
    EXPECT_EQ(tracker->getTotalHabitCount(), 0);
    EXPECT_TRUE(tracker->getAllHabits().empty());
}

// Habit Creation Tests
TEST_F(HabitTrackerTest, CreateHabit_WithName_ReturnsValidId) {
    uint32_t habitId = tracker->createHabit("Exercise");
    
    EXPECT_GT(habitId, 0);
    EXPECT_EQ(tracker->getTotalHabitCount(), 1);
    EXPECT_TRUE(tracker->habitExists(habitId));
}

TEST_F(HabitTrackerTest, CreateHabit_WithNameAndFrequency_SetsCorrectValues) {
    uint32_t habitId = tracker->createHabit("Read", Habit::Frequency::WEEKLY);
    
    auto habit = tracker->getHabit(habitId);
    ASSERT_TRUE(habit.has_value());
    EXPECT_EQ(habit->getName(), "Read");
    EXPECT_EQ(habit->getFrequency(), Habit::Frequency::WEEKLY);
}

TEST_F(HabitTrackerTest, CreateMultipleHabits_ReturnsUniqueIds) {
    uint32_t id1 = tracker->createHabit("Habit 1");
    uint32_t id2 = tracker->createHabit("Habit 2");
    uint32_t id3 = tracker->createHabit("Habit 3");
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
    EXPECT_EQ(tracker->getTotalHabitCount(), 3);
}

// Habit Modification Tests
TEST_F(HabitTrackerTest, SetHabitName_ValidHabit_UpdatesName) {
    uint32_t habitId = tracker->createHabit("Original Name");
    
    bool result = tracker->setHabitName(habitId, "New Name");
    
    EXPECT_TRUE(result);
    auto habit = tracker->getHabit(habitId);
    ASSERT_TRUE(habit.has_value());
    EXPECT_EQ(habit->getName(), "New Name");
}

TEST_F(HabitTrackerTest, SetHabitName_InvalidHabit_ReturnsFalse) {
    bool result = tracker->setHabitName(99999, "New Name");
    EXPECT_FALSE(result);
}

TEST_F(HabitTrackerTest, SetHabitFrequency_ValidHabit_UpdatesFrequency) {
    uint32_t habitId = tracker->createHabit("Exercise", Habit::Frequency::DAILY);
    
    bool result = tracker->setHabitFrequency(habitId, Habit::Frequency::WEEKLY);
    
    EXPECT_TRUE(result);
    auto habit = tracker->getHabit(habitId);
    ASSERT_TRUE(habit.has_value());
    EXPECT_EQ(habit->getFrequency(), Habit::Frequency::WEEKLY);
}

// Check-in Tests
TEST_F(HabitTrackerTest, CheckInHabit_ValidHabit_ReturnsTrue) {
    uint32_t habitId = tracker->createHabit("Exercise");
    
    bool result = tracker->checkInHabit(habitId);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tracker->getHabitStreak(habitId), 1);
}

TEST_F(HabitTrackerTest, CheckInHabit_InvalidHabit_ReturnsFalse) {
    bool result = tracker->checkInHabit(99999);
    EXPECT_FALSE(result);
}

TEST_F(HabitTrackerTest, CheckInHabit_WithSpecificDate_UpdatesStreak) {
    uint32_t habitId = tracker->createHabit("Exercise");
    auto yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
    
    bool result = tracker->checkInHabit(habitId, yesterday);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tracker->getHabitStreak(habitId), 1);
}

TEST_F(HabitTrackerTest, CheckInHabit_ConsecutiveDays_IncreasesStreak) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 3);
    
    EXPECT_EQ(tracker->getHabitStreak(habitId), 3);
    EXPECT_TRUE(tracker->isStreakActive(habitId));
}

// Streak Management Tests
TEST_F(HabitTrackerTest, ResetHabitStreak_ValidHabit_ResetsToZero) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 5);
    
    bool result = tracker->resetHabitStreak(habitId);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tracker->getHabitStreak(habitId), 0);
}

TEST_F(HabitTrackerTest, IsStreakActive_RecentCheckIn_ReturnsTrue) {
    uint32_t habitId = tracker->createHabit("Exercise");
    tracker->checkInHabit(habitId);
    
    EXPECT_TRUE(tracker->isStreakActive(habitId));
}

// Habit Deletion Tests
TEST_F(HabitTrackerTest, DeleteHabit_ValidHabit_RemovesHabit) {
    uint32_t habitId = tracker->createHabit("Exercise");
    
    bool result = tracker->deleteHabit(habitId);
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(tracker->habitExists(habitId));
    EXPECT_EQ(tracker->getTotalHabitCount(), 0);
}

TEST_F(HabitTrackerTest, DeleteHabit_InvalidHabit_ReturnsFalse) {
    bool result = tracker->deleteHabit(99999);
    EXPECT_FALSE(result);
}

// Querying Tests
TEST_F(HabitTrackerTest, GetAllHabits_WithMultipleHabits_ReturnsAllHabits) {
    tracker->createHabit("Exercise");
    tracker->createHabit("Read");
    tracker->createHabit("Meditate");
    
    auto habits = tracker->getAllHabits();
    
    EXPECT_EQ(habits.size(), 3);
}

TEST_F(HabitTrackerTest, GetHabitsByFrequency_WithMixedFrequencies_ReturnsMatchingHabits) {
    tracker->createHabit("Daily Exercise", Habit::Frequency::DAILY);
    tracker->createHabit("Weekly Review", Habit::Frequency::WEEKLY);
    tracker->createHabit("Daily Reading", Habit::Frequency::DAILY);
    
    auto dailyHabits = tracker->getHabitsByFrequency(Habit::Frequency::DAILY);
    
    EXPECT_EQ(dailyHabits.size(), 2);
}

TEST_F(HabitTrackerTest, GetActiveHabits_WithMixedHabits_ReturnsOnlyActive) {
    uint32_t activeId = createHabitWithCheckIns("Active Habit", 1);
    uint32_t inactiveId = tracker->createHabit("Inactive Habit");
    
    auto activeHabits = tracker->getActiveHabits();
    
    EXPECT_EQ(activeHabits.size(), 1);
    EXPECT_EQ(activeHabits[0].getId(), activeId);
}

TEST_F(HabitTrackerTest, GetInactiveHabits_WithMixedHabits_ReturnsOnlyInactive) {
    uint32_t activeId = createHabitWithCheckIns("Active Habit", 1);
    uint32_t inactiveId = tracker->createHabit("Inactive Habit");
    
    auto inactiveHabits = tracker->getInactiveHabits();
    
    EXPECT_EQ(inactiveHabits.size(), 1);
    EXPECT_EQ(inactiveHabits[0].getId(), inactiveId);
}

// Progress Tracking Tests
TEST_F(HabitTrackerTest, GetHabitCompletionRate_WithCheckIns_ReturnsCorrectRate) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 15); // 15 out of 30 days
    
    float rate = tracker->getHabitCompletionRate(habitId, 30);
    
    EXPECT_FLOAT_EQ(rate, 0.5f); // 15/30 = 0.5
}

TEST_F(HabitTrackerTest, GetHabitCompletions_WithCheckIns_ReturnsCompletionDates) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 3);
    
    auto completions = tracker->getHabitCompletions(habitId, 7);
    
    EXPECT_EQ(completions.size(), 3);
}

TEST_F(HabitTrackerTest, GetDaysSinceLastCompletion_WithRecentCheckIn_ReturnsCorrectDays) {
    uint32_t habitId = tracker->createHabit("Exercise");
    auto twoDaysAgo = std::chrono::system_clock::now() - std::chrono::hours(48);
    tracker->checkInHabit(habitId, twoDaysAgo);
    
    int days = tracker->getDaysSinceLastCompletion(habitId);
    
    EXPECT_GE(days, 1); // At least 1 day, could be 2 depending on timing
    EXPECT_LE(days, 2);
}

// Milestone Tests
TEST_F(HabitTrackerTest, GetHabitMilestones_WithStreak_ReturnsCorrectMilestones) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 7);
    
    auto milestones = tracker->getHabitMilestones(habitId);
    
    EXPECT_GT(milestones.size(), 0);
    
    // Check that 7-day milestone is achieved
    bool found7Day = false;
    for (const auto& milestone : milestones) {
        if (milestone.streakTarget == 7) {
            EXPECT_TRUE(milestone.isAchieved);
            found7Day = true;
            break;
        }
    }
    EXPECT_TRUE(found7Day);
}

TEST_F(HabitTrackerTest, GetAchievedMilestones_WithStreak_ReturnsOnlyAchieved) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 7);
    
    auto achieved = tracker->getAchievedMilestones(habitId);
    
    EXPECT_GT(achieved.size(), 0);
    for (const auto& milestone : achieved) {
        EXPECT_TRUE(milestone.isAchieved);
        EXPECT_LE(milestone.streakTarget, 7);
    }
}

TEST_F(HabitTrackerTest, GetUpcomingMilestones_WithStreak_ReturnsOnlyUpcoming) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 5);
    
    auto upcoming = tracker->getUpcomingMilestones(habitId);
    
    EXPECT_GT(upcoming.size(), 0);
    for (const auto& milestone : upcoming) {
        EXPECT_FALSE(milestone.isAchieved);
        EXPECT_GT(milestone.streakTarget, 5);
    }
}

TEST_F(HabitTrackerTest, HasReachedMilestone_WithSufficientStreak_ReturnsTrue) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 7);
    
    EXPECT_TRUE(tracker->hasReachedMilestone(habitId, 7));
    EXPECT_TRUE(tracker->hasReachedMilestone(habitId, 3));
    EXPECT_FALSE(tracker->hasReachedMilestone(habitId, 14));
}

// Statistics Tests
TEST_F(HabitTrackerTest, GetHabitStatistics_WithHabit_ReturnsCorrectStats) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 5);
    
    auto stats = tracker->getHabitStatistics(habitId);
    
    EXPECT_EQ(stats.habitId, habitId);
    EXPECT_EQ(stats.name, "Exercise");
    EXPECT_EQ(stats.currentStreak, 5);
    EXPECT_TRUE(stats.isActive);
}

TEST_F(HabitTrackerTest, GetAllHabitStatistics_WithMultipleHabits_ReturnsAllStats) {
    tracker->createHabit("Exercise");
    tracker->createHabit("Read");
    
    auto allStats = tracker->getAllHabitStatistics();
    
    EXPECT_EQ(allStats.size(), 2);
}

// Sorting Tests
TEST_F(HabitTrackerTest, GetSortedHabits_ByNameAsc_ReturnsSortedHabits) {
    tracker->createHabit("Zebra");
    tracker->createHabit("Apple");
    tracker->createHabit("Banana");
    
    auto sorted = tracker->getSortedHabits(HabitTracker::SortBy::NAME_ASC);
    
    EXPECT_EQ(sorted[0].getName(), "Apple");
    EXPECT_EQ(sorted[1].getName(), "Banana");
    EXPECT_EQ(sorted[2].getName(), "Zebra");
}

TEST_F(HabitTrackerTest, GetSortedHabits_ByStreakDesc_ReturnsSortedHabits) {
    uint32_t id1 = createHabitWithCheckIns("Low Streak", 2);
    uint32_t id2 = createHabitWithCheckIns("High Streak", 5);
    uint32_t id3 = createHabitWithCheckIns("Medium Streak", 3);
    
    auto sorted = tracker->getSortedHabits(HabitTracker::SortBy::STREAK_DESC);
    
    EXPECT_EQ(sorted[0].getCurrentStreak(), 5);
    EXPECT_EQ(sorted[1].getCurrentStreak(), 3);
    EXPECT_EQ(sorted[2].getCurrentStreak(), 2);
}

// Calendar View Tests
TEST_F(HabitTrackerTest, GetHabitCalendar_WithCheckIns_ReturnsCorrectCalendar) {
    uint32_t habitId = createHabitWithCheckIns("Exercise", 3);
    
    auto calendar = tracker->getHabitCalendar(habitId, 7);
    
    EXPECT_EQ(calendar.size(), 7);
    
    // Last 3 days should be completed
    int completedCount = 0;
    for (const auto& day : calendar) {
        if (day.completed) {
            completedCount++;
        }
    }
    EXPECT_EQ(completedCount, 3);
}

// Bulk Operations Tests
TEST_F(HabitTrackerTest, CreateMultipleHabits_WithHabitData_CreatesAllHabits) {
    std::vector<std::pair<std::string, Habit::Frequency>> data = {
        {"Exercise", Habit::Frequency::DAILY},
        {"Read", Habit::Frequency::WEEKLY},
        {"Meditate", Habit::Frequency::DAILY}
    };
    
    auto ids = tracker->createMultipleHabits(data);
    
    EXPECT_EQ(ids.size(), 3);
    EXPECT_EQ(tracker->getTotalHabitCount(), 3);
}

TEST_F(HabitTrackerTest, DeleteMultipleHabits_WithValidIds_DeletesAllHabits) {
    auto id1 = tracker->createHabit("Habit 1");
    auto id2 = tracker->createHabit("Habit 2");
    auto id3 = tracker->createHabit("Habit 3");
    
    bool result = tracker->deleteMultipleHabits({id1, id2, id3});
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tracker->getTotalHabitCount(), 0);
}

TEST_F(HabitTrackerTest, CheckInMultipleHabits_WithValidIds_ChecksInAll) {
    auto id1 = tracker->createHabit("Habit 1");
    auto id2 = tracker->createHabit("Habit 2");
    
    bool result = tracker->checkInMultipleHabits({id1, id2});
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tracker->getHabitStreak(id1), 1);
    EXPECT_EQ(tracker->getHabitStreak(id2), 1);
}

// Global Statistics Tests
TEST_F(HabitTrackerTest, GetActiveHabitCount_WithMixedHabits_ReturnsCorrectCount) {
    createHabitWithCheckIns("Active 1", 1);
    createHabitWithCheckIns("Active 2", 1);
    tracker->createHabit("Inactive");
    
    EXPECT_EQ(tracker->getActiveHabitCount(), 2);
    EXPECT_EQ(tracker->getInactiveHabitCount(), 1);
}

TEST_F(HabitTrackerTest, GetTotalCompletionsToday_WithTodayCheckIns_ReturnsCorrectCount) {
    auto id1 = tracker->createHabit("Habit 1");
    auto id2 = tracker->createHabit("Habit 2");
    auto id3 = tracker->createHabit("Habit 3");
    
    tracker->checkInHabit(id1); // Today
    tracker->checkInHabit(id2); // Today
    
    // Check in for yesterday (should not count)
    auto yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
    tracker->checkInHabit(id3, yesterday);
    
    EXPECT_EQ(tracker->getTotalCompletionsToday(), 2);
}

TEST_F(HabitTrackerTest, GetOverallCompletionRate_WithHabits_ReturnsAverageRate) {
    uint32_t id1 = createHabitWithCheckIns("Habit 1", 10); // 10/30 = 0.33
    uint32_t id2 = createHabitWithCheckIns("Habit 2", 20); // 20/30 = 0.67
    
    float rate = tracker->getOverallCompletionRate(30);
    
    EXPECT_FLOAT_EQ(rate, 0.5f); // (0.33 + 0.67) / 2 = 0.5
}

// Streak Analysis Tests
TEST_F(HabitTrackerTest, GetLongestCurrentStreak_WithMultipleHabits_ReturnsLongest) {
    createHabitWithCheckIns("Short", 3);
    createHabitWithCheckIns("Long", 7);
    createHabitWithCheckIns("Medium", 5);
    
    EXPECT_EQ(tracker->getLongestCurrentStreak(), 7);
}

TEST_F(HabitTrackerTest, GetHabitsWithActiveStreaks_WithMixedHabits_ReturnsOnlyActive) {
    uint32_t activeId = createHabitWithCheckIns("Active", 3);
    uint32_t inactiveId = tracker->createHabit("Inactive");
    
    auto activeStreaks = tracker->getHabitsWithActiveStreaks();
    
    EXPECT_EQ(activeStreaks.size(), 1);
    EXPECT_EQ(activeStreaks[0], activeId);
}

TEST_F(HabitTrackerTest, GetHabitsAtMilestone_WithSpecificStreak_ReturnsMatchingHabits) {
    uint32_t id1 = createHabitWithCheckIns("Habit 1", 7);
    uint32_t id2 = createHabitWithCheckIns("Habit 2", 5);
    uint32_t id3 = createHabitWithCheckIns("Habit 3", 7);
    
    auto habitsAt7 = tracker->getHabitsAtMilestone(7);
    
    EXPECT_EQ(habitsAt7.size(), 2);
}

// Data Management Tests
TEST_F(HabitTrackerTest, LoadHabits_WithHabitList_LoadsAllHabits) {
    Habit habit1("Exercise", Habit::Frequency::DAILY);
    Habit habit2("Read", Habit::Frequency::WEEKLY);
    std::vector<Habit> habits = {habit1, habit2};
    
    tracker->loadHabits(habits);
    
    EXPECT_EQ(tracker->getTotalHabitCount(), 2);
}

TEST_F(HabitTrackerTest, ExportHabits_WithHabits_ReturnsAllHabits) {
    tracker->createHabit("Habit 1");
    tracker->createHabit("Habit 2");
    
    auto exported = tracker->exportHabits();
    
    EXPECT_EQ(exported.size(), 2);
}

TEST_F(HabitTrackerTest, ClearAllHabits_WithHabits_RemovesAllHabits) {
    tracker->createHabit("Habit 1");
    tracker->createHabit("Habit 2");
    
    tracker->clearAllHabits();
    
    EXPECT_EQ(tracker->getTotalHabitCount(), 0);
}

// Callback Tests
TEST_F(HabitTrackerTest, HabitCreationCallback_WhenHabitCreated_CallsCallback) {
    bool callbackCalled = false;
    std::string createdName;
    
    tracker->setHabitCreationCallback([&](const Habit& habit) {
        callbackCalled = true;
        createdName = habit.getName();
    });
    
    tracker->createHabit("Test Habit");
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(createdName, "Test Habit");
}

TEST_F(HabitTrackerTest, CheckInCallback_WhenCheckedIn_CallsCallback) {
    bool callbackCalled = false;
    uint32_t checkedInId = 0;
    
    tracker->setCheckInCallback([&](uint32_t habitId, const std::chrono::system_clock::time_point& date) {
        callbackCalled = true;
        checkedInId = habitId;
    });
    
    uint32_t habitId = tracker->createHabit("Exercise");
    tracker->checkInHabit(habitId);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(checkedInId, habitId);
}

// Validation Tests
TEST_F(HabitTrackerTest, HabitExists_WithValidId_ReturnsTrue) {
    uint32_t habitId = tracker->createHabit("Exercise");
    
    EXPECT_TRUE(tracker->habitExists(habitId));
    EXPECT_TRUE(tracker->isValidHabitId(habitId));
}

TEST_F(HabitTrackerTest, HabitExists_WithInvalidId_ReturnsFalse) {
    EXPECT_FALSE(tracker->habitExists(99999));
    EXPECT_FALSE(tracker->isValidHabitId(99999));
}

TEST_F(HabitTrackerTest, CanCheckInToday_WithoutTodayCheckIn_ReturnsTrue) {
    uint32_t habitId = tracker->createHabit("Exercise");
    
    EXPECT_TRUE(tracker->canCheckInToday(habitId));
    EXPECT_FALSE(tracker->hasCheckedInToday(habitId));
}

TEST_F(HabitTrackerTest, CanCheckInToday_WithTodayCheckIn_ReturnsFalse) {
    uint32_t habitId = tracker->createHabit("Exercise");
    tracker->checkInHabit(habitId);
    
    EXPECT_FALSE(tracker->canCheckInToday(habitId));
    EXPECT_TRUE(tracker->hasCheckedInToday(habitId));
}

// Reminder Tests
TEST_F(HabitTrackerTest, GetHabitsNeedingAttention_WithDailyHabits_ReturnsUncheckedHabits) {
    uint32_t checkedId = tracker->createHabit("Checked Habit", Habit::Frequency::DAILY);
    uint32_t uncheckedId = tracker->createHabit("Unchecked Habit", Habit::Frequency::DAILY);
    
    tracker->checkInHabit(checkedId);
    
    auto needingAttention = tracker->getHabitsNeedingAttention();
    
    EXPECT_EQ(needingAttention.size(), 1);
    EXPECT_EQ(needingAttention[0], uncheckedId);
}

TEST_F(HabitTrackerTest, GetHabitsAtRisk_WithActiveStreaks_ReturnsUncheckedHabits) {
    // Create a habit with check-ins but not today (so it's at risk)
    uint32_t atRiskId = tracker->createHabit("At Risk");
    auto now = std::chrono::system_clock::now();
    
    // Check in for the past 3 days but not today
    for (int i = 3; i >= 1; i--) {
        auto checkInDate = now - std::chrono::hours(24 * i);
        tracker->checkInHabit(atRiskId, checkInDate);
    }
    
    uint32_t safeId = tracker->createHabit("Safe Habit");
    tracker->checkInHabit(safeId); // Check in today
    
    auto atRisk = tracker->getHabitsAtRisk();
    
    EXPECT_EQ(atRisk.size(), 1);
    EXPECT_EQ(atRisk[0], atRiskId);
}