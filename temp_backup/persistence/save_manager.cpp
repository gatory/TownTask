#include "save_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>

SaveManager::SaveManager(const std::string& saveDirectory)
    : saveDirectory(saveDirectory.empty() ? getDefaultSaveDirectory() : saveDirectory),
      compressionEnabled(false), autoSaveEnabled(false), shouldStop(false), autoSaveInterval(30) {
    
    // Ensure the save directory exists
    if (!ensureDirectoryExists(this->saveDirectory)) {
        setLastError("Failed to create save directory: " + this->saveDirectory);
    }
    
    // Set up file paths
    mainSaveFile = this->saveDirectory + "/game_save.json";
    backupSaveFile = this->saveDirectory + "/game_save_backup.json";
}

SaveManager::~SaveManager() {
    disableAutoSave();
}

SaveManager::SaveResult SaveManager::saveGameState(const GameState& state) {
    clearLastError();
    
    try {
        // Serialize the game state
        nlohmann::json json = JsonSerializer::serializeGameState(state);
        
        // Create backup of existing save file if it exists
        if (saveFileExists()) {
            if (createBackup() != SaveResult::SUCCESS) {
                // Continue anyway, but log the warning
                std::cerr << "Warning: Failed to create backup before saving" << std::endl;
            }
        }
        
        // Write to main save file
        SaveResult result = writeJsonToFile(json, mainSaveFile);
        
        if (result == SaveResult::SUCCESS) {
            std::cout << "Game saved successfully to: " << mainSaveFile << std::endl;
        }
        
        return result;
        
    } catch (const std::exception& e) {
        setLastError("Exception during save: " + std::string(e.what()));
        return SaveResult::UNKNOWN_ERROR;
    }
}

std::pair<SaveManager::LoadResult, std::optional<GameState>> SaveManager::loadGameState() {
    clearLastError();
    
    if (!saveFileExists()) {
        setLastError("Save file does not exist: " + mainSaveFile);
        return {LoadResult::FILE_NOT_FOUND, std::nullopt};
    }
    
    try {
        // Read JSON from file
        auto [loadResult, jsonOpt] = readJsonFromFile(mainSaveFile);
        
        if (loadResult != LoadResult::SUCCESS || !jsonOpt.has_value()) {
            // Try to restore from backup
            if (hasBackup()) {
                std::cout << "Main save file corrupted, attempting to restore from backup..." << std::endl;
                auto [backupResult, backupJsonOpt] = readJsonFromFile(backupSaveFile);
                
                if (backupResult == LoadResult::SUCCESS && backupJsonOpt.has_value()) {
                    jsonOpt = backupJsonOpt;
                    loadResult = LoadResult::SUCCESS;
                } else {
                    setLastError("Both main save and backup are corrupted");
                    return {LoadResult::CORRUPTED_DATA, std::nullopt};
                }
            } else {
                return {loadResult, std::nullopt};
            }
        }
        
        // Validate the JSON structure
        if (!validateGameStateJson(jsonOpt.value())) {
            setLastError("Save file has invalid structure");
            return {LoadResult::CORRUPTED_DATA, std::nullopt};
        }
        
        // Deserialize the game state
        GameState gameState = JsonSerializer::deserializeGameState(jsonOpt.value());
        
        std::cout << "Game loaded successfully from: " << mainSaveFile << std::endl;
        return {LoadResult::SUCCESS, gameState};
        
    } catch (const std::exception& e) {
        setLastError("Exception during load: " + std::string(e.what()));
        return {LoadResult::UNKNOWN_ERROR, std::nullopt};
    }
}

SaveManager::SaveResult SaveManager::createBackup() {
    clearLastError();
    
    if (!saveFileExists()) {
        setLastError("Cannot create backup: main save file does not exist");
        return SaveResult::WRITE_ERROR;
    }
    
    if (copyFile(mainSaveFile, backupSaveFile)) {
        return SaveResult::SUCCESS;
    } else {
        setLastError("Failed to copy save file to backup location");
        return SaveResult::WRITE_ERROR;
    }
}

SaveManager::SaveResult SaveManager::restoreFromBackup() {
    clearLastError();
    
    if (!hasBackup()) {
        setLastError("Cannot restore: backup file does not exist");
        return SaveResult::WRITE_ERROR;
    }
    
    if (copyFile(backupSaveFile, mainSaveFile)) {
        std::cout << "Successfully restored from backup" << std::endl;
        return SaveResult::SUCCESS;
    } else {
        setLastError("Failed to restore from backup");
        return SaveResult::WRITE_ERROR;
    }
}

