#include "input_manager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

// InputState implementation
InputState::InputState() 
    : shiftPressed(false), controlPressed(false), altPressed(false), deltaTime(0.0f) {
    timestamp = std::chrono::steady_clock::now();
    
    // Initialize mouse state
    mouse.x = 0.0f;
    mouse.y = 0.0f;
    mouse.deltaX = 0.0f;
    mouse.deltaY = 0.0f;
    mouse.leftButton = false;
    mouse.rightButton = false;
    mouse.middleButton = false;
    mouse.scrollX = 0.0f;
    mouse.scrollY = 0.0f;
}

bool InputState::isKeyPressed(KeyCode key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == KeyState::PRESSED || it->second == KeyState::HELD;
}

bool InputState::isKeyJustPressed(KeyCode key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == KeyState::JUST_PRESSED;
}

bool InputState::isKeyJustReleased(KeyCode key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == KeyState::JUST_RELEASED;
}

bool InputState::isKeyHeld(KeyCode key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == KeyState::HELD;
}

bool InputState::isActionActive(InputAction action) const {
    auto it = actionStates.find(action);
    return it != actionStates.end() && it->second;
}

bool InputState::isActionJustPressed(InputAction action) const {
    auto it = actionJustPressed.find(action);
    return it != actionJustPressed.end() && it->second;
}

bool InputState::isActionJustReleased(InputAction action) const {
    auto it = actionJustReleased.find(action);
    return it != actionJustReleased.end() && it->second;
}

float InputState::getMovementX() const {
    float movement = 0.0f;
    if (isActionActive(InputAction::MOVE_LEFT)) movement -= 1.0f;
    if (isActionActive(InputAction::MOVE_RIGHT)) movement += 1.0f;
    return movement;
}

float InputState::getMovementY() const {
    float movement = 0.0f;
    if (isActionActive(InputAction::MOVE_DOWN)) movement -= 1.0f;
    if (isActionActive(InputAction::MOVE_UP)) movement += 1.0f;
    return movement;
}

bool InputState::hasMovementInput() const {
    return isActionActive(InputAction::MOVE_UP) || 
           isActionActive(InputAction::MOVE_DOWN) ||
           isActionActive(InputAction::MOVE_LEFT) || 
           isActionActive(InputAction::MOVE_RIGHT);
}

// InputManager implementation
InputManager::InputManager() 
    : keyRepeatDelay(0.5f), keyRepeatRate(0.05f), mouseSensitivity(1.0f), 
      scrollSensitivity(1.0f), deadzone(0.1f), inputSmoothingEnabled(false),
      debugMode(false), platformWindow(nullptr), recording(false), 
      playingBack(false), playbackIndex(0) {
}

InputManager::~InputManager() {
    shutdown();
}

bool InputManager::initialize() {
    // Initialize platform-specific input system
    if (!initializePlatform()) {
        return false;
    }
    
    // Load key bindings
    keyBindings.loadDefaultBindings();
    
    return true;
}

void InputManager::shutdown() {
    if (recording) {
        stopRecording();
    }
    
    if (playingBack) {
        stopPlayback();
    }
    
    shutdownPlatform();
}

void InputManager::update(float deltaTime) {
    // Store previous state
    previousState = currentState;
    currentState.deltaTime = deltaTime;
    currentState.timestamp = std::chrono::steady_clock::now();
    
    // Process platform events
    if (!playingBack) {
        processEvents();
    } else {
        playbackNextState();
    }
    
    // Update internal state
    updateKeyStates(deltaTime);
    updateActionStates();
    updateMouseState();
    updateModifierStates();
    processKeyRepeat(deltaTime);
    
    // Apply input processing
    if (inputSmoothingEnabled) {
        applyInputSmoothing();
    }
    applyDeadzone();
    
    // Record state if recording
    if (recording) {
        recordCurrentState();
    }
    
    // Debug logging
    if (debugMode) {
        logInputState();
    }
}

void InputManager::processEvents() {
    processEventsImpl();
}

// Input state access
const InputState& InputManager::getCurrentState() const {
    return currentState;
}

const InputState& InputManager::getPreviousState() const {
    return previousState;
}

// Key state queries
bool InputManager::isKeyPressed(KeyCode key) const {
    return currentState.isKeyPressed(key);
}

bool InputManager::isKeyJustPressed(KeyCode key) const {
    return currentState.isKeyJustPressed(key);
}

bool InputManager::isKeyJustReleased(KeyCode key) const {
    return currentState.isKeyJustReleased(key);
}

