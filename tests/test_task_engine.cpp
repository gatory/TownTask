#include <gtest/gtest.h>
#include "../src/core/engines/task_engine.h"
#include <chrono>
#include <thread>
#include <algorithm>

class TaskEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        taskEngine = std::make_unique<TaskEngine>();
    }
    
    void TearDown() override {
        taskEngine.reset();
    }
    
    std::unique_ptr<TaskEngine> taskEngine;
    
    // Helper method to create a task with specific due date
    uint32_t createTaskWithDueDate(const std::string& title, int daysFromNow) {
        auto dueDate = std::chrono::system_clock::now() + std::chrono::hours(24 * daysFromNow);
        return taskEngine->createTask(title, Task::MEDIUM, dueDate);
    }
};

// Task Creation Tests
TEST_F(TaskEngineTest, CreateTask_ValidInput_ReturnsTaskId) {
    uint32_t taskId = taskEngine->createTask("Test Task", Task::HIGH);
    
    EXPECT_GT(taskId, 0);
    EXPECT_TRUE(taskEngine->taskExists(taskId));
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getTitle(), "Test Task");
    EXPECT_EQ(task->getPriority(), Task::HIGH);
    EXPECT_EQ(task->getStatus(), Task::PENDING);
}

TEST_F(TaskEngineTest, CreateTaskWithDueDate_ValidInput_SetsCorrectDueDate) {
    auto dueDate = std::chrono::system_clock::now() + std::chrono::hours(24);
    uint32_t taskId = taskEngine->createTask("Due Tomorrow", Task::MEDIUM, dueDate);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getTitle(), "Due Tomorrow");
    
    // Check due date (allow small time difference)
    auto timeDiff = std::chrono::duration_cast<std::chrono::minutes>(task->getDueDate() - dueDate);
    EXPECT_LE(std::abs(timeDiff.count()), 1);
}

TEST_F(TaskEngineTest, CreateMultipleTasks_ValidInput_ReturnsAllTaskIds) {
    std::vector<std::string> titles = {"Task 1", "Task 2", "Task 3"};
    auto taskIds = taskEngine->createMultipleTasks(titles, Task::LOW);
    
    EXPECT_EQ(taskIds.size(), 3);
    
    for (size_t i = 0; i < taskIds.size(); ++i) {
        auto task = taskEngine->getTask(taskIds[i]);
        ASSERT_TRUE(task.has_value());
        EXPECT_EQ(task->getTitle(), titles[i]);
        EXPECT_EQ(task->getPriority(), Task::LOW);
    }
}

// Task Update Tests
TEST_F(TaskEngineTest, UpdateTask_ValidTask_UpdatesSuccessfully) {
    uint32_t taskId = taskEngine->createTask("Original Task");
    
    Task updatedTask("Updated Task", Task::HIGH);
    updatedTask.setStatus(Task::IN_PROGRESS);
    
    bool result = taskEngine->updateTask(taskId, updatedTask);
    
    EXPECT_TRUE(result);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getTitle(), "Updated Task");
    EXPECT_EQ(task->getPriority(), Task::HIGH);
    EXPECT_EQ(task->getStatus(), Task::IN_PROGRESS);
}

TEST_F(TaskEngineTest, UpdateTask_NonexistentTask_ReturnsFalse) {
    Task updatedTask("Updated Task", Task::HIGH);
    bool result = taskEngine->updateTask(99999, updatedTask);
    
    EXPECT_FALSE(result);
}

TEST_F(TaskEngineTest, SetTaskStatus_ValidTask_UpdatesStatus) {
    uint32_t taskId = taskEngine->createTask("Test Task");
    
    bool result = taskEngine->setTaskStatus(taskId, Task::IN_PROGRESS);
    
    EXPECT_TRUE(result);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getStatus(), Task::IN_PROGRESS);
}

TEST_F(TaskEngineTest, SetTaskPriority_ValidTask_UpdatesPriority) {
    uint32_t taskId = taskEngine->createTask("Test Task", Task::LOW);
    
    bool result = taskEngine->setTaskPriority(taskId, Task::HIGH);
    
    EXPECT_TRUE(result);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getPriority(), Task::HIGH);
}

// Task Completion Tests
TEST_F(TaskEngineTest, CompleteTask_ValidTask_MarksAsCompleted) {
    uint32_t taskId = taskEngine->createTask("Test Task");
    
    bool result = taskEngine->completeTask(taskId);
    
    EXPECT_TRUE(result);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getStatus(), Task::COMPLETED);
    EXPECT_TRUE(task->isCompleted());
}

