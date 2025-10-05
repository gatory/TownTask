#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

class Note {
public:
    // Constructors
    Note(const std::string& title, const std::string& content = "");
    Note(uint32_t id, const std::string& title, const std::string& content = "");
    
    // Getters
    uint32_t getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getContent() const { return content; }
    std::vector<std::string> getTags() const { return tags; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt; }
    std::chrono::system_clock::time_point getModifiedAt() const { return modifiedAt; }
    
    // Setters
    void setTitle(const std::string& newTitle);
    void setContent(const std::string& newContent);
    
    // Tag management
    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    void clearTags();
    size_t getTagCount() const { return tags.size(); }
    
    // Search functionality
    bool containsText(const std::string& searchText, bool caseSensitive = false) const;
    bool matchesTag(const std::string& tag, bool caseSensitive = false) const;
    
    // Utility methods
    size_t getContentLength() const { return content.length(); }
    bool isEmpty() const { return content.empty(); }
    std::string getPreview(size_t maxLength = 100) const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Note fromJson(const nlohmann::json& json);
    
private:
    uint32_t id;
    std::string title;
    std::string content;
    std::vector<std::string> tags;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point modifiedAt;
    
    static uint32_t nextId;
    static uint32_t generateId();
    void updateModifiedTime();
    
    // Helper methods
    std::string toLowerCase(const std::string& str) const;
};