#include <gtest/gtest.h>
#include "../src/core/models/task.h"
#include <chrono>
#include <thread>

class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset static ID counter for consistent testing
        // Note: This is a test-only approach; in production we'd handle this differently
    }
    
    void TearDown() override {
        // Clean up after each test
    }
};

TEST_F(TaskTest, CreateTask_ValidInput_SetsPropertiesCorrectly) {
    Task task("Test Task", Task::HIGH);
    
    EXPECT_GT(task.getId(), 0);
    EXPECT_EQ(task.getTitle(), "Test Task");
    EXPECT_EQ(task.getPriority(), Task::HIGH);
    EXPECT_EQ(task.getStatus(), Task::PENDING);
    EXPECT_FALSE(task.isCompleted());
    EXPECT_FALSE(task.hasSubtasks());
}

TEST_F(TaskTest, CreateTaskWithId_ValidInput_UsesProvidedId) {
    uint32_t testId = 12345;
    Task task(testId, "Test Task", Task::MEDIUM);
    
    EXPECT_EQ(task.getId(), testId);
    EXPECT_EQ(task.getTitle(), "Test Task");
    EXPECT_EQ(task.getPriority(), Task::MEDIUM);
}

TEST_F(TaskTest, SetTitle_ValidString_UpdatesTitle) {
    Task task("Original Title");
    task.setTitle("New Title");
    
    EXPECT_EQ(task.getTitle(), "New Title");
}

TEST_F(TaskTest, SetPriority_ValidPriority_UpdatesPriority) {
    Task task("Test Task", Task::LOW);
    task.setPriority(Task::HIGH);
    
    EXPECT_EQ(task.getPriority(), Task::HIGH);
    EXPECT_EQ(task.getPriorityString(), "HIGH");
}

TEST_F(TaskTest, SetStatus_ValidStatus_UpdatesStatus) {
    Task task("Test Task");
    task.setStatus(Task::COMPLETED);
    
    EXPECT_EQ(task.getStatus(), Task::COMPLETED);
    EXPECT_TRUE(task.isCompleted());
    EXPECT_EQ(task.getStatusString(), "COMPLETED");
}

TEST_F(TaskTest, AddSubtask_ValidSubtask_AddsToCollection) {
    Task parentTask("Parent Task");
    auto subtask = std::make_shared<Task>("Subtask 1");
    
    parentTask.addSubtask(subtask);
    
    EXPECT_TRUE(parentTask.hasSubtasks());
    EXPECT_EQ(parentTask.getSubtaskCount(), 1);
    
    auto subtasks = parentTask.getSubtasks();
    EXPECT_EQ(subtasks.size(), 1);
    EXPECT_EQ(subtasks[0]->getTitle(), "Subtask 1");
}

TEST_F(TaskTest, AddSubtask_DuplicateSubtask_DoesNotAddDuplicate) {
    Task parentTask("Parent Task");
    auto subtask = std::make_shared<Task>("Subtask 1");
    
    parentTask.addSubtask(subtask);
    parentTask.addSubtask(subtask); // Add same subtask again
    
    EXPECT_EQ(parentTask.getSubtaskCount(), 1);
}

TEST_F(TaskTest, RemoveSubtask_ExistingSubtask_RemovesFromCollection) {
    Task parentTask("Parent Task");
    auto subtask1 = std::make_shared<Task>("Subtask 1");
    auto subtask2 = std::make_shared<Task>("Subtask 2");
    
    parentTask.addSubtask(subtask1);
    parentTask.addSubtask(subtask2);
    EXPECT_EQ(parentTask.getSubtaskCount(), 2);
    
    parentTask.removeSubtask(subtask1->getId());
    EXPECT_EQ(parentTask.getSubtaskCount(), 1);
    
    auto remainingSubtasks = parentTask.getSubtasks();
    EXPECT_EQ(remainingSubtasks[0]->getTitle(), "Subtask 2");
}

TEST_F(TaskTest, IsOverdue_TaskPastDueDate_ReturnsTrue) {
    Task task("Test Task");
    
    // Set due date to 1 hour ago
    auto pastTime = std::chrono::system_clock::now() - std::chrono::hours(1);
    task.setDueDate(pastTime);
    
    EXPECT_TRUE(task.isOverdue());
}

TEST_F(TaskTest, IsOverdue_TaskNotDue_ReturnsFalse) {
    Task task("Test Task");
    
    // Set due date to 1 hour from now
    auto futureTime = std::chrono::system_clock::now() + std::chrono::hours(1);
    task.setDueDate(futureTime);
    
    EXPECT_FALSE(task.isOverdue());
}