bool SaveManager::hasBackup() const {
    return std::filesystem::exists(backupSaveFile);
}

bool SaveManager::saveFileExists() const {
    return std::filesystem::exists(mainSaveFile);
}

bool SaveManager::isValidSaveFile(const std::string& filePath) const {
    auto [result, jsonOpt] = readJsonFromFile(filePath);
    return result == LoadResult::SUCCESS && jsonOpt.has_value() && 
           validateGameStateJson(jsonOpt.value());
}

std::optional<std::string> SaveManager::getSaveFileVersion() const {
    if (!saveFileExists()) {
        return std::nullopt;
    }
    
    auto [result, jsonOpt] = readJsonFromFile(mainSaveFile);
    if (result == LoadResult::SUCCESS && jsonOpt.has_value()) {
        const auto& json = jsonOpt.value();
        if (json.contains("version")) {
            return json.at("version").get<std::string>();
        }
    }
    
    return std::nullopt;
}

std::chrono::system_clock::time_point SaveManager::getLastSaveTime() const {
    if (!saveFileExists()) {
        return std::chrono::system_clock::time_point{};
    }
    
    auto [result, jsonOpt] = readJsonFromFile(mainSaveFile);
    if (result == LoadResult::SUCCESS && jsonOpt.has_value()) {
        const auto& json = jsonOpt.value();
        if (json.contains("savedAt")) {
            auto timestamp = json.at("savedAt").get<int64_t>();
            return std::chrono::system_clock::from_time_t(timestamp);
        }
    }
    
    return std::chrono::system_clock::time_point{};
}

size_t SaveManager::getSaveFileSize() const {
    if (!saveFileExists()) {
        return 0;
    }
    
    try {
        return std::filesystem::file_size(mainSaveFile);
    } catch (const std::filesystem::filesystem_error&) {
        return 0;
    }
}

