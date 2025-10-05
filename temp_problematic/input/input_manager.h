#pragma once

#include "key_bindings.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>

// Input state structure for easy querying
struct InputState {
    // Key states
    std::unordered_map<KeyCode, KeyState> keyStates;
    std::unordered_map<InputAction, bool> actionStates;
    std::unordered_map<InputAction, bool> actionJustPressed;
    std::unordered_map<InputAction, bool> actionJustReleased;
    
    // Mouse state
    struct MouseState {
        float x, y;
        float deltaX, deltaY;
        bool leftButton;
        bool rightButton;
        bool middleButton;
        float scrollX, scrollY;
    } mouse;
    
    // Modifier keys
    bool shiftPressed;
    bool controlPressed;
    bool altPressed;
    
    // Timing
    std::chrono::steady_clock::time_point timestamp;
    float deltaTime;
    
    InputState();
    
    // Convenience methods
    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    bool isKeyJustReleased(KeyCode key) const;
    bool isKeyHeld(KeyCode key) const;
    
    bool isActionActive(InputAction action) const;
    bool isActionJustPressed(InputAction action) const;
    bool isActionJustReleased(InputAction action) const;
    
    // Movement helpers
    float getMovementX() const;
    float getMovementY() const;
    bool hasMovementInput() const;
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    // Initialization and cleanup
    bool initialize();
    void shutdown();
    
    // Main update loop
    void update(float deltaTime);
    void processEvents(); // Platform-specific event processing
    
    // Input state access
    const InputState& getCurrentState() const;
    const InputState& getPreviousState() const;
    
    // Key state queries
    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    bool isKeyJustReleased(KeyCode key) const;
    bool isKeyHeld(KeyCode key) const;
    KeyState getKeyState(KeyCode key) const;
    
    // Action state queries
    bool isActionActive(InputAction action) const;
    bool isActionJustPressed(InputAction action) const;
    bool isActionJustReleased(InputAction action) const;
    
    // Mouse state queries
    float getMouseX() const;
    float getMouseY() const;
    float getMouseDeltaX() const;
    float getMouseDeltaY() const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;
    float getScrollX() const;
    float getScrollY() const;
    
    // Modifier key queries
    bool isShiftPressed() const;
    bool isControlPressed() const;
    bool isAltPressed() const;
    
    // Key binding management
    KeyBindings& getKeyBindings();
    const KeyBindings& getKeyBindings() const;
    void reloadKeyBindings();
    
    // Input configuration
    void setKeyRepeatDelay(float delay);
    void setKeyRepeatRate(float rate);
    float getKeyRepeatDelay() const;
    float getKeyRepeatRate() const;
    
    // Input sensitivity
    void setMouseSensitivity(float sensitivity);
    float getMouseSensitivity() const;
    void setScrollSensitivity(float sensitivity);
    float getScrollSensitivity() const;
    
    // Input filtering and processing
    void setDeadzone(float deadzone);
    float getDeadzone() const;
    void enableInputSmoothing(bool enable);
    bool isInputSmoothingEnabled() const;
    
    // Event callbacks
    void setOnKeyPressed(std::function<void(KeyCode)> callback);
    void setOnKeyReleased(std::function<void(KeyCode)> callback);
    void setOnActionTriggered(std::function<void(InputAction)> callback);
    void setOnActionReleased(std::function<void(InputAction)> callback);
    void setOnMouseMoved(std::function<void(float, float)> callback);
    void setOnMouseClicked(std::function<void(int, float, float)> callback);
    void setOnScrolled(std::function<void(float, float)> callback);
    
    // Input recording and playback (for testing/debugging)
    void startRecording(const std::string& filename);
    void stopRecording();
    void startPlayback(const std::string& filename);
    void stopPlayback();
    bool isRecording() const;
    bool isPlayingBack() const;
    
    // Debug and diagnostics
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const;
    std::vector<std::string> getDebugInfo() const;
    void logInputState() const;
    
    // Platform-specific methods (to be implemented per platform)
    void setPlatformWindow(void* window);
    void* getPlatformWindow() const;
    
private:
    // Core state
    InputState currentState;
    InputState previousState;
    KeyBindings keyBindings;
    
    // Configuration
    float keyRepeatDelay;
    float keyRepeatRate;
    float mouseSensitivity;
    float scrollSensitivity;
    float deadzone;
    bool inputSmoothingEnabled;
    bool debugMode;
    
    // Platform-specific data
    void* platformWindow;
    
    // Key repeat tracking
    struct KeyRepeatState {
        bool isRepeating;
        float timeSincePress;
        float timeSinceLastRepeat;
    };
    std::unordered_map<KeyCode, KeyRepeatState> keyRepeatStates;
    
    // Event callbacks
    std::function<void(KeyCode)> onKeyPressed;
    std::function<void(KeyCode)> onKeyReleased;
    std::function<void(InputAction)> onActionTriggered;
    std::function<void(InputAction)> onActionReleased;
    std::function<void(float, float)> onMouseMoved;
    std::function<void(int, float, float)> onMouseClicked;
    std::function<void(float, float)> onScrolled;
    
    // Recording/playback
    bool recording;
    bool playingBack;
    std::string recordingFilename;
    std::string playbackFilename;
    std::vector<InputState> recordedStates;
    size_t playbackIndex;
    
    // Internal methods
    void updateKeyStates(float deltaTime);
    void updateActionStates();
    void updateMouseState();
    void updateModifierStates();
    void processKeyRepeat(float deltaTime);
    void applyInputSmoothing();
    void applyDeadzone();
    
    // Event processing helpers
    void handleKeyPress(KeyCode key);
    void handleKeyRelease(KeyCode key);
    void handleMouseMove(float x, float y);
    void handleMouseButton(int button, bool pressed);
    void handleScroll(float x, float y);
    
    // Callback triggers
    void triggerKeyPressedCallback(KeyCode key);
    void triggerKeyReleasedCallback(KeyCode key);
    void triggerActionTriggeredCallback(InputAction action);
    void triggerActionReleasedCallback(InputAction action);
    void triggerMouseMovedCallback(float x, float y);
    void triggerMouseClickedCallback(int button, float x, float y);
    void triggerScrolledCallback(float x, float y);
    
    // Recording/playback helpers
    void recordCurrentState();
    void playbackNextState();
    void saveRecording() const;
    void loadPlayback();
    
    // Debug helpers
    std::string keyStateToString(KeyState state) const;
    std::string inputActionToString(InputAction action) const;
    
    // Platform-specific implementations (to be defined in platform-specific files)
    bool initializePlatform();
    void shutdownPlatform();
    void processEventsImpl();
    KeyCode platformKeyToKeyCode(int platformKey) const;
    int keyCodeToPlatformKey(KeyCode key) const;
};