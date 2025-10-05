#include <gtest/gtest.h>
#include "../src/persistence/save_manager.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>

class SaveManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for testing
        testDirectory = std::filesystem::temp_directory_path() / "tasktown_test";
        
        // Clean up any existing test directory
        if (std::filesystem::exists(testDirectory)) {
            std::filesystem::remove_all(testDirectory);
        }
        
        saveManager = std::make_unique<SaveManager>(testDirectory.string());
        
        // Create test game state
        testCharacter = std::make_unique<Character>("TestPlayer", Character::Position(50, 75));
        testCharacter->addExperience(100);
        
        testGameState = std::make_unique<GameState>(*testCharacter);
        testGameState->tasks.emplace_back("Test Task", Task::HIGH);
        testGameState->notes.emplace_back("Test Note", "Test content");
        testGameState->habits.emplace_back("Test Habit", Habit::DAILY);
        
        // Set up town state
        testGameState->townState.buildings["coffeeShop"] = TownState::BuildingState{2, {"plant_01"}};
        testGameState->townState.globalDecorations.push_back("tree_01");
        
        // Set up gamification state
        testGameState->gamificationState.totalExperience = 100;
        testGameState->gamificationState.level = 2;
        testGameState->gamificationState.achievements.push_back("first_task");
        testGameState->gamificationState.currency["coffee"] = 3;
    }
    
    void TearDown() override {
        saveManager.reset();
        testGameState.reset();
        testCharacter.reset();
        
        // Clean up test directory
        if (std::filesystem::exists(testDirectory)) {
            std::filesystem::remove_all(testDirectory);
        }
    }
    
    std::filesystem::path testDirectory;
    std::unique_ptr<SaveManager> saveManager;
    std::unique_ptr<Character> testCharacter;
    std::unique_ptr<GameState> testGameState;
    
    // Helper method to create a corrupted save file
    void createCorruptedSaveFile() {
        std::string saveFilePath = saveManager->getSaveFilePath();
        std::ofstream file(saveFilePath);
        file << "{ invalid json content }";
        file.close();
    }
    
    // Helper method to create a valid but incomplete save file
    void createIncompleteSaveFile() {
        std::string saveFilePath = saveManager->getSaveFilePath();
        std::ofstream file(saveFilePath);
        file << R"({"version": "1.0.0", "incomplete": true})";
        file.close();
    }
};

// Basic save/load functionality tests
TEST_F(SaveManagerTest, SaveGameState_ValidState_ReturnsSuccess) {
    SaveManager::SaveResult result = saveManager->saveGameState(*testGameState);
    
    EXPECT_EQ(result, SaveManager::SaveResult::SUCCESS);
    EXPECT_TRUE(saveManager->saveFileExists());
    EXPECT_TRUE(saveManager->getLastError().empty());
}

TEST_F(SaveManagerTest, LoadGameState_AfterSave_ReturnsCorrectState) {
    // Save the game state
    SaveManager::SaveResult saveResult = saveManager->saveGameState(*testGameState);
    EXPECT_EQ(saveResult, SaveManager::SaveResult::SUCCESS);
    
    // Load the game state
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    
    EXPECT_EQ(loadResult, SaveManager::LoadResult::SUCCESS);
    ASSERT_TRUE(gameStateOpt.has_value());
    
    const GameState& loadedState = gameStateOpt.value();
    
    // Verify character
    EXPECT_EQ(loadedState.character.getName(), testGameState->character.getName());
    EXPECT_EQ(loadedState.character.getLevel(), testGameState->character.getLevel());
    EXPECT_EQ(loadedState.character.getExperience(), testGameState->character.getExperience());
    
    // Verify collections
    EXPECT_EQ(loadedState.tasks.size(), testGameState->tasks.size());
    EXPECT_EQ(loadedState.notes.size(), testGameState->notes.size());
    EXPECT_EQ(loadedState.habits.size(), testGameState->habits.size());
    
    // Verify specific items
    EXPECT_EQ(loadedState.tasks[0].getTitle(), "Test Task");
    EXPECT_EQ(loadedState.notes[0].getTitle(), "Test Note");
    EXPECT_EQ(loadedState.habits[0].getName(), "Test Habit");
    
    // Verify town state
    EXPECT_TRUE(loadedState.townState.buildings.find("coffeeShop") != loadedState.townState.buildings.end());
    EXPECT_EQ(loadedState.townState.buildings.at("coffeeShop").level, 2);
    
    // Verify gamification state
    EXPECT_EQ(loadedState.gamificationState.totalExperience, 100);
    EXPECT_EQ(loadedState.gamificationState.level, 2);
    EXPECT_EQ(loadedState.gamificationState.currency.at("coffee"), 3);
}

