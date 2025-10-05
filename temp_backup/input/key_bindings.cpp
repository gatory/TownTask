#include "key_bindings.h"
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

KeyBindings::KeyBindings() {
    initializeStringMaps();
    loadDefaultBindings();
}

// Key binding management
void KeyBindings::bindKey(KeyCode key, InputAction action) {
    if (!isValidBinding(key, action)) {
        return;
    }
    
    // Remove existing binding for this key if it exists
    if (isKeyBound(key)) {
        unbindKey(key);
    }
    
    addBinding(key, action);
    triggerBindingChangedCallback(key, action);
}

void KeyBindings::unbindKey(KeyCode key) {
    auto it = keyToAction.find(key);
    if (it == keyToAction.end()) {
        return;
    }
    
    InputAction action = it->second;
    
    // Remove from keyToAction map
    keyToAction.erase(it);
    
    // Remove from actionToKeys map
    auto actionIt = actionToKeys.find(action);
    if (actionIt != actionToKeys.end()) {
        auto& keys = actionIt->second;
        keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
        
        // If no keys left for this action, remove the action entry
        if (keys.empty()) {
            actionToKeys.erase(actionIt);
        }
    }
    
    triggerBindingRemovedCallback(key);
}

void KeyBindings::unbindAction(InputAction action) {
    auto it = actionToKeys.find(action);
    if (it == actionToKeys.end()) {
        return;
    }
    
    // Get all keys bound to this action
    std::vector<KeyCode> keysToRemove = it->second;
    
    // Remove each key binding
    for (KeyCode key : keysToRemove) {
        keyToAction.erase(key);
        triggerBindingRemovedCallback(key);
    }
    
    // Remove the action entry
    actionToKeys.erase(it);
}

void KeyBindings::clearAllBindings() {
    keyToAction.clear();
    actionToKeys.clear();
}

// Query bindings
InputAction KeyBindings::getActionForKey(KeyCode key) const {
    auto it = keyToAction.find(key);
    return it != keyToAction.end() ? it->second : InputAction::UNKNOWN_ACTION;
}

std::vector<KeyCode> KeyBindings::getKeysForAction(InputAction action) const {
    auto it = actionToKeys.find(action);
    return it != actionToKeys.end() ? it->second : std::vector<KeyCode>();
}

bool KeyBindings::isKeyBound(KeyCode key) const {
    return keyToAction.find(key) != keyToAction.end();
}

bool KeyBindings::isActionBound(InputAction action) const {
    return actionToKeys.find(action) != actionToKeys.end();
}

// Default bindings
void KeyBindings::loadDefaultBindings() {
    clearAllBindings();
    
    // Movement bindings (WASD + Arrow keys)
    bindKey(KeyCode::W, InputAction::MOVE_UP);
    bindKey(KeyCode::UP, InputAction::MOVE_UP);
    bindKey(KeyCode::S, InputAction::MOVE_DOWN);
    bindKey(KeyCode::DOWN, InputAction::MOVE_DOWN);
    bindKey(KeyCode::A, InputAction::MOVE_LEFT);
    bindKey(KeyCode::LEFT, InputAction::MOVE_LEFT);
    bindKey(KeyCode::D, InputAction::MOVE_RIGHT);
    bindKey(KeyCode::RIGHT, InputAction::MOVE_RIGHT);
    
    // Interaction bindings
    bindKey(KeyCode::E, InputAction::INTERACT);
    bindKey(KeyCode::SPACE, InputAction::INTERACT);
    bindKey(KeyCode::ENTER, InputAction::CONFIRM);
    bindKey(KeyCode::ESCAPE, InputAction::CANCEL);
    
    // UI navigation (separate from movement keys to avoid conflicts)
    bindKey(KeyCode::TAB, InputAction::MENU_SELECT);
    // Note: ESCAPE is already bound to CANCEL, which can also serve as MENU_BACK
    
    // Application controls
    bindKey(KeyCode::F11, InputAction::TOGGLE_FULLSCREEN);
    bindKey(KeyCode::F12, InputAction::TOGGLE_DESKTOP_MODE);
    
    // Building-specific shortcuts
    bindKey(KeyCode::KEY_1, InputAction::START_POMODORO);
    bindKey(KeyCode::KEY_2, InputAction::PAUSE_POMODORO);
    bindKey(KeyCode::KEY_3, InputAction::CREATE_TASK);
    bindKey(KeyCode::KEY_4, InputAction::CREATE_NOTE);
    bindKey(KeyCode::KEY_5, InputAction::CHECK_HABIT);
    
    // Debug bindings
    bindKey(KeyCode::F1, InputAction::TOGGLE_DEBUG_INFO);
    bindKey(KeyCode::F5, InputAction::RELOAD_CONFIG);
}

