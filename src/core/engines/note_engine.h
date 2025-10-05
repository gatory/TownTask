#pragma once

#include "../models/note.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <set>
#include <memory>

class NoteEngine {
public:
    // Constructor
    NoteEngine();
    
    // CRUD Operations
    uint32_t createNote(const std::string& title = "", const std::string& content = "");
    uint32_t createNoteFromTemplate(const std::string& templateName, const std::string& title = "");
    bool updateNote(const Note& note);
    bool deleteNote(uint32_t noteId);
    bool noteExists(uint32_t noteId) const;
    
    // Note retrieval
    std::shared_ptr<Note> getNote(uint32_t noteId) const;
    std::vector<Note> getAllNotes() const;
    std::vector<Note> getRecentNotes(int count = 10) const;
    std::vector<Note> getNotesByTag(const std::string& tag) const;
    std::vector<Note> getNotesModifiedSince(const std::chrono::system_clock::time_point& since) const;
    
    // Search functionality
    std::vector<Note> searchNotes(const std::string& query) const;
    std::vector<Note> searchNotesByContent(const std::string& content) const;
    std::vector<Note> searchNotesByTitle(const std::string& title) const;
    std::vector<Note> searchNotesByTag(const std::string& tag) const;
    std::vector<Note> advancedSearch(const std::string& title, const std::string& content, 
                                   const std::vector<std::string>& tags, 
                                   const std::chrono::system_clock::time_point& createdAfter = {},
                                   const std::chrono::system_clock::time_point& modifiedAfter = {}) const;
    
    // Tag management
    std::vector<std::string> getAllTags() const;
    std::vector<std::string> getPopularTags(int count = 10) const;
    bool addTagToNote(uint32_t noteId, const std::string& tag);
    bool removeTagFromNote(uint32_t noteId, const std::string& tag);
    void clearTagsFromNote(uint32_t noteId);
    int getTagUsageCount(const std::string& tag) const;
    
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
    std::vector<Note> getFilteredNotes(const std::function<bool(const Note&)>& filter) const;
    
    // Templates
    void addTemplate(const std::string& name, const std::string& content, const std::vector<std::string>& defaultTags = {});
    void removeTemplate(const std::string& name);
    std::vector<std::string> getTemplateNames() const;
    bool hasTemplate(const std::string& name) const;
    
    // Export functionality
    enum class ExportFormat {
        MARKDOWN,
        PLAIN_TEXT,
        HTML,
        JSON
    };
    
    std::string exportNote(uint32_t noteId, ExportFormat format) const;
    std::string exportNotes(const std::vector<uint32_t>& noteIds, ExportFormat format) const;
    std::string exportAllNotes(ExportFormat format) const;
    
    // Statistics
    size_t getTotalNoteCount() const;
    size_t getNotesWithTagCount(const std::string& tag) const;
    size_t getEmptyNotesCount() const;
    double getAverageNoteLength() const;
    Note getMostRecentNote() const;
    Note getOldestNote() const;
    
    // Bulk operations
    std::vector<uint32_t> createMultipleNotes(const std::vector<std::pair<std::string, std::string>>& titleContentPairs);
    bool deleteMultipleNotes(const std::vector<uint32_t>& noteIds);
    bool addTagToMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag);
    bool removeTagFromMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag);
    
    // Data management
    void loadNotes(const std::vector<Note>& notes);
    std::vector<Note> exportNotes() const;
    void clearAllNotes();
    
    // Callbacks
    void setOnNoteCreated(std::function<void(uint32_t)> callback);
    void setOnNoteUpdated(std::function<void(uint32_t)> callback);
    void setOnNoteDeleted(std::function<void(uint32_t)> callback);
    
private:
    // Data storage
    std::unordered_map<uint32_t, Note> notes;
    uint32_t nextNoteId;
    
    // Templates storage
    struct NoteTemplate {
        std::string content;
        std::vector<std::string> defaultTags;
    };
    std::unordered_map<std::string, NoteTemplate> templates;
    
    // Callbacks
    std::function<void(uint32_t)> onNoteCreated;
    std::function<void(uint32_t)> onNoteUpdated;
    std::function<void(uint32_t)> onNoteDeleted;
    
    // Helper methods
    void triggerNoteCreatedCallback(uint32_t noteId);
    void triggerNoteUpdatedCallback(uint32_t noteId);
    void triggerNoteDeletedCallback(uint32_t noteId);
    
    // Search helpers
    bool matchesSearchQuery(const Note& note, const std::string& query) const;
    std::vector<std::string> tokenizeQuery(const std::string& query) const;
    bool containsAllTokens(const std::string& text, const std::vector<std::string>& tokens) const;
    
    // Export helpers
    std::string noteToMarkdown(const Note& note) const;
    std::string noteToPlainText(const Note& note) const;
    std::string noteToHtml(const Note& note) const;
    std::string noteToJson(const Note& note) const;
    
    // Utility methods
    std::string toLowerCase(const std::string& str) const;
    std::string escapeHtml(const std::string& str) const;
    void initializeDefaultTemplates();
};