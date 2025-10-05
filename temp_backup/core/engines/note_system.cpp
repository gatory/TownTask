#include "note_system.h"
#include <algorithm>
#include <sstream>
#include <regex>
#include <cctype>

NoteSystem::NoteSystem() = default;

// Note management
uint32_t NoteSystem::createNote(const std::string& title, const std::string& content) {
    Note newNote(title, content);
    uint32_t noteId = newNote.getId();
    
    auto [it, inserted] = notes.emplace(noteId, std::move(newNote));
    triggerNoteCreationCallback(it->second);
    
    return noteId;
}

bool NoteSystem::updateNote(uint32_t noteId, const Note& updatedNote) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    it->second = updatedNote;
    triggerNoteUpdateCallback(it->second);
    return true;
}

bool NoteSystem::deleteNote(uint32_t noteId) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    notes.erase(it);
    triggerNoteDeletionCallback(noteId);
    return true;
}

// Content modification
bool NoteSystem::setNoteTitle(uint32_t noteId, const std::string& title) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    it->second.setTitle(title);
    triggerNoteUpdateCallback(it->second);
    return true;
}

bool NoteSystem::setNoteContent(uint32_t noteId, const std::string& content) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    it->second.setContent(content);
    triggerNoteUpdateCallback(it->second);
    return true;
}

bool NoteSystem::addNoteTag(uint32_t noteId, const std::string& tag) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    // Check if tag already exists
    if (it->second.hasTag(tag)) {
        return false; // Tag already exists
    }
    
    it->second.addTag(tag);
    triggerTagAddedCallback(noteId, tag);
    triggerNoteUpdateCallback(it->second);
    return true;
}

bool NoteSystem::removeNoteTag(uint32_t noteId, const std::string& tag) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    // Check if tag exists before removing
    if (!it->second.hasTag(tag)) {
        return false; // Tag doesn't exist
    }
    
    it->second.removeTag(tag);
    triggerTagRemovedCallback(noteId, tag);
    triggerNoteUpdateCallback(it->second);
    return true;
}

bool NoteSystem::clearNoteTags(uint32_t noteId) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    auto tags = it->second.getTags();
    it->second.clearTags();
    
    // Trigger callbacks for each removed tag
    for (const auto& tag : tags) {
        triggerTagRemovedCallback(noteId, tag);
    }
    
    triggerNoteUpdateCallback(it->second);
    return true;
}

// Querying
std::vector<Note> NoteSystem::getAllNotes() const {
    std::vector<Note> result;
    result.reserve(notes.size());
    
    for (const auto& [id, note] : notes) {
        result.push_back(note);
    }
    
    return result;
}

std::optional<Note> NoteSystem::getNote(uint32_t noteId) const {
    auto it = notes.find(noteId);
    if (it != notes.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Note> NoteSystem::getNotesByTag(const std::string& tag) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.matchesTag(tag)) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getNotesWithAnyTag(const std::vector<std::string>& tags) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        for (const auto& tag : tags) {
            if (note.matchesTag(tag)) {
                result.push_back(note);
                break; // Found at least one matching tag
            }
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getNotesWithAllTags(const std::vector<std::string>& tags) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        bool hasAllTags = true;
        for (const auto& tag : tags) {
            if (!note.matchesTag(tag)) {
                hasAllTags = false;
                break;
            }
        }
        if (hasAllTags) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Search functionality
std::vector<Note> NoteSystem::searchNotes(const std::string& searchText) const {
    std::vector<Note> result;
    
    if (searchText.empty()) {
        return getAllNotes();
    }
    
    for (const auto& [id, note] : notes) {
        if (note.containsText(searchText, false)) { // Case-insensitive search
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::searchNotesByTitle(const std::string& searchText) const {
    std::vector<Note> result;
    
    if (searchText.empty()) {
        return getAllNotes();
    }
    
    std::string lowerSearchText = searchText;
    std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::tolower);
    
    for (const auto& [id, note] : notes) {
        std::string lowerTitle = note.getTitle();
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        
        if (lowerTitle.find(lowerSearchText) != std::string::npos) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::searchNotesByContent(const std::string& searchText) const {
    std::vector<Note> result;
    
    if (searchText.empty()) {
        return getAllNotes();
    }
    
    std::string lowerSearchText = searchText;
    std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::tolower);
    
    for (const auto& [id, note] : notes) {
        std::string lowerContent = note.getContent();
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
        
        if (lowerContent.find(lowerSearchText) != std::string::npos) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::searchNotesByTag(const std::string& searchText) const {
    std::vector<Note> result;
    
    if (searchText.empty()) {
        return getAllNotes();
    }
    
    for (const auto& [id, note] : notes) {
        if (note.matchesTag(searchText, false)) { // Case-insensitive tag search
            result.push_back(note);
        }
    }
    
    return result;
}

// Advanced search
std::vector<Note> NoteSystem::advancedSearch(const SearchCriteria& criteria) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (matchesSearchCriteria(note, criteria)) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Tag management
std::set<std::string> NoteSystem::getAllTags() const {
    std::set<std::string> allTags;
    
    for (const auto& [id, note] : notes) {
        auto noteTags = note.getTags();
        allTags.insert(noteTags.begin(), noteTags.end());
    }
    
    return allTags;
}

std::vector<std::string> NoteSystem::getTagsForNote(uint32_t noteId) const {
    auto it = notes.find(noteId);
    if (it != notes.end()) {
        return it->second.getTags();
    }
    return {};
}

size_t NoteSystem::getTagUsageCount(const std::string& tag) const {
    size_t count = 0;
    
    for (const auto& [id, note] : notes) {
        if (note.matchesTag(tag)) {
            count++;
        }
    }
    
    return count;
}

std::vector<std::pair<std::string, size_t>> NoteSystem::getTagUsageStatistics() const {
    std::unordered_map<std::string, size_t> tagCounts;
    
    for (const auto& [id, note] : notes) {
        auto tags = note.getTags();
        for (const auto& tag : tags) {
            tagCounts[tag]++;
        }
    }
    
    std::vector<std::pair<std::string, size_t>> result;
    for (const auto& [tag, count] : tagCounts) {
        result.emplace_back(tag, count);
    }
    
    // Sort by usage count (descending)
    std::sort(result.begin(), result.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return result;
}

// Organization and sorting
std::vector<Note> NoteSystem::getSortedNotes(SortBy sortBy) const {
    std::vector<Note> result = getAllNotes();
    
    switch (sortBy) {
        case SortBy::CREATED_DATE_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getCreatedAt() < b.getCreatedAt(); });
            break;
        case SortBy::CREATED_DATE_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getCreatedAt() > b.getCreatedAt(); });
            break;
        case SortBy::MODIFIED_DATE_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getModifiedAt() < b.getModifiedAt(); });
            break;
        case SortBy::MODIFIED_DATE_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getModifiedAt() > b.getModifiedAt(); });
            break;
        case SortBy::TITLE_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getTitle() < b.getTitle(); });
            break;
        case SortBy::TITLE_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getTitle() > b.getTitle(); });
            break;
        case SortBy::CONTENT_LENGTH_ASC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getContent().length() < b.getContent().length(); });
            break;
        case SortBy::CONTENT_LENGTH_DESC:
            std::sort(result.begin(), result.end(), 
                     [](const Note& a, const Note& b) { return a.getContent().length() > b.getContent().length(); });
            break;
    }
    
    return result;
}

