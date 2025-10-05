#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>

class Task {
public:
    enum Priority { LOW, MEDIUM, HIGH };
    enum Status { PENDING, IN_PROGRESS, COMPLETED };
    
    // Constructors
    Task(const std::string& title, Priority priority = MEDIUM);
    Task(uint32_t id, const std::string& title, Priority priority = MEDIUM);
    
    // Getters
    uint32_t getId() const { return id; }
    std::string getTitle() const { return title; }
    Priority getPriority() const { return priority; }
    Status getStatus() const { return status; }
    std::chrono::system_clock::time_point getDueDate() const { return dueDate; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt; }
    
    // Setters
    void setTitle(const std::string& newTitle) { title = newTitle; }
    void setPriority(Priority newPriority) { priority = newPriority; }
    void setStatus(Status newStatus) { status = newStatus; }
    void setDueDate(const std::chrono::system_clock::time_point& date) { dueDate = date; }
    
    // Subtask management
    void addSubtask(std::shared_ptr<Task> subtask);
    void removeSubtask(uint32_t subtaskId);
    std::vector<std::shared_ptr<Task>> getSubtasks() const { return subtasks; }
    bool hasSubtasks() const { return !subtasks.empty(); }
    size_t getSubtaskCount() const { return subtasks.size(); }
    
    // Utility methods
    bool isOverdue() const;
    bool isCompleted() const { return status == COMPLETED; }
    std::string getPriorityString() const;
    std::string getStatusString() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& json);
    
    // Static utility methods
    static std::string priorityToString(Priority priority);
    static Priority stringToPriority(const std::string& priorityStr);
    static std::string statusToString(Status status);
    static Status stringToStatus(const std::string& statusStr);
    
private:
    uint32_t id;
    std::string title;
    Priority priority;
    Status status;
    std::chrono::system_clock::time_point dueDate;
    std::chrono::system_clock::time_point createdAt;
    std::vector<std::shared_ptr<Task>> subtasks;
    
    static uint32_t nextId;
    static uint32_t generateId();
};