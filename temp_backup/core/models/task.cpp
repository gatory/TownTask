#include "task.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// Static member initialization
uint32_t Task::nextId = 1;

Task::Task(const std::string& title, Priority priority)
    : id(generateId()), title(title), priority(priority), status(PENDING),
      createdAt(std::chrono::system_clock::now()) {
    // Set default due date to one week from creation
    dueDate = createdAt + std::chrono::hours(24 * 7);
}

Task::Task(uint32_t id, const std::string& title, Priority priority)
    : id(id), title(title), priority(priority), status(PENDING),
      createdAt(std::chrono::system_clock::now()) {
    // Update nextId to ensure no conflicts
    if (id >= nextId) {
        nextId = id + 1;
    }
    dueDate = createdAt + std::chrono::hours(24 * 7);
}

uint32_t Task::generateId() {
    return nextId++;
}

void Task::addSubtask(std::shared_ptr<Task> subtask) {
    if (subtask && subtask->getId() != id) {
        // Check if subtask already exists
        auto it = std::find_if(subtasks.begin(), subtasks.end(),
            [subtask](const std::shared_ptr<Task>& existing) {
                return existing->getId() == subtask->getId();
            });
        
        if (it == subtasks.end()) {
            subtasks.push_back(subtask);
        }
    }
}

void Task::removeSubtask(uint32_t subtaskId) {
    subtasks.erase(
        std::remove_if(subtasks.begin(), subtasks.end(),
            [subtaskId](const std::shared_ptr<Task>& task) {
                return task->getId() == subtaskId;
            }),
        subtasks.end()
    );
}

bool Task::isOverdue() const {
    if (status == COMPLETED) {
        return false;
    }
    return std::chrono::system_clock::now() > dueDate;
}

std::string Task::getPriorityString() const {
    return priorityToString(priority);
}

std::string Task::getStatusString() const {
    return statusToString(status);
}

std::string Task::priorityToString(Priority priority) {
    switch (priority) {
        case LOW: return "LOW";
        case MEDIUM: return "MEDIUM";
        case HIGH: return "HIGH";
        default: return "MEDIUM";
    }
}

Task::Priority Task::stringToPriority(const std::string& priorityStr) {
    if (priorityStr == "LOW") return LOW;
    if (priorityStr == "HIGH") return HIGH;
    return MEDIUM; // Default
}

std::string Task::statusToString(Status status) {
    switch (status) {
        case PENDING: return "PENDING";
        case IN_PROGRESS: return "IN_PROGRESS";
        case COMPLETED: return "COMPLETED";
        default: return "PENDING";
    }
}

Task::Status Task::stringToStatus(const std::string& statusStr) {
    if (statusStr == "IN_PROGRESS") return IN_PROGRESS;
    if (statusStr == "COMPLETED") return COMPLETED;
    return PENDING; // Default
}

nlohmann::json Task::toJson() const {
    nlohmann::json j;
    
    j["id"] = id;
    j["title"] = title;
    j["priority"] = priorityToString(priority);
    j["status"] = statusToString(status);
    
    // Convert time points to ISO 8601 strings
    auto dueTime = std::chrono::system_clock::to_time_t(dueDate);
    auto createdTime = std::chrono::system_clock::to_time_t(createdAt);
    
    std::stringstream dueSs, createdSs;
    dueSs << std::put_time(std::gmtime(&dueTime), "%Y-%m-%dT%H:%M:%SZ");
    createdSs << std::put_time(std::gmtime(&createdTime), "%Y-%m-%dT%H:%M:%SZ");
    
    j["dueDate"] = dueSs.str();
    j["createdAt"] = createdSs.str();
    
    // Serialize subtasks as array of IDs
    nlohmann::json subtaskIds = nlohmann::json::array();
    for (const auto& subtask : subtasks) {
        subtaskIds.push_back(subtask->getId());
    }
    j["subtasks"] = subtaskIds;
    
    return j;
}

Task Task::fromJson(const nlohmann::json& json) {
    uint32_t id = json.at("id").get<uint32_t>();
    std::string title = json.at("title").get<std::string>();
    Priority priority = stringToPriority(json.at("priority").get<std::string>());
    
    Task task(id, title, priority);
    
    // Set status
    if (json.contains("status")) {
        task.setStatus(stringToStatus(json.at("status").get<std::string>()));
    }
    
    // Parse dates
    if (json.contains("dueDate")) {
        std::string dueDateStr = json.at("dueDate").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(dueDateStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            task.dueDate = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    if (json.contains("createdAt")) {
        std::string createdAtStr = json.at("createdAt").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(createdAtStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            task.createdAt = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    // Note: Subtasks will be resolved by the TaskEngine after all tasks are loaded
    // to avoid circular dependency issues during deserialization
    
    return task;
}