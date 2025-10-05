#pragma once

#include <string>
#include <optional>
#include <functional>
#include <filesystem>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "json_serializer.h"

class SaveManager {
public:
    enum class SaveResult {
        SUCCESS,
        WRITE_ERROR,
        PERMISSION_ERROR,
        DISK_FULL,
        UNKNOWN_ERROR
    };
    
    enum class LoadResult {
        SUCCESS,
        FILE_NOT_FOUND,
        READ_ERROR,
        CORRUPTED_DATA,
        VERSION_MISMATCH,
        UNKNOWN_ERROR
    };
    
    explicit SaveManager(const std::string& saveDirectory = "");
    ~SaveManager();
    
    // Core save/load operations
    SaveResult saveGameState(const GameState& state);
    std::pair<LoadResult, std::optional<GameState>> loadGameState();
    
    // Backup operations
    SaveResult createBackup();
    SaveResult restoreFromBackup();
    bool hasBackup() const;
    
    // File management
    bool saveFileExists() const;
    bool isValidSaveFile(const std::string& filePath) const;
    std::string getSaveFilePath() const { return mainSaveFile; }
    std::string getBackupFilePath() const { return backupSaveFile; }
    
    // Error handling
    std::string getLastError() const { return lastError; }
    void clearLastError() { lastError.clear(); }
    
    // Utility methods
    std::optional<std::string> getSaveFileVersion() const;
    std::chrono::system_clock::time_point getLastSaveTime() const;
    size_t getSaveFileSize() const;
    
    // Auto-save functionality
    void enableAutoSave(int intervalSeconds = 30);
    void disableAutoSave();
    bool isAutoSaveEnabled() const { return autoSaveEnabled; }
    void triggerAutoSave(const GameState& state);
    std::chrono::system_clock::time_point getLastAutoSaveTime() const { return lastAutoSaveTime; }
    
    // Settings
    void setCompressionEnabled(bool enabled) { compressionEnabled = enabled; }
    bool isCompressionEnabled() const { return compressionEnabled; }
    
private:
    std::string saveDirectory;
    std::string mainSaveFile;
    std::string backupSaveFile;
    std::string lastError;
    bool compressionEnabled;
    
    // Auto-save members
    std::atomic<bool> autoSaveEnabled;
    std::atomic<bool> shouldStop;
    int autoSaveInterval; // seconds
    std::thread autoSaveThread;
    std::mutex autoSaveMutex;
    std::condition_variable autoSaveCondition;
    std::queue<GameState> pendingSaves;
    std::chrono::system_clock::time_point lastAutoSaveTime;
    
    // Helper methods
    bool ensureDirectoryExists(const std::string& directory);
    SaveResult writeJsonToFile(const nlohmann::json& json, const std::string& filePath);
    std::pair<LoadResult, std::optional<nlohmann::json>> readJsonFromFile(const std::string& filePath) const;
    std::string getDefaultSaveDirectory() const;
    bool validateGameStateJson(const nlohmann::json& json) const;
    void setLastError(const std::string& error);
    
    // Auto-save helper methods
    void autoSaveLoop();
    void processPendingSaves();
    
    // File operations
    bool copyFile(const std::string& source, const std::string& destination);
    bool deleteFile(const std::string& filePath);
    std::optional<std::string> readFileToString(const std::string& filePath) const;
    bool writeStringToFile(const std::string& content, const std::string& filePath);
};