TEST_F(TaskEngineTest, CompleteMultipleTasks_ValidTasks_CompletesAll) {
    auto taskIds = taskEngine->createMultipleTasks({"Task 1", "Task 2", "Task 3"});
    
    bool result = taskEngine->completeMultipleTasks(taskIds);
    
    EXPECT_TRUE(result);
    
    for (uint32_t taskId : taskIds) {
        auto task = taskEngine->getTask(taskId);
        ASSERT_TRUE(task.has_value());
        EXPECT_EQ(task->getStatus(), Task::COMPLETED);
    }
}

// Task Deletion Tests
TEST_F(TaskEngineTest, DeleteTask_ValidTask_RemovesTask) {
    uint32_t taskId = taskEngine->createTask("Test Task");
    EXPECT_TRUE(taskEngine->taskExists(taskId));
    
    bool result = taskEngine->deleteTask(taskId);
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(taskEngine->taskExists(taskId));
    EXPECT_FALSE(taskEngine->getTask(taskId).has_value());
}

TEST_F(TaskEngineTest, DeleteTask_NonexistentTask_ReturnsFalse) {
    bool result = taskEngine->deleteTask(99999);
    EXPECT_FALSE(result);
}

TEST_F(TaskEngineTest, DeleteMultipleTasks_ValidTasks_DeletesAll) {
    auto taskIds = taskEngine->createMultipleTasks({"Task 1", "Task 2", "Task 3"});
    
    bool result = taskEngine->deleteMultipleTasks(taskIds);
    
    EXPECT_TRUE(result);
    
    for (uint32_t taskId : taskIds) {
        EXPECT_FALSE(taskEngine->taskExists(taskId));
    }
}

// Querying Tests
TEST_F(TaskEngineTest, GetAllTasks_WithTasks_ReturnsAllTasks) {
    taskEngine->createTask("Task 1");
    taskEngine->createTask("Task 2");
    taskEngine->createTask("Task 3");
    
    auto tasks = taskEngine->getAllTasks();
    
    EXPECT_EQ(tasks.size(), 3);
}

TEST_F(TaskEngineTest, GetTasksByStatus_WithMixedStatuses_ReturnsCorrectTasks) {
    uint32_t task1 = taskEngine->createTask("Task 1");
    uint32_t task2 = taskEngine->createTask("Task 2");
    uint32_t task3 = taskEngine->createTask("Task 3");
    
    taskEngine->completeTask(task1);
    taskEngine->setTaskStatus(task2, Task::IN_PROGRESS);
    // task3 remains PENDING
    
    auto completedTasks = taskEngine->getTasksByStatus(Task::COMPLETED);
    auto inProgressTasks = taskEngine->getTasksByStatus(Task::IN_PROGRESS);
    auto pendingTasks = taskEngine->getTasksByStatus(Task::PENDING);
    
    EXPECT_EQ(completedTasks.size(), 1);
    EXPECT_EQ(inProgressTasks.size(), 1);
    EXPECT_EQ(pendingTasks.size(), 1);
    
    EXPECT_EQ(completedTasks[0].getId(), task1);
    EXPECT_EQ(inProgressTasks[0].getId(), task2);
    EXPECT_EQ(pendingTasks[0].getId(), task3);
}

TEST_F(TaskEngineTest, GetTasksByPriority_WithMixedPriorities_ReturnsCorrectTasks) {
    taskEngine->createTask("High Task", Task::HIGH);
    taskEngine->createTask("Medium Task", Task::MEDIUM);
    taskEngine->createTask("Low Task", Task::LOW);
    
    auto highTasks = taskEngine->getTasksByPriority(Task::HIGH);
    auto mediumTasks = taskEngine->getTasksByPriority(Task::MEDIUM);
    auto lowTasks = taskEngine->getTasksByPriority(Task::LOW);
    
    EXPECT_EQ(highTasks.size(), 1);
    EXPECT_EQ(mediumTasks.size(), 1);
    EXPECT_EQ(lowTasks.size(), 1);
    
    EXPECT_EQ(highTasks[0].getTitle(), "High Task");
    EXPECT_EQ(mediumTasks[0].getTitle(), "Medium Task");
    EXPECT_EQ(lowTasks[0].getTitle(), "Low Task");
}

TEST_F(TaskEngineTest, GetOverdueTasks_WithOverdueTasks_ReturnsCorrectTasks) {
    // Create overdue task (due yesterday)
    uint32_t overdueTask = createTaskWithDueDate("Overdue Task", -1);
    
    // Create future task (due tomorrow)
    uint32_t futureTask = createTaskWithDueDate("Future Task", 1);
    
    auto overdueTasks = taskEngine->getOverdueTasks();
    
    EXPECT_EQ(overdueTasks.size(), 1);
    EXPECT_EQ(overdueTasks[0].getId(), overdueTask);
}