void KeyBindings::loadAlternativeBindings() {
    clearAllBindings();
    
    // Alternative movement (Arrow keys only)
    bindKey(KeyCode::UP, InputAction::MOVE_UP);
    bindKey(KeyCode::DOWN, InputAction::MOVE_DOWN);
    bindKey(KeyCode::LEFT, InputAction::MOVE_LEFT);
    bindKey(KeyCode::RIGHT, InputAction::MOVE_RIGHT);
    
    // Alternative interaction (Space only)
    bindKey(KeyCode::SPACE, InputAction::INTERACT);
    bindKey(KeyCode::ENTER, InputAction::CONFIRM);
    bindKey(KeyCode::ESCAPE, InputAction::CANCEL);
    
    // UI navigation using different keys to avoid conflicts with movement
    bindKey(KeyCode::TAB, InputAction::MENU_SELECT);
    bindKey(KeyCode::ESCAPE, InputAction::MENU_BACK);
    
    // Same application controls
    bindKey(KeyCode::F11, InputAction::TOGGLE_FULLSCREEN);
    bindKey(KeyCode::F12, InputAction::TOGGLE_DESKTOP_MODE);
}

// Configuration
void KeyBindings::loadFromConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // If config file doesn't exist, use defaults
        loadDefaultBindings();
        return;
    }
    
    try {
        nlohmann::json json;
        file >> json;
        loadFromJson(json);
    } catch (const std::exception& e) {
        // If config is corrupted, use defaults
        loadDefaultBindings();
    }
}

void KeyBindings::saveToConfig(const std::string& configPath) const {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        return;
    }
    
    nlohmann::json json = saveToJson();
    file << json.dump(2);
}

void KeyBindings::loadFromJson(const nlohmann::json& json) {
    clearAllBindings();
    
    if (!json.contains("bindings")) {
        loadDefaultBindings();
        return;
    }
    
    for (const auto& binding : json["bindings"]) {
        if (!binding.contains("key") || !binding.contains("action")) {
            continue;
        }
        
        std::string keyStr = binding["key"];
        std::string actionStr = binding["action"];
        
        KeyCode key = stringToKeyCode(keyStr);
        InputAction action = stringToAction(actionStr);
        
        if (key != KeyCode::UNKNOWN && action != InputAction::UNKNOWN_ACTION) {
            bindKey(key, action);
        }
    }
    
    // If no valid bindings were loaded, use defaults
    if (keyToAction.empty()) {
        loadDefaultBindings();
    }
}

nlohmann::json KeyBindings::saveToJson() const {
    nlohmann::json json;
    nlohmann::json bindings = nlohmann::json::array();
    
    for (const auto& [key, action] : keyToAction) {
        nlohmann::json binding;
        binding["key"] = keyCodeToString(key);
        binding["action"] = actionToString(action);
        bindings.push_back(binding);
    }
    
    json["bindings"] = bindings;
    return json;
}

