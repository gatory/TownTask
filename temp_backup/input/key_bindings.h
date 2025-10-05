#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

// Key codes - using a simple enum for cross-platform compatibility
enum class KeyCode {
    // Movement keys
    W = 87,
    A = 65,
    S = 83,
    D = 68,
    UP = 265,
    DOWN = 264,
    LEFT = 263,
    RIGHT = 262,
    
    // Interaction keys
    E = 69,
    F = 70,
    G = 71,
    SPACE = 32,
    ENTER = 257,
    ESCAPE = 256,
    
    // Function keys
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    
    // Number keys
    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,
    
    // Modifier keys
    LEFT_SHIFT = 340,
    RIGHT_SHIFT = 344,
    LEFT_CONTROL = 341,
    RIGHT_CONTROL = 345,
    LEFT_ALT = 342,
    RIGHT_ALT = 346,
    
    // Other common keys
    TAB = 258,
    BACKSPACE = 259,
    DELETE = 261,
    HOME = 268,
    END = 269,
    PAGE_UP = 266,
    PAGE_DOWN = 267,
    
    // Mouse buttons (for completeness)
    MOUSE_LEFT = 0,
    MOUSE_RIGHT = 1,
    MOUSE_MIDDLE = 2,
    
    UNKNOWN = -1
};

// Action types that can be bound to keys
enum class InputAction {
    // Character movement
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    
    // Interactions
    INTERACT,
    CANCEL,
    CONFIRM,
    
    // UI navigation
    MENU_UP,
    MENU_DOWN,
    MENU_LEFT,
    MENU_RIGHT,
    MENU_SELECT,
    MENU_BACK,
    
    // Application controls
    TOGGLE_FULLSCREEN,
    TOGGLE_DESKTOP_MODE,
    QUIT_APPLICATION,
    
    // Debug/Development
    TOGGLE_DEBUG_INFO,
    RELOAD_CONFIG,
    
    // Building-specific actions
    START_POMODORO,
    PAUSE_POMODORO,
    CREATE_TASK,
    CREATE_NOTE,
    CHECK_HABIT,
    
    UNKNOWN_ACTION
};

// Key state for tracking press/release
enum class KeyState {
    RELEASED,
    PRESSED,
    HELD,
    JUST_PRESSED,
    JUST_RELEASED
};

class KeyBindings {
public:
    KeyBindings();
    
    // Key binding management
    void bindKey(KeyCode key, InputAction action);
    void unbindKey(KeyCode key);
    void unbindAction(InputAction action);
    void clearAllBindings();
    
    // Query bindings
    InputAction getActionForKey(KeyCode key) const;
    std::vector<KeyCode> getKeysForAction(InputAction action) const;
    bool isKeyBound(KeyCode key) const;
    bool isActionBound(InputAction action) const;
    
    // Default bindings
    void loadDefaultBindings();
    void loadAlternativeBindings(); // For users who prefer different layouts
    
    // Configuration
    void loadFromConfig(const std::string& configPath);
    void saveToConfig(const std::string& configPath) const;
    void loadFromJson(const nlohmann::json& json);
    nlohmann::json saveToJson() const;
    
    // Utility functions
    std::string keyCodeToString(KeyCode key) const;
    KeyCode stringToKeyCode(const std::string& keyStr) const;
    std::string actionToString(InputAction action) const;
    InputAction stringToAction(const std::string& actionStr) const;
    
    // Validation
    bool isValidBinding(KeyCode key, InputAction action) const;
    std::vector<std::string> getConflicts(KeyCode key, InputAction action) const;
    
    // Event callbacks
    void setOnBindingChanged(std::function<void(KeyCode, InputAction)> callback);
    void setOnBindingRemoved(std::function<void(KeyCode)> callback);
    
private:
    // Core binding storage
    std::unordered_map<KeyCode, InputAction> keyToAction;
    std::unordered_map<InputAction, std::vector<KeyCode>> actionToKeys;
    
    // Event callbacks
    std::function<void(KeyCode, InputAction)> onBindingChanged;
    std::function<void(KeyCode)> onBindingRemoved;
    
    // Helper methods
    void addBinding(KeyCode key, InputAction action);
    void removeBinding(KeyCode key);
    void triggerBindingChangedCallback(KeyCode key, InputAction action);
    void triggerBindingRemovedCallback(KeyCode key);
    
    // String conversion maps
    void initializeStringMaps();
    std::unordered_map<KeyCode, std::string> keyCodeToStringMap;
    std::unordered_map<std::string, KeyCode> stringToKeyCodeMap;
    std::unordered_map<InputAction, std::string> actionToStringMap;
    std::unordered_map<std::string, InputAction> stringToActionMap;
};