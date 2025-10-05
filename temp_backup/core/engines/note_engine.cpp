#include "note_engine.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <iomanip>

NoteEngine::NoteEngine() : nextNoteId(1) {
    initializeDefaultTemplates();
}

// CRUD Operations

uint32_t NoteEngine::createNote(const std::string& title, const std::string& content) {
    uint32_t noteId = nextNoteId++;
    Note newNote(noteId, title);
    if (!content.empty()) {
        newNote.setContent(content);
    }
    
    notes.emplace(noteId, std::move(newNote));
    triggerNoteCreatedCallback(noteId);
    
    return noteId;
}

uint32_t NoteEngine::createNoteFromTemplate(const std::string& templateName, const std::string& title) {
    auto templateIt = templates.find(templateName);
    if (templateIt == templates.end()) {
        return 0; // Template not found
    }
    
    uint32_t noteId = createNote(title, templateIt->second.content);
    
    // Add default tags from template
    for (const auto& tag : templateIt->second.defaultTags) {
        addTagToNote(noteId, tag);
    }
    
    return noteId;
}

bool NoteEngine::updateNote(const Note& note) {
    auto it = notes.find(note.getId());
    if (it == notes.end()) {
        return false;
    }
    
    it->second = note;
    triggerNoteUpdatedCallback(note.getId());
    
    return true;
}

bool NoteEngine::deleteNote(uint32_t noteId) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    notes.erase(it);
    triggerNoteDeletedCallback(noteId);
    
    return true;
}

bool NoteEngine::noteExists(uint32_t noteId) const {
    return notes.find(noteId) != notes.end();
}

// Note retrieval

std::shared_ptr<Note> NoteEngine::getNote(uint32_t noteId) const {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return nullptr;
    }
    return std::make_shared<Note>(it->second);
}

std::vector<Note> NoteEngine::getAllNotes() const {
    std::vector<Note> result;
    result.reserve(notes.size());
    
    for (const auto& [id, note] : notes) {
        result.push_back(note);
    }
    
    return result;
}

std::vector<Note> NoteEngine::getRecentNotes(int count) const {
    auto allNotes = getSortedNotes(SortBy::MODIFIED_DATE_DESC);
    
    if (static_cast<int>(allNotes.size()) <= count) {
        return allNotes;
    }
    
    return std::vector<Note>(allNotes.begin(), allNotes.begin() + count);
}

std::vector<Note> NoteEngine::getNotesByTag(const std::string& tag) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.matchesTag(tag)) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteEngine::getNotesModifiedSince(const std::chrono::system_clock::time_point& since) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (note.getModifiedAt() >= since) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Search functionality