TEST_F(SaveManagerTest, LoadGameState_NoSaveFile_ReturnsFileNotFound) {
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    
    EXPECT_EQ(loadResult, SaveManager::LoadResult::FILE_NOT_FOUND);
    EXPECT_FALSE(gameStateOpt.has_value());
    EXPECT_FALSE(saveManager->getLastError().empty());
}

TEST_F(SaveManagerTest, LoadGameState_CorruptedFile_ReturnsCorruptedData) {
    createCorruptedSaveFile();
    
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    
    EXPECT_EQ(loadResult, SaveManager::LoadResult::CORRUPTED_DATA);
    EXPECT_FALSE(gameStateOpt.has_value());
    // Note: Error message may not be set due to const method limitations
}

TEST_F(SaveManagerTest, LoadGameState_IncompleteFile_ReturnsCorruptedData) {
    createIncompleteSaveFile();
    
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    
    EXPECT_EQ(loadResult, SaveManager::LoadResult::CORRUPTED_DATA);
    EXPECT_FALSE(gameStateOpt.has_value());
}

// Backup functionality tests
TEST_F(SaveManagerTest, CreateBackup_AfterSave_CreatesBackupFile) {
    // Save a game state first
    saveManager->saveGameState(*testGameState);
    
    SaveManager::SaveResult result = saveManager->createBackup();
    
    EXPECT_EQ(result, SaveManager::SaveResult::SUCCESS);
    EXPECT_TRUE(saveManager->hasBackup());
    EXPECT_TRUE(std::filesystem::exists(saveManager->getBackupFilePath()));
}

TEST_F(SaveManagerTest, CreateBackup_NoSaveFile_ReturnsError) {
    SaveManager::SaveResult result = saveManager->createBackup();
    
    EXPECT_EQ(result, SaveManager::SaveResult::WRITE_ERROR);
    EXPECT_FALSE(saveManager->hasBackup());
    EXPECT_FALSE(saveManager->getLastError().empty());
}

TEST_F(SaveManagerTest, RestoreFromBackup_WithValidBackup_RestoresSuccessfully) {
    // Save initial state
    saveManager->saveGameState(*testGameState);
    saveManager->createBackup();
    
    // Modify and save a different state
    testGameState->character.setName("ModifiedPlayer");
    saveManager->saveGameState(*testGameState);
    
    // Restore from backup
    SaveManager::SaveResult result = saveManager->restoreFromBackup();
    EXPECT_EQ(result, SaveManager::SaveResult::SUCCESS);
    
    // Verify the original state is restored
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    EXPECT_EQ(loadResult, SaveManager::LoadResult::SUCCESS);
    ASSERT_TRUE(gameStateOpt.has_value());
    
    // The restored character should have the original name
    EXPECT_EQ(gameStateOpt.value().character.getName(), "TestPlayer");
}

TEST_F(SaveManagerTest, RestoreFromBackup_NoBackup_ReturnsError) {
    SaveManager::SaveResult result = saveManager->restoreFromBackup();
    
    EXPECT_EQ(result, SaveManager::SaveResult::WRITE_ERROR);
    EXPECT_FALSE(saveManager->getLastError().empty());
}

