#include "task_engine.h"
#include <algorithm>
#include <chrono>
#include <ctime>

TaskEngine::TaskEngine() = default;

// Task management
uint32_t TaskEngine::createTask(const std::string& title, Task::Priority priority) {
    Task newTask(title, priority);
    uint32_t taskId = newTask.getId();
    
    auto [it, inserted] = tasks.emplace(taskId, std::move(newTask));
    triggerTaskCreationCallback(it->second);
    
    return taskId;
}

uint32_t TaskEngine::createTask(const std::string& title, Task::Priority priority, 
                               const std::chrono::system_clock::time_point& dueDate) {
    Task newTask(title, priority);
    newTask.setDueDate(dueDate);
    uint32_t taskId = newTask.getId();
    
    auto [it, inserted] = tasks.emplace(taskId, std::move(newTask));
    triggerTaskCreationCallback(it->second);
    
    return taskId;
}

bool TaskEngine::updateTask(uint32_t taskId, const Task& updatedTask) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    // Preserve the original ID and subtask relationships
    Task oldTask = it->second;
    it->second = updatedTask;
    
    // Update subtask mappings if needed
    updateSubtaskMappings(it->second);
    
    triggerTaskUpdateCallback(it->second);
    return true;
}

bool TaskEngine::deleteTask(uint32_t taskId) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    // Get all subtasks of this task using the subtaskToParent mapping
    // We need to collect them first to avoid iterator invalidation
    std::vector<uint32_t> subtaskIds = getSubtaskIds(taskId);
    
    // Delete the task first
    tasks.erase(it);
    triggerTaskDeletionCallback(taskId);
    
    // Remove from subtask mappings (remove this task as a subtask)
    subtaskToParent.erase(taskId);
    
    // Remove this task as a parent from all subtask mappings
    auto mapIt = subtaskToParent.begin();
    while (mapIt != subtaskToParent.end()) {
        if (mapIt->second == taskId) {
            mapIt = subtaskToParent.erase(mapIt);
        } else {
            ++mapIt;
        }
    }
    
    // Then delete all subtasks recursively
    for (uint32_t subtaskId : subtaskIds) {
        deleteTask(subtaskId); // Recursive deletion
    }
    
    return true;
}

bool TaskEngine::completeTask(uint32_t taskId) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    it->second.setStatus(Task::COMPLETED);
    triggerTaskCompletionCallback(it->second);
    triggerTaskUpdateCallback(it->second);
    
    return true;
}

bool TaskEngine::setTaskStatus(uint32_t taskId, Task::Status status) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    Task::Status oldStatus = it->second.getStatus();
    it->second.setStatus(status);
    
    if (status == Task::COMPLETED && oldStatus != Task::COMPLETED) {
        triggerTaskCompletionCallback(it->second);
    }
    
    triggerTaskUpdateCallback(it->second);
    return true;
}

bool TaskEngine::setTaskPriority(uint32_t taskId, Task::Priority priority) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    it->second.setPriority(priority);
    triggerTaskUpdateCallback(it->second);
    return true;
}

bool TaskEngine::setTaskDueDate(uint32_t taskId, const std::chrono::system_clock::time_point& dueDate) {
    auto it = tasks.find(taskId);
    if (it == tasks.end()) {
        return false;
    }
    
    it->second.setDueDate(dueDate);
    triggerTaskUpdateCallback(it->second);
    return true;
}

