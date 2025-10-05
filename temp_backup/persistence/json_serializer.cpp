#include "json_serializer.h"
#include <unordered_map>
#include <algorithm>

// TownState::BuildingState implementation
nlohmann::json TownState::BuildingState::toJson() const {
    nlohmann::json j;
    j["level"] = level;
    j["decorations"] = decorations;
    return j;
}

TownState::BuildingState TownState::BuildingState::fromJson(const nlohmann::json& json) {
    BuildingState state;
    if (json.contains("level")) {
        state.level = json.at("level").get<int>();
    }
    if (json.contains("decorations") && json.at("decorations").is_array()) {
        state.decorations = json.at("decorations").get<std::vector<std::string>>();
    }
    return state;
}

// TownState implementation
nlohmann::json TownState::toJson() const {
    nlohmann::json j;
    
    nlohmann::json buildingsJson;
    for (const auto& [name, building] : buildings) {
        buildingsJson[name] = building.toJson();
    }
    j["buildings"] = buildingsJson;
    j["globalDecorations"] = globalDecorations;
    j["unlockedItems"] = unlockedItems;
    
    return j;
}

TownState TownState::fromJson(const nlohmann::json& json) {
    TownState state;
    
    if (json.contains("buildings") && json.at("buildings").is_object()) {
        for (const auto& [name, buildingJson] : json.at("buildings").items()) {
            state.buildings[name] = BuildingState::fromJson(buildingJson);
        }
    }
    
    if (json.contains("globalDecorations") && json.at("globalDecorations").is_array()) {
        state.globalDecorations = json.at("globalDecorations").get<std::vector<std::string>>();
    }
    
    if (json.contains("unlockedItems") && json.at("unlockedItems").is_array()) {
        state.unlockedItems = json.at("unlockedItems").get<std::vector<std::string>>();
    }
    
    return state;
}

// GamificationState implementation
nlohmann::json GamificationState::toJson() const {
    nlohmann::json j;
    j["totalExperience"] = totalExperience;
    j["level"] = level;
    j["achievements"] = achievements;
    j["currency"] = currency;
    return j;
}

GamificationState GamificationState::fromJson(const nlohmann::json& json) {
    GamificationState state;
    
    if (json.contains("totalExperience")) {
        state.totalExperience = json.at("totalExperience").get<int>();
    }
    
    if (json.contains("level")) {
        state.level = json.at("level").get<int>();
    }
    
    if (json.contains("achievements") && json.at("achievements").is_array()) {
        state.achievements = json.at("achievements").get<std::vector<std::string>>();
    }
    
    if (json.contains("currency") && json.at("currency").is_object()) {
        state.currency = json.at("currency").get<std::unordered_map<std::string, int>>();
    }
    
    return state;
}

// GameState implementation
nlohmann::json GameState::toJson() const {
    nlohmann::json j;
    
    j["character"] = character.toJson();
    j["tasks"] = JsonSerializer::serializeTasks(tasks);
    j["notes"] = JsonSerializer::serializeNotes(notes);
    j["habits"] = JsonSerializer::serializeHabits(habits);
    j["townState"] = townState.toJson();
    j["gamificationState"] = gamificationState.toJson();
    
    // Add metadata
    j["version"] = "1.0.0";
    j["savedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return j;
}

GameState GameState::fromJson(const nlohmann::json& json) {
    Character character = JsonSerializer::deserializeCharacter(json.at("character"));
    GameState state(character);
    
    if (json.contains("tasks")) {
        state.tasks = JsonSerializer::deserializeTasks(json.at("tasks"));
    }
    
    if (json.contains("notes")) {
        state.notes = JsonSerializer::deserializeNotes(json.at("notes"));
    }
    
    if (json.contains("habits")) {
        state.habits = JsonSerializer::deserializeHabits(json.at("habits"));
    }
    
    if (json.contains("townState")) {
        state.townState = TownState::fromJson(json.at("townState"));
    }
    
    if (json.contains("gamificationState")) {
        state.gamificationState = GamificationState::fromJson(json.at("gamificationState"));
    }
    
    return state;
}

// JsonSerializer implementation
nlohmann::json JsonSerializer::serializeTask(const Task& task) {
    return task.toJson();
}

nlohmann::json JsonSerializer::serializeTasks(const std::vector<Task>& tasks) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& task : tasks) {
        j.push_back(serializeTask(task));
    }
    return j;
}