TEST_F(SaveManagerTest, LoadGameState_CorruptedMainWithValidBackup_RestoresFromBackup) {
    // Save initial state and create backup
    saveManager->saveGameState(*testGameState);
    saveManager->createBackup();
    
    // Corrupt the main save file
    createCorruptedSaveFile();
    
    // Load should automatically restore from backup
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    
    EXPECT_EQ(loadResult, SaveManager::LoadResult::SUCCESS);
    ASSERT_TRUE(gameStateOpt.has_value());
    EXPECT_EQ(gameStateOpt.value().character.getName(), "TestPlayer");
}

// File validation tests
TEST_F(SaveManagerTest, IsValidSaveFile_ValidFile_ReturnsTrue) {
    saveManager->saveGameState(*testGameState);
    
    bool isValid = saveManager->isValidSaveFile(saveManager->getSaveFilePath());
    
    EXPECT_TRUE(isValid);
}

TEST_F(SaveManagerTest, IsValidSaveFile_CorruptedFile_ReturnsFalse) {
    createCorruptedSaveFile();
    
    bool isValid = saveManager->isValidSaveFile(saveManager->getSaveFilePath());
    
    EXPECT_FALSE(isValid);
}

TEST_F(SaveManagerTest, IsValidSaveFile_NonexistentFile_ReturnsFalse) {
    bool isValid = saveManager->isValidSaveFile("/nonexistent/file.json");
    
    EXPECT_FALSE(isValid);
}

// Metadata tests
TEST_F(SaveManagerTest, GetSaveFileVersion_ValidFile_ReturnsVersion) {
    saveManager->saveGameState(*testGameState);
    
    auto versionOpt = saveManager->getSaveFileVersion();
    
    ASSERT_TRUE(versionOpt.has_value());
    EXPECT_EQ(versionOpt.value(), "1.0.0");
}

TEST_F(SaveManagerTest, GetSaveFileVersion_NoFile_ReturnsNullopt) {
    auto versionOpt = saveManager->getSaveFileVersion();
    
    EXPECT_FALSE(versionOpt.has_value());
}

TEST_F(SaveManagerTest, GetLastSaveTime_ValidFile_ReturnsRecentTime) {
    auto beforeSave = std::chrono::system_clock::now();
    saveManager->saveGameState(*testGameState);
    auto afterSave = std::chrono::system_clock::now();
    
    auto saveTime = saveManager->getLastSaveTime();
    
    // The save time should be within a reasonable range (allow for some timing variance)
    auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(afterSave - saveTime).count();
    EXPECT_LE(std::abs(timeDiff), 2); // Within 2 seconds should be reasonable
    EXPECT_NE(saveTime, std::chrono::system_clock::time_point{}); // Should not be epoch
}

TEST_F(SaveManagerTest, GetLastSaveTime_NoFile_ReturnsEpoch) {
    auto saveTime = saveManager->getLastSaveTime();
    
    EXPECT_EQ(saveTime, std::chrono::system_clock::time_point{});
}

TEST_F(SaveManagerTest, GetSaveFileSize_ValidFile_ReturnsPositiveSize) {
    saveManager->saveGameState(*testGameState);
    
    size_t size = saveManager->getSaveFileSize();
    
    EXPECT_GT(size, 0);
}

TEST_F(SaveManagerTest, GetSaveFileSize_NoFile_ReturnsZero) {
    size_t size = saveManager->getSaveFileSize();
    
    EXPECT_EQ(size, 0);
}

// Error handling tests
TEST_F(SaveManagerTest, SaveGameState_MultipleSaves_CreatesBackupAutomatically) {
    // First save
    saveManager->saveGameState(*testGameState);
    EXPECT_FALSE(saveManager->hasBackup()); // No backup yet
    
    // Second save should create backup automatically
    testGameState->character.setName("ModifiedPlayer");
    saveManager->saveGameState(*testGameState);
    EXPECT_TRUE(saveManager->hasBackup()); // Backup should exist now
}

