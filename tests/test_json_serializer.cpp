#include <gtest/gtest.h>
#include "../src/persistence/json_serializer.h"
#include <chrono>

class JsonSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data
        testCharacter = std::make_unique<Character>("TestPlayer", Character::Position(100, 200));
        testCharacter->addExperience(150);
        testCharacter->setState(Character::WALKING);
        testCharacter->setFacingDirection(Character::RIGHT);
        
        testTask = std::make_unique<Task>("Test Task", Task::HIGH);
        testTask->setStatus(Task::IN_PROGRESS);
        
        testNote = std::make_unique<Note>("Test Note", "Test content");
        testNote->addTag("test");
        testNote->addTag("serialization");
        
        testHabit = std::make_unique<Habit>("Test Habit", Habit::DAILY);
        testHabit->checkIn();
    }
    
    void TearDown() override {
        testCharacter.reset();
        testTask.reset();
        testNote.reset();
        testHabit.reset();
    }
    
    std::unique_ptr<Character> testCharacter;
    std::unique_ptr<Task> testTask;
    std::unique_ptr<Note> testNote;
    std::unique_ptr<Habit> testHabit;
};

// Individual model serialization tests
TEST_F(JsonSerializerTest, SerializeTask_ValidTask_ProducesValidJson) {
    nlohmann::json json = JsonSerializer::serializeTask(*testTask);
    
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("title"));
    EXPECT_TRUE(json.contains("priority"));
    EXPECT_TRUE(json.contains("status"));
    EXPECT_EQ(json["title"], "Test Task");
    EXPECT_EQ(json["priority"], "HIGH");
    EXPECT_EQ(json["status"], "IN_PROGRESS");
}

TEST_F(JsonSerializerTest, DeserializeTask_ValidJson_CreatesCorrectTask) {
    nlohmann::json json = JsonSerializer::serializeTask(*testTask);
    Task deserializedTask = JsonSerializer::deserializeTask(json);
    
    EXPECT_EQ(deserializedTask.getId(), testTask->getId());
    EXPECT_EQ(deserializedTask.getTitle(), testTask->getTitle());
    EXPECT_EQ(deserializedTask.getPriority(), testTask->getPriority());
    EXPECT_EQ(deserializedTask.getStatus(), testTask->getStatus());
}

TEST_F(JsonSerializerTest, SerializeTasks_MultipleTask_ProducesArray) {
    std::vector<Task> tasks;
    tasks.push_back(*testTask);
    tasks.emplace_back("Second Task", Task::LOW);
    
    nlohmann::json json = JsonSerializer::serializeTasks(tasks);
    
    EXPECT_TRUE(json.is_array());
    EXPECT_EQ(json.size(), 2);
    EXPECT_EQ(json[0]["title"], "Test Task");
    EXPECT_EQ(json[1]["title"], "Second Task");
}

TEST_F(JsonSerializerTest, DeserializeTasks_ValidArray_CreatesCorrectTasks) {
    std::vector<Task> originalTasks;
    originalTasks.push_back(*testTask);
    originalTasks.emplace_back("Second Task", Task::LOW);
    
    nlohmann::json json = JsonSerializer::serializeTasks(originalTasks);
    std::vector<Task> deserializedTasks = JsonSerializer::deserializeTasks(json);
    
    EXPECT_EQ(deserializedTasks.size(), 2);
    EXPECT_EQ(deserializedTasks[0].getTitle(), "Test Task");
    EXPECT_EQ(deserializedTasks[1].getTitle(), "Second Task");
}

TEST_F(JsonSerializerTest, SerializeCharacter_ValidCharacter_ProducesValidJson) {
    nlohmann::json json = JsonSerializer::serializeCharacter(*testCharacter);
    
    EXPECT_TRUE(json.contains("name"));
    EXPECT_TRUE(json.contains("position"));
    EXPECT_TRUE(json.contains("level"));
    EXPECT_TRUE(json.contains("experience"));
    EXPECT_EQ(json["name"], "TestPlayer");
    EXPECT_GT(json["level"], 1); // Should have leveled up from experience
}

TEST_F(JsonSerializerTest, DeserializeCharacter_ValidJson_CreatesCorrectCharacter) {
    nlohmann::json json = JsonSerializer::serializeCharacter(*testCharacter);
    Character deserializedCharacter = JsonSerializer::deserializeCharacter(json);
    
    EXPECT_EQ(deserializedCharacter.getName(), testCharacter->getName());
    EXPECT_EQ(deserializedCharacter.getPosition().x, testCharacter->getPosition().x);
    EXPECT_EQ(deserializedCharacter.getPosition().y, testCharacter->getPosition().y);
    EXPECT_EQ(deserializedCharacter.getLevel(), testCharacter->getLevel());
    EXPECT_EQ(deserializedCharacter.getExperience(), testCharacter->getExperience());
}