TEST_F(TaskTest, IsOverdue_CompletedTask_ReturnsFalse) {
    Task task("Test Task");
    
    // Set due date to past and mark as completed
    auto pastTime = std::chrono::system_clock::now() - std::chrono::hours(1);
    task.setDueDate(pastTime);
    task.setStatus(Task::COMPLETED);
    
    EXPECT_FALSE(task.isOverdue()); // Completed tasks are never overdue
}

TEST_F(TaskTest, PriorityStringConversion_AllPriorities_ConvertsCorrectly) {
    EXPECT_EQ(Task::priorityToString(Task::LOW), "LOW");
    EXPECT_EQ(Task::priorityToString(Task::MEDIUM), "MEDIUM");
    EXPECT_EQ(Task::priorityToString(Task::HIGH), "HIGH");
    
    EXPECT_EQ(Task::stringToPriority("LOW"), Task::LOW);
    EXPECT_EQ(Task::stringToPriority("MEDIUM"), Task::MEDIUM);
    EXPECT_EQ(Task::stringToPriority("HIGH"), Task::HIGH);
    EXPECT_EQ(Task::stringToPriority("INVALID"), Task::MEDIUM); // Default
}

TEST_F(TaskTest, StatusStringConversion_AllStatuses_ConvertsCorrectly) {
    EXPECT_EQ(Task::statusToString(Task::PENDING), "PENDING");
    EXPECT_EQ(Task::statusToString(Task::IN_PROGRESS), "IN_PROGRESS");
    EXPECT_EQ(Task::statusToString(Task::COMPLETED), "COMPLETED");
    
    EXPECT_EQ(Task::stringToStatus("PENDING"), Task::PENDING);
    EXPECT_EQ(Task::stringToStatus("IN_PROGRESS"), Task::IN_PROGRESS);
    EXPECT_EQ(Task::stringToStatus("COMPLETED"), Task::COMPLETED);
    EXPECT_EQ(Task::stringToStatus("INVALID"), Task::PENDING); // Default
}

TEST_F(TaskTest, JsonSerialization_ValidTask_SerializesCorrectly) {
    Task task("Test Task", Task::HIGH);
    task.setStatus(Task::IN_PROGRESS);
    
    nlohmann::json json = task.toJson();
    
    EXPECT_EQ(json["id"], task.getId());
    EXPECT_EQ(json["title"], "Test Task");
    EXPECT_EQ(json["priority"], "HIGH");
    EXPECT_EQ(json["status"], "IN_PROGRESS");
    EXPECT_TRUE(json.contains("dueDate"));
    EXPECT_TRUE(json.contains("createdAt"));
    EXPECT_TRUE(json["subtasks"].is_array());
}

TEST_F(TaskTest, JsonDeserialization_ValidJson_CreatesTaskCorrectly) {
    nlohmann::json json = {
        {"id", 123},
        {"title", "Test Task"},
        {"priority", "HIGH"},
        {"status", "COMPLETED"},
        {"dueDate", "2025-12-31T23:59:59Z"},
        {"createdAt", "2025-01-01T00:00:00Z"},
        {"subtasks", nlohmann::json::array()}
    };
    
    Task task = Task::fromJson(json);
    
    EXPECT_EQ(task.getId(), 123);
    EXPECT_EQ(task.getTitle(), "Test Task");
    EXPECT_EQ(task.getPriority(), Task::HIGH);
    EXPECT_EQ(task.getStatus(), Task::COMPLETED);
    EXPECT_FALSE(task.hasSubtasks());
}

TEST_F(TaskTest, JsonRoundTrip_TaskWithSubtasks_PreservesData) {
    Task parentTask("Parent Task", Task::MEDIUM);
    auto subtask = std::make_shared<Task>("Subtask 1");
    parentTask.addSubtask(subtask);
    
    // Serialize to JSON
    nlohmann::json json = parentTask.toJson();
    
    // Deserialize from JSON
    Task deserializedTask = Task::fromJson(json);
    
    EXPECT_EQ(deserializedTask.getId(), parentTask.getId());
    EXPECT_EQ(deserializedTask.getTitle(), parentTask.getTitle());
    EXPECT_EQ(deserializedTask.getPriority(), parentTask.getPriority());
    EXPECT_EQ(deserializedTask.getStatus(), parentTask.getStatus());
    
    // Note: Subtasks are not automatically restored in fromJson
    // This is handled by TaskEngine to avoid circular dependencies
}