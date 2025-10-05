#include <gtest/gtest.h>
#include "../src/core/engines/note_engine.h"
#include <chrono>
#include <thread>

class NoteEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        noteEngine = std::make_unique<NoteEngine>();
    }
    
    void TearDown() override {
        noteEngine.reset();
    }
    
    std::unique_ptr<NoteEngine> noteEngine;
    
    // Helper method to wait for a short duration
    void waitMs(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
};

// Constructor and Initial State Tests
TEST_F(NoteEngineTest, Constructor_DefaultState_HasNoNotes) {
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 0);
    EXPECT_TRUE(noteEngine->getAllNotes().empty());
    EXPECT_TRUE(noteEngine->getAllTags().empty());
}

TEST_F(NoteEngineTest, Constructor_DefaultTemplates_AreLoaded) {
    auto templateNames = noteEngine->getTemplateNames();
    EXPECT_FALSE(templateNames.empty());
    EXPECT_TRUE(noteEngine->hasTemplate("Meeting Notes"));
    EXPECT_TRUE(noteEngine->hasTemplate("Daily Journal"));
    EXPECT_TRUE(noteEngine->hasTemplate("Project Notes"));
}

// CRUD Operations Tests
TEST_F(NoteEngineTest, CreateNote_ValidInput_ReturnsNoteId) {
    uint32_t noteId = noteEngine->createNote("Test Note", "Test content");
    
    EXPECT_GT(noteId, 0);
    EXPECT_TRUE(noteEngine->noteExists(noteId));
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 1);
}

TEST_F(NoteEngineTest, CreateNote_EmptyTitleAndContent_CreatesNote) {
    uint32_t noteId = noteEngine->createNote();
    
    EXPECT_GT(noteId, 0);
    EXPECT_TRUE(noteEngine->noteExists(noteId));
    
    auto note = noteEngine->getNote(noteId);
    EXPECT_TRUE(note != nullptr);
    EXPECT_TRUE(note->getTitle().empty());
    EXPECT_TRUE(note->getContent().empty());
}

TEST_F(NoteEngineTest, CreateMultipleNotes_ValidInput_ReturnsUniqueIds) {
    uint32_t id1 = noteEngine->createNote("Note 1");
    uint32_t id2 = noteEngine->createNote("Note 2");
    uint32_t id3 = noteEngine->createNote("Note 3");
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 3);
}

TEST_F(NoteEngineTest, GetNote_ValidId_ReturnsCorrectNote) {
    uint32_t noteId = noteEngine->createNote("Test Title", "Test Content");
    
    auto note = noteEngine->getNote(noteId);
    EXPECT_TRUE(note != nullptr);
    EXPECT_EQ(note->getTitle(), "Test Title");
    EXPECT_EQ(note->getContent(), "Test Content");
}

TEST_F(NoteEngineTest, GetNote_InvalidId_ReturnsNull) {
    auto note = noteEngine->getNote(999);
    EXPECT_TRUE(note == nullptr);
}

TEST_F(NoteEngineTest, UpdateNote_ValidNote_UpdatesSuccessfully) {
    uint32_t noteId = noteEngine->createNote("Original Title", "Original Content");
    
    auto note = noteEngine->getNote(noteId);
    note->setTitle("Updated Title");
    note->setContent("Updated Content");
    
    bool result = noteEngine->updateNote(*note);
    EXPECT_TRUE(result);
    
    auto updatedNote = noteEngine->getNote(noteId);
    EXPECT_EQ(updatedNote->getTitle(), "Updated Title");
    EXPECT_EQ(updatedNote->getContent(), "Updated Content");
}

TEST_F(NoteEngineTest, UpdateNote_NonexistentNote_ReturnsFalse) {
    Note nonexistentNote(999, "Title");
    bool result = noteEngine->updateNote(nonexistentNote);
    EXPECT_FALSE(result);
}

TEST_F(NoteEngineTest, DeleteNote_ValidId_RemovesNote) {
    uint32_t noteId = noteEngine->createNote("Test Note");
    EXPECT_TRUE(noteEngine->noteExists(noteId));
    
    bool result = noteEngine->deleteNote(noteId);
    EXPECT_TRUE(result);
    EXPECT_FALSE(noteEngine->noteExists(noteId));
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 0);
}

TEST_F(NoteEngineTest, DeleteNote_InvalidId_ReturnsFalse) {
    bool result = noteEngine->deleteNote(999);
    EXPECT_FALSE(result);
}

// Search Functionality Tests
TEST_F(NoteEngineTest, SearchNotes_EmptyQuery_ReturnsAllNotes) {
    noteEngine->createNote("Note 1", "Content 1");
    noteEngine->createNote("Note 2", "Content 2");
    
    auto results = noteEngine->searchNotes("");
    EXPECT_EQ(results.size(), 2);
}

TEST_F(NoteEngineTest, SearchNotes_TitleMatch_ReturnsMatchingNotes) {
    noteEngine->createNote("Important Meeting", "Content 1");
    noteEngine->createNote("Daily Standup", "Content 2");
    noteEngine->createNote("Meeting Notes", "Content 3");
    
    auto results = noteEngine->searchNotes("meeting");
    EXPECT_EQ(results.size(), 2);
}

TEST_F(NoteEngineTest, SearchNotes_ContentMatch_ReturnsMatchingNotes) {
    noteEngine->createNote("Note 1", "This contains important information");
    noteEngine->createNote("Note 2", "This is just regular content");
    noteEngine->createNote("Note 3", "Another important note");
    
    auto results = noteEngine->searchNotes("important");
    EXPECT_EQ(results.size(), 2);
}

