#include <gtest/gtest.h>
#include "../src/core/models/character.h"
#include <cmath>

class CharacterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test character
        testCharacter = std::make_unique<Character>("TestPlayer", Character::Position(100, 200));
    }
    
    void TearDown() override {
        testCharacter.reset();
    }
    
    std::unique_ptr<Character> testCharacter;
    
    // Helper function to compare floats with epsilon
    bool floatEquals(float a, float b, float epsilon = 0.001f) {
        return std::abs(a - b) < epsilon;
    }
};

// Position Tests
TEST_F(CharacterTest, Position_DefaultConstructor_SetsZeroPosition) {
    Character::Position pos;
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
}

TEST_F(CharacterTest, Position_ParameterizedConstructor_SetsCorrectValues) {
    Character::Position pos(10.5f, 20.3f);
    EXPECT_FLOAT_EQ(pos.x, 10.5f);
    EXPECT_FLOAT_EQ(pos.y, 20.3f);
}

TEST_F(CharacterTest, Position_DistanceTo_CalculatesCorrectDistance) {
    Character::Position pos1(0, 0);
    Character::Position pos2(3, 4);
    
    float distance = pos1.distanceTo(pos2);
    EXPECT_FLOAT_EQ(distance, 5.0f); // 3-4-5 triangle
}

TEST_F(CharacterTest, Position_Addition_WorksCorrectly) {
    Character::Position pos1(10, 20);
    Character::Position pos2(5, 15);
    Character::Position result = pos1 + pos2;
    
    EXPECT_FLOAT_EQ(result.x, 15.0f);
    EXPECT_FLOAT_EQ(result.y, 35.0f);
}

TEST_F(CharacterTest, Position_Subtraction_WorksCorrectly) {
    Character::Position pos1(10, 20);
    Character::Position pos2(5, 15);
    Character::Position result = pos1 - pos2;
    
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 5.0f);
}

TEST_F(CharacterTest, Position_Equality_WorksCorrectly) {
    Character::Position pos1(10.0f, 20.0f);
    Character::Position pos2(10.0f, 20.0f);
    Character::Position pos3(10.1f, 20.0f);
    
    EXPECT_TRUE(pos1 == pos2);
    EXPECT_FALSE(pos1 == pos3);
    EXPECT_TRUE(pos1 != pos3);
}

// Character Creation Tests
TEST_F(CharacterTest, CreateCharacter_ValidInput_SetsPropertiesCorrectly) {
    EXPECT_EQ(testCharacter->getName(), "TestPlayer");
    EXPECT_FLOAT_EQ(testCharacter->getPosition().x, 100.0f);
    EXPECT_FLOAT_EQ(testCharacter->getPosition().y, 200.0f);
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::DOWN);
    EXPECT_EQ(testCharacter->getState(), Character::IDLE);
    EXPECT_EQ(testCharacter->getLevel(), 1);
    EXPECT_EQ(testCharacter->getExperience(), 0);
}

TEST_F(CharacterTest, SetName_ValidString_UpdatesName) {
    testCharacter->setName("NewName");
    EXPECT_EQ(testCharacter->getName(), "NewName");
}

TEST_F(CharacterTest, SetPosition_ValidPosition_UpdatesPosition) {
    Character::Position newPos(50, 75);
    testCharacter->setPosition(newPos);
    
    EXPECT_FLOAT_EQ(testCharacter->getPosition().x, 50.0f);
    EXPECT_FLOAT_EQ(testCharacter->getPosition().y, 75.0f);
}

// Movement Tests
TEST_F(CharacterTest, Move_UpDirection_MovesCorrectly) {
    Character::Position initialPos = testCharacter->getPosition();
    float deltaTime = 1.0f; // 1 second
    
    testCharacter->move(Character::UP, deltaTime);
    
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::UP);
    EXPECT_EQ(testCharacter->getState(), Character::WALKING);
    EXPECT_FLOAT_EQ(testCharacter->getPosition().x, initialPos.x);
    EXPECT_LT(testCharacter->getPosition().y, initialPos.y); // Y decreases when moving up
}

TEST_F(CharacterTest, Move_DownDirection_MovesCorrectly) {
    Character::Position initialPos = testCharacter->getPosition();
    float deltaTime = 1.0f;
    
    testCharacter->move(Character::DOWN, deltaTime);
    
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::DOWN);
    EXPECT_EQ(testCharacter->getState(), Character::WALKING);
    EXPECT_FLOAT_EQ(testCharacter->getPosition().x, initialPos.x);
    EXPECT_GT(testCharacter->getPosition().y, initialPos.y); // Y increases when moving down
}

