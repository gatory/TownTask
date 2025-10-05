#include "note.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

// Static member initialization
uint32_t Note::nextId = 1;

Note::Note(const std::string& title, const std::string& content)
    : id(generateId()), title(title), content(content),
      createdAt(std::chrono::system_clock::now()) {
    modifiedAt = createdAt;
}

Note::Note(uint32_t id, const std::string& title, const std::string& content)
    : id(id), title(title), content(content),
      createdAt(std::chrono::system_clock::now()) {
    // Update nextId to ensure no conflicts
    if (id >= nextId) {
        nextId = id + 1;
    }
    modifiedAt = createdAt;
}

uint32_t Note::generateId() {
    return nextId++;
}

void Note::updateModifiedTime() {
    modifiedAt = std::chrono::system_clock::now();
}

void Note::setTitle(const std::string& newTitle) {
    if (title != newTitle) {
        title = newTitle;
        updateModifiedTime();
    }
}

void Note::setContent(const std::string& newContent) {
    if (content != newContent) {
        content = newContent;
        updateModifiedTime();
    }
}

void Note::addTag(const std::string& tag) {
    if (tag.empty()) return;
    
    // Check if tag already exists (case-insensitive)
    auto it = std::find_if(tags.begin(), tags.end(),
        [this, &tag](const std::string& existingTag) {
            return toLowerCase(existingTag) == toLowerCase(tag);
        });
    
    if (it == tags.end()) {
        tags.push_back(tag);
        updateModifiedTime();
    }
}

void Note::removeTag(const std::string& tag) {
    auto it = std::find_if(tags.begin(), tags.end(),
        [this, &tag](const std::string& existingTag) {
            return toLowerCase(existingTag) == toLowerCase(tag);
        });
    
    if (it != tags.end()) {
        tags.erase(it);
        updateModifiedTime();
    }
}

bool Note::hasTag(const std::string& tag) const {
    return std::find_if(tags.begin(), tags.end(),
        [this, &tag](const std::string& existingTag) {
            return toLowerCase(existingTag) == toLowerCase(tag);
        }) != tags.end();
}

void Note::clearTags() {
    if (!tags.empty()) {
        tags.clear();
        updateModifiedTime();
    }
}

bool Note::containsText(const std::string& searchText, bool caseSensitive) const {
    if (searchText.empty()) return true;
    
    std::string searchIn = title + " " + content;
    std::string searchFor = searchText;
    
    if (!caseSensitive) {
        searchIn = toLowerCase(searchIn);
        searchFor = toLowerCase(searchFor);
    }
    
    return searchIn.find(searchFor) != std::string::npos;
}

bool Note::matchesTag(const std::string& tag, bool caseSensitive) const {
    if (tag.empty()) return false;
    
    return std::find_if(tags.begin(), tags.end(),
        [this, &tag, caseSensitive](const std::string& existingTag) {
            if (caseSensitive) {
                return existingTag == tag;
            } else {
                return toLowerCase(existingTag) == toLowerCase(tag);
            }
        }) != tags.end();
}

std::string Note::getPreview(size_t maxLength) const {
    if (content.length() <= maxLength) {
        return content;
    }
    
    std::string preview = content.substr(0, maxLength);
    
    // Try to break at a word boundary
    size_t lastSpace = preview.find_last_of(" \t\n");
    if (lastSpace != std::string::npos && lastSpace > maxLength / 2) {
        preview = preview.substr(0, lastSpace);
    }
    
    return preview + "...";
}

std::string Note::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

nlohmann::json Note::toJson() const {
    nlohmann::json j;
    
    j["id"] = id;
    j["title"] = title;
    j["content"] = content;
    j["tags"] = tags;
    
    // Convert time points to ISO 8601 strings
    auto createdTime = std::chrono::system_clock::to_time_t(createdAt);
    auto modifiedTime = std::chrono::system_clock::to_time_t(modifiedAt);
    
    std::stringstream createdSs, modifiedSs;
    createdSs << std::put_time(std::gmtime(&createdTime), "%Y-%m-%dT%H:%M:%SZ");
    modifiedSs << std::put_time(std::gmtime(&modifiedTime), "%Y-%m-%dT%H:%M:%SZ");
    
    j["createdAt"] = createdSs.str();
    j["modifiedAt"] = modifiedSs.str();
    
    return j;
}

Note Note::fromJson(const nlohmann::json& json) {
    uint32_t id = json.at("id").get<uint32_t>();
    std::string title = json.at("title").get<std::string>();
    std::string content = json.contains("content") ? json.at("content").get<std::string>() : "";
    
    Note note(id, title, content);
    
    // Set tags
    if (json.contains("tags") && json.at("tags").is_array()) {
        for (const auto& tag : json.at("tags")) {
            note.addTag(tag.get<std::string>());
        }
    }
    
    // Parse dates
    if (json.contains("createdAt")) {
        std::string createdAtStr = json.at("createdAt").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(createdAtStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            note.createdAt = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    if (json.contains("modifiedAt")) {
        std::string modifiedAtStr = json.at("modifiedAt").get<std::string>();
        std::tm tm = {};
        std::istringstream ss(modifiedAtStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (!ss.fail()) {
            note.modifiedAt = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }
    }
    
    return note;
}