// Tag Management Tests
TEST_F(NoteEngineTest, AddTagToNote_ValidTag_AddsSuccessfully) {
    uint32_t noteId = noteEngine->createNote("Test Note");
    
    bool result = noteEngine->addTagToNote(noteId, "work");
    EXPECT_TRUE(result);
    
    auto note = noteEngine->getNote(noteId);
    EXPECT_TRUE(note->matchesTag("work"));
}

TEST_F(NoteEngineTest, AddTagToNote_InvalidNoteId_ReturnsFalse) {
    bool result = noteEngine->addTagToNote(999, "work");
    EXPECT_FALSE(result);
}

TEST_F(NoteEngineTest, RemoveTagFromNote_ExistingTag_RemovesSuccessfully) {
    uint32_t noteId = noteEngine->createNote("Test Note");
    noteEngine->addTagToNote(noteId, "work");
    noteEngine->addTagToNote(noteId, "important");
    
    bool result = noteEngine->removeTagFromNote(noteId, "work");
    EXPECT_TRUE(result);
    
    auto note = noteEngine->getNote(noteId);
    EXPECT_FALSE(note->matchesTag("work"));
    EXPECT_TRUE(note->matchesTag("important"));
}

TEST_F(NoteEngineTest, GetAllTags_WithTaggedNotes_ReturnsUniqueTags) {
    uint32_t note1 = noteEngine->createNote("Note 1");
    uint32_t note2 = noteEngine->createNote("Note 2");
    
    noteEngine->addTagToNote(note1, "work");
    noteEngine->addTagToNote(note1, "important");
    noteEngine->addTagToNote(note2, "work");
    noteEngine->addTagToNote(note2, "personal");
    
    auto tags = noteEngine->getAllTags();
    EXPECT_EQ(tags.size(), 3);
    
    std::set<std::string> tagSet(tags.begin(), tags.end());
    EXPECT_TRUE(tagSet.count("work"));
    EXPECT_TRUE(tagSet.count("important"));
    EXPECT_TRUE(tagSet.count("personal"));
}

// Template Tests
TEST_F(NoteEngineTest, CreateNoteFromTemplate_ValidTemplate_CreatesNoteWithContent) {
    uint32_t noteId = noteEngine->createNoteFromTemplate("Meeting Notes", "Weekly Standup");
    
    EXPECT_GT(noteId, 0);
    
    auto note = noteEngine->getNote(noteId);
    EXPECT_EQ(note->getTitle(), "Weekly Standup");
    EXPECT_FALSE(note->getContent().empty());
    EXPECT_TRUE(note->getContent().find("Meeting Notes") != std::string::npos);
}

TEST_F(NoteEngineTest, CreateNoteFromTemplate_InvalidTemplate_ReturnsZero) {
    uint32_t noteId = noteEngine->createNoteFromTemplate("Nonexistent Template", "Title");
    EXPECT_EQ(noteId, 0);
}

// Export Functionality Tests
TEST_F(NoteEngineTest, ExportNote_MarkdownFormat_ReturnsFormattedString) {
    uint32_t noteId = noteEngine->createNote("Test Note", "This is test content");
    noteEngine->addTagToNote(noteId, "test");
    
    std::string exported = noteEngine->exportNote(noteId, NoteEngine::ExportFormat::MARKDOWN);
    
    EXPECT_FALSE(exported.empty());
    EXPECT_TRUE(exported.find("# Test Note") != std::string::npos);
    EXPECT_TRUE(exported.find("This is test content") != std::string::npos);
    EXPECT_TRUE(exported.find("`test`") != std::string::npos);
}

// Statistics Tests
TEST_F(NoteEngineTest, GetTotalNoteCount_WithNotes_ReturnsCorrectCount) {
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 0);
    
    noteEngine->createNote("Note 1");
    noteEngine->createNote("Note 2");
    noteEngine->createNote("Note 3");
    
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 3);
}

// Bulk Operations Tests
TEST_F(NoteEngineTest, CreateMultipleNotes_ValidInput_CreatesAllNotes) {
    std::vector<std::pair<std::string, std::string>> notes = {
        {"Note 1", "Content 1"},
        {"Note 2", "Content 2"},
        {"Note 3", "Content 3"}
    };
    
    auto noteIds = noteEngine->createMultipleNotes(notes);
    
    EXPECT_EQ(noteIds.size(), 3);
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 3);
    
    for (uint32_t id : noteIds) {
        EXPECT_TRUE(noteEngine->noteExists(id));
    }
}

// Data Management Tests
TEST_F(NoteEngineTest, ClearAllNotes_WithNotes_RemovesAllNotes) {
    noteEngine->createNote("Note 1");
    noteEngine->createNote("Note 2");
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 2);
    
    noteEngine->clearAllNotes();
    EXPECT_EQ(noteEngine->getTotalNoteCount(), 0);
    EXPECT_TRUE(noteEngine->getAllNotes().empty());
}

// Callback Tests
TEST_F(NoteEngineTest, Callbacks_NoteCreated_TriggersCallback) {
    uint32_t callbackNoteId = 0;
    bool callbackTriggered = false;
    
    noteEngine->setOnNoteCreated([&](uint32_t noteId) {
        callbackNoteId = noteId;
        callbackTriggered = true;
    });
    
    uint32_t createdId = noteEngine->createNote("Test Note");
    
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(callbackNoteId, createdId);
}