bool InputManager::isKeyHeld(KeyCode key) const {
    return currentState.isKeyHeld(key);
}

KeyState InputManager::getKeyState(KeyCode key) const {
    auto it = currentState.keyStates.find(key);
    return it != currentState.keyStates.end() ? it->second : KeyState::RELEASED;
}

// Action state queries
bool InputManager::isActionActive(InputAction action) const {
    return currentState.isActionActive(action);
}

bool InputManager::isActionJustPressed(InputAction action) const {
    return currentState.isActionJustPressed(action);
}

bool InputManager::isActionJustReleased(InputAction action) const {
    return currentState.isActionJustReleased(action);
}

// Mouse state queries
float InputManager::getMouseX() const {
    return currentState.mouse.x;
}

float InputManager::getMouseY() const {
    return currentState.mouse.y;
}

float InputManager::getMouseDeltaX() const {
    return currentState.mouse.deltaX;
}

float InputManager::getMouseDeltaY() const {
    return currentState.mouse.deltaY;
}

bool InputManager::isMouseButtonPressed(int button) const {
    switch (button) {
        case 0: return currentState.mouse.leftButton;
        case 1: return currentState.mouse.rightButton;
        case 2: return currentState.mouse.middleButton;
        default: return false;
    }
}

bool InputManager::isMouseButtonJustPressed(int button) const {
    bool currentPressed = isMouseButtonPressed(button);
    bool previousPressed = false;
    
    switch (button) {
        case 0: previousPressed = previousState.mouse.leftButton; break;
        case 1: previousPressed = previousState.mouse.rightButton; break;
        case 2: previousPressed = previousState.mouse.middleButton; break;
    }
    
    return currentPressed && !previousPressed;
}

bool InputManager::isMouseButtonJustReleased(int button) const {
    bool currentPressed = isMouseButtonPressed(button);
    bool previousPressed = false;
    
    switch (button) {
        case 0: previousPressed = previousState.mouse.leftButton; break;
        case 1: previousPressed = previousState.mouse.rightButton; break;
        case 2: previousPressed = previousState.mouse.middleButton; break;
    }
    
    return !currentPressed && previousPressed;
}

float InputManager::getScrollX() const {
    return currentState.mouse.scrollX;
}

float InputManager::getScrollY() const {
    return currentState.mouse.scrollY;
}

// Modifier key queries
bool InputManager::isShiftPressed() const {
    return currentState.shiftPressed;
}

bool InputManager::isControlPressed() const {
    return currentState.controlPressed;
}

bool InputManager::isAltPressed() const {
    return currentState.altPressed;
}

// Key binding management
KeyBindings& InputManager::getKeyBindings() {
    return keyBindings;
}

const KeyBindings& InputManager::getKeyBindings() const {
    return keyBindings;
}

void InputManager::reloadKeyBindings() {
    keyBindings.loadDefaultBindings();
}

// Input configuration
void InputManager::setKeyRepeatDelay(float delay) {
    keyRepeatDelay = std::max(0.0f, delay);
}

void InputManager::setKeyRepeatRate(float rate) {
    keyRepeatRate = std::max(0.01f, rate);
}

float InputManager::getKeyRepeatDelay() const {
    return keyRepeatDelay;
}

float InputManager::getKeyRepeatRate() const {
    return keyRepeatRate;
}

void InputManager::setMouseSensitivity(float sensitivity) {
    mouseSensitivity = std::max(0.1f, sensitivity);
}

float InputManager::getMouseSensitivity() const {
    return mouseSensitivity;
}

void InputManager::setScrollSensitivity(float sensitivity) {
    scrollSensitivity = std::max(0.1f, sensitivity);
}

float InputManager::getScrollSensitivity() const {
    return scrollSensitivity;
}

void InputManager::setDeadzone(float deadzone) {
    this->deadzone = std::clamp(deadzone, 0.0f, 0.5f);
}

float InputManager::getDeadzone() const {
    return deadzone;
}

void InputManager::enableInputSmoothing(bool enable) {
    inputSmoothingEnabled = enable;
}

bool InputManager::isInputSmoothingEnabled() const {
    return inputSmoothingEnabled;
}

// Event callbacks
void InputManager::setOnKeyPressed(std::function<void(KeyCode)> callback) {
    onKeyPressed = std::move(callback);
}

void InputManager::setOnKeyReleased(std::function<void(KeyCode)> callback) {
    onKeyReleased = std::move(callback);
}

void InputManager::setOnActionTriggered(std::function<void(InputAction)> callback) {
    onActionTriggered = std::move(callback);
}