// Filtering
std::vector<Note> NoteSystem::getNotesCreatedAfter(const std::chrono::system_clock::time_point& date) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.getCreatedAt() > date) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getNotesModifiedAfter(const std::chrono::system_clock::time_point& date) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.getModifiedAt() > date) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getNotesCreatedBetween(const std::chrono::system_clock::time_point& startDate,
                                                   const std::chrono::system_clock::time_point& endDate) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        auto createdAt = note.getCreatedAt();
        if (createdAt >= startDate && createdAt <= endDate) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getEmptyNotes() const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.isEmpty()) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteSystem::getNonEmptyNotes() const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (!note.isEmpty()) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Bulk operations
std::vector<uint32_t> NoteSystem::createMultipleNotes(const std::vector<std::pair<std::string, std::string>>& titleContentPairs) {
    std::vector<uint32_t> noteIds;
    noteIds.reserve(titleContentPairs.size());
    
    for (const auto& [title, content] : titleContentPairs) {
        noteIds.push_back(createNote(title, content));
    }
    
    return noteIds;
}

bool NoteSystem::deleteMultipleNotes(const std::vector<uint32_t>& noteIds) {
    bool allDeleted = true;
    
    for (uint32_t noteId : noteIds) {
        if (!deleteNote(noteId)) {
            allDeleted = false;
        }
    }
    
    return allDeleted;
}

