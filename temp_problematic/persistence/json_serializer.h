#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include "../core/models/task.h"
#include "../core/models/character.h"
#include "../core/models/note.h"
#include "../core/models/habit.h"

// Forward declarations for game state structures
struct TownState {
    struct BuildingState {
        int level = 1;
        std::vector<std::string> decorations;
        
        nlohmann::json toJson() const;
        static BuildingState fromJson(const nlohmann::json& json);
    };
    
    std::unordered_map<std::string, BuildingState> buildings;
    std::vector<std::string> globalDecorations;
    std::vector<std::string> unlockedItems;
    
    nlohmann::json toJson() const;
    static TownState fromJson(const nlohmann::json& json);
};

struct GamificationState {
    int totalExperience = 0;
    int level = 1;
    std::vector<std::string> achievements;
    std::unordered_map<std::string, int> currency; // coffee, experience, etc.
    
    nlohmann::json toJson() const;
    static GamificationState fromJson(const nlohmann::json& json);
};

struct GameState {
    Character character;
    std::vector<Task> tasks;
    std::vector<Note> notes;
    std::vector<Habit> habits;
    TownState townState;
    GamificationState gamificationState;
    
    GameState(const Character& character) : character(character) {}
    
    nlohmann::json toJson() const;
    static GameState fromJson(const nlohmann::json& json);
};

class JsonSerializer {
public:
    JsonSerializer() = default;
    
    // Individual model serialization
    static nlohmann::json serializeTask(const Task& task);
    static nlohmann::json serializeTasks(const std::vector<Task>& tasks);
    static Task deserializeTask(const nlohmann::json& json);
    static std::vector<Task> deserializeTasks(const nlohmann::json& json);
    
    static nlohmann::json serializeCharacter(const Character& character);
    static Character deserializeCharacter(const nlohmann::json& json);
    
    static nlohmann::json serializeNote(const Note& note);
    static nlohmann::json serializeNotes(const std::vector<Note>& notes);
    static Note deserializeNote(const nlohmann::json& json);
    static std::vector<Note> deserializeNotes(const nlohmann::json& json);
    
    static nlohmann::json serializeHabit(const Habit& habit);
    static nlohmann::json serializeHabits(const std::vector<Habit>& habits);
    static Habit deserializeHabit(const nlohmann::json& json);
    static std::vector<Habit> deserializeHabits(const nlohmann::json& json);
    
    // Game state serialization
    static nlohmann::json serializeGameState(const GameState& gameState);
    static GameState deserializeGameState(const nlohmann::json& json);
    
    // Utility methods
    static bool isValidJson(const std::string& jsonString);
    static std::string formatJson(const nlohmann::json& json, bool pretty = true);
    
private:
    // Helper methods for subtask resolution
    static void resolveTaskSubtasks(std::vector<Task>& tasks);
};