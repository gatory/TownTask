#include <gtest/gtest.h>
#include "../src/core/engines/note_system.h"
#include <chrono>
#include <thread>

class NoteSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        noteSystem = std::make_unique<NoteSystem>();
    }
    
    void TearDown() override {
        noteSystem.reset();
    }
    
    std::unique_ptr<NoteSystem> noteSystem;
};

// Constructor and Initial State Tests
TEST_F(NoteSystemTest, Constructor_DefaultState_HasNoNotes) {
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 0);
    EXPECT_TRUE(noteSystem->getAllNotes().empty());
    EXPECT_TRUE(noteSystem->getAllTags().empty());
}

// Note Creation Tests
TEST_F(NoteSystemTest, CreateNote_EmptyNote_ReturnsValidId) {
    uint32_t noteId = noteSystem->createNote();
    
    EXPECT_GT(noteId, 0);
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 1);
    EXPECT_TRUE(noteSystem->noteExists(noteId));
}

TEST_F(NoteSystemTest, CreateNote_WithTitleAndContent_SetsCorrectValues) {
    uint32_t noteId = noteSystem->createNote("Test Title", "Test Content");
    
    auto note = noteSystem->getNote(noteId);
    ASSERT_TRUE(note.has_value());
    EXPECT_EQ(note->getTitle(), "Test Title");
    EXPECT_EQ(note->getContent(), "Test Content");
}

TEST_F(NoteSystemTest, CreateMultipleNotes_ReturnsUniqueIds) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    uint32_t id3 = noteSystem->createNote("Note 3");
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 3);
}

// Note Modification Tests
TEST_F(NoteSystemTest, SetNoteTitle_ValidNote_UpdatesTitle) {
    uint32_t noteId = noteSystem->createNote("Original Title");
    
    bool result = noteSystem->setNoteTitle(noteId, "New Title");
    
    EXPECT_TRUE(result);
    auto note = noteSystem->getNote(noteId);
    ASSERT_TRUE(note.has_value());
    EXPECT_EQ(note->getTitle(), "New Title");
}

TEST_F(NoteSystemTest, SetNoteTitle_InvalidNote_ReturnsFalse) {
    bool result = noteSystem->setNoteTitle(99999, "New Title");
    EXPECT_FALSE(result);
}

TEST_F(NoteSystemTest, SetNoteContent_ValidNote_UpdatesContent) {
    uint32_t noteId = noteSystem->createNote("Title", "Original Content");
    
    bool result = noteSystem->setNoteContent(noteId, "New Content");
    
    EXPECT_TRUE(result);
    auto note = noteSystem->getNote(noteId);
    ASSERT_TRUE(note.has_value());
    EXPECT_EQ(note->getContent(), "New Content");
}

// Tag Management Tests
TEST_F(NoteSystemTest, AddNoteTag_ValidNote_AddsTag) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    
    bool result = noteSystem->addNoteTag(noteId, "important");
    
    EXPECT_TRUE(result);
    auto tags = noteSystem->getTagsForNote(noteId);
    EXPECT_EQ(tags.size(), 1);
    EXPECT_EQ(tags[0], "important");
}

TEST_F(NoteSystemTest, AddNoteTag_DuplicateTag_ReturnsFalse) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    
    noteSystem->addNoteTag(noteId, "tag1");
    bool result = noteSystem->addNoteTag(noteId, "tag1");
    
    EXPECT_FALSE(result);
    auto tags = noteSystem->getTagsForNote(noteId);
    EXPECT_EQ(tags.size(), 1);
}

TEST_F(NoteSystemTest, RemoveNoteTag_ExistingTag_RemovesTag) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    noteSystem->addNoteTag(noteId, "tag1");
    noteSystem->addNoteTag(noteId, "tag2");
    
    bool result = noteSystem->removeNoteTag(noteId, "tag1");
    
    EXPECT_TRUE(result);
    auto tags = noteSystem->getTagsForNote(noteId);
    EXPECT_EQ(tags.size(), 1);
    EXPECT_EQ(tags[0], "tag2");
}

TEST_F(NoteSystemTest, ClearNoteTags_WithTags_RemovesAllTags) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    noteSystem->addNoteTag(noteId, "tag1");
    noteSystem->addNoteTag(noteId, "tag2");
    noteSystem->addNoteTag(noteId, "tag3");
    
    bool result = noteSystem->clearNoteTags(noteId);
    
    EXPECT_TRUE(result);
    auto tags = noteSystem->getTagsForNote(noteId);
    EXPECT_TRUE(tags.empty());
}

// Note Deletion Tests
TEST_F(NoteSystemTest, DeleteNote_ValidNote_RemovesNote) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    
    bool result = noteSystem->deleteNote(noteId);
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(noteSystem->noteExists(noteId));
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 0);
}