TEST_F(JsonSerializerTest, SerializeNote_ValidNote_ProducesValidJson) {
    nlohmann::json json = JsonSerializer::serializeNote(*testNote);
    
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("title"));
    EXPECT_TRUE(json.contains("content"));
    EXPECT_TRUE(json.contains("tags"));
    EXPECT_EQ(json["title"], "Test Note");
    EXPECT_EQ(json["content"], "Test content");
    EXPECT_TRUE(json["tags"].is_array());
    EXPECT_EQ(json["tags"].size(), 2);
}

TEST_F(JsonSerializerTest, DeserializeNote_ValidJson_CreatesCorrectNote) {
    nlohmann::json json = JsonSerializer::serializeNote(*testNote);
    Note deserializedNote = JsonSerializer::deserializeNote(json);
    
    EXPECT_EQ(deserializedNote.getId(), testNote->getId());
    EXPECT_EQ(deserializedNote.getTitle(), testNote->getTitle());
    EXPECT_EQ(deserializedNote.getContent(), testNote->getContent());
    EXPECT_EQ(deserializedNote.getTagCount(), testNote->getTagCount());
    EXPECT_TRUE(deserializedNote.hasTag("test"));
    EXPECT_TRUE(deserializedNote.hasTag("serialization"));
}

TEST_F(JsonSerializerTest, SerializeHabit_ValidHabit_ProducesValidJson) {
    nlohmann::json json = JsonSerializer::serializeHabit(*testHabit);
    
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("name"));
    EXPECT_TRUE(json.contains("frequency"));
    EXPECT_TRUE(json.contains("currentStreak"));
    EXPECT_TRUE(json.contains("completionHistory"));
    EXPECT_EQ(json["name"], "Test Habit");
    EXPECT_EQ(json["frequency"], "DAILY");
    EXPECT_EQ(json["currentStreak"], 1);
}

TEST_F(JsonSerializerTest, DeserializeHabit_ValidJson_CreatesCorrectHabit) {
    nlohmann::json json = JsonSerializer::serializeHabit(*testHabit);
    Habit deserializedHabit = JsonSerializer::deserializeHabit(json);
    
    EXPECT_EQ(deserializedHabit.getId(), testHabit->getId());
    EXPECT_EQ(deserializedHabit.getName(), testHabit->getName());
    EXPECT_EQ(deserializedHabit.getFrequency(), testHabit->getFrequency());
    EXPECT_EQ(deserializedHabit.getCurrentStreak(), testHabit->getCurrentStreak());
    EXPECT_EQ(deserializedHabit.getTotalCompletions(), testHabit->getTotalCompletions());
}

// Game state serialization tests
TEST_F(JsonSerializerTest, SerializeGameState_CompleteState_ProducesValidJson) {
    GameState gameState(*testCharacter);
    gameState.tasks.push_back(*testTask);
    gameState.notes.push_back(*testNote);
    gameState.habits.push_back(*testHabit);
    
    // Set up town state
    gameState.townState.buildings["coffeeShop"] = TownState::BuildingState{2, {"plant_01"}};
    gameState.townState.globalDecorations.push_back("tree_01");
    gameState.townState.unlockedItems.push_back("coffee_varieties_basic");
    
    // Set up gamification state
    gameState.gamificationState.totalExperience = 150;
    gameState.gamificationState.level = 2;
    gameState.gamificationState.achievements.push_back("first_task");
    gameState.gamificationState.currency["coffee"] = 5;
    
    nlohmann::json json = JsonSerializer::serializeGameState(gameState);
    
    EXPECT_TRUE(json.contains("character"));
    EXPECT_TRUE(json.contains("tasks"));
    EXPECT_TRUE(json.contains("notes"));
    EXPECT_TRUE(json.contains("habits"));
    EXPECT_TRUE(json.contains("townState"));
    EXPECT_TRUE(json.contains("gamificationState"));
    EXPECT_TRUE(json.contains("version"));
    EXPECT_TRUE(json.contains("savedAt"));
    
    // Verify arrays
    EXPECT_TRUE(json["tasks"].is_array());
    EXPECT_TRUE(json["notes"].is_array());
    EXPECT_TRUE(json["habits"].is_array());
    EXPECT_EQ(json["tasks"].size(), 1);
    EXPECT_EQ(json["notes"].size(), 1);
    EXPECT_EQ(json["habits"].size(), 1);
    
    // Verify town state
    EXPECT_TRUE(json["townState"]["buildings"].contains("coffeeShop"));
    EXPECT_EQ(json["townState"]["buildings"]["coffeeShop"]["level"], 2);
    
    // Verify gamification state
    EXPECT_EQ(json["gamificationState"]["totalExperience"], 150);
    EXPECT_EQ(json["gamificationState"]["level"], 2);
    EXPECT_EQ(json["gamificationState"]["currency"]["coffee"], 5);
}

