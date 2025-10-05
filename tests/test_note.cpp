#include <gtest/gtest.h>
#include "../src/core/models/note.h"
#include <chrono>
#include <thread>

class NoteTest : public ::testing::Test {
protected:
    void SetUp() override {
        testNote = std::make_unique<Note>("Test Note", "This is test content");
    }
    
    void TearDown() override {
        testNote.reset();
    }
    
    std::unique_ptr<Note> testNote;
};

// Note Creation Tests
TEST_F(NoteTest, CreateNote_ValidInput_SetsPropertiesCorrectly) {
    EXPECT_GT(testNote->getId(), 0);
    EXPECT_EQ(testNote->getTitle(), "Test Note");
    EXPECT_EQ(testNote->getContent(), "This is test content");
    EXPECT_EQ(testNote->getTagCount(), 0);
    EXPECT_FALSE(testNote->isEmpty());
    EXPECT_EQ(testNote->getContentLength(), 20); // "This is test content"
}

TEST_F(NoteTest, CreateNoteWithId_ValidInput_UsesProvidedId) {
    uint32_t testId = 12345;
    Note note(testId, "Custom Note", "Custom content");
    
    EXPECT_EQ(note.getId(), testId);
    EXPECT_EQ(note.getTitle(), "Custom Note");
    EXPECT_EQ(note.getContent(), "Custom content");
}

TEST_F(NoteTest, CreateEmptyNote_NoContent_IsEmpty) {
    Note emptyNote("Empty Note");
    
    EXPECT_TRUE(emptyNote.isEmpty());
    EXPECT_EQ(emptyNote.getContentLength(), 0);
    EXPECT_EQ(emptyNote.getContent(), "");
}

// Title and Content Tests
TEST_F(NoteTest, SetTitle_ValidString_UpdatesTitle) {
    auto originalModified = testNote->getModifiedAt();
    
    // Small delay to ensure modified time changes
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    testNote->setTitle("New Title");
    
    EXPECT_EQ(testNote->getTitle(), "New Title");
    EXPECT_GT(testNote->getModifiedAt(), originalModified);
}

TEST_F(NoteTest, SetTitle_SameTitle_DoesNotUpdateModifiedTime) {
    auto originalModified = testNote->getModifiedAt();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    testNote->setTitle("Test Note"); // Same title
    
    EXPECT_EQ(testNote->getModifiedAt(), originalModified);
}

TEST_F(NoteTest, SetContent_ValidString_UpdatesContent) {
    auto originalModified = testNote->getModifiedAt();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    testNote->setContent("New content here");
    
    EXPECT_EQ(testNote->getContent(), "New content here");
    EXPECT_GT(testNote->getModifiedAt(), originalModified);
    EXPECT_FALSE(testNote->isEmpty());
    EXPECT_EQ(testNote->getContentLength(), 16);
}

TEST_F(NoteTest, SetContent_EmptyString_MakesNoteEmpty) {
    testNote->setContent("");
    
    EXPECT_TRUE(testNote->isEmpty());
    EXPECT_EQ(testNote->getContentLength(), 0);
}

// Tag Management Tests
TEST_F(NoteTest, AddTag_ValidTag_AddsToCollection) {
    testNote->addTag("programming");
    
    EXPECT_EQ(testNote->getTagCount(), 1);
    EXPECT_TRUE(testNote->hasTag("programming"));
    EXPECT_TRUE(testNote->hasTag("Programming")); // Case insensitive
}

TEST_F(NoteTest, AddTag_DuplicateTag_DoesNotAddDuplicate) {
    testNote->addTag("programming");
    testNote->addTag("Programming"); // Different case
    testNote->addTag("programming"); // Exact duplicate
    
    EXPECT_EQ(testNote->getTagCount(), 1);
}

TEST_F(NoteTest, AddTag_EmptyTag_DoesNotAdd) {
    testNote->addTag("");
    
    EXPECT_EQ(testNote->getTagCount(), 0);
}

TEST_F(NoteTest, RemoveTag_ExistingTag_RemovesFromCollection) {
    testNote->addTag("programming");
    testNote->addTag("cpp");
    EXPECT_EQ(testNote->getTagCount(), 2);
    
    testNote->removeTag("programming");
    
    EXPECT_EQ(testNote->getTagCount(), 1);
    EXPECT_FALSE(testNote->hasTag("programming"));
    EXPECT_TRUE(testNote->hasTag("cpp"));
}

TEST_F(NoteTest, RemoveTag_CaseInsensitive_RemovesCorrectly) {
    testNote->addTag("Programming");
    
    testNote->removeTag("programming"); // Different case
    
    EXPECT_EQ(testNote->getTagCount(), 0);
    EXPECT_FALSE(testNote->hasTag("Programming"));
}

TEST_F(NoteTest, ClearTags_WithTags_RemovesAllTags) {
    testNote->addTag("tag1");
    testNote->addTag("tag2");
    testNote->addTag("tag3");
    EXPECT_EQ(testNote->getTagCount(), 3);
    
    testNote->clearTags();
    
    EXPECT_EQ(testNote->getTagCount(), 0);
    EXPECT_FALSE(testNote->hasTag("tag1"));
}

// Search Functionality Tests
TEST_F(NoteTest, ContainsText_ExistingText_ReturnsTrue) {
    EXPECT_TRUE(testNote->containsText("test"));
    EXPECT_TRUE(testNote->containsText("Test")); // Case insensitive by default
    EXPECT_TRUE(testNote->containsText("content"));
    EXPECT_TRUE(testNote->containsText("Test Note")); // In title
}

