#include <gtest/gtest.h>
#include "../src/ui/screens/library_screen.h"
#include "../src/core/engines/note_engine.h"
#include "../src/core/engines/gamification_engine.h"
#include "../src/input/input_manager.h"
#include "../src/ui/animations/animation_manager.h"

class LibraryScreenTest : public ::testing::Test {
protected:
    void SetUp() override {
        inputManager.initialize();
        libraryScreen = std::make_unique<LibraryScreen>(
            noteEngine, gamificationEngine, inputManager, animationManager
        );
    }
    
    void TearDown() override {
        inputManager.shutdown();
    }
    
    NoteEngine noteEngine;
    GamificationEngine gamificationEngine;
    InputManager inputManager;
    AnimationManager animationManager;
    std::unique_ptr<LibraryScreen> libraryScreen;
};

TEST_F(LibraryScreenTest, InitializationTest) {
    EXPECT_FALSE(libraryScreen->isActive());
    EXPECT_EQ(libraryScreen->getName(), "LibraryScreen");
    EXPECT_TRUE(libraryScreen->isShowingNoteEditor());
    EXPECT_TRUE(libraryScreen->isShowingNoteList());
    EXPECT_EQ(libraryScreen->getSelectedNoteId(), 0);
    EXPECT_EQ(libraryScreen->getViewMode(), "split");
}

TEST_F(LibraryScreenTest, ActivationTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    EXPECT_TRUE(libraryScreen->isActive());
    
    libraryScreen->onExit();
    libraryScreen->setActive(false);
    
    EXPECT_FALSE(libraryScreen->isActive());
}

TEST_F(LibraryScreenTest, NoteCreationTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note
    std::string noteTitle = "Test Note";
    std::string noteContent = "This is a test note content.";
    std::vector<std::string> tags = {"test", "example"};
    
    libraryScreen->createNote(noteTitle, noteContent, tags);
    
    // Verify note was created in the engine
    std::vector<Note> notes = noteEngine.getAllNotes();
    EXPECT_EQ(notes.size(), 1);
    EXPECT_EQ(notes[0].getTitle(), noteTitle);
    EXPECT_EQ(notes[0].getContent(), noteContent);
    EXPECT_EQ(notes[0].getTags(), tags);
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, NoteDeletionTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note first
    libraryScreen->createNote("Test Note", "Content", {});
    
    std::vector<Note> notes = noteEngine.getAllNotes();
    ASSERT_EQ(notes.size(), 1);
    uint32_t noteId = notes[0].getId();
    
    // Delete the note
    libraryScreen->deleteNote(noteId);
    
    // Verify note was deleted
    notes = noteEngine.getAllNotes();
    EXPECT_EQ(notes.size(), 0);
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, NoteDuplicationTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note first
    libraryScreen->createNote("Original Note", "Original content", {"original"});
    
    std::vector<Note> notes = noteEngine.getAllNotes();
    ASSERT_EQ(notes.size(), 1);
    uint32_t originalId = notes[0].getId();
    
    // Duplicate the note
    libraryScreen->duplicateNote(originalId);
    
    // Verify note was duplicated
    notes = noteEngine.getAllNotes();
    EXPECT_EQ(notes.size(), 2);
    
    // Find the duplicate (should have "Copy" in title)
    bool foundDuplicate = false;
    for (const Note& note : notes) {
        if (note.getTitle().find("Copy") != std::string::npos) {
            foundDuplicate = true;
            EXPECT_EQ(note.getContent(), "Original content");
            break;
        }
    }
    EXPECT_TRUE(foundDuplicate);
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, SearchFilterTest) {
    // Test search functionality
    EXPECT_EQ(libraryScreen->getSearchQuery(), "");
    
    libraryScreen->setSearchQuery("test query");
    EXPECT_EQ(libraryScreen->getSearchQuery(), "test query");
    
    libraryScreen->setSearchQuery("");
    EXPECT_EQ(libraryScreen->getSearchQuery(), "");
}