// Querying
std::vector<Task> TaskEngine::getAllTasks() const {
    std::vector<Task> result;
    result.reserve(tasks.size());
    
    for (const auto& [id, task] : tasks) {
        result.push_back(task);
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksByStatus(Task::Status status) const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (task.getStatus() == status) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksByPriority(Task::Priority priority) const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (task.getPriority() == priority) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getOverdueTasks() const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (task.isOverdue()) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksDueToday() const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (isTaskDueToday(task)) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksDueThisWeek() const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (isTaskDueThisWeek(task)) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getCompletedTasks() const {
    return getTasksByStatus(Task::COMPLETED);
}

std::vector<Task> TaskEngine::getPendingTasks() const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (task.getStatus() != Task::COMPLETED) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::optional<Task> TaskEngine::getTask(uint32_t taskId) const {
    auto it = tasks.find(taskId);
    if (it != tasks.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Subtask management
bool TaskEngine::addSubtask(uint32_t parentId, uint32_t subtaskId) {
    auto parentIt = tasks.find(parentId);
    auto subtaskIt = tasks.find(subtaskId);
    
    if (parentIt == tasks.end() || subtaskIt == tasks.end()) {
        return false;
    }
    
    // Prevent circular dependencies
    if (parentId == subtaskId) {
        return false;
    }
    
    // Check if subtask would create a cycle
    uint32_t currentParent = parentId;
    while (auto parent = getParentTask(currentParent)) {
        if (parent.value() == subtaskId) {
            return false; // Would create a cycle
        }
        currentParent = parent.value();
    }
    
    // Add subtask relationship
    parentIt->second.addSubtask(std::make_shared<Task>(subtaskIt->second));
    subtaskToParent[subtaskId] = parentId;
    
    return true;
}

bool TaskEngine::removeSubtask(uint32_t parentId, uint32_t subtaskId) {
    auto parentIt = tasks.find(parentId);
    if (parentIt == tasks.end()) {
        return false;
    }
    
    parentIt->second.removeSubtask(subtaskId);
    subtaskToParent.erase(subtaskId);
    
    return true;
}

std::vector<Task> TaskEngine::getSubtasks(uint32_t parentId) const {
    auto it = tasks.find(parentId);
    if (it == tasks.end()) {
        return {};
    }
    
    std::vector<Task> result;
    auto subtaskPtrs = it->second.getSubtasks();
    
    for (const auto& subtaskPtr : subtaskPtrs) {
        if (subtaskPtr) {
            result.push_back(*subtaskPtr);
        }
    }
    
    return result;
}

std::vector<uint32_t> TaskEngine::getSubtaskIds(uint32_t parentId) const {
    std::vector<uint32_t> result;
    
    for (const auto& [subtaskId, parentTaskId] : subtaskToParent) {
        if (parentTaskId == parentId) {
            result.push_back(subtaskId);
        }
    }
    
    return result;
}

bool TaskEngine::hasSubtasks(uint32_t taskId) const {
    auto it = tasks.find(taskId);
    if (it != tasks.end()) {
        return it->second.hasSubtasks();
    }
    return false;
}

std::optional<uint32_t> TaskEngine::getParentTask(uint32_t subtaskId) const {
    auto it = subtaskToParent.find(subtaskId);
    if (it != subtaskToParent.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Bulk operations
std::vector<uint32_t> TaskEngine::createMultipleTasks(const std::vector<std::string>& titles, 
                                                      Task::Priority priority) {
    std::vector<uint32_t> taskIds;
    taskIds.reserve(titles.size());
    
    for (const auto& title : titles) {
        taskIds.push_back(createTask(title, priority));
    }
    
    return taskIds;
}

bool TaskEngine::deleteMultipleTasks(const std::vector<uint32_t>& taskIds) {
    bool allDeleted = true;
    
    for (uint32_t taskId : taskIds) {
        if (!deleteTask(taskId)) {
            allDeleted = false;
        }
    }
    
    return allDeleted;
}

bool TaskEngine::completeMultipleTasks(const std::vector<uint32_t>& taskIds) {
    bool allCompleted = true;
    
    for (uint32_t taskId : taskIds) {
        if (!completeTask(taskId)) {
            allCompleted = false;
        }
    }
    
    return allCompleted;
}

// Search and filtering
std::vector<Task> TaskEngine::searchTasks(const std::string& searchText) const {
    std::vector<Task> result;
    
    if (searchText.empty()) {
        return getAllTasks();
    }
    
    std::string lowerSearchText = searchText;
    std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::tolower);
    
    for (const auto& [id, task] : tasks) {
        std::string lowerTitle = task.getTitle();
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        
        if (lowerTitle.find(lowerSearchText) != std::string::npos) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksCreatedAfter(const std::chrono::system_clock::time_point& date) const {
    std::vector<Task> result;
    
    for (const auto& [id, task] : tasks) {
        if (task.getCreatedAt() > date) {
            result.push_back(task);
        }
    }
    
    return result;
}

std::vector<Task> TaskEngine::getTasksModifiedAfter(const std::chrono::system_clock::time_point& date) const {
    // Note: Task model doesn't have modifiedAt field, so we'll use createdAt for now
    // This can be enhanced when Task model is extended with modification tracking
    return getTasksCreatedAfter(date);
}

// Statistics
size_t TaskEngine::getTotalTaskCount() const {
    return tasks.size();
}

size_t TaskEngine::getCompletedTaskCount() const {
    return getCompletedTasks().size();
}

size_t TaskEngine::getPendingTaskCount() const {
    return getPendingTasks().size();
}

size_t TaskEngine::getOverdueTaskCount() const {
    return getOverdueTasks().size();
}

float TaskEngine::getCompletionRate() const {
    if (tasks.empty()) {
        return 0.0f;
    }
    
    return static_cast<float>(getCompletedTaskCount()) / static_cast<float>(getTotalTaskCount());
}

// Data management
void TaskEngine::loadTasks(const std::vector<Task>& taskList) {
    tasks.clear();
    subtaskToParent.clear();
    
    // First pass: load all tasks
    for (const auto& task : taskList) {
        tasks.emplace(task.getId(), task);
    }
    
    // Second pass: rebuild subtask relationships
    for (const auto& [id, task] : tasks) {
        updateSubtaskMappings(task);
    }
}

std::vector<Task> TaskEngine::exportTasks() const {
    return getAllTasks();
}

void TaskEngine::clearAllTasks() {
    tasks.clear();
    subtaskToParent.clear();
}

// Events and callbacks
void TaskEngine::setTaskCompletionCallback(std::function<void(const Task&)> callback) {
    onTaskCompleted = std::move(callback);
}

void TaskEngine::setTaskCreationCallback(std::function<void(const Task&)> callback) {
    onTaskCreated = std::move(callback);
}

void TaskEngine::setTaskDeletionCallback(std::function<void(uint32_t)> callback) {
    onTaskDeleted = std::move(callback);
}

void TaskEngine::setTaskUpdateCallback(std::function<void(const Task&)> callback) {
    onTaskUpdated = std::move(callback);
}

// Validation
bool TaskEngine::taskExists(uint32_t taskId) const {
    return tasks.find(taskId) != tasks.end();
}

bool TaskEngine::isValidTaskId(uint32_t taskId) const {
    return taskExists(taskId);
}

// Private helper methods
void TaskEngine::triggerTaskCompletionCallback(const Task& task) {
    if (onTaskCompleted) {
        onTaskCompleted(task);
    }
}

void TaskEngine::triggerTaskCreationCallback(const Task& task) {
    if (onTaskCreated) {
        onTaskCreated(task);
    }
}

void TaskEngine::triggerTaskDeletionCallback(uint32_t taskId) {
    if (onTaskDeleted) {
        onTaskDeleted(taskId);
    }
}

void TaskEngine::triggerTaskUpdateCallback(const Task& task) {
    if (onTaskUpdated) {
        onTaskUpdated(task);
    }
}

bool TaskEngine::isTaskDueToday(const Task& task) const {
    auto startOfDay = getStartOfDay();
    auto endOfDay = startOfDay + std::chrono::hours(24);
    
    auto dueDate = task.getDueDate();
    return dueDate >= startOfDay && dueDate < endOfDay;
}

bool TaskEngine::isTaskDueThisWeek(const Task& task) const {
    auto startOfWeek = getStartOfWeek();
    auto endOfWeek = startOfWeek + std::chrono::hours(24 * 7);
    
    auto dueDate = task.getDueDate();
    return dueDate >= startOfWeek && dueDate < endOfWeek;
}

std::chrono::system_clock::time_point TaskEngine::getStartOfDay() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::chrono::system_clock::time_point TaskEngine::getStartOfWeek() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time);
    
    // Calculate days since Monday (assuming Monday is start of week)
    int daysSinceMonday = (tm.tm_wday + 6) % 7; // Convert Sunday=0 to Monday=0
    
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_mday -= daysSinceMonday;
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void TaskEngine::removeTaskFromSubtaskMappings(uint32_t taskId) {
    // Remove as a subtask
    subtaskToParent.erase(taskId);
    
    // Remove as a parent (remove all its subtasks from the mapping)
    auto it = subtaskToParent.begin();
    while (it != subtaskToParent.end()) {
        if (it->second == taskId) {
            it = subtaskToParent.erase(it);
        } else {
            ++it;
        }
    }
}

void TaskEngine::updateSubtaskMappings(const Task& task) {
    // This is a simplified implementation
    // In a full implementation, we would need to sync the Task's subtask list
    // with our subtaskToParent mapping
    auto subtasks = task.getSubtasks();
    for (const auto& subtaskPtr : subtasks) {
        if (subtaskPtr) {
            subtaskToParent[subtaskPtr->getId()] = task.getId();
        }
    }
}