// Utility functions
std::string KeyBindings::keyCodeToString(KeyCode key) const {
    auto it = keyCodeToStringMap.find(key);
    return it != keyCodeToStringMap.end() ? it->second : "UNKNOWN";
}

KeyCode KeyBindings::stringToKeyCode(const std::string& keyStr) const {
    auto it = stringToKeyCodeMap.find(keyStr);
    return it != stringToKeyCodeMap.end() ? it->second : KeyCode::UNKNOWN;
}

std::string KeyBindings::actionToString(InputAction action) const {
    auto it = actionToStringMap.find(action);
    return it != actionToStringMap.end() ? it->second : "UNKNOWN_ACTION";
}

InputAction KeyBindings::stringToAction(const std::string& actionStr) const {
    auto it = stringToActionMap.find(actionStr);
    return it != stringToActionMap.end() ? it->second : InputAction::UNKNOWN_ACTION;
}

// Validation
bool KeyBindings::isValidBinding(KeyCode key, InputAction action) const {
    // Don't allow binding unknown keys or actions
    if (key == KeyCode::UNKNOWN || action == InputAction::UNKNOWN_ACTION) {
        return false;
    }
    
    // Don't allow rebinding system keys that should remain fixed
    if (key == KeyCode::ESCAPE && action != InputAction::CANCEL && action != InputAction::MENU_BACK) {
        return false;
    }
    
    return true;
}

std::vector<std::string> KeyBindings::getConflicts(KeyCode key, InputAction action) const {
    std::vector<std::string> conflicts;
    
    // Check if key is already bound to a different action
    if (isKeyBound(key)) {
        InputAction existingAction = getActionForKey(key);
        if (existingAction != action) {
            conflicts.push_back("Key " + keyCodeToString(key) + " is already bound to " + actionToString(existingAction));
        }
    }
    
    return conflicts;
}

// Event callbacks
void KeyBindings::setOnBindingChanged(std::function<void(KeyCode, InputAction)> callback) {
    onBindingChanged = std::move(callback);
}

void KeyBindings::setOnBindingRemoved(std::function<void(KeyCode)> callback) {
    onBindingRemoved = std::move(callback);
}

// Private helper methods
void KeyBindings::addBinding(KeyCode key, InputAction action) {
    keyToAction[key] = action;
    actionToKeys[action].push_back(key);
}

void KeyBindings::removeBinding(KeyCode key) {
    unbindKey(key);
}

void KeyBindings::triggerBindingChangedCallback(KeyCode key, InputAction action) {
    if (onBindingChanged) {
        onBindingChanged(key, action);
    }
}

void KeyBindings::triggerBindingRemovedCallback(KeyCode key) {
    if (onBindingRemoved) {
        onBindingRemoved(key);
    }
}