TEST_F(JsonSerializerTest, DeserializeGameState_ValidJson_CreatesCorrectState) {
    GameState originalState(*testCharacter);
    originalState.tasks.push_back(*testTask);
    originalState.notes.push_back(*testNote);
    originalState.habits.push_back(*testHabit);
    originalState.townState.buildings["coffeeShop"] = TownState::BuildingState{2, {"plant_01"}};
    originalState.gamificationState.totalExperience = 150;
    originalState.gamificationState.currency["coffee"] = 5;
    
    nlohmann::json json = JsonSerializer::serializeGameState(originalState);
    GameState deserializedState = JsonSerializer::deserializeGameState(json);
    
    // Verify character
    EXPECT_EQ(deserializedState.character.getName(), originalState.character.getName());
    EXPECT_EQ(deserializedState.character.getLevel(), originalState.character.getLevel());
    
    // Verify collections
    EXPECT_EQ(deserializedState.tasks.size(), 1);
    EXPECT_EQ(deserializedState.notes.size(), 1);
    EXPECT_EQ(deserializedState.habits.size(), 1);
    
    // Verify task
    EXPECT_EQ(deserializedState.tasks[0].getTitle(), "Test Task");
    EXPECT_EQ(deserializedState.tasks[0].getPriority(), Task::HIGH);
    
    // Verify note
    EXPECT_EQ(deserializedState.notes[0].getTitle(), "Test Note");
    EXPECT_TRUE(deserializedState.notes[0].hasTag("test"));
    
    // Verify habit
    EXPECT_EQ(deserializedState.habits[0].getName(), "Test Habit");
    EXPECT_EQ(deserializedState.habits[0].getCurrentStreak(), 1);
    
    // Verify town state
    EXPECT_TRUE(deserializedState.townState.buildings.find("coffeeShop") != deserializedState.townState.buildings.end());
    EXPECT_EQ(deserializedState.townState.buildings.at("coffeeShop").level, 2);
    
    // Verify gamification state
    EXPECT_EQ(deserializedState.gamificationState.totalExperience, 150);
    EXPECT_EQ(deserializedState.gamificationState.currency.at("coffee"), 5);
}

// Utility method tests
TEST_F(JsonSerializerTest, IsValidJson_ValidJsonString_ReturnsTrue) {
    std::string validJson = R"({"key": "value", "number": 42})";
    EXPECT_TRUE(JsonSerializer::isValidJson(validJson));
}

TEST_F(JsonSerializerTest, IsValidJson_InvalidJsonString_ReturnsFalse) {
    std::string invalidJson = R"({"key": "value", "number": })";
    EXPECT_FALSE(JsonSerializer::isValidJson(invalidJson));
}

TEST_F(JsonSerializerTest, FormatJson_ValidJson_ProducesFormattedString) {
    nlohmann::json json = {{"key", "value"}, {"number", 42}};
    
    std::string formatted = JsonSerializer::formatJson(json, true);
    std::string compact = JsonSerializer::formatJson(json, false);
    
    EXPECT_GT(formatted.length(), compact.length()); // Pretty format should be longer
    EXPECT_TRUE(formatted.find("\n") != std::string::npos); // Should contain newlines
    EXPECT_TRUE(compact.find("\n") == std::string::npos); // Compact should not
}

// Edge case tests
TEST_F(JsonSerializerTest, DeserializeTasks_EmptyArray_ReturnsEmptyVector) {
    nlohmann::json emptyArray = nlohmann::json::array();
    std::vector<Task> tasks = JsonSerializer::deserializeTasks(emptyArray);
    
    EXPECT_TRUE(tasks.empty());
}

TEST_F(JsonSerializerTest, DeserializeTasks_InvalidJson_ReturnsEmptyVector) {
    nlohmann::json invalidJson = "not an array";
    std::vector<Task> tasks = JsonSerializer::deserializeTasks(invalidJson);
    
    EXPECT_TRUE(tasks.empty());
}

TEST_F(JsonSerializerTest, DeserializeNotes_EmptyArray_ReturnsEmptyVector) {
    nlohmann::json emptyArray = nlohmann::json::array();
    std::vector<Note> notes = JsonSerializer::deserializeNotes(emptyArray);
    
    EXPECT_TRUE(notes.empty());
}

TEST_F(JsonSerializerTest, DeserializeHabits_EmptyArray_ReturnsEmptyVector) {
    nlohmann::json emptyArray = nlohmann::json::array();
    std::vector<Habit> habits = JsonSerializer::deserializeHabits(emptyArray);
    
    EXPECT_TRUE(habits.empty());
}