TEST_F(LibraryScreenTest, TagFilterTest) {
    // Test tag filtering
    EXPECT_EQ(libraryScreen->getTagFilter(), "");
    
    libraryScreen->setTagFilter("important");
    EXPECT_EQ(libraryScreen->getTagFilter(), "important");
    
    libraryScreen->setTagFilter("");
    EXPECT_EQ(libraryScreen->getTagFilter(), "");
}

TEST_F(LibraryScreenTest, CategoryFilterTest) {
    // Test category filtering
    EXPECT_EQ(libraryScreen->getCategoryFilter(), "");
    
    libraryScreen->setCategoryFilter("work");
    EXPECT_EQ(libraryScreen->getCategoryFilter(), "work");
    
    libraryScreen->setCategoryFilter("");
    EXPECT_EQ(libraryScreen->getCategoryFilter(), "");
}

TEST_F(LibraryScreenTest, SortingTest) {
    // Test sorting options
    EXPECT_EQ(libraryScreen->getSortBy(), "modified"); // Default
    
    libraryScreen->setSortBy("title");
    EXPECT_EQ(libraryScreen->getSortBy(), "title");
    
    libraryScreen->setSortBy("created");
    EXPECT_EQ(libraryScreen->getSortBy(), "created");
    
    libraryScreen->setSortBy("category");
    EXPECT_EQ(libraryScreen->getSortBy(), "category");
}

TEST_F(LibraryScreenTest, ViewModeTest) {
    // Test different view modes
    EXPECT_EQ(libraryScreen->getViewMode(), "split"); // Default
    
    libraryScreen->setViewMode("list");
    EXPECT_EQ(libraryScreen->getViewMode(), "list");
    EXPECT_TRUE(libraryScreen->isShowingNoteList());
    EXPECT_FALSE(libraryScreen->isShowingNoteEditor());
    
    libraryScreen->setViewMode("editor");
    EXPECT_EQ(libraryScreen->getViewMode(), "editor");
    EXPECT_FALSE(libraryScreen->isShowingNoteList());
    EXPECT_TRUE(libraryScreen->isShowingNoteEditor());
    
    libraryScreen->setViewMode("split");
    EXPECT_EQ(libraryScreen->getViewMode(), "split");
    EXPECT_TRUE(libraryScreen->isShowingNoteList());
    EXPECT_TRUE(libraryScreen->isShowingNoteEditor());
}

TEST_F(LibraryScreenTest, NoteSelectionTest) {
    // Test note selection
    EXPECT_EQ(libraryScreen->getSelectedNoteId(), 0); // No selection
    
    libraryScreen->setSelectedNoteId(123);
    EXPECT_EQ(libraryScreen->getSelectedNoteId(), 123);
    
    libraryScreen->setSelectedNoteId(0);
    EXPECT_EQ(libraryScreen->getSelectedNoteId(), 0);
}

TEST_F(LibraryScreenTest, VisualSettingsTest) {
    // Test tags visibility
    EXPECT_TRUE(libraryScreen->isShowingTags()); // Default
    libraryScreen->setShowTags(false);
    EXPECT_FALSE(libraryScreen->isShowingTags());
    
    // Test categories visibility
    EXPECT_TRUE(libraryScreen->isShowingCategories()); // Default
    libraryScreen->setShowCategories(false);
    EXPECT_FALSE(libraryScreen->isShowingCategories());
    
    // Test timestamps visibility
    EXPECT_TRUE(libraryScreen->isShowingTimestamps()); // Default
    libraryScreen->setShowTimestamps(false);
    EXPECT_FALSE(libraryScreen->isShowingTimestamps());
    
    // Test font size
    EXPECT_EQ(libraryScreen->getFontSize(), 12); // Default
    libraryScreen->setFontSize(16);
    EXPECT_EQ(libraryScreen->getFontSize(), 16);
    
    // Test font size clamping
    libraryScreen->setFontSize(5); // Too small
    EXPECT_EQ(libraryScreen->getFontSize(), 8); // Should clamp to minimum
    
    libraryScreen->setFontSize(30); // Too large
    EXPECT_EQ(libraryScreen->getFontSize(), 24); // Should clamp to maximum
}

