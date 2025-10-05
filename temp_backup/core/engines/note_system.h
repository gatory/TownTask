#pragma once

#include "../models/note.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <optional>
#include <set>

class NoteSystem {
public:
    // Constructor
    NoteSystem();
    
    // Note management
    uint32_t createNote(const std::string& title = "", const std::string& content = "");
    bool updateNote(uint32_t noteId, const Note& updatedNote);
    bool deleteNote(uint32_t noteId);
    
    // Content modification
    bool setNoteTitle(uint32_t noteId, const std::string& title);
    bool setNoteContent(uint32_t noteId, const std::string& content);
    bool addNoteTag(uint32_t noteId, const std::string& tag);
    bool removeNoteTag(uint32_t noteId, const std::string& tag);
    bool clearNoteTags(uint32_t noteId);
    
    // Querying
    std::vector<Note> getAllNotes() const;
    std::optional<Note> getNote(uint32_t noteId) const;
    std::vector<Note> getNotesByTag(const std::string& tag) const;
    std::vector<Note> getNotesWithAnyTag(const std::vector<std::string>& tags) const;
    std::vector<Note> getNotesWithAllTags(const std::vector<std::string>& tags) const;
    
    // Search functionality
    std::vector<Note> searchNotes(const std::string& searchText) const;
    std::vector<Note> searchNotesByTitle(const std::string& searchText) const;
    std::vector<Note> searchNotesByContent(const std::string& searchText) const;
    std::vector<Note> searchNotesByTag(const std::string& searchText) const;
    
    // Advanced search
    struct SearchCriteria {
        std::string titleQuery;
        std::string contentQuery;
        std::vector<std::string> requiredTags;
        std::vector<std::string> excludedTags;
        bool caseSensitive = false;
        bool wholeWordsOnly = false;
    };
    std::vector<Note> advancedSearch(const SearchCriteria& criteria) const;
    
    // Tag management
    std::set<std::string> getAllTags() const;
    std::vector<std::string> getTagsForNote(uint32_t noteId) const;
    size_t getTagUsageCount(const std::string& tag) const;
    std::vector<std::pair<std::string, size_t>> getTagUsageStatistics() const;
    
    // Organization and sorting
    enum class SortBy {
        CREATED_DATE_ASC,
        CREATED_DATE_DESC,
        MODIFIED_DATE_ASC,
        MODIFIED_DATE_DESC,
        TITLE_ASC,
        TITLE_DESC,
        CONTENT_LENGTH_ASC,
        CONTENT_LENGTH_DESC
    };
    std::vector<Note> getSortedNotes(SortBy sortBy) const;
    
    // Filtering
    std::vector<Note> getNotesCreatedAfter(const std::chrono::system_clock::time_point& date) const;
    std::vector<Note> getNotesModifiedAfter(const std::chrono::system_clock::time_point& date) const;
    std::vector<Note> getNotesCreatedBetween(const std::chrono::system_clock::time_point& startDate,
                                           const std::chrono::system_clock::time_point& endDate) const;
    std::vector<Note> getEmptyNotes() const;
    std::vector<Note> getNonEmptyNotes() const;
    
    // Bulk operations
    std::vector<uint32_t> createMultipleNotes(const std::vector<std::pair<std::string, std::string>>& titleContentPairs);
    bool deleteMultipleNotes(const std::vector<uint32_t>& noteIds);
    bool addTagToMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag);
    bool removeTagFromMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag);
    
    // Statistics
    size_t getTotalNoteCount() const;
    size_t getEmptyNoteCount() const;
    size_t getNonEmptyNoteCount() const;
    size_t getTotalTagCount() const;
    size_t getAverageNotesPerTag() const;
    size_t getAverageTagsPerNote() const;
    
    // Data management
    void loadNotes(const std::vector<Note>& noteList);
    std::vector<Note> exportNotes() const;
    void clearAllNotes();
    
    // Events and callbacks
    void setNoteCreationCallback(std::function<void(const Note&)> callback);
    void setNoteUpdateCallback(std::function<void(const Note&)> callback);
    void setNoteDeletionCallback(std::function<void(uint32_t)> callback);
    void setTagAddedCallback(std::function<void(uint32_t, const std::string&)> callback);
    void setTagRemovedCallback(std::function<void(uint32_t, const std::string&)> callback);
    
    // Validation
    bool noteExists(uint32_t noteId) const;
    bool isValidNoteId(uint32_t noteId) const;
    bool tagExists(const std::string& tag) const;
    
    // Import/Export utilities
    std::string exportNotesToJson() const;
    bool importNotesFromJson(const std::string& jsonData);

private:
    // Storage
    std::unordered_map<uint32_t, Note> notes;
    
    // Callbacks
    std::function<void(const Note&)> onNoteCreated;
    std::function<void(const Note&)> onNoteUpdated;
    std::function<void(uint32_t)> onNoteDeleted;
    std::function<void(uint32_t, const std::string&)> onTagAdded;
    std::function<void(uint32_t, const std::string&)> onTagRemoved;
    
    // Helper methods
    void triggerNoteCreationCallback(const Note& note);
    void triggerNoteUpdateCallback(const Note& note);
    void triggerNoteDeletionCallback(uint32_t noteId);
    void triggerTagAddedCallback(uint32_t noteId, const std::string& tag);
    void triggerTagRemovedCallback(uint32_t noteId, const std::string& tag);
    
    // Search helpers
    bool matchesSearchCriteria(const Note& note, const SearchCriteria& criteria) const;
    bool containsText(const std::string& text, const std::string& query, bool caseSensitive, bool wholeWords) const;
    std::vector<std::string> tokenizeText(const std::string& text) const;
};