TEST_F(CharacterTest, Move_LeftDirection_MovesCorrectly) {
    Character::Position initialPos = testCharacter->getPosition();
    float deltaTime = 1.0f;
    
    testCharacter->move(Character::LEFT, deltaTime);
    
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::LEFT);
    EXPECT_EQ(testCharacter->getState(), Character::WALKING);
    EXPECT_LT(testCharacter->getPosition().x, initialPos.x); // X decreases when moving left
    EXPECT_FLOAT_EQ(testCharacter->getPosition().y, initialPos.y);
}

TEST_F(CharacterTest, Move_RightDirection_MovesCorrectly) {
    Character::Position initialPos = testCharacter->getPosition();
    float deltaTime = 1.0f;
    
    testCharacter->move(Character::RIGHT, deltaTime);
    
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::RIGHT);
    EXPECT_EQ(testCharacter->getState(), Character::WALKING);
    EXPECT_GT(testCharacter->getPosition().x, initialPos.x); // X increases when moving right
    EXPECT_FLOAT_EQ(testCharacter->getPosition().y, initialPos.y);
}

TEST_F(CharacterTest, Move_CustomMovementSpeed_MovesCorrectDistance) {
    testCharacter->setMovementSpeed(200.0f); // 200 pixels per second
    Character::Position initialPos = testCharacter->getPosition();
    float deltaTime = 0.5f; // Half second
    
    testCharacter->move(Character::RIGHT, deltaTime);
    
    float expectedDistance = 200.0f * 0.5f; // 100 pixels
    EXPECT_FLOAT_EQ(testCharacter->getPosition().x, initialPos.x + expectedDistance);
}

// State Management Tests
TEST_F(CharacterTest, SetState_ValidState_UpdatesState) {
    testCharacter->setState(Character::INTERACTING);
    EXPECT_EQ(testCharacter->getState(), Character::INTERACTING);
    EXPECT_FALSE(testCharacter->isIdle());
    EXPECT_FALSE(testCharacter->isMoving());
}

TEST_F(CharacterTest, SetFacingDirection_ValidDirection_UpdatesDirection) {
    testCharacter->setFacingDirection(Character::LEFT);
    EXPECT_EQ(testCharacter->getFacingDirection(), Character::LEFT);
}

// Experience and Leveling Tests
TEST_F(CharacterTest, AddExperience_PositivePoints_IncreasesExperience) {
    testCharacter->addExperience(50);
    EXPECT_EQ(testCharacter->getExperience(), 50);
}

TEST_F(CharacterTest, AddExperience_ZeroOrNegativePoints_DoesNotChange) {
    int initialExp = testCharacter->getExperience();
    
    testCharacter->addExperience(0);
    EXPECT_EQ(testCharacter->getExperience(), initialExp);
    
    testCharacter->addExperience(-10);
    EXPECT_EQ(testCharacter->getExperience(), initialExp);
}

TEST_F(CharacterTest, LevelUp_SufficientExperience_IncreasesLevel) {
    // Add enough experience to level up (base is 100 XP for level 2)
    testCharacter->addExperience(150);
    
    EXPECT_GE(testCharacter->getLevel(), 2);
    EXPECT_LT(testCharacter->getExperienceToNextLevel(), 150);
}

TEST_F(CharacterTest, GetExperienceToNextLevel_Level1_ReturnsCorrectAmount) {
    int expToNext = testCharacter->getExperienceToNextLevel();
    EXPECT_GT(expToNext, 0);
    EXPECT_EQ(expToNext, testCharacter->getExperienceForLevel(2));
}

TEST_F(CharacterTest, CanLevelUp_InsufficientExperience_ReturnsFalse) {
    EXPECT_FALSE(testCharacter->canLevelUp());
}

TEST_F(CharacterTest, AddExperience_SufficientForLevelUp_AutomaticallyLevelsUp) {
    // Test that addExperience automatically levels up the character
    int initialLevel = testCharacter->getLevel();
    int expForLevel2 = testCharacter->getExperienceForLevel(2);
    
    testCharacter->addExperience(expForLevel2);
    
    EXPECT_GT(testCharacter->getLevel(), initialLevel);
    EXPECT_EQ(testCharacter->getLevel(), 2);
}

// String Conversion Tests
TEST_F(CharacterTest, StateStringConversion_AllStates_ConvertsCorrectly) {
    EXPECT_EQ(Character::stateToString(Character::IDLE), "IDLE");
    EXPECT_EQ(Character::stateToString(Character::WALKING), "WALKING");
    EXPECT_EQ(Character::stateToString(Character::INTERACTING), "INTERACTING");
    EXPECT_EQ(Character::stateToString(Character::FOCUSED), "FOCUSED");
    
    EXPECT_EQ(Character::stringToState("IDLE"), Character::IDLE);
    EXPECT_EQ(Character::stringToState("WALKING"), Character::WALKING);
    EXPECT_EQ(Character::stringToState("INTERACTING"), Character::INTERACTING);
    EXPECT_EQ(Character::stringToState("FOCUSED"), Character::FOCUSED);
    EXPECT_EQ(Character::stringToState("INVALID"), Character::IDLE); // Default
}