TEST_F(NoteSystemTest, DeleteNote_InvalidNote_ReturnsFalse) {
    bool result = noteSystem->deleteNote(99999);
    EXPECT_FALSE(result);
}

// Querying Tests
TEST_F(NoteSystemTest, GetAllNotes_WithMultipleNotes_ReturnsAllNotes) {
    noteSystem->createNote("Note 1");
    noteSystem->createNote("Note 2");
    noteSystem->createNote("Note 3");
    
    auto notes = noteSystem->getAllNotes();
    
    EXPECT_EQ(notes.size(), 3);
}

TEST_F(NoteSystemTest, GetNotesByTag_WithTaggedNotes_ReturnsMatchingNotes) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    uint32_t id3 = noteSystem->createNote("Note 3");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id2, "work");
    noteSystem->addNoteTag(id3, "personal");
    
    auto workNotes = noteSystem->getNotesByTag("work");
    
    EXPECT_EQ(workNotes.size(), 2);
}

TEST_F(NoteSystemTest, GetNotesWithAnyTag_WithMultipleTags_ReturnsMatchingNotes) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    uint32_t id3 = noteSystem->createNote("Note 3");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id2, "personal");
    noteSystem->addNoteTag(id3, "hobby");
    
    auto notes = noteSystem->getNotesWithAnyTag({"work", "personal"});
    
    EXPECT_EQ(notes.size(), 2);
}

TEST_F(NoteSystemTest, GetNotesWithAllTags_WithMultipleTags_ReturnsMatchingNotes) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id1, "urgent");
    noteSystem->addNoteTag(id2, "work");
    
    auto notes = noteSystem->getNotesWithAllTags({"work", "urgent"});
    
    EXPECT_EQ(notes.size(), 1);
}

// Search Tests
TEST_F(NoteSystemTest, SearchNotes_WithMatchingContent_ReturnsResults) {
    noteSystem->createNote("Meeting Notes", "Discuss project timeline");
    noteSystem->createNote("Shopping List", "Buy groceries for dinner");
    noteSystem->createNote("Ideas", "Project ideas for next quarter");
    
    auto results = noteSystem->searchNotes("project");
    
    EXPECT_EQ(results.size(), 2);
}

TEST_F(NoteSystemTest, SearchNotes_EmptyQuery_ReturnsAllNotes) {
    noteSystem->createNote("Note 1");
    noteSystem->createNote("Note 2");
    
    auto results = noteSystem->searchNotes("");
    
    EXPECT_EQ(results.size(), 2);
}

TEST_F(NoteSystemTest, SearchNotesByTitle_WithMatchingTitle_ReturnsResults) {
    noteSystem->createNote("Meeting Notes", "Content 1");
    noteSystem->createNote("Project Meeting", "Content 2");
    noteSystem->createNote("Shopping List", "Content 3");
    
    auto results = noteSystem->searchNotesByTitle("meeting");
    
    EXPECT_EQ(results.size(), 2);
}

TEST_F(NoteSystemTest, SearchNotesByContent_WithMatchingContent_ReturnsResults) {
    noteSystem->createNote("Title 1", "Important meeting notes");
    noteSystem->createNote("Title 2", "Shopping list items");
    noteSystem->createNote("Title 3", "Meeting agenda items");
    
    auto results = noteSystem->searchNotesByContent("meeting");
    
    EXPECT_EQ(results.size(), 2);
}

// Advanced Search Tests
TEST_F(NoteSystemTest, AdvancedSearch_WithCriteria_ReturnsMatchingNotes) {
    uint32_t id1 = noteSystem->createNote("Work Meeting", "Discuss project timeline");
    uint32_t id2 = noteSystem->createNote("Personal Notes", "Meeting with friends");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id2, "personal");
    
    NoteSystem::SearchCriteria criteria;
    criteria.titleQuery = "meeting";
    criteria.requiredTags = {"work"};
    criteria.caseSensitive = false;
    
    auto results = noteSystem->advancedSearch(criteria);
    
    EXPECT_EQ(results.size(), 1);
}

// Tag Statistics Tests
TEST_F(NoteSystemTest, GetAllTags_WithTaggedNotes_ReturnsAllUniqueTags) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id1, "urgent");
    noteSystem->addNoteTag(id2, "work");
    noteSystem->addNoteTag(id2, "personal");
    
    auto tags = noteSystem->getAllTags();
    
    EXPECT_EQ(tags.size(), 3);
    EXPECT_TRUE(tags.find("work") != tags.end());
    EXPECT_TRUE(tags.find("urgent") != tags.end());
    EXPECT_TRUE(tags.find("personal") != tags.end());
}