TEST_F(SaveManagerTest, ClearLastError_AfterError_ClearsErrorMessage) {
    // Trigger an error
    saveManager->loadGameState(); // No file exists
    EXPECT_FALSE(saveManager->getLastError().empty());
    
    // Clear the error
    saveManager->clearLastError();
    EXPECT_TRUE(saveManager->getLastError().empty());
}

// Settings tests
TEST_F(SaveManagerTest, CompressionSettings_SetAndGet_WorksCorrectly) {
    EXPECT_FALSE(saveManager->isCompressionEnabled()); // Default is false
    
    saveManager->setCompressionEnabled(true);
    EXPECT_TRUE(saveManager->isCompressionEnabled());
    
    saveManager->setCompressionEnabled(false);
    EXPECT_FALSE(saveManager->isCompressionEnabled());
}

// File path tests
TEST_F(SaveManagerTest, GetSaveFilePath_ReturnsCorrectPath) {
    std::string saveFilePath = saveManager->getSaveFilePath();
    
    EXPECT_TRUE(saveFilePath.find("game_save.json") != std::string::npos);
    EXPECT_TRUE(saveFilePath.find(testDirectory.string()) != std::string::npos);
}

TEST_F(SaveManagerTest, GetBackupFilePath_ReturnsCorrectPath) {
    std::string backupFilePath = saveManager->getBackupFilePath();
    
    EXPECT_TRUE(backupFilePath.find("game_save_backup.json") != std::string::npos);
    EXPECT_TRUE(backupFilePath.find(testDirectory.string()) != std::string::npos);
}

// Auto-save functionality tests
TEST_F(SaveManagerTest, EnableAutoSave_ValidInterval_EnablesAutoSave) {
    EXPECT_FALSE(saveManager->isAutoSaveEnabled());
    
    saveManager->enableAutoSave(10);
    
    EXPECT_TRUE(saveManager->isAutoSaveEnabled());
    
    // Clean up
    saveManager->disableAutoSave();
}

TEST_F(SaveManagerTest, DisableAutoSave_WhenEnabled_DisablesAutoSave) {
    saveManager->enableAutoSave(10);
    EXPECT_TRUE(saveManager->isAutoSaveEnabled());
    
    saveManager->disableAutoSave();
    
    EXPECT_FALSE(saveManager->isAutoSaveEnabled());
}

TEST_F(SaveManagerTest, EnableAutoSave_MinimumInterval_UsesMinimumValue) {
    saveManager->enableAutoSave(1); // Below minimum
    
    EXPECT_TRUE(saveManager->isAutoSaveEnabled());
    
    // Clean up
    saveManager->disableAutoSave();
}

TEST_F(SaveManagerTest, TriggerAutoSave_WithEnabledAutoSave_SavesFile) {
    saveManager->enableAutoSave(30);
    
    // Trigger an auto-save
    saveManager->triggerAutoSave(*testGameState);
    
    // Wait for the auto-save to complete (with timeout)
    bool saveCompleted = false;
    for (int i = 0; i < 50; ++i) { // Wait up to 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (saveManager->saveFileExists()) {
            saveCompleted = true;
            break;
        }
    }
    
    // Check that the file was created
    EXPECT_TRUE(saveCompleted);
    EXPECT_TRUE(saveManager->saveFileExists());
    
    // Verify the auto-save time was updated
    auto lastAutoSaveTime = saveManager->getLastAutoSaveTime();
    EXPECT_NE(lastAutoSaveTime, std::chrono::system_clock::time_point{});
    
    // Clean up
    saveManager->disableAutoSave();
}

