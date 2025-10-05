#include <gtest/gtest.h>
#include "../src/ui/screens/bulletin_board_screen.h"
#include "../src/core/engines/task_engine.h"
#include "../src/core/engines/gamification_engine.h"
#include "../src/input/input_manager.h"
#include "../src/ui/animations/animation_manager.h"

class BulletinBoardScreenTest : public ::testing::Test {
protected:
    void SetUp() override {
        inputManager.initialize();
        bulletinBoardScreen = std::make_unique<BulletinBoardScreen>(
            taskEngine, gamificationEngine, inputManager, animationManager
        );
    }
    
    void TearDown() override {
        inputManager.shutdown();
    }
    
    TaskEngine taskEngine;
    GamificationEngine gamificationEngine;
    InputManager inputManager;
    AnimationManager animationManager;
    std::unique_ptr<BulletinBoardScreen> bulletinBoardScreen;
};

TEST_F(BulletinBoardScreenTest, InitializationTest) {
    EXPECT_FALSE(bulletinBoardScreen->isActive());
    EXPECT_EQ(bulletinBoardScreen->getName(), "BulletinBoardScreen");
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskCreationDialog());
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskEditDialog());
    EXPECT_EQ(bulletinBoardScreen->getSelectedTaskId(), 0);
    EXPECT_EQ(bulletinBoardScreen->getBoardLayout(), "grid");
}

TEST_F(BulletinBoardScreenTest, ActivationTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    EXPECT_TRUE(bulletinBoardScreen->isActive());
    
    bulletinBoardScreen->onExit();
    bulletinBoardScreen->setActive(false);
    
    EXPECT_FALSE(bulletinBoardScreen->isActive());
}

TEST_F(BulletinBoardScreenTest, TaskCreationTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Create a task
    std::string taskTitle = "Test Task";
    Task::Priority priority = Task::Priority::HIGH;
    
    bulletinBoardScreen->createTask(taskTitle, priority);
    
    // Verify task was created in the engine
    std::vector<Task> tasks = taskEngine.getAllTasks();
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].getTitle(), taskTitle);
    EXPECT_EQ(tasks[0].getPriority(), priority);
    
    bulletinBoardScreen->onExit();
}

TEST_F(BulletinBoardScreenTest, TaskCompletionTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Create a task first
    bulletinBoardScreen->createTask("Test Task", Task::Priority::MEDIUM);
    
    std::vector<Task> tasks = taskEngine.getAllTasks();
    ASSERT_EQ(tasks.size(), 1);
    uint32_t taskId = tasks[0].getId();
    
    // Complete the task
    bulletinBoardScreen->completeTask(taskId);
    
    // Verify task was completed
    Task completedTask = taskEngine.getTask(taskId);
    EXPECT_EQ(completedTask.getStatus(), Task::Status::COMPLETED);
    
    bulletinBoardScreen->onExit();
}

TEST_F(BulletinBoardScreenTest, TaskDeletionTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Create a task first
    bulletinBoardScreen->createTask("Test Task", Task::Priority::LOW);
    
    std::vector<Task> tasks = taskEngine.getAllTasks();
    ASSERT_EQ(tasks.size(), 1);
    uint32_t taskId = tasks[0].getId();
    
    // Delete the task
    bulletinBoardScreen->deleteTask(taskId);
    
    // Verify task was deleted
    tasks = taskEngine.getAllTasks();
    EXPECT_EQ(tasks.size(), 0);
    
    bulletinBoardScreen->onExit();
}

TEST_F(BulletinBoardScreenTest, TaskFilterTest) {
    // Test task filtering
    EXPECT_EQ(bulletinBoardScreen->getTaskFilter(), "");
    
    bulletinBoardScreen->setTaskFilter("important");
    EXPECT_EQ(bulletinBoardScreen->getTaskFilter(), "important");
    
    bulletinBoardScreen->setTaskFilter("");
    EXPECT_EQ(bulletinBoardScreen->getTaskFilter(), "");
}

TEST_F(BulletinBoardScreenTest, TaskSortingTest) {
    // Test task sorting options
    EXPECT_EQ(bulletinBoardScreen->getSortBy(), "priority"); // Default
    
    bulletinBoardScreen->setSortBy("dueDate");
    EXPECT_EQ(bulletinBoardScreen->getSortBy(), "dueDate");
    
    bulletinBoardScreen->setSortBy("title");
    EXPECT_EQ(bulletinBoardScreen->getSortBy(), "title");
    
    bulletinBoardScreen->setSortBy("created");
    EXPECT_EQ(bulletinBoardScreen->getSortBy(), "created");
}

TEST_F(BulletinBoardScreenTest, CompletedTasksVisibilityTest) {
    // Test showing/hiding completed tasks
    EXPECT_FALSE(bulletinBoardScreen->isShowingCompletedTasks()); // Default
    
    bulletinBoardScreen->setShowCompletedTasks(true);
    EXPECT_TRUE(bulletinBoardScreen->isShowingCompletedTasks());
    
    bulletinBoardScreen->setShowCompletedTasks(false);
    EXPECT_FALSE(bulletinBoardScreen->isShowingCompletedTasks());
}

