#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>
#include <memory>
#include "../models/task.h"

class TaskEngine {
public:
    TaskEngine();
    ~TaskEngine() = default;
    
    // Task management
    uint32_t createTask(const std::string& title, Task::Priority priority = Task::MEDIUM);
    uint32_t createTask(const std::string& title, Task::Priority priority, 
                       const std::chrono::system_clock::time_point& dueDate);
    bool updateTask(uint32_t taskId, const Task& updatedTask);
    bool deleteTask(uint32_t taskId);
    bool completeTask(uint32_t taskId);
    bool setTaskStatus(uint32_t taskId, Task::Status status);
    bool setTaskPriority(uint32_t taskId, Task::Priority priority);
    bool setTaskDueDate(uint32_t taskId, const std::chrono::system_clock::time_point& dueDate);
    
    // Querying
    std::vector<Task> getAllTasks() const;
    std::vector<Task> getTasksByStatus(Task::Status status) const;
    std::vector<Task> getTasksByPriority(Task::Priority priority) const;
    std::vector<Task> getOverdueTasks() const;
    std::vector<Task> getTasksDueToday() const;
    std::vector<Task> getTasksDueThisWeek() const;
    std::vector<Task> getCompletedTasks() const;
    std::vector<Task> getPendingTasks() const;
    std::optional<Task> getTask(uint32_t taskId) const;
    
    // Subtask management
    bool addSubtask(uint32_t parentId, uint32_t subtaskId);
    bool removeSubtask(uint32_t parentId, uint32_t subtaskId);
    std::vector<Task> getSubtasks(uint32_t parentId) const;
    std::vector<uint32_t> getSubtaskIds(uint32_t parentId) const;
    bool hasSubtasks(uint32_t taskId) const;
    std::optional<uint32_t> getParentTask(uint32_t subtaskId) const;
    
    // Bulk operations
    std::vector<uint32_t> createMultipleTasks(const std::vector<std::string>& titles, 
                                             Task::Priority priority = Task::MEDIUM);
    bool deleteMultipleTasks(const std::vector<uint32_t>& taskIds);
    bool completeMultipleTasks(const std::vector<uint32_t>& taskIds);
    
    // Search and filtering
    std::vector<Task> searchTasks(const std::string& searchText) const;
    std::vector<Task> getTasksCreatedAfter(const std::chrono::system_clock::time_point& date) const;
    std::vector<Task> getTasksModifiedAfter(const std::chrono::system_clock::time_point& date) const;
    
    // Statistics
    size_t getTotalTaskCount() const;
    size_t getCompletedTaskCount() const;
    size_t getPendingTaskCount() const;
    size_t getOverdueTaskCount() const;
    float getCompletionRate() const;
    
    // Data management
    void loadTasks(const std::vector<Task>& tasks);
    std::vector<Task> exportTasks() const;
    void clearAllTasks();
    
    // Events and callbacks
    void setTaskCompletionCallback(std::function<void(const Task&)> callback);
    void setTaskCreationCallback(std::function<void(const Task&)> callback);
    void setTaskDeletionCallback(std::function<void(uint32_t)> callback);
    void setTaskUpdateCallback(std::function<void(const Task&)> callback);
    
    // Validation
    bool taskExists(uint32_t taskId) const;
    bool isValidTaskId(uint32_t taskId) const;
    
private:
    std::unordered_map<uint32_t, Task> tasks;
    std::unordered_map<uint32_t, uint32_t> subtaskToParent; // subtask ID -> parent ID
    
    // Event callbacks
    std::function<void(const Task&)> onTaskCompleted;
    std::function<void(const Task&)> onTaskCreated;
    std::function<void(uint32_t)> onTaskDeleted;
    std::function<void(const Task&)> onTaskUpdated;
    
    // Helper methods
    void triggerTaskCompletionCallback(const Task& task);
    void triggerTaskCreationCallback(const Task& task);
    void triggerTaskDeletionCallback(uint32_t taskId);
    void triggerTaskUpdateCallback(const Task& task);
    
    bool isTaskDueToday(const Task& task) const;
    bool isTaskDueThisWeek(const Task& task) const;
    std::chrono::system_clock::time_point getStartOfDay() const;
    std::chrono::system_clock::time_point getStartOfWeek() const;
    
    // Subtask helper methods
    void removeTaskFromSubtaskMappings(uint32_t taskId);
    void updateSubtaskMappings(const Task& task);
};