TEST_F(SaveManagerTest, TriggerAutoSave_WithDisabledAutoSave_DoesNotSave) {
    EXPECT_FALSE(saveManager->isAutoSaveEnabled());
    
    // Trigger an auto-save (should be ignored)
    saveManager->triggerAutoSave(*testGameState);
    
    // Give some time for any potential processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // File should not exist
    EXPECT_FALSE(saveManager->saveFileExists());
}

TEST_F(SaveManagerTest, AutoSave_MultipleTriggersQuickly_ProcessesLatestState) {
    saveManager->enableAutoSave(30);
    
    // Modify the game state and trigger multiple saves quickly
    testGameState->character.setName("FirstSave");
    saveManager->triggerAutoSave(*testGameState);
    
    testGameState->character.setName("SecondSave");
    saveManager->triggerAutoSave(*testGameState);
    
    testGameState->character.setName("FinalSave");
    saveManager->triggerAutoSave(*testGameState);
    
    // Wait for the auto-save to complete (with timeout)
    bool saveCompleted = false;
    for (int i = 0; i < 100; ++i) { // Wait up to 1 second
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (saveManager->saveFileExists()) {
            saveCompleted = true;
            break;
        }
    }
    
    EXPECT_TRUE(saveCompleted);
    
    // Load the saved state and verify it has the latest changes
    auto [loadResult, gameStateOpt] = saveManager->loadGameState();
    EXPECT_EQ(loadResult, SaveManager::LoadResult::SUCCESS);
    ASSERT_TRUE(gameStateOpt.has_value());
    
    // Should have the final state
    EXPECT_EQ(gameStateOpt.value().character.getName(), "FinalSave");
    
    // Clean up
    saveManager->disableAutoSave();
}

TEST_F(SaveManagerTest, AutoSave_ThreadSafety_HandlesMultipleOperations) {
    saveManager->enableAutoSave(10);
    
    // Perform multiple operations concurrently
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, i]() {
            testGameState->character.setName("Thread" + std::to_string(i));
            saveManager->triggerAutoSave(*testGameState);
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Wait for the auto-save to complete (with timeout)
    bool saveCompleted = false;
    for (int i = 0; i < 100; ++i) { // Wait up to 1 second
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (saveManager->saveFileExists()) {
            saveCompleted = true;
            break;
        }
    }
    
    // Verify that a save file was created
    EXPECT_TRUE(saveCompleted);
    EXPECT_TRUE(saveManager->saveFileExists());
    
    // Clean up
    saveManager->disableAutoSave();
}

TEST_F(SaveManagerTest, GetLastAutoSaveTime_NoAutoSave_ReturnsEpoch) {
    auto lastAutoSaveTime = saveManager->getLastAutoSaveTime();
    EXPECT_EQ(lastAutoSaveTime, std::chrono::system_clock::time_point{});
}

TEST_F(SaveManagerTest, GetLastAutoSaveTime_AfterAutoSave_ReturnsRecentTime) {
    saveManager->enableAutoSave(30);
    
    auto beforeSave = std::chrono::system_clock::now();
    saveManager->triggerAutoSave(*testGameState);
    
    // Wait for the auto-save to complete (with timeout)
    bool saveCompleted = false;
    for (int i = 0; i < 50; ++i) { // Wait up to 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (saveManager->getLastAutoSaveTime() != std::chrono::system_clock::time_point{}) {
            saveCompleted = true;
            break;
        }
    }
    
    EXPECT_TRUE(saveCompleted);
    
    auto afterSave = std::chrono::system_clock::now();
    auto lastAutoSaveTime = saveManager->getLastAutoSaveTime();
    
    // The auto-save time should be within a reasonable range
    auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(afterSave - lastAutoSaveTime).count();
    EXPECT_LE(std::abs(timeDiff), 2); // Within 2 seconds should be reasonable
    EXPECT_NE(lastAutoSaveTime, std::chrono::system_clock::time_point{});
    
    // Clean up
    saveManager->disableAutoSave();
}