TEST_F(LibraryScreenTest, TagManagementTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note first
    libraryScreen->createNote("Test Note", "Content", {"initial"});
    
    std::vector<Note> notes = noteEngine.getAllNotes();
    ASSERT_EQ(notes.size(), 1);
    uint32_t noteId = notes[0].getId();
    
    // Add a tag
    libraryScreen->addTagToNote(noteId, "new-tag");
    
    // Verify tag was added
    Note updatedNote = noteEngine.getNote(noteId);
    std::vector<std::string> tags = updatedNote.getTags();
    EXPECT_TRUE(std::find(tags.begin(), tags.end(), "new-tag") != tags.end());
    
    // Remove a tag
    libraryScreen->removeTagFromNote(noteId, "initial");
    
    // Verify tag was removed
    updatedNote = noteEngine.getNote(noteId);
    tags = updatedNote.getTags();
    EXPECT_TRUE(std::find(tags.begin(), tags.end(), "initial") == tags.end());
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, CategoryManagementTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note first
    libraryScreen->createNote("Test Note", "Content", {});
    
    std::vector<Note> notes = noteEngine.getAllNotes();
    ASSERT_EQ(notes.size(), 1);
    uint32_t noteId = notes[0].getId();
    
    // Set category
    libraryScreen->setNoteCategory(noteId, "work");
    
    // Verify category was set
    Note updatedNote = noteEngine.getNote(noteId);
    EXPECT_EQ(updatedNote.getCategory(), "work");
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, CallbackTest) {
    bool noteCreatedCalled = false;
    bool noteModifiedCalled = false;
    bool noteDeletedCalled = false;
    bool exitRequestedCalled = false;
    
    uint32_t createdNoteId = 0;
    uint32_t modifiedNoteId = 0;
    uint32_t deletedNoteId = 0;
    
    // Set up callbacks
    libraryScreen->setOnNoteCreated([&](uint32_t noteId) {
        noteCreatedCalled = true;
        createdNoteId = noteId;
    });
    
    libraryScreen->setOnNoteModified([&](uint32_t noteId) {
        noteModifiedCalled = true;
        modifiedNoteId = noteId;
    });
    
    libraryScreen->setOnNoteDeleted([&](uint32_t noteId) {
        noteDeletedCalled = true;
        deletedNoteId = noteId;
    });
    
    libraryScreen->setOnExitRequested([&]() {
        exitRequestedCalled = true;
    });
    
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Test note creation callback
    libraryScreen->createNote("Test Note", "Content", {});
    EXPECT_TRUE(noteCreatedCalled);
    EXPECT_NE(createdNoteId, 0);
    
    // Test note deletion callback
    libraryScreen->deleteNote(createdNoteId);
    EXPECT_TRUE(noteDeletedCalled);
    EXPECT_EQ(deletedNoteId, createdNoteId);
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, UpdateTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Test that update doesn't crash
    float deltaTime = 0.016f; // ~60 FPS
    
    for (int i = 0; i < 10; i++) {
        libraryScreen->update(deltaTime);
    }
    
    // If we get here without crashing, the test passes
    EXPECT_TRUE(true);
    
    libraryScreen->onExit();
}

TEST_F(LibraryScreenTest, NoteEditTest) {
    libraryScreen->setActive(true);
    libraryScreen->onEnter();
    
    // Create a note first
    libraryScreen->createNote("Original Title", "Original content", {"original"});
    
    std::vector<Note> notes = noteEngine.getAllNotes();
    ASSERT_EQ(notes.size(), 1);
    uint32_t noteId = notes[0].getId();
    
    // Edit the note (this should load it into the editor)
    libraryScreen->editNote(noteId);
    
    // Verify note is selected and editor is shown
    EXPECT_EQ(libraryScreen->getSelectedNoteId(), noteId);
    EXPECT_TRUE(libraryScreen->isShowingNoteEditor());
    
    libraryScreen->onExit();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}