TEST_F(NoteSystemTest, GetTagUsageCount_WithMultipleNotes_ReturnsCorrectCount) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    uint32_t id3 = noteSystem->createNote("Note 3");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id2, "work");
    noteSystem->addNoteTag(id3, "personal");
    
    EXPECT_EQ(noteSystem->getTagUsageCount("work"), 2);
    EXPECT_EQ(noteSystem->getTagUsageCount("personal"), 1);
    EXPECT_EQ(noteSystem->getTagUsageCount("nonexistent"), 0);
}

TEST_F(NoteSystemTest, GetTagUsageStatistics_WithTags_ReturnsSortedStatistics) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    uint32_t id3 = noteSystem->createNote("Note 3");
    
    noteSystem->addNoteTag(id1, "work");
    noteSystem->addNoteTag(id2, "work");
    noteSystem->addNoteTag(id3, "work");
    noteSystem->addNoteTag(id1, "urgent");
    noteSystem->addNoteTag(id2, "personal");
    
    auto stats = noteSystem->getTagUsageStatistics();
    
    EXPECT_EQ(stats.size(), 3);
    EXPECT_EQ(stats[0].first, "work"); // Most used tag first
    EXPECT_EQ(stats[0].second, 3);
}

// Sorting Tests
TEST_F(NoteSystemTest, GetSortedNotes_ByTitleAsc_ReturnsSortedNotes) {
    noteSystem->createNote("Zebra");
    noteSystem->createNote("Apple");
    noteSystem->createNote("Banana");
    
    auto sorted = noteSystem->getSortedNotes(NoteSystem::SortBy::TITLE_ASC);
    
    EXPECT_EQ(sorted[0].getTitle(), "Apple");
    EXPECT_EQ(sorted[1].getTitle(), "Banana");
    EXPECT_EQ(sorted[2].getTitle(), "Zebra");
}

TEST_F(NoteSystemTest, GetSortedNotes_ByTitleDesc_ReturnsSortedNotes) {
    noteSystem->createNote("Apple");
    noteSystem->createNote("Zebra");
    noteSystem->createNote("Banana");
    
    auto sorted = noteSystem->getSortedNotes(NoteSystem::SortBy::TITLE_DESC);
    
    EXPECT_EQ(sorted[0].getTitle(), "Zebra");
    EXPECT_EQ(sorted[1].getTitle(), "Banana");
    EXPECT_EQ(sorted[2].getTitle(), "Apple");
}

// Filtering Tests
TEST_F(NoteSystemTest, GetEmptyNotes_WithMixedNotes_ReturnsOnlyEmpty) {
    noteSystem->createNote("Empty Note", "");
    noteSystem->createNote("Non-empty Note", "Some content");
    noteSystem->createNote("", "");
    
    auto emptyNotes = noteSystem->getEmptyNotes();
    
    EXPECT_EQ(emptyNotes.size(), 2);
}

TEST_F(NoteSystemTest, GetNonEmptyNotes_WithMixedNotes_ReturnsOnlyNonEmpty) {
    noteSystem->createNote("Empty Note", "");
    noteSystem->createNote("Non-empty Note", "Some content");
    noteSystem->createNote("Another Note", "More content");
    
    auto nonEmptyNotes = noteSystem->getNonEmptyNotes();
    
    EXPECT_EQ(nonEmptyNotes.size(), 2);
}

// Bulk Operations Tests
TEST_F(NoteSystemTest, CreateMultipleNotes_WithTitleContentPairs_CreatesAllNotes) {
    std::vector<std::pair<std::string, std::string>> data = {
        {"Note 1", "Content 1"},
        {"Note 2", "Content 2"},
        {"Note 3", "Content 3"}
    };
    
    auto ids = noteSystem->createMultipleNotes(data);
    
    EXPECT_EQ(ids.size(), 3);
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 3);
}

TEST_F(NoteSystemTest, DeleteMultipleNotes_WithValidIds_DeletesAllNotes) {
    auto id1 = noteSystem->createNote("Note 1");
    auto id2 = noteSystem->createNote("Note 2");
    auto id3 = noteSystem->createNote("Note 3");
    
    bool result = noteSystem->deleteMultipleNotes({id1, id2, id3});
    
    EXPECT_TRUE(result);
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 0);
}

TEST_F(NoteSystemTest, AddTagToMultipleNotes_WithValidIds_AddsTagToAll) {
    auto id1 = noteSystem->createNote("Note 1");
    auto id2 = noteSystem->createNote("Note 2");
    
    bool result = noteSystem->addTagToMultipleNotes({id1, id2}, "batch-tag");
    
    EXPECT_TRUE(result);
    EXPECT_EQ(noteSystem->getTagUsageCount("batch-tag"), 2);
}

