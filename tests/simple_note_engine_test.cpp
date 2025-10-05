#include <iostream>
#include <cassert>
#include "../src/core/engines/note_engine.h"

void testBasicNoteOperations() {
    std::cout << "Testing basic note operations..." << std::endl;
    
    NoteEngine engine;
    
    // Test creating a note
    uint32_t noteId = engine.createNote("Test Note", "Test content");
    assert(noteId > 0);
    assert(engine.noteExists(noteId));
    assert(engine.getTotalNoteCount() == 1);
    
    // Test retrieving a note
    auto note = engine.getNote(noteId);
    assert(note != nullptr);
    assert(note->getTitle() == "Test Note");
    assert(note->getContent() == "Test content");
    
    // Test updating a note
    note->setTitle("Updated Title");
    note->setContent("Updated content");
    bool updated = engine.updateNote(*note);
    assert(updated);
    
    auto updatedNote = engine.getNote(noteId);
    assert(updatedNote->getTitle() == "Updated Title");
    assert(updatedNote->getContent() == "Updated content");
    
    // Test deleting a note
    bool deleted = engine.deleteNote(noteId);
    assert(deleted);
    assert(!engine.noteExists(noteId));
    assert(engine.getTotalNoteCount() == 0);
    
    std::cout << "Basic note operations test passed!" << std::endl;
}

void testTagOperations() {
    std::cout << "Testing tag operations..." << std::endl;
    
    NoteEngine engine;
    uint32_t noteId = engine.createNote("Tagged Note", "Content");
    
    // Test adding tags
    bool tagAdded = engine.addTagToNote(noteId, "work");
    assert(tagAdded);
    
    auto note = engine.getNote(noteId);
    assert(note->matchesTag("work"));
    
    // Test multiple tags
    engine.addTagToNote(noteId, "important");
    engine.addTagToNote(noteId, "project");
    
    auto tags = engine.getAllTags();
    assert(tags.size() == 3);
    
    // Test removing tags
    bool tagRemoved = engine.removeTagFromNote(noteId, "work");
    assert(tagRemoved);
    
    note = engine.getNote(noteId);
    assert(!note->matchesTag("work"));
    assert(note->matchesTag("important"));
    
    std::cout << "Tag operations test passed!" << std::endl;
}

void testSearchOperations() {
    std::cout << "Testing search operations..." << std::endl;
    
    NoteEngine engine;
    
    // Create test notes
    engine.createNote("Meeting Notes", "Important project discussion");
    engine.createNote("Daily Journal", "Today was productive");
    engine.createNote("Project Plan", "Important milestones ahead");
    
    // Test search by content
    auto results = engine.searchNotes("important");
    assert(results.size() == 2);
    
    // Test search by title
    results = engine.searchNotesByTitle("project");
    std::cout << "Search by title 'project' returned " << results.size() << " results" << std::endl;
    for (const auto& result : results) {
        std::cout << "  - " << result.getTitle() << std::endl;
    }
    assert(results.size() == 1); // Only "Project Plan" has "project" in title
    
    // Test empty search returns all
    results = engine.searchNotes("");
    assert(results.size() == 3);
    
    std::cout << "Search operations test passed!" << std::endl;
}

void testTemplateOperations() {
    std::cout << "Testing template operations..." << std::endl;
    
    NoteEngine engine;
    
    // Test default templates exist
    auto templateNames = engine.getTemplateNames();
    assert(!templateNames.empty());
    assert(engine.hasTemplate("Meeting Notes"));
    
    // Test creating note from template
    uint32_t noteId = engine.createNoteFromTemplate("Meeting Notes", "Weekly Standup");
    assert(noteId > 0);
    
    auto note = engine.getNote(noteId);
    assert(note->getTitle() == "Weekly Standup");
    assert(!note->getContent().empty());
    
    // Test custom template
    engine.addTemplate("Custom Template", "Custom content", {"custom", "test"});
    assert(engine.hasTemplate("Custom Template"));
    
    uint32_t customNoteId = engine.createNoteFromTemplate("Custom Template", "Custom Note");
    auto customNote = engine.getNote(customNoteId);
    assert(customNote->getContent() == "Custom content");
    assert(customNote->matchesTag("custom"));
    assert(customNote->matchesTag("test"));
    
    std::cout << "Template operations test passed!" << std::endl;
}

void testBulkOperations() {
    std::cout << "Testing bulk operations..." << std::endl;
    
    NoteEngine engine;
    
    // Test creating multiple notes
    std::vector<std::pair<std::string, std::string>> notes = {
        {"Note 1", "Content 1"},
        {"Note 2", "Content 2"},
        {"Note 3", "Content 3"}
    };
    
    auto noteIds = engine.createMultipleNotes(notes);
    assert(noteIds.size() == 3);
    assert(engine.getTotalNoteCount() == 3);
    
    // Test bulk tag addition
    bool allTagged = engine.addTagToMultipleNotes(noteIds, "bulk-tag");
    assert(allTagged);
    
    for (uint32_t id : noteIds) {
        auto note = engine.getNote(id);
        assert(note->matchesTag("bulk-tag"));
    }
    
    // Test bulk deletion
    std::vector<uint32_t> idsToDelete = {noteIds[0], noteIds[2]};
    bool allDeleted = engine.deleteMultipleNotes(idsToDelete);
    assert(allDeleted);
    assert(engine.getTotalNoteCount() == 1);
    assert(engine.noteExists(noteIds[1]));
    
    std::cout << "Bulk operations test passed!" << std::endl;
}

int main() {
    try {
        testBasicNoteOperations();
        testTagOperations();
        testSearchOperations();
        testTemplateOperations();
        testBulkOperations();
        
        std::cout << "\nAll tests passed! NoteEngine implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}