std::vector<Note> NoteEngine::searchNotes(const std::string& query) const {
    if (query.empty()) {
        return getAllNotes();
    }
    
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (matchesSearchQuery(note, query)) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteEngine::searchNotesByContent(const std::string& content) const {
    std::vector<Note> result;
    std::string lowerContent = toLowerCase(content);
    
    for (const auto& [id, note] : notes) {
        if (note.containsText(content)) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteEngine::searchNotesByTitle(const std::string& title) const {
    std::vector<Note> result;
    std::string lowerTitle = toLowerCase(title);
    
    for (const auto& [id, note] : notes) {
        std::string noteTitle = toLowerCase(note.getTitle());
        if (noteTitle.find(lowerTitle) != std::string::npos) {
            result.push_back(note);
        }
    }
    
    return result;
}

std::vector<Note> NoteEngine::searchNotesByTag(const std::string& tag) const {
    return getNotesByTag(tag);
}

std::vector<Note> NoteEngine::advancedSearch(const std::string& title, const std::string& content,
                                           const std::vector<std::string>& tags,
                                           const std::chrono::system_clock::time_point& createdAfter,
                                           const std::chrono::system_clock::time_point& modifiedAfter) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        bool matches = true;
        
        // Check title
        if (!title.empty()) {
            std::string noteTitle = toLowerCase(note.getTitle());
            std::string searchTitle = toLowerCase(title);
            if (noteTitle.find(searchTitle) == std::string::npos) {
                matches = false;
            }
        }
        
        // Check content
        if (matches && !content.empty()) {
            if (!note.containsText(content)) {
                matches = false;
            }
        }
        
        // Check tags
        if (matches && !tags.empty()) {
            for (const auto& tag : tags) {
                if (!note.matchesTag(tag)) {
                    matches = false;
                    break;
                }
            }
        }
        
        // Check created date
        if (matches && createdAfter != std::chrono::system_clock::time_point{}) {
            if (note.getCreatedAt() < createdAfter) {
                matches = false;
            }
        }
        
        // Check modified date
        if (matches && modifiedAfter != std::chrono::system_clock::time_point{}) {
            if (note.getModifiedAt() < modifiedAfter) {
                matches = false;
            }
        }
        
        if (matches) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Tag management

std::vector<std::string> NoteEngine::getAllTags() const {
    std::set<std::string> uniqueTags;
    
    for (const auto& [id, note] : notes) {
        auto noteTags = note.getTags();
        uniqueTags.insert(noteTags.begin(), noteTags.end());
    }
    
    return std::vector<std::string>(uniqueTags.begin(), uniqueTags.end());
}

std::vector<std::string> NoteEngine::getPopularTags(int count) const {
    std::unordered_map<std::string, int> tagCounts;
    
    // Count tag usage
    for (const auto& [id, note] : notes) {
        auto noteTags = note.getTags();
        for (const auto& tag : noteTags) {
            tagCounts[tag]++;
        }
    }
    
    // Sort by count
    std::vector<std::pair<std::string, int>> sortedTags(tagCounts.begin(), tagCounts.end());
    std::sort(sortedTags.begin(), sortedTags.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Extract top tags
    std::vector<std::string> result;
    int limit = std::min(count, static_cast<int>(sortedTags.size()));
    for (int i = 0; i < limit; i++) {
        result.push_back(sortedTags[i].first);
    }
    
    return result;
}

bool NoteEngine::addTagToNote(uint32_t noteId, const std::string& tag) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    it->second.addTag(tag);
    triggerNoteUpdatedCallback(noteId);
    
    return true;
}

bool NoteEngine::removeTagFromNote(uint32_t noteId, const std::string& tag) {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return false;
    }
    
    it->second.removeTag(tag);
    triggerNoteUpdatedCallback(noteId);
    
    return true;
}

void NoteEngine::clearTagsFromNote(uint32_t noteId) {
    auto it = notes.find(noteId);
    if (it != notes.end()) {
        it->second.clearTags();
        triggerNoteUpdatedCallback(noteId);
    }
}

int NoteEngine::getTagUsageCount(const std::string& tag) const {
    int count = 0;
    for (const auto& [id, note] : notes) {
        if (note.matchesTag(tag)) {
            count++;
        }
    }
    return count;
}

// Organization and sorting

std::vector<Note> NoteEngine::getSortedNotes(SortBy sortBy) const {
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

std::vector<Note> NoteEngine::getFilteredNotes(const std::function<bool(const Note&)>& filter) const {
    std::vector<Note> result;
    
    for (const auto& [id, note] : notes) {
        if (filter(note)) {
            result.push_back(note);
        }
    }
    
    return result;
}

// Templates

void NoteEngine::addTemplate(const std::string& name, const std::string& content, const std::vector<std::string>& defaultTags) {
    templates[name] = {content, defaultTags};
}

void NoteEngine::removeTemplate(const std::string& name) {
    templates.erase(name);
}

std::vector<std::string> NoteEngine::getTemplateNames() const {
    std::vector<std::string> result;
    result.reserve(templates.size());
    
    for (const auto& [name, template_] : templates) {
        result.push_back(name);
    }
    
    return result;
}

bool NoteEngine::hasTemplate(const std::string& name) const {
    return templates.find(name) != templates.end();
}

// Export functionality

std::string NoteEngine::exportNote(uint32_t noteId, ExportFormat format) const {
    auto it = notes.find(noteId);
    if (it == notes.end()) {
        return "";
    }
    
    const Note& note = it->second;
    
    switch (format) {
        case ExportFormat::MARKDOWN:
            return noteToMarkdown(note);
        case ExportFormat::PLAIN_TEXT:
            return noteToPlainText(note);
        case ExportFormat::HTML:
            return noteToHtml(note);
        case ExportFormat::JSON:
            return noteToJson(note);
        default:
            return noteToPlainText(note);
    }
}

std::string NoteEngine::exportNotes(const std::vector<uint32_t>& noteIds, ExportFormat format) const {
    std::ostringstream oss;
    
    for (size_t i = 0; i < noteIds.size(); i++) {
        if (i > 0) {
            oss << "\n\n";
            if (format == ExportFormat::MARKDOWN) {
                oss << "---\n\n";
            }
        }
        oss << exportNote(noteIds[i], format);
    }
    
    return oss.str();
}

std::string NoteEngine::exportAllNotes(ExportFormat format) const {
    std::vector<uint32_t> allIds;
    allIds.reserve(notes.size());
    
    for (const auto& [id, note] : notes) {
        allIds.push_back(id);
    }
    
    return exportNotes(allIds, format);
}

// Statistics

size_t NoteEngine::getTotalNoteCount() const {
    return notes.size();
}

size_t NoteEngine::getNotesWithTagCount(const std::string& tag) const {
    return static_cast<size_t>(getTagUsageCount(tag));
}

size_t NoteEngine::getEmptyNotesCount() const {
    size_t count = 0;
    for (const auto& [id, note] : notes) {
        if (note.isEmpty()) {
            count++;
        }
    }
    return count;
}

double NoteEngine::getAverageNoteLength() const {
    if (notes.empty()) {
        return 0.0;
    }
    
    size_t totalLength = 0;
    for (const auto& [id, note] : notes) {
        totalLength += note.getContent().length();
    }
    
    return static_cast<double>(totalLength) / static_cast<double>(notes.size());
}

Note NoteEngine::getMostRecentNote() const {
    if (notes.empty()) {
        return Note("");
    }
    
    auto mostRecent = notes.begin();
    for (auto it = notes.begin(); it != notes.end(); ++it) {
        if (it->second.getModifiedAt() > mostRecent->second.getModifiedAt()) {
            mostRecent = it;
        }
    }
    
    return mostRecent->second;
}

Note NoteEngine::getOldestNote() const {
    if (notes.empty()) {
        return Note("");
    }
    
    auto oldest = notes.begin();
    for (auto it = notes.begin(); it != notes.end(); ++it) {
        if (it->second.getCreatedAt() < oldest->second.getCreatedAt()) {
            oldest = it;
        }
    }
    
    return oldest->second;
}

// Bulk operations

std::vector<uint32_t> NoteEngine::createMultipleNotes(const std::vector<std::pair<std::string, std::string>>& titleContentPairs) {
    std::vector<uint32_t> result;
    result.reserve(titleContentPairs.size());
    
    for (const auto& [title, content] : titleContentPairs) {
        result.push_back(createNote(title, content));
    }
    
    return result;
}

bool NoteEngine::deleteMultipleNotes(const std::vector<uint32_t>& noteIds) {
    bool allDeleted = true;
    
    for (uint32_t noteId : noteIds) {
        if (!deleteNote(noteId)) {
            allDeleted = false;
        }
    }
    
    return allDeleted;
}

bool NoteEngine::addTagToMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag) {
    bool allUpdated = true;
    
    for (uint32_t noteId : noteIds) {
        if (!addTagToNote(noteId, tag)) {
            allUpdated = false;
        }
    }
    
    return allUpdated;
}

bool NoteEngine::removeTagFromMultipleNotes(const std::vector<uint32_t>& noteIds, const std::string& tag) {
    bool allUpdated = true;
    
    for (uint32_t noteId : noteIds) {
        if (!removeTagFromNote(noteId, tag)) {
            allUpdated = false;
        }
    }
    
    return allUpdated;
}

// Data management

void NoteEngine::loadNotes(const std::vector<Note>& notesToLoad) {
    notes.clear();
    nextNoteId = 1;
    
    for (const Note& note : notesToLoad) {
        uint32_t noteId = note.getId();
        notes.emplace(noteId, note);
        
        if (noteId >= nextNoteId) {
            nextNoteId = noteId + 1;
        }
    }
}

std::vector<Note> NoteEngine::exportNotes() const {
    return getAllNotes();
}

void NoteEngine::clearAllNotes() {
    notes.clear();
    nextNoteId = 1;
}

// Callbacks

void NoteEngine::setOnNoteCreated(std::function<void(uint32_t)> callback) {
    onNoteCreated = std::move(callback);
}

void NoteEngine::setOnNoteUpdated(std::function<void(uint32_t)> callback) {
    onNoteUpdated = std::move(callback);
}

void NoteEngine::setOnNoteDeleted(std::function<void(uint32_t)> callback) {
    onNoteDeleted = std::move(callback);
}

// Private helper methods

void NoteEngine::triggerNoteCreatedCallback(uint32_t noteId) {
    if (onNoteCreated) {
        onNoteCreated(noteId);
    }
}

void NoteEngine::triggerNoteUpdatedCallback(uint32_t noteId) {
    if (onNoteUpdated) {
        onNoteUpdated(noteId);
    }
}

void NoteEngine::triggerNoteDeletedCallback(uint32_t noteId) {
    if (onNoteDeleted) {
        onNoteDeleted(noteId);
    }
}

// Search helpers

bool NoteEngine::matchesSearchQuery(const Note& note, const std::string& query) const {
    std::vector<std::string> tokens = tokenizeQuery(query);
    
    // Search in title
    if (containsAllTokens(toLowerCase(note.getTitle()), tokens)) {
        return true;
    }
    
    // Search in content
    if (containsAllTokens(toLowerCase(note.getContent()), tokens)) {
        return true;
    }
    
    // Search in tags
    auto tags = note.getTags();
    for (const auto& tag : tags) {
        if (containsAllTokens(toLowerCase(tag), tokens)) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> NoteEngine::tokenizeQuery(const std::string& query) const {
    std::vector<std::string> tokens;
    std::istringstream iss(toLowerCase(query));
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool NoteEngine::containsAllTokens(const std::string& text, const std::vector<std::string>& tokens) const {
    for (const auto& token : tokens) {
        if (text.find(token) == std::string::npos) {
            return false;
        }
    }
    return true;
}

// Export helpers

std::string NoteEngine::noteToMarkdown(const Note& note) const {
    std::ostringstream oss;
    
    // Title
    oss << "# " << note.getTitle() << "\n\n";
    
    // Tags
    auto tags = note.getTags();
    if (!tags.empty()) {
        oss << "**Tags:** ";
        for (size_t i = 0; i < tags.size(); i++) {
            if (i > 0) oss << ", ";
            oss << "`" << tags[i] << "`";
        }
        oss << "\n\n";
    }
    
    // Content
    oss << note.getContent() << "\n\n";
    
    // Metadata
    auto created = std::chrono::system_clock::to_time_t(note.getCreatedAt());
    auto modified = std::chrono::system_clock::to_time_t(note.getModifiedAt());
    
    oss << "---\n";
    oss << "*Created: " << std::put_time(std::localtime(&created), "%Y-%m-%d %H:%M:%S") << "*\n";
    oss << "*Modified: " << std::put_time(std::localtime(&modified), "%Y-%m-%d %H:%M:%S") << "*\n";
    
    return oss.str();
}

std::string NoteEngine::noteToPlainText(const Note& note) const {
    std::ostringstream oss;
    
    oss << note.getTitle() << "\n";
    oss << std::string(note.getTitle().length(), '=') << "\n\n";
    
    auto tags = note.getTags();
    if (!tags.empty()) {
        oss << "Tags: ";
        for (size_t i = 0; i < tags.size(); i++) {
            if (i > 0) oss << ", ";
            oss << tags[i];
        }
        oss << "\n\n";
    }
    
    oss << note.getContent() << "\n";
    
    return oss.str();
}

std::string NoteEngine::noteToHtml(const Note& note) const {
    std::ostringstream oss;
    
    oss << "<article>\n";
    oss << "  <h1>" << escapeHtml(note.getTitle()) << "</h1>\n";
    
    auto tags = note.getTags();
    if (!tags.empty()) {
        oss << "  <div class=\"tags\">\n";
        oss << "    <strong>Tags:</strong> ";
        for (size_t i = 0; i < tags.size(); i++) {
            if (i > 0) oss << ", ";
            oss << "<span class=\"tag\">" << escapeHtml(tags[i]) << "</span>";
        }
        oss << "\n  </div>\n";
    }
    
    oss << "  <div class=\"content\">\n";
    oss << "    <p>" << escapeHtml(note.getContent()) << "</p>\n";
    oss << "  </div>\n";
    
    auto created = std::chrono::system_clock::to_time_t(note.getCreatedAt());
    auto modified = std::chrono::system_clock::to_time_t(note.getModifiedAt());
    
    oss << "  <footer>\n";
    oss << "    <small>Created: " << std::put_time(std::localtime(&created), "%Y-%m-%d %H:%M:%S") << "</small><br>\n";
    oss << "    <small>Modified: " << std::put_time(std::localtime(&modified), "%Y-%m-%d %H:%M:%S") << "</small>\n";
    oss << "  </footer>\n";
    oss << "</article>\n";
    
    return oss.str();
}

std::string NoteEngine::noteToJson(const Note& note) const {
    return note.toJson().dump(2);
}

// Utility methods

std::string NoteEngine::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string NoteEngine::escapeHtml(const std::string& str) const {
    std::string result;
    result.reserve(str.length() * 1.1); // Reserve some extra space
    
    for (char c : str) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c; break;
        }
    }
    
    return result;
}

void NoteEngine::initializeDefaultTemplates() {
    // Meeting Notes template
    addTemplate("Meeting Notes", 
                "# Meeting Notes\n\n"
                "**Date:** \n"
                "**Attendees:** \n"
                "**Agenda:** \n\n"
                "## Discussion Points\n\n"
                "## Action Items\n\n"
                "## Next Steps\n\n",
                {"meeting", "work"});
    
    // Daily Journal template
    addTemplate("Daily Journal",
                "# Daily Journal\n\n"
                "**Date:** \n\n"
                "## What went well today?\n\n"
                "## What could be improved?\n\n"
                "## Tomorrow's priorities\n\n",
                {"journal", "personal"});
    
    // Project Notes template
    addTemplate("Project Notes",
                "# Project: \n\n"
                "**Status:** \n"
                "**Priority:** \n"
                "**Due Date:** \n\n"
                "## Objectives\n\n"
                "## Progress\n\n"
                "## Challenges\n\n"
                "## Resources\n\n",
                {"project", "work"});
    
    // Book Notes template
    addTemplate("Book Notes",
                "# Book: \n\n"
                "**Author:** \n"
                "**Genre:** \n"
                "**Rating:** /5\n\n"
                "## Summary\n\n"
                "## Key Takeaways\n\n"
                "## Favorite Quotes\n\n",
                {"book", "reading", "notes"});
    
    // Recipe template
    addTemplate("Recipe",
                "# Recipe: \n\n"
                "**Prep Time:** \n"
                "**Cook Time:** \n"
                "**Servings:** \n\n"
                "## Ingredients\n\n"
                "## Instructions\n\n"
                "## Notes\n\n",
                {"recipe", "cooking"});
}