TEST_F(TaskEngineTest, GetCompletedTasks_WithMixedStatuses_ReturnsOnlyCompleted) {
    uint32_t task1 = taskEngine->createTask("Task 1");
    uint32_t task2 = taskEngine->createTask("Task 2");
    uint32_t task3 = taskEngine->createTask("Task 3");
    
    taskEngine->completeTask(task1);
    taskEngine->completeTask(task3);
    // task2 remains pending
    
    auto completedTasks = taskEngine->getCompletedTasks();
    
    EXPECT_EQ(completedTasks.size(), 2);
    
    std::vector<uint32_t> completedIds;
    for (const auto& task : completedTasks) {
        completedIds.push_back(task.getId());
    }
    
    EXPECT_TRUE(std::find(completedIds.begin(), completedIds.end(), task1) != completedIds.end());
    EXPECT_TRUE(std::find(completedIds.begin(), completedIds.end(), task3) != completedIds.end());
}

TEST_F(TaskEngineTest, GetPendingTasks_WithMixedStatuses_ReturnsOnlyPending) {
    uint32_t task1 = taskEngine->createTask("Task 1");
    uint32_t task2 = taskEngine->createTask("Task 2");
    uint32_t task3 = taskEngine->createTask("Task 3");
    
    taskEngine->completeTask(task2);
    // task1 and task3 remain pending
    
    auto pendingTasks = taskEngine->getPendingTasks();
    
    EXPECT_EQ(pendingTasks.size(), 2);
    
    std::vector<uint32_t> pendingIds;
    for (const auto& task : pendingTasks) {
        pendingIds.push_back(task.getId());
    }
    
    EXPECT_TRUE(std::find(pendingIds.begin(), pendingIds.end(), task1) != pendingIds.end());
    EXPECT_TRUE(std::find(pendingIds.begin(), pendingIds.end(), task3) != pendingIds.end());
}

// Subtask Tests
TEST_F(TaskEngineTest, AddSubtask_ValidTasks_CreatesRelationship) {
    uint32_t parentId = taskEngine->createTask("Parent Task");
    uint32_t subtaskId = taskEngine->createTask("Subtask");
    
    bool result = taskEngine->addSubtask(parentId, subtaskId);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(taskEngine->hasSubtasks(parentId));
    
    auto subtasks = taskEngine->getSubtasks(parentId);
    EXPECT_EQ(subtasks.size(), 1);
    EXPECT_EQ(subtasks[0].getId(), subtaskId);
    
    auto parent = taskEngine->getParentTask(subtaskId);
    ASSERT_TRUE(parent.has_value());
    EXPECT_EQ(parent.value(), parentId);
}

TEST_F(TaskEngineTest, AddSubtask_SameTask_ReturnsFalse) {
    uint32_t taskId = taskEngine->createTask("Task");
    
    bool result = taskEngine->addSubtask(taskId, taskId);
    
    EXPECT_FALSE(result);
}

TEST_F(TaskEngineTest, AddSubtask_NonexistentTasks_ReturnsFalse) {
    uint32_t validTask = taskEngine->createTask("Valid Task");
    
    bool result1 = taskEngine->addSubtask(99999, validTask);
    bool result2 = taskEngine->addSubtask(validTask, 99999);
    
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
}

TEST_F(TaskEngineTest, RemoveSubtask_ValidRelationship_RemovesRelationship) {
    uint32_t parentId = taskEngine->createTask("Parent Task");
    uint32_t subtaskId = taskEngine->createTask("Subtask");
    
    taskEngine->addSubtask(parentId, subtaskId);
    EXPECT_TRUE(taskEngine->hasSubtasks(parentId));
    
    bool result = taskEngine->removeSubtask(parentId, subtaskId);
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(taskEngine->hasSubtasks(parentId));
    EXPECT_FALSE(taskEngine->getParentTask(subtaskId).has_value());
}

TEST_F(TaskEngineTest, DeleteTask_WithSubtasks_DeletesSubtasksRecursively) {
    uint32_t parentId = taskEngine->createTask("Parent Task");
    uint32_t subtask1Id = taskEngine->createTask("Subtask 1");
    uint32_t subtask2Id = taskEngine->createTask("Subtask 2");
    
    taskEngine->addSubtask(parentId, subtask1Id);
    taskEngine->addSubtask(parentId, subtask2Id);
    
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 3);
    
    bool result = taskEngine->deleteTask(parentId);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 0);
    EXPECT_FALSE(taskEngine->taskExists(parentId));
    EXPECT_FALSE(taskEngine->taskExists(subtask1Id));
    EXPECT_FALSE(taskEngine->taskExists(subtask2Id));
}