TEST_F(CharacterTest, DirectionStringConversion_AllDirections_ConvertsCorrectly) {
    EXPECT_EQ(Character::directionToString(Character::UP), "UP");
    EXPECT_EQ(Character::directionToString(Character::DOWN), "DOWN");
    EXPECT_EQ(Character::directionToString(Character::LEFT), "LEFT");
    EXPECT_EQ(Character::directionToString(Character::RIGHT), "RIGHT");
    
    EXPECT_EQ(Character::stringToDirection("UP"), Character::UP);
    EXPECT_EQ(Character::stringToDirection("DOWN"), Character::DOWN);
    EXPECT_EQ(Character::stringToDirection("LEFT"), Character::LEFT);
    EXPECT_EQ(Character::stringToDirection("RIGHT"), Character::RIGHT);
    EXPECT_EQ(Character::stringToDirection("INVALID"), Character::DOWN); // Default
}

// JSON Serialization Tests
TEST_F(CharacterTest, JsonSerialization_ValidCharacter_SerializesCorrectly) {
    testCharacter->setState(Character::WALKING);
    testCharacter->setFacingDirection(Character::RIGHT);
    testCharacter->addExperience(75);
    
    nlohmann::json json = testCharacter->toJson();
    
    EXPECT_EQ(json["name"], "TestPlayer");
    EXPECT_FLOAT_EQ(json["position"]["x"], 100.0f);
    EXPECT_FLOAT_EQ(json["position"]["y"], 200.0f);
    EXPECT_EQ(json["facingDirection"], "RIGHT");
    EXPECT_EQ(json["currentState"], "WALKING");
    EXPECT_EQ(json["level"], testCharacter->getLevel());
    EXPECT_EQ(json["experience"], 75);
    EXPECT_TRUE(json.contains("movementSpeed"));
}

TEST_F(CharacterTest, JsonDeserialization_ValidJson_CreatesCharacterCorrectly) {
    nlohmann::json json = {
        {"name", "JsonPlayer"},
        {"position", {{"x", 50.5f}, {"y", 75.3f}}},
        {"facingDirection", "LEFT"},
        {"currentState", "INTERACTING"},
        {"level", 3},
        {"experience", 250},
        {"movementSpeed", 150.0f}
    };
    
    Character character = Character::fromJson(json);
    
    EXPECT_EQ(character.getName(), "JsonPlayer");
    EXPECT_FLOAT_EQ(character.getPosition().x, 50.5f);
    EXPECT_FLOAT_EQ(character.getPosition().y, 75.3f);
    EXPECT_EQ(character.getFacingDirection(), Character::LEFT);
    EXPECT_EQ(character.getState(), Character::INTERACTING);
    EXPECT_EQ(character.getLevel(), 3);
    EXPECT_EQ(character.getExperience(), 250);
    EXPECT_FLOAT_EQ(character.getMovementSpeed(), 150.0f);
}

TEST_F(CharacterTest, JsonRoundTrip_ComplexCharacter_PreservesAllData) {
    // Set up complex character state
    testCharacter->setName("RoundTripPlayer");
    testCharacter->setPosition(Character::Position(123.45f, 678.90f));
    testCharacter->setState(Character::FOCUSED);
    testCharacter->setFacingDirection(Character::UP);
    testCharacter->addExperience(500);
    testCharacter->setMovementSpeed(250.0f);
    
    // Serialize to JSON
    nlohmann::json json = testCharacter->toJson();
    
    // Deserialize from JSON
    Character deserializedCharacter = Character::fromJson(json);
    
    // Verify all properties are preserved
    EXPECT_EQ(deserializedCharacter.getName(), testCharacter->getName());
    EXPECT_FLOAT_EQ(deserializedCharacter.getPosition().x, testCharacter->getPosition().x);
    EXPECT_FLOAT_EQ(deserializedCharacter.getPosition().y, testCharacter->getPosition().y);
    EXPECT_EQ(deserializedCharacter.getFacingDirection(), testCharacter->getFacingDirection());
    EXPECT_EQ(deserializedCharacter.getState(), testCharacter->getState());
    EXPECT_EQ(deserializedCharacter.getLevel(), testCharacter->getLevel());
    EXPECT_EQ(deserializedCharacter.getExperience(), testCharacter->getExperience());
    EXPECT_FLOAT_EQ(deserializedCharacter.getMovementSpeed(), testCharacter->getMovementSpeed());
}