void KeyBindings::initializeStringMaps() {
    // Initialize key code to string mappings
    keyCodeToStringMap = {
        // Movement keys
        {KeyCode::W, "W"},
        {KeyCode::A, "A"},
        {KeyCode::S, "S"},
        {KeyCode::D, "D"},
        {KeyCode::UP, "UP"},
        {KeyCode::DOWN, "DOWN"},
        {KeyCode::LEFT, "LEFT"},
        {KeyCode::RIGHT, "RIGHT"},
        
        // Interaction keys
        {KeyCode::E, "E"},
        {KeyCode::F, "F"},
        {KeyCode::G, "G"},
        {KeyCode::SPACE, "SPACE"},
        {KeyCode::ENTER, "ENTER"},
        {KeyCode::ESCAPE, "ESCAPE"},
        
        // Function keys
        {KeyCode::F1, "F1"},
        {KeyCode::F2, "F2"},
        {KeyCode::F3, "F3"},
        {KeyCode::F4, "F4"},
        {KeyCode::F5, "F5"},
        {KeyCode::F6, "F6"},
        {KeyCode::F7, "F7"},
        {KeyCode::F8, "F8"},
        {KeyCode::F9, "F9"},
        {KeyCode::F10, "F10"},
        {KeyCode::F11, "F11"},
        {KeyCode::F12, "F12"},
        
        // Number keys
        {KeyCode::KEY_0, "0"},
        {KeyCode::KEY_1, "1"},
        {KeyCode::KEY_2, "2"},
        {KeyCode::KEY_3, "3"},
        {KeyCode::KEY_4, "4"},
        {KeyCode::KEY_5, "5"},
        {KeyCode::KEY_6, "6"},
        {KeyCode::KEY_7, "7"},
        {KeyCode::KEY_8, "8"},
        {KeyCode::KEY_9, "9"},
        
        // Modifier keys
        {KeyCode::LEFT_SHIFT, "LEFT_SHIFT"},
        {KeyCode::RIGHT_SHIFT, "RIGHT_SHIFT"},
        {KeyCode::LEFT_CONTROL, "LEFT_CONTROL"},
        {KeyCode::RIGHT_CONTROL, "RIGHT_CONTROL"},
        {KeyCode::LEFT_ALT, "LEFT_ALT"},
        {KeyCode::RIGHT_ALT, "RIGHT_ALT"},
        
        // Other keys
        {KeyCode::TAB, "TAB"},
        {KeyCode::BACKSPACE, "BACKSPACE"},
        {KeyCode::DELETE, "DELETE"},
        {KeyCode::HOME, "HOME"},
        {KeyCode::END, "END"},
        {KeyCode::PAGE_UP, "PAGE_UP"},
        {KeyCode::PAGE_DOWN, "PAGE_DOWN"}
    };
    
    // Create reverse mapping
    for (const auto& [key, str] : keyCodeToStringMap) {
        stringToKeyCodeMap[str] = key;
    }
    
    // Initialize action to string mappings
    actionToStringMap = {
        // Character movement
        {InputAction::MOVE_UP, "MOVE_UP"},
        {InputAction::MOVE_DOWN, "MOVE_DOWN"},
        {InputAction::MOVE_LEFT, "MOVE_LEFT"},
        {InputAction::MOVE_RIGHT, "MOVE_RIGHT"},
        
        // Interactions
        {InputAction::INTERACT, "INTERACT"},
        {InputAction::CANCEL, "CANCEL"},
        {InputAction::CONFIRM, "CONFIRM"},
        
        // UI navigation
        {InputAction::MENU_UP, "MENU_UP"},
        {InputAction::MENU_DOWN, "MENU_DOWN"},
        {InputAction::MENU_LEFT, "MENU_LEFT"},
        {InputAction::MENU_RIGHT, "MENU_RIGHT"},
        {InputAction::MENU_SELECT, "MENU_SELECT"},
        {InputAction::MENU_BACK, "MENU_BACK"},
        
        // Application controls
        {InputAction::TOGGLE_FULLSCREEN, "TOGGLE_FULLSCREEN"},
        {InputAction::TOGGLE_DESKTOP_MODE, "TOGGLE_DESKTOP_MODE"},
        {InputAction::QUIT_APPLICATION, "QUIT_APPLICATION"},
        
        // Debug/Development
        {InputAction::TOGGLE_DEBUG_INFO, "TOGGLE_DEBUG_INFO"},
        {InputAction::RELOAD_CONFIG, "RELOAD_CONFIG"},
        
        // Building-specific actions
        {InputAction::START_POMODORO, "START_POMODORO"},
        {InputAction::PAUSE_POMODORO, "PAUSE_POMODORO"},
        {InputAction::CREATE_TASK, "CREATE_TASK"},
        {InputAction::CREATE_NOTE, "CREATE_NOTE"},
        {InputAction::CHECK_HABIT, "CHECK_HABIT"}
    };
    
    // Create reverse mapping
    for (const auto& [action, str] : actionToStringMap) {
        stringToActionMap[str] = action;
    }
}