Task JsonSerializer::deserializeTask(const nlohmann::json& json) {
    return Task::fromJson(json);
}

std::vector<Task> JsonSerializer::deserializeTasks(const nlohmann::json& json) {
    std::vector<Task> tasks;
    
    if (!json.is_array()) {
        return tasks;
    }
    
    // First pass: create all tasks
    for (const auto& taskJson : json) {
        tasks.push_back(deserializeTask(taskJson));
    }
    
    // Second pass: resolve subtask relationships
    resolveTaskSubtasks(tasks);
    
    return tasks;
}

nlohmann::json JsonSerializer::serializeCharacter(const Character& character) {
    return character.toJson();
}

Character JsonSerializer::deserializeCharacter(const nlohmann::json& json) {
    return Character::fromJson(json);
}

nlohmann::json JsonSerializer::serializeNote(const Note& note) {
    return note.toJson();
}

nlohmann::json JsonSerializer::serializeNotes(const std::vector<Note>& notes) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& note : notes) {
        j.push_back(serializeNote(note));
    }
    return j;
}

Note JsonSerializer::deserializeNote(const nlohmann::json& json) {
    return Note::fromJson(json);
}

std::vector<Note> JsonSerializer::deserializeNotes(const nlohmann::json& json) {
    std::vector<Note> notes;
    
    if (!json.is_array()) {
        return notes;
    }
    
    for (const auto& noteJson : json) {
        notes.push_back(deserializeNote(noteJson));
    }
    
    return notes;
}

nlohmann::json JsonSerializer::serializeHabit(const Habit& habit) {
    return habit.toJson();
}

nlohmann::json JsonSerializer::serializeHabits(const std::vector<Habit>& habits) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& habit : habits) {
        j.push_back(serializeHabit(habit));
    }
    return j;
}

Habit JsonSerializer::deserializeHabit(const nlohmann::json& json) {
    return Habit::fromJson(json);
}

std::vector<Habit> JsonSerializer::deserializeHabits(const nlohmann::json& json) {
    std::vector<Habit> habits;
    
    if (!json.is_array()) {
        return habits;
    }
    
    for (const auto& habitJson : json) {
        habits.push_back(deserializeHabit(habitJson));
    }
    
    return habits;
}

nlohmann::json JsonSerializer::serializeGameState(const GameState& gameState) {
    return gameState.toJson();
}

GameState JsonSerializer::deserializeGameState(const nlohmann::json& json) {
    return GameState::fromJson(json);
}

bool JsonSerializer::isValidJson(const std::string& jsonString) {
    try {
        [[maybe_unused]] auto parsed = nlohmann::json::parse(jsonString);
        return true;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

std::string JsonSerializer::formatJson(const nlohmann::json& json, bool pretty) {
    if (pretty) {
        return json.dump(2); // 2-space indentation
    } else {
        return json.dump();
    }
}

void JsonSerializer::resolveTaskSubtasks(std::vector<Task>& tasks) {
    // Create a map of task ID to task pointer for quick lookup
    std::unordered_map<uint32_t, Task*> taskMap;
    for (auto& task : tasks) {
        taskMap[task.getId()] = &task;
    }
    
    // For each task, resolve its subtask IDs to actual Task objects
    // Note: This is a simplified approach. In a real implementation,
    // we would need to handle the subtask relationships properly.
    // For now, we'll leave the subtask resolution to be handled
    // by the TaskEngine when it loads the tasks.
    
    // Suppress unused variable warning for now
    (void)taskMap;
}