// Statistics Tests
TEST_F(NoteSystemTest, GetTotalTagCount_WithMultipleTags_ReturnsCorrectCount) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    
    noteSystem->addNoteTag(id1, "tag1");
    noteSystem->addNoteTag(id1, "tag2");
    noteSystem->addNoteTag(id2, "tag3");
    
    EXPECT_EQ(noteSystem->getTotalTagCount(), 3);
}

TEST_F(NoteSystemTest, GetAverageTagsPerNote_WithTaggedNotes_ReturnsCorrectAverage) {
    uint32_t id1 = noteSystem->createNote("Note 1");
    uint32_t id2 = noteSystem->createNote("Note 2");
    
    noteSystem->addNoteTag(id1, "tag1");
    noteSystem->addNoteTag(id1, "tag2");
    noteSystem->addNoteTag(id2, "tag3");
    
    EXPECT_EQ(noteSystem->getAverageTagsPerNote(), 1); // (2 + 1) / 2 = 1.5, truncated to 1
}

// Data Management Tests
TEST_F(NoteSystemTest, LoadNotes_WithNoteList_LoadsAllNotes) {
    Note note1("Title 1", "Content 1");
    Note note2("Title 2", "Content 2");
    std::vector<Note> notes = {note1, note2};
    
    noteSystem->loadNotes(notes);
    
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 2);
}

TEST_F(NoteSystemTest, ExportNotes_WithNotes_ReturnsAllNotes) {
    noteSystem->createNote("Note 1");
    noteSystem->createNote("Note 2");
    
    auto exported = noteSystem->exportNotes();
    
    EXPECT_EQ(exported.size(), 2);
}

TEST_F(NoteSystemTest, ClearAllNotes_WithNotes_RemovesAllNotes) {
    noteSystem->createNote("Note 1");
    noteSystem->createNote("Note 2");
    
    noteSystem->clearAllNotes();
    
    EXPECT_EQ(noteSystem->getTotalNoteCount(), 0);
}

// Callback Tests
TEST_F(NoteSystemTest, NoteCreationCallback_WhenNoteCreated_CallsCallback) {
    bool callbackCalled = false;
    std::string createdTitle;
    
    noteSystem->setNoteCreationCallback([&](const Note& note) {
        callbackCalled = true;
        createdTitle = note.getTitle();
    });
    
    noteSystem->createNote("Test Note");
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(createdTitle, "Test Note");
}

TEST_F(NoteSystemTest, NoteDeletionCallback_WhenNoteDeleted_CallsCallback) {
    bool callbackCalled = false;
    uint32_t deletedId = 0;
    
    noteSystem->setNoteDeletionCallback([&](uint32_t noteId) {
        callbackCalled = true;
        deletedId = noteId;
    });
    
    uint32_t noteId = noteSystem->createNote("Test Note");
    noteSystem->deleteNote(noteId);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(deletedId, noteId);
}

TEST_F(NoteSystemTest, TagAddedCallback_WhenTagAdded_CallsCallback) {
    bool callbackCalled = false;
    uint32_t taggedNoteId = 0;
    std::string addedTag;
    
    noteSystem->setTagAddedCallback([&](uint32_t noteId, const std::string& tag) {
        callbackCalled = true;
        taggedNoteId = noteId;
        addedTag = tag;
    });
    
    uint32_t noteId = noteSystem->createNote("Test Note");
    noteSystem->addNoteTag(noteId, "test-tag");
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(taggedNoteId, noteId);
    EXPECT_EQ(addedTag, "test-tag");
}

// Validation Tests
TEST_F(NoteSystemTest, NoteExists_WithValidId_ReturnsTrue) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    
    EXPECT_TRUE(noteSystem->noteExists(noteId));
    EXPECT_TRUE(noteSystem->isValidNoteId(noteId));
}

TEST_F(NoteSystemTest, NoteExists_WithInvalidId_ReturnsFalse) {
    EXPECT_FALSE(noteSystem->noteExists(99999));
    EXPECT_FALSE(noteSystem->isValidNoteId(99999));
}

TEST_F(NoteSystemTest, TagExists_WithExistingTag_ReturnsTrue) {
    uint32_t noteId = noteSystem->createNote("Test Note");
    noteSystem->addNoteTag(noteId, "existing-tag");
    
    EXPECT_TRUE(noteSystem->tagExists("existing-tag"));
}

TEST_F(NoteSystemTest, TagExists_WithNonExistingTag_ReturnsFalse) {
    EXPECT_FALSE(noteSystem->tagExists("non-existing-tag"));
}