void InputManager::setOnActionReleased(std::function<void(InputAction)> callback) {
    onActionReleased = std::move(callback);
}

void InputManager::setOnMouseMoved(std::function<void(float, float)> callback) {
    onMouseMoved = std::move(callback);
}

void InputManager::setOnMouseClicked(std::function<void(int, float, float)> callback) {
    onMouseClicked = std::move(callback);
}

void InputManager::setOnScrolled(std::function<void(float, float)> callback) {
    onScrolled = std::move(callback);
}

// Input recording and playback
void InputManager::startRecording(const std::string& filename) {
    if (recording) {
        stopRecording();
    }
    
    recordingFilename = filename;
    recordedStates.clear();
    recording = true;
}

void InputManager::stopRecording() {
    if (!recording) return;
    
    recording = false;
    saveRecording();
}

void InputManager::startPlayback(const std::string& filename) {
    if (playingBack) {
        stopPlayback();
    }
    
    playbackFilename = filename;
    playbackIndex = 0;
    loadPlayback();
    playingBack = true;
}

void InputManager::stopPlayback() {
    playingBack = false;
    playbackIndex = 0;
}

bool InputManager::isRecording() const {
    return recording;
}

bool InputManager::isPlayingBack() const {
    return playingBack;
}

// Debug and diagnostics
void InputManager::enableDebugMode(bool enable) {
    debugMode = enable;
}

bool InputManager::isDebugModeEnabled() const {
    return debugMode;
}

std::vector<std::string> InputManager::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== Input Manager Debug Info ===");
    info.push_back("Key Repeat Delay: " + std::to_string(keyRepeatDelay));
    info.push_back("Key Repeat Rate: " + std::to_string(keyRepeatRate));
    info.push_back("Mouse Sensitivity: " + std::to_string(mouseSensitivity));
    info.push_back("Scroll Sensitivity: " + std::to_string(scrollSensitivity));
    info.push_back("Deadzone: " + std::to_string(deadzone));
    info.push_back("Input Smoothing: " + std::string(inputSmoothingEnabled ? "Enabled" : "Disabled"));
    info.push_back("Recording: " + std::string(recording ? "Yes" : "No"));
    info.push_back("Playback: " + std::string(playingBack ? "Yes" : "No"));
    
    info.push_back("\n=== Active Keys ===");
    for (const auto& [key, state] : currentState.keyStates) {
        if (state != KeyState::RELEASED) {
            info.push_back(keyBindings.keyCodeToString(key) + ": " + keyStateToString(state));
        }
    }
    
    info.push_back("\n=== Active Actions ===");
    for (const auto& [action, active] : currentState.actionStates) {
        if (active) {
            info.push_back(keyBindings.actionToString(action));
        }
    }
    
    info.push_back("\n=== Mouse State ===");
    info.push_back("Position: (" + std::to_string(currentState.mouse.x) + ", " + std::to_string(currentState.mouse.y) + ")");
    info.push_back("Delta: (" + std::to_string(currentState.mouse.deltaX) + ", " + std::to_string(currentState.mouse.deltaY) + ")");
    info.push_back("Left Button: " + std::string(currentState.mouse.leftButton ? "Pressed" : "Released"));
    info.push_back("Right Button: " + std::string(currentState.mouse.rightButton ? "Pressed" : "Released"));
    info.push_back("Middle Button: " + std::string(currentState.mouse.middleButton ? "Pressed" : "Released"));
    
    return info;
}

void InputManager::logInputState() const {
    if (!debugMode) return;
    
    // Only log when there's activity to avoid spam
    bool hasActivity = false;
    
    // Check for key activity
    for (const auto& [key, state] : currentState.keyStates) {
        if (state == KeyState::JUST_PRESSED || state == KeyState::JUST_RELEASED) {
            hasActivity = true;
            break;
        }
    }
    
    // Check for mouse activity
    if (std::abs(currentState.mouse.deltaX) > 0.1f || std::abs(currentState.mouse.deltaY) > 0.1f) {
        hasActivity = true;
    }
    
    if (hasActivity) {
        auto debugInfo = getDebugInfo();
        for (const auto& line : debugInfo) {
            std::cout << line << std::endl;
        }
        std::cout << "---" << std::endl;
    }
}

// Platform-specific methods
void InputManager::setPlatformWindow(void* window) {
    platformWindow = window;
}

void* InputManager::getPlatformWindow() const {
    return platformWindow;
}