TEST_F(BulletinBoardScreenTest, BoardLayoutTest) {
    // Test different board layouts
    EXPECT_EQ(bulletinBoardScreen->getBoardLayout(), "grid"); // Default
    
    bulletinBoardScreen->setBoardLayout("list");
    EXPECT_EQ(bulletinBoardScreen->getBoardLayout(), "list");
    
    bulletinBoardScreen->setBoardLayout("kanban");
    EXPECT_EQ(bulletinBoardScreen->getBoardLayout(), "kanban");
    
    bulletinBoardScreen->setBoardLayout("grid");
    EXPECT_EQ(bulletinBoardScreen->getBoardLayout(), "grid");
}

TEST_F(BulletinBoardScreenTest, TaskCreationDialogTest) {
    // Test task creation dialog state
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskCreationDialog());
    
    bulletinBoardScreen->setShowTaskCreationDialog(true);
    EXPECT_TRUE(bulletinBoardScreen->isShowingTaskCreationDialog());
    
    bulletinBoardScreen->setShowTaskCreationDialog(false);
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskCreationDialog());
}

TEST_F(BulletinBoardScreenTest, TaskEditDialogTest) {
    // Test task edit dialog state
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskEditDialog());
    
    bulletinBoardScreen->setShowTaskEditDialog(true);
    EXPECT_TRUE(bulletinBoardScreen->isShowingTaskEditDialog());
    
    bulletinBoardScreen->setShowTaskEditDialog(false);
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskEditDialog());
}

TEST_F(BulletinBoardScreenTest, TaskSelectionTest) {
    // Test task selection
    EXPECT_EQ(bulletinBoardScreen->getSelectedTaskId(), 0); // No selection
    
    bulletinBoardScreen->setSelectedTaskId(123);
    EXPECT_EQ(bulletinBoardScreen->getSelectedTaskId(), 123);
    
    bulletinBoardScreen->setSelectedTaskId(0);
    EXPECT_EQ(bulletinBoardScreen->getSelectedTaskId(), 0);
}

TEST_F(BulletinBoardScreenTest, VisualSettingsTest) {
    // Test task details visibility
    EXPECT_TRUE(bulletinBoardScreen->isShowingTaskDetails()); // Default
    bulletinBoardScreen->setShowTaskDetails(false);
    EXPECT_FALSE(bulletinBoardScreen->isShowingTaskDetails());
    
    // Test overdue tasks visibility
    EXPECT_TRUE(bulletinBoardScreen->isShowingOverdueTasks()); // Default
    bulletinBoardScreen->setShowOverdueTasks(false);
    EXPECT_FALSE(bulletinBoardScreen->isShowingOverdueTasks());
}

TEST_F(BulletinBoardScreenTest, CallbackTest) {
    bool taskCompletedCalled = false;
    bool taskCreatedCalled = false;
    bool exitRequestedCalled = false;
    
    uint32_t completedTaskId = 0;
    int awardedXP = 0;
    uint32_t createdTaskId = 0;
    
    // Set up callbacks
    bulletinBoardScreen->setOnTaskCompleted([&](uint32_t taskId, int xp) {
        taskCompletedCalled = true;
        completedTaskId = taskId;
        awardedXP = xp;
    });
    
    bulletinBoardScreen->setOnTaskCreated([&](uint32_t taskId) {
        taskCreatedCalled = true;
        createdTaskId = taskId;
    });
    
    bulletinBoardScreen->setOnExitRequested([&]() {
        exitRequestedCalled = true;
    });
    
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Test task creation callback
    bulletinBoardScreen->createTask("Test Task", Task::Priority::HIGH);
    EXPECT_TRUE(taskCreatedCalled);
    EXPECT_NE(createdTaskId, 0);
    
    // Test task completion callback
    bulletinBoardScreen->completeTask(createdTaskId);
    EXPECT_TRUE(taskCompletedCalled);
    EXPECT_EQ(completedTaskId, createdTaskId);
    EXPECT_GT(awardedXP, 0);
    
    bulletinBoardScreen->onExit();
}

TEST_F(BulletinBoardScreenTest, UpdateTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Test that update doesn't crash
    float deltaTime = 0.016f; // ~60 FPS
    
    for (int i = 0; i < 10; i++) {
        bulletinBoardScreen->update(deltaTime);
    }
    
    // If we get here without crashing, the test passes
    EXPECT_TRUE(true);
    
    bulletinBoardScreen->onExit();
}

TEST_F(BulletinBoardScreenTest, TaskEditTest) {
    bulletinBoardScreen->setActive(true);
    bulletinBoardScreen->onEnter();
    
    // Create a task first
    bulletinBoardScreen->createTask("Original Task", Task::Priority::LOW);
    
    std::vector<Task> tasks = taskEngine.getAllTasks();
    ASSERT_EQ(tasks.size(), 1);
    uint32_t taskId = tasks[0].getId();
    
    // Edit the task (this should open the edit dialog)
    bulletinBoardScreen->editTask(taskId);
    
    // Verify edit dialog is shown and task is selected
    EXPECT_TRUE(bulletinBoardScreen->isShowingTaskEditDialog());
    EXPECT_EQ(bulletinBoardScreen->getSelectedTaskId(), taskId);
    
    bulletinBoardScreen->onExit();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}