// Search Tests
TEST_F(TaskEngineTest, SearchTasks_WithMatchingTasks_ReturnsMatches) {
    taskEngine->createTask("Important Meeting");
    taskEngine->createTask("Buy Groceries");
    taskEngine->createTask("Important Project");
    
    auto results = taskEngine->searchTasks("important");
    
    EXPECT_EQ(results.size(), 2);
    
    std::vector<std::string> titles;
    for (const auto& task : results) {
        titles.push_back(task.getTitle());
    }
    
    EXPECT_TRUE(std::find(titles.begin(), titles.end(), "Important Meeting") != titles.end());
    EXPECT_TRUE(std::find(titles.begin(), titles.end(), "Important Project") != titles.end());
}

TEST_F(TaskEngineTest, SearchTasks_EmptyQuery_ReturnsAllTasks) {
    taskEngine->createTask("Task 1");
    taskEngine->createTask("Task 2");
    
    auto results = taskEngine->searchTasks("");
    
    EXPECT_EQ(results.size(), 2);
}

// Statistics Tests
TEST_F(TaskEngineTest, GetStatistics_WithMixedTasks_ReturnsCorrectCounts) {
    uint32_t task1 = taskEngine->createTask("Task 1");
    uint32_t task2 = taskEngine->createTask("Task 2");
    uint32_t task3 = createTaskWithDueDate("Overdue Task", -1);
    
    taskEngine->completeTask(task1);
    
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 3);
    EXPECT_EQ(taskEngine->getCompletedTaskCount(), 1);
    EXPECT_EQ(taskEngine->getPendingTaskCount(), 2);
    EXPECT_EQ(taskEngine->getOverdueTaskCount(), 1);
    EXPECT_FLOAT_EQ(taskEngine->getCompletionRate(), 1.0f / 3.0f);
}

TEST_F(TaskEngineTest, GetCompletionRate_NoTasks_ReturnsZero) {
    EXPECT_FLOAT_EQ(taskEngine->getCompletionRate(), 0.0f);
}

// Data Management Tests
TEST_F(TaskEngineTest, LoadTasks_ValidTasks_LoadsAllTasks) {
    std::vector<Task> tasksToLoad;
    tasksToLoad.emplace_back("Task 1", Task::HIGH);
    tasksToLoad.emplace_back("Task 2", Task::MEDIUM);
    tasksToLoad.emplace_back("Task 3", Task::LOW);
    
    taskEngine->loadTasks(tasksToLoad);
    
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 3);
    
    auto loadedTasks = taskEngine->getAllTasks();
    EXPECT_EQ(loadedTasks.size(), 3);
}

TEST_F(TaskEngineTest, ExportTasks_WithTasks_ReturnsAllTasks) {
    taskEngine->createTask("Task 1");
    taskEngine->createTask("Task 2");
    
    auto exportedTasks = taskEngine->exportTasks();
    
    EXPECT_EQ(exportedTasks.size(), 2);
}

TEST_F(TaskEngineTest, ClearAllTasks_WithTasks_RemovesAllTasks) {
    taskEngine->createTask("Task 1");
    taskEngine->createTask("Task 2");
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 2);
    
    taskEngine->clearAllTasks();
    
    EXPECT_EQ(taskEngine->getTotalTaskCount(), 0);
}

// Callback Tests
TEST_F(TaskEngineTest, TaskCompletionCallback_WhenTaskCompleted_TriggersCallback) {
    bool callbackTriggered = false;
    Task completedTask("", Task::MEDIUM);
    
    taskEngine->setTaskCompletionCallback([&](const Task& task) {
        callbackTriggered = true;
        completedTask = task;
    });
    
    uint32_t taskId = taskEngine->createTask("Test Task");
    taskEngine->completeTask(taskId);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(completedTask.getId(), taskId);
    EXPECT_EQ(completedTask.getTitle(), "Test Task");
}

TEST_F(TaskEngineTest, TaskCreationCallback_WhenTaskCreated_TriggersCallback) {
    bool callbackTriggered = false;
    Task createdTask("", Task::MEDIUM);
    
    taskEngine->setTaskCreationCallback([&](const Task& task) {
        callbackTriggered = true;
        createdTask = task;
    });
    
    uint32_t taskId = taskEngine->createTask("New Task", Task::HIGH);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(createdTask.getId(), taskId);
    EXPECT_EQ(createdTask.getTitle(), "New Task");
    EXPECT_EQ(createdTask.getPriority(), Task::HIGH);
}

TEST_F(TaskEngineTest, TaskDeletionCallback_WhenTaskDeleted_TriggersCallback) {
    bool callbackTriggered = false;
    uint32_t deletedTaskId = 0;
    
    taskEngine->setTaskDeletionCallback([&](uint32_t taskId) {
        callbackTriggered = true;
        deletedTaskId = taskId;
    });
    
    uint32_t taskId = taskEngine->createTask("Test Task");
    taskEngine->deleteTask(taskId);
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(deletedTaskId, taskId);
}