TEST_F(NoteTest, ContainsText_NonExistingText_ReturnsFalse) {
    EXPECT_FALSE(testNote->containsText("nonexistent"));
    EXPECT_FALSE(testNote->containsText("xyz"));
}

TEST_F(NoteTest, ContainsText_CaseSensitive_RespectsCase) {
    EXPECT_TRUE(testNote->containsText("Test", true)); // Case sensitive - "Test" in title
    EXPECT_TRUE(testNote->containsText("test", true)); // Case sensitive - "test" in content
    EXPECT_FALSE(testNote->containsText("TEST", true)); // Case sensitive - not found
}

TEST_F(NoteTest, ContainsText_EmptySearch_ReturnsTrue) {
    EXPECT_TRUE(testNote->containsText(""));
}

TEST_F(NoteTest, MatchesTag_ExistingTag_ReturnsTrue) {
    testNote->addTag("programming");
    
    EXPECT_TRUE(testNote->matchesTag("programming"));
    EXPECT_TRUE(testNote->matchesTag("Programming")); // Case insensitive by default
}

TEST_F(NoteTest, MatchesTag_NonExistingTag_ReturnsFalse) {
    testNote->addTag("programming");
    
    EXPECT_FALSE(testNote->matchesTag("design"));
    EXPECT_FALSE(testNote->matchesTag(""));
}

TEST_F(NoteTest, MatchesTag_CaseSensitive_RespectsCase) {
    testNote->addTag("Programming");
    
    EXPECT_TRUE(testNote->matchesTag("Programming", true));
    EXPECT_FALSE(testNote->matchesTag("programming", true));
}

// Preview Tests
TEST_F(NoteTest, GetPreview_ShortContent_ReturnsFullContent) {
    std::string preview = testNote->getPreview(100);
    EXPECT_EQ(preview, "This is test content");
}

TEST_F(NoteTest, GetPreview_LongContent_ReturnsTruncatedWithEllipsis) {
    testNote->setContent("This is a very long piece of content that should be truncated when we request a preview that is shorter than the full content length.");
    
    std::string preview = testNote->getPreview(20);
    
    EXPECT_LT(preview.length(), testNote->getContentLength());
    EXPECT_TRUE(preview.find("...") != std::string::npos);
}

TEST_F(NoteTest, GetPreview_BreaksAtWordBoundary_WhenPossible) {
    testNote->setContent("This is a test of word boundary breaking functionality");
    
    std::string preview = testNote->getPreview(15);
    
    // Should break at a space, not in the middle of a word
    EXPECT_TRUE(preview.find("...") != std::string::npos);
    EXPECT_FALSE(preview.back() == ' '); // Should not end with space before ellipsis
}

// JSON Serialization Tests
TEST_F(NoteTest, JsonSerialization_ValidNote_SerializesCorrectly) {
    testNote->addTag("programming");
    testNote->addTag("cpp");
    
    nlohmann::json json = testNote->toJson();
    
    EXPECT_EQ(json["id"], testNote->getId());
    EXPECT_EQ(json["title"], "Test Note");
    EXPECT_EQ(json["content"], "This is test content");
    EXPECT_TRUE(json["tags"].is_array());
    EXPECT_EQ(json["tags"].size(), 2);
    EXPECT_TRUE(json.contains("createdAt"));
    EXPECT_TRUE(json.contains("modifiedAt"));
}

TEST_F(NoteTest, JsonDeserialization_ValidJson_CreatesNoteCorrectly) {
    nlohmann::json json = {
        {"id", 123},
        {"title", "JSON Note"},
        {"content", "Content from JSON"},
        {"tags", {"tag1", "tag2"}},
        {"createdAt", "2025-01-01T00:00:00Z"},
        {"modifiedAt", "2025-01-01T12:00:00Z"}
    };
    
    Note note = Note::fromJson(json);
    
    EXPECT_EQ(note.getId(), 123);
    EXPECT_EQ(note.getTitle(), "JSON Note");
    EXPECT_EQ(note.getContent(), "Content from JSON");
    EXPECT_EQ(note.getTagCount(), 2);
    EXPECT_TRUE(note.hasTag("tag1"));
    EXPECT_TRUE(note.hasTag("tag2"));
}

TEST_F(NoteTest, JsonRoundTrip_ComplexNote_PreservesAllData) {
    // Set up complex note
    testNote->setTitle("Complex Note");
    testNote->setContent("This is complex content with multiple lines\nand special characters!");
    testNote->addTag("complex");
    testNote->addTag("testing");
    testNote->addTag("json");
    
    // Serialize to JSON
    nlohmann::json json = testNote->toJson();
    
    // Deserialize from JSON
    Note deserializedNote = Note::fromJson(json);
    
    // Verify all properties are preserved
    EXPECT_EQ(deserializedNote.getId(), testNote->getId());
    EXPECT_EQ(deserializedNote.getTitle(), testNote->getTitle());
    EXPECT_EQ(deserializedNote.getContent(), testNote->getContent());
    EXPECT_EQ(deserializedNote.getTagCount(), testNote->getTagCount());
    
    auto originalTags = testNote->getTags();
    auto deserializedTags = deserializedNote.getTags();
    EXPECT_EQ(originalTags.size(), deserializedTags.size());
    
    for (const auto& tag : originalTags) {
        EXPECT_TRUE(deserializedNote.hasTag(tag));
    }
}