bool NoteSystem::addTagToMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag) {
    bool allSucceeded = true;
    
    for (uint32_t noteId : noteIds) {
        if (!addNoteTag(noteId, tag)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

bool NoteSystem::removeTagFromMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag) {
    bool allSucceeded = true;
    
    for (uint32_t noteId : noteIds) {
        if (!removeNoteTag(noteId, tag)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

// Statistics
size_t NoteSystem::getTotalNoteCount() const {
    return notes.size();
}

size_t NoteSystem::getEmptyNoteCount() const {
    return getEmptyNotes().size();
}

size_t NoteSystem::getNonEmptyNoteCount() const {
    return getNonEmptyNotes().size();
}

size_t NoteSystem::getTotalTagCount() const {
    return getAllTags().size();
}

size_t NoteSystem::getAverageNotesPerTag() const {
    auto tagStats = getTagUsageStatistics();
    if (tagStats.empty()) {
        return 0;
    }
    
    size_t totalUsage = 0;
    for (const auto& [tag, count] : tagStats) {
        totalUsage += count;
    }
    
    return totalUsage / tagStats.size();
}

size_t NoteSystem::getAverageTagsPerNote() const {
    if (notes.empty()) {
        return 0;
    }
    
    size_t totalTags = 0;
    for (const auto& [id, note] : notes) {
        totalTags += note.getTags().size();
    }
    
    return totalTags / notes.size();
}

// Data management
void NoteSystem::loadNotes(const std::vector<Note>& noteList) {
    notes.clear();
    
    for (const auto& note : noteList) {
        notes.emplace(note.getId(), note);
    }
}

std::vector<Note> NoteSystem::exportNotes() const {
    return getAllNotes();
}

void NoteSystem::clearAllNotes() {
    notes.clear();
}

// Events and callbacks
void NoteSystem::setNoteCreationCallback(std::function<void(const Note&)> callback) {
    onNoteCreated = std::move(callback);
}

void NoteSystem::setNoteUpdateCallback(std::function<void(const Note&)> callback) {
    onNoteUpdated = std::move(callback);
}

void NoteSystem::setNoteDeletionCallback(std::function<void(uint32_t)> callback) {
    onNoteDeleted = std::move(callback);
}

void NoteSystem::setTagAddedCallback(std::function<void(uint32_t, const std::string&)> callback) {
    onTagAdded = std::move(callback);
}

void NoteSystem::setTagRemovedCallback(std::function<void(uint32_t, const std::string&)> callback) {
    onTagRemoved = std::move(callback);
}

// Validation
bool NoteSystem::noteExists(uint32_t noteId) const {
    return notes.find(noteId) != notes.end();
}

bool NoteSystem::isValidNoteId(uint32_t noteId) const {
    return noteExists(noteId);
}

bool NoteSystem::tagExists(const std::string& tag) const {
    auto allTags = getAllTags();
    return allTags.find(tag) != allTags.end();
}

// Import/Export utilities
std::string NoteSystem::exportNotesToJson() const {
    // This would use the JsonSerializer, but for now return a placeholder
    return "{}"; // TODO: Implement with JsonSerializer
}

bool NoteSystem::importNotesFromJson(const std::string& /* jsonData */) {
    // This would use the JsonSerializer, but for now return false
    return false; // TODO: Implement with JsonSerializer
}

// Private helper methods
void NoteSystem::triggerNoteCreationCallback(const Note& note) {
    if (onNoteCreated) {
        onNoteCreated(note);
    }
}

void NoteSystem::triggerNoteUpdateCallback(const Note& note) {
    if (onNoteUpdated) {
        onNoteUpdated(note);
    }
}

void NoteSystem::triggerNoteDeletionCallback(uint32_t noteId) {
    if (onNoteDeleted) {
        onNoteDeleted(noteId);
    }
}

void NoteSystem::triggerTagAddedCallback(uint32_t noteId, const std::string& tag) {
    if (onTagAdded) {
        onTagAdded(noteId, tag);
    }
}

void NoteSystem::triggerTagRemovedCallback(uint32_t noteId, const std::string& tag) {
    if (onTagRemoved) {
        onTagRemoved(noteId, tag);
    }
}

// Search helpers
bool NoteSystem::matchesSearchCriteria(const Note& note, const SearchCriteria& criteria) const {
    // Check title query
    if (!criteria.titleQuery.empty()) {
        if (!containsText(note.getTitle(), criteria.titleQuery, criteria.caseSensitive, criteria.wholeWordsOnly)) {
            return false;
        }
    }
    
    // Check content query
    if (!criteria.contentQuery.empty()) {
        if (!containsText(note.getContent(), criteria.contentQuery, criteria.caseSensitive, criteria.wholeWordsOnly)) {
            return false;
        }
    }
    
    // Check required tags
    for (const auto& requiredTag : criteria.requiredTags) {
        if (!note.matchesTag(requiredTag, criteria.caseSensitive)) {
            return false;
        }
    }
    
    // Check excluded tags
    for (const auto& excludedTag : criteria.excludedTags) {
        if (note.matchesTag(excludedTag, criteria.caseSensitive)) {
            return false;
        }
    }
    
    return true;
}

bool NoteSystem::containsText(const std::string& text, const std::string& query, bool caseSensitive, bool wholeWords) const {
    std::string searchText = text;
    std::string searchQuery = query;
    
    if (!caseSensitive) {
        std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
        std::transform(searchQuery.begin(), searchQuery.end(), searchQuery.begin(), ::tolower);
    }
    
    if (!wholeWords) {
        return searchText.find(searchQuery) != std::string::npos;
    }
    
    // Whole words search using regex
    // Escape special regex characters manually
    std::string escapedQuery = searchQuery;
    std::string specialChars = "\\^$.|?*+()[]{}";
    for (char c : specialChars) {
        size_t pos = 0;
        std::string target(1, c);
        std::string replacement = "\\" + target;
        while ((pos = escapedQuery.find(target, pos)) != std::string::npos) {
            escapedQuery.replace(pos, 1, replacement);
            pos += replacement.length();
        }
    }
    
    std::string pattern = "\\b" + escapedQuery + "\\b";
    std::regex wordRegex(pattern, caseSensitive ? std::regex::ECMAScript : std::regex::icase);
    return std::regex_search(searchText, wordRegex);
}

std::vector<std::string> NoteSystem::tokenizeText(const std::string& text) const {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (iss >> token) {
        // Remove punctuation
        token.erase(std::remove_if(token.begin(), token.end(), ::ispunct), token.end());
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}