// Private methods
void InputManager::updateKeyStates(float deltaTime) {
    // Update key states based on transitions
    for (auto& [key, state] : currentState.keyStates) {
        switch (state) {
            case KeyState::JUST_PRESSED:
                state = KeyState::PRESSED;
                break;
            case KeyState::JUST_RELEASED:
                state = KeyState::RELEASED;
                break;
            case KeyState::PRESSED:
                state = KeyState::HELD;
                break;
            default:
                break;
        }
    }
}

void InputManager::updateActionStates() {
    // Clear previous frame's action states
    currentState.actionStates.clear();
    currentState.actionJustPressed.clear();
    currentState.actionJustReleased.clear();
    
    // Update action states based on key bindings
    for (const auto& [key, state] : currentState.keyStates) {
        InputAction action = keyBindings.getActionForKey(key);
        if (action == InputAction::UNKNOWN_ACTION) continue;
        
        bool isActive = (state == KeyState::PRESSED || state == KeyState::HELD);
        bool justPressed = (state == KeyState::JUST_PRESSED);
        bool justReleased = (state == KeyState::JUST_RELEASED);
        
        // Set action state (OR with existing to handle multiple keys for same action)
        currentState.actionStates[action] = currentState.actionStates[action] || isActive;
        currentState.actionJustPressed[action] = currentState.actionJustPressed[action] || justPressed;
        currentState.actionJustReleased[action] = currentState.actionJustReleased[action] || justReleased;
        
        // Trigger callbacks
        if (justPressed) {
            triggerActionTriggeredCallback(action);
        }
        if (justReleased) {
            triggerActionReleasedCallback(action);
        }
    }
}

void InputManager::updateMouseState() {
    // Mouse delta is calculated in platform-specific code
    // Apply sensitivity
    currentState.mouse.deltaX *= mouseSensitivity;
    currentState.mouse.deltaY *= mouseSensitivity;
    currentState.mouse.scrollX *= scrollSensitivity;
    currentState.mouse.scrollY *= scrollSensitivity;
}

void InputManager::updateModifierStates() {
    currentState.shiftPressed = isKeyPressed(KeyCode::LEFT_SHIFT) || isKeyPressed(KeyCode::RIGHT_SHIFT);
    currentState.controlPressed = isKeyPressed(KeyCode::LEFT_CONTROL) || isKeyPressed(KeyCode::RIGHT_CONTROL);
    currentState.altPressed = isKeyPressed(KeyCode::LEFT_ALT) || isKeyPressed(KeyCode::RIGHT_ALT);
}

void InputManager::processKeyRepeat(float deltaTime) {
    for (auto& [key, repeatState] : keyRepeatStates) {
        KeyState keyState = getKeyState(key);
        
        if (keyState == KeyState::JUST_PRESSED) {
            repeatState.isRepeating = false;
            repeatState.timeSincePress = 0.0f;
            repeatState.timeSinceLastRepeat = 0.0f;
        } else if (keyState == KeyState::HELD) {
            repeatState.timeSincePress += deltaTime;
            
            if (!repeatState.isRepeating && repeatState.timeSincePress >= keyRepeatDelay) {
                repeatState.isRepeating = true;
                repeatState.timeSinceLastRepeat = 0.0f;
                // Trigger repeat event
                handleKeyPress(key);
            } else if (repeatState.isRepeating) {
                repeatState.timeSinceLastRepeat += deltaTime;
                if (repeatState.timeSinceLastRepeat >= keyRepeatRate) {
                    repeatState.timeSinceLastRepeat = 0.0f;
                    // Trigger repeat event
                    handleKeyPress(key);
                }
            }
        } else if (keyState == KeyState::RELEASED) {
            repeatState.isRepeating = false;
        }
    }
}

void InputManager::applyInputSmoothing() {
    // Simple smoothing for mouse movement
    const float smoothingFactor = 0.8f;
    currentState.mouse.deltaX = currentState.mouse.deltaX * smoothingFactor + 
                               previousState.mouse.deltaX * (1.0f - smoothingFactor);
    currentState.mouse.deltaY = currentState.mouse.deltaY * smoothingFactor + 
                               previousState.mouse.deltaY * (1.0f - smoothingFactor);
}

void InputManager::applyDeadzone() {
    // Apply deadzone to mouse movement
    if (std::abs(currentState.mouse.deltaX) < deadzone) {
        currentState.mouse.deltaX = 0.0f;
    }
    if (std::abs(currentState.mouse.deltaY) < deadzone) {
        currentState.mouse.deltaY = 0.0f;
    }
}