bool SaveManager::ensureDirectoryExists(const std::string& directory) {
    try {
        if (!std::filesystem::exists(directory)) {
            return std::filesystem::create_directories(directory);
        }
        return true;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

SaveManager::SaveResult SaveManager::writeJsonToFile(const nlohmann::json& json, const std::string& filePath) {
    try {
        std::string jsonString = JsonSerializer::formatJson(json, true);
        
        if (writeStringToFile(jsonString, filePath)) {
            return SaveResult::SUCCESS;
        } else {
            setLastError("Failed to write to file: " + filePath);
            return SaveResult::WRITE_ERROR;
        }
        
    } catch (const std::exception& e) {
        setLastError("Exception writing JSON: " + std::string(e.what()));
        return SaveResult::UNKNOWN_ERROR;
    }
}

std::pair<SaveManager::LoadResult, std::optional<nlohmann::json>> SaveManager::readJsonFromFile(const std::string& filePath) const {
    try {
        auto contentOpt = readFileToString(filePath);
        if (!contentOpt.has_value()) {
            return {LoadResult::READ_ERROR, std::nullopt};
        }
        
        if (!JsonSerializer::isValidJson(contentOpt.value())) {
            // Can't call setLastError from const method, so we'll handle this differently
            return {LoadResult::CORRUPTED_DATA, std::nullopt};
        }
        
        nlohmann::json json = nlohmann::json::parse(contentOpt.value());
        return {LoadResult::SUCCESS, json};
        
    } catch (const nlohmann::json::exception&) {
        return {LoadResult::CORRUPTED_DATA, std::nullopt};
    } catch (const std::exception&) {
        return {LoadResult::UNKNOWN_ERROR, std::nullopt};
    }
}

std::string SaveManager::getDefaultSaveDirectory() const {
    // Use a platform-appropriate save directory
    std::string homeDir;
    
#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        homeDir = std::string(appData) + "/TaskTown";
    } else {
        homeDir = "./save_data";
    }
#elif __APPLE__
    const char* home = std::getenv("HOME");
    if (home) {
        homeDir = std::string(home) + "/Library/Application Support/TaskTown";
    } else {
        homeDir = "./save_data";
    }
#else // Linux and others
    const char* home = std::getenv("HOME");
    if (home) {
        homeDir = std::string(home) + "/.local/share/TaskTown";
    } else {
        homeDir = "./save_data";
    }
#endif
    
    return homeDir;
}

bool SaveManager::validateGameStateJson(const nlohmann::json& json) const {
    // Check for required top-level fields
    if (!json.contains("character") || !json.contains("version")) {
        return false;
    }
    
    // Check that arrays are actually arrays
    if (json.contains("tasks") && !json.at("tasks").is_array()) {
        return false;
    }
    
    if (json.contains("notes") && !json.at("notes").is_array()) {
        return false;
    }
    
    if (json.contains("habits") && !json.at("habits").is_array()) {
        return false;
    }
    
    // Basic structure validation passed
    return true;
}

void SaveManager::setLastError(const std::string& error) {
    lastError = error;
    std::cerr << "SaveManager Error: " << error << std::endl;
}

bool SaveManager::copyFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::copy_file(source, destination, 
                                  std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool SaveManager::deleteFile(const std::string& filePath) {
    try {
        return std::filesystem::remove(filePath);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::optional<std::string> SaveManager::readFileToString(const std::string& filePath) const {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

bool SaveManager::writeStringToFile(const std::string& content, const std::string& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        file << content;
        return file.good();
        
    } catch (const std::exception&) {
        return false;
    }
}

// Auto-save functionality implementation
void SaveManager::enableAutoSave(int intervalSeconds) {
    if (autoSaveEnabled) {
        disableAutoSave(); // Stop existing auto-save first
    }
    
    autoSaveInterval = std::max(5, intervalSeconds); // Minimum 5 seconds
    autoSaveEnabled = true;
    shouldStop = false;
    
    // Start the auto-save thread
    autoSaveThread = std::thread(&SaveManager::autoSaveLoop, this);
    
    std::cout << "Auto-save enabled with " << autoSaveInterval << " second interval" << std::endl;
}

void SaveManager::disableAutoSave() {
    if (!autoSaveEnabled) {
        return;
    }
    
    autoSaveEnabled = false;
    shouldStop = true;
    
    // Wake up the auto-save thread
    autoSaveCondition.notify_all();
    
    // Wait for the thread to finish
    if (autoSaveThread.joinable()) {
        autoSaveThread.join();
    }
    
    // Process any remaining saves
    processPendingSaves();
    
    std::cout << "Auto-save disabled" << std::endl;
}

void SaveManager::triggerAutoSave(const GameState& state) {
    if (!autoSaveEnabled) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(autoSaveMutex);
        
        // Replace any pending save with the latest state
        // This prevents queue buildup if saves are triggered faster than they can be processed
        while (!pendingSaves.empty()) {
            pendingSaves.pop();
        }
        pendingSaves.push(state);
    }
    
    // Notify the auto-save thread
    autoSaveCondition.notify_one();
}

void SaveManager::autoSaveLoop() {
    while (!shouldStop) {
        std::unique_lock<std::mutex> lock(autoSaveMutex);
        
        // Wait for either a save request or timeout
        autoSaveCondition.wait_for(lock, std::chrono::seconds(autoSaveInterval), [this] {
            return shouldStop || !pendingSaves.empty();
        });
        
        if (shouldStop) {
            break;
        }
        
        // Extract all pending saves while holding the lock
        std::queue<GameState> savesToProcess;
        savesToProcess.swap(pendingSaves);
        
        // Release the lock before processing saves
        lock.unlock();
        
        // Process saves without holding the lock
        while (!savesToProcess.empty()) {
            const GameState& state = savesToProcess.front();
            
            SaveResult result = saveGameState(state);
            if (result == SaveResult::SUCCESS) {
                lastAutoSaveTime = std::chrono::system_clock::now();
                std::cout << "Auto-save completed successfully" << std::endl;
            } else {
                std::cerr << "Auto-save failed: " << getLastError() << std::endl;
            }
            
            savesToProcess.pop();
        }
    }
}

void SaveManager::processPendingSaves() {
    std::queue<GameState> savesToProcess;
    {
        std::lock_guard<std::mutex> lock(autoSaveMutex);
        savesToProcess.swap(pendingSaves);
    }
    
    // Process saves without holding the lock
    while (!savesToProcess.empty()) {
        const GameState& state = savesToProcess.front();
        
        SaveResult result = saveGameState(state);
        if (result == SaveResult::SUCCESS) {
            lastAutoSaveTime = std::chrono::system_clock::now();
            std::cout << "Auto-save completed successfully" << std::endl;
        } else {
            std::cerr << "Auto-save failed: " << getLastError() << std::endl;
        }
        
        savesToProcess.pop();
    }
}