// Event processing helpers
void InputManager::handleKeyPress(KeyCode key) {
    auto& keyState = currentState.keyStates[key];
    if (keyState == KeyState::RELEASED) {
        keyState = KeyState::JUST_PRESSED;
        
        // Initialize repeat state if not exists
        if (keyRepeatStates.find(key) == keyRepeatStates.end()) {
            keyRepeatStates[key] = {false, 0.0f, 0.0f};
        }
        
        triggerKeyPressedCallback(key);
    }
}

void InputManager::handleKeyRelease(KeyCode key) {
    auto& keyState = currentState.keyStates[key];
    if (keyState != KeyState::RELEASED) {
        keyState = KeyState::JUST_RELEASED;
        triggerKeyReleasedCallback(key);
    }
}

void InputManager::handleMouseMove(float x, float y) {
    currentState.mouse.deltaX = x - currentState.mouse.x;
    currentState.mouse.deltaY = y - currentState.mouse.y;
    currentState.mouse.x = x;
    currentState.mouse.y = y;
    
    triggerMouseMovedCallback(x, y);
}

void InputManager::handleMouseButton(int button, bool pressed) {
    bool* buttonState = nullptr;
    switch (button) {
        case 0: buttonState = &currentState.mouse.leftButton; break;
        case 1: buttonState = &currentState.mouse.rightButton; break;
        case 2: buttonState = &currentState.mouse.middleButton; break;
        default: return;
    }
    
    if (*buttonState != pressed) {
        *buttonState = pressed;
        if (pressed) {
            triggerMouseClickedCallback(button, currentState.mouse.x, currentState.mouse.y);
        }
    }
}

void InputManager::handleScroll(float x, float y) {
    currentState.mouse.scrollX = x;
    currentState.mouse.scrollY = y;
    triggerScrolledCallback(x, y);
}

// Callback triggers
void InputManager::triggerKeyPressedCallback(KeyCode key) {
    if (onKeyPressed) {
        onKeyPressed(key);
    }
}

void InputManager::triggerKeyReleasedCallback(KeyCode key) {
    if (onKeyReleased) {
        onKeyReleased(key);
    }
}

void InputManager::triggerActionTriggeredCallback(InputAction action) {
    if (onActionTriggered) {
        onActionTriggered(action);
    }
}

void InputManager::triggerActionReleasedCallback(InputAction action) {
    if (onActionReleased) {
        onActionReleased(action);
    }
}

void InputManager::triggerMouseMovedCallback(float x, float y) {
    if (onMouseMoved) {
        onMouseMoved(x, y);
    }
}

void InputManager::triggerMouseClickedCallback(int button, float x, float y) {
    if (onMouseClicked) {
        onMouseClicked(button, x, y);
    }
}

void InputManager::triggerScrolledCallback(float x, float y) {
    if (onScrolled) {
        onScrolled(x, y);
    }
}

// Recording/playback helpers
void InputManager::recordCurrentState() {
    recordedStates.push_back(currentState);
}

void InputManager::playbackNextState() {
    if (playbackIndex < recordedStates.size()) {
        currentState = recordedStates[playbackIndex];
        playbackIndex++;
    } else {
        stopPlayback();
    }
}

void InputManager::saveRecording() const {
    // Implementation would save recorded states to file
    // For now, just a placeholder
}

void InputManager::loadPlayback() {
    // Implementation would load recorded states from file
    // For now, just a placeholder
}

// Debug helpers
std::string InputManager::keyStateToString(KeyState state) const {
    switch (state) {
        case KeyState::RELEASED: return "Released";
        case KeyState::PRESSED: return "Pressed";
        case KeyState::HELD: return "Held";
        case KeyState::JUST_PRESSED: return "Just Pressed";
        case KeyState::JUST_RELEASED: return "Just Released";
        default: return "Unknown";
    }
}

std::string InputManager::inputActionToString(InputAction action) const {
    return keyBindings.actionToString(action);
}

// Platform-specific implementations (stubs for now)
bool InputManager::initializePlatform() {
    // Platform-specific initialization
    return true;
}

void InputManager::shutdownPlatform() {
    // Platform-specific cleanup
}

void InputManager::processEventsImpl() {
    // Platform-specific event processing
    // This would be implemented differently for each platform (Windows, macOS, Linux)
    // For now, this is a stub
}

KeyCode InputManager::platformKeyToKeyCode(int platformKey) const {
    // Platform-specific key code conversion
    return static_cast<KeyCode>(platformKey);
}

int InputManager::keyCodeToPlatformKey(KeyCode key) const {
    // Platform-specific key code conversion
    return static_cast<int>(key);
}