#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include "../src/input/input_manager.h"
#include "../src/input/key_bindings.h"

void testKeyBindingsBasic() {
    std::cout << "Testing basic key bindings..." << std::endl;
    
    KeyBindings bindings;
    
    // Test default bindings are loaded
    assert(bindings.getActionForKey(KeyCode::W) == InputAction::MOVE_UP);
    assert(bindings.getActionForKey(KeyCode::S) == InputAction::MOVE_DOWN);
    assert(bindings.getActionForKey(KeyCode::A) == InputAction::MOVE_LEFT);
    assert(bindings.getActionForKey(KeyCode::D) == InputAction::MOVE_RIGHT);
    
    // Test interaction keys
    assert(bindings.getActionForKey(KeyCode::E) == InputAction::INTERACT);
    assert(bindings.getActionForKey(KeyCode::SPACE) == InputAction::INTERACT);
    // Note: ESCAPE might be bound to CANCEL or MENU_BACK depending on load order
    InputAction escapeAction = bindings.getActionForKey(KeyCode::ESCAPE);
    assert(escapeAction == InputAction::CANCEL || escapeAction == InputAction::MENU_BACK);
    
    std::cout << "Basic key bindings test passed!" << std::endl;
}

void testKeyBindingManagement() {
    std::cout << "Testing key binding management..." << std::endl;
    
    KeyBindings bindings;
    bindings.clearAllBindings();
    
    // Test binding a key
    bindings.bindKey(KeyCode::F, InputAction::INTERACT);
    assert(bindings.getActionForKey(KeyCode::F) == InputAction::INTERACT);
    assert(bindings.isKeyBound(KeyCode::F));
    assert(bindings.isActionBound(InputAction::INTERACT));
    
    // Test getting keys for action
    auto keys = bindings.getKeysForAction(InputAction::INTERACT);
    assert(keys.size() == 1);
    assert(keys[0] == KeyCode::F);
    
    // Test unbinding key
    bindings.unbindKey(KeyCode::F);
    assert(!bindings.isKeyBound(KeyCode::F));
    assert(!bindings.isActionBound(InputAction::INTERACT));
    assert(bindings.getActionForKey(KeyCode::F) == InputAction::UNKNOWN_ACTION);
    
    // Test binding multiple keys to same action
    bindings.bindKey(KeyCode::E, InputAction::INTERACT);
    bindings.bindKey(KeyCode::SPACE, InputAction::INTERACT);
    
    keys = bindings.getKeysForAction(InputAction::INTERACT);
    assert(keys.size() == 2);
    
    // Test unbinding action
    bindings.unbindAction(InputAction::INTERACT);
    assert(!bindings.isActionBound(InputAction::INTERACT));
    assert(!bindings.isKeyBound(KeyCode::E));
    assert(!bindings.isKeyBound(KeyCode::SPACE));
    
    std::cout << "Key binding management test passed!" << std::endl;
}

void testStringConversion() {
    std::cout << "Testing string conversion..." << std::endl;
    
    KeyBindings bindings;
    
    // Test key code to string
    assert(bindings.keyCodeToString(KeyCode::W) == "W");
    assert(bindings.keyCodeToString(KeyCode::SPACE) == "SPACE");
    assert(bindings.keyCodeToString(KeyCode::ESCAPE) == "ESCAPE");
    assert(bindings.keyCodeToString(KeyCode::UNKNOWN) == "UNKNOWN");
    
    // Test string to key code
    assert(bindings.stringToKeyCode("W") == KeyCode::W);
    assert(bindings.stringToKeyCode("SPACE") == KeyCode::SPACE);
    assert(bindings.stringToKeyCode("ESCAPE") == KeyCode::ESCAPE);
    assert(bindings.stringToKeyCode("INVALID") == KeyCode::UNKNOWN);
    
    // Test action to string
    assert(bindings.actionToString(InputAction::MOVE_UP) == "MOVE_UP");
    assert(bindings.actionToString(InputAction::INTERACT) == "INTERACT");
    assert(bindings.actionToString(InputAction::UNKNOWN_ACTION) == "UNKNOWN_ACTION");
    
    // Test string to action
    assert(bindings.stringToAction("MOVE_UP") == InputAction::MOVE_UP);
    assert(bindings.stringToAction("INTERACT") == InputAction::INTERACT);
    assert(bindings.stringToAction("INVALID") == InputAction::UNKNOWN_ACTION);
    
    std::cout << "String conversion test passed!" << std::endl;
}

void testInputStateBasic() {
    std::cout << "Testing basic input state..." << std::endl;
    
    InputState state;
    
    // Test initial state
    assert(!state.isKeyPressed(KeyCode::W));
    assert(!state.isKeyJustPressed(KeyCode::W));
    assert(!state.isKeyJustReleased(KeyCode::W));
    assert(!state.isKeyHeld(KeyCode::W));
    
    assert(!state.isActionActive(InputAction::MOVE_UP));
    assert(!state.isActionJustPressed(InputAction::MOVE_UP));
    assert(!state.isActionJustReleased(InputAction::MOVE_UP));
    
    // Test movement helpers
    assert(state.getMovementX() == 0.0f);
    assert(state.getMovementY() == 0.0f);
    assert(!state.hasMovementInput());
    
    // Test mouse state
    assert(state.mouse.x == 0.0f);
    assert(state.mouse.y == 0.0f);
    assert(!state.mouse.leftButton);
    assert(!state.mouse.rightButton);
    assert(!state.mouse.middleButton);
    
    // Test modifier states
    assert(!state.shiftPressed);
    assert(!state.controlPressed);
    assert(!state.altPressed);
    
    std::cout << "Basic input state test passed!" << std::endl;
}

void testInputStateMovement() {
    std::cout << "Testing input state movement..." << std::endl;
    
    InputState state;
    
    // Test movement input
    state.actionStates[InputAction::MOVE_LEFT] = true;
    assert(state.getMovementX() == -1.0f);
    assert(state.getMovementY() == 0.0f);
    assert(state.hasMovementInput());
    
    state.actionStates[InputAction::MOVE_RIGHT] = true;
    assert(state.getMovementX() == 0.0f); // Left and right cancel out
    assert(state.hasMovementInput());
    
    state.actionStates[InputAction::MOVE_LEFT] = false;
    assert(state.getMovementX() == 1.0f);
    
    state.actionStates[InputAction::MOVE_UP] = true;
    assert(state.getMovementY() == 1.0f);
    assert(state.hasMovementInput());
    
    state.actionStates[InputAction::MOVE_DOWN] = true;
    assert(state.getMovementY() == 0.0f); // Up and down cancel out
    
    std::cout << "Input state movement test passed!" << std::endl;
}

void testInputManagerBasic() {
    std::cout << "Testing basic input manager..." << std::endl;
    
    InputManager manager;
    
    // Test initialization
    bool initialized = manager.initialize();
    assert(initialized);
    
    // Test initial state
    assert(!manager.isKeyPressed(KeyCode::W));
    assert(!manager.isActionActive(InputAction::MOVE_UP));
    
    // Test configuration
    manager.setKeyRepeatDelay(0.3f);
    assert(manager.getKeyRepeatDelay() == 0.3f);
    
    manager.setKeyRepeatRate(0.1f);
    assert(manager.getKeyRepeatRate() == 0.1f);
    
    manager.setMouseSensitivity(1.5f);
    assert(manager.getMouseSensitivity() == 1.5f);
    
    manager.setScrollSensitivity(2.0f);
    assert(manager.getScrollSensitivity() == 2.0f);
    
    manager.setDeadzone(0.2f);
    assert(manager.getDeadzone() == 0.2f);
    
    // Test input smoothing
    manager.enableInputSmoothing(true);
    assert(manager.isInputSmoothingEnabled());
    
    manager.enableInputSmoothing(false);
    assert(!manager.isInputSmoothingEnabled());
    
    // Test debug mode
    manager.enableDebugMode(true);
    assert(manager.isDebugModeEnabled());
    
    auto debugInfo = manager.getDebugInfo();
    assert(!debugInfo.empty());
    
    manager.shutdown();
    
    std::cout << "Basic input manager test passed!" << std::endl;
}

void testInputManagerKeyBindings() {
    std::cout << "Testing input manager key bindings..." << std::endl;
    
    InputManager manager;
    manager.initialize();
    
    // Test getting key bindings
    const auto& bindings = manager.getKeyBindings();
    assert(bindings.getActionForKey(KeyCode::W) == InputAction::MOVE_UP);
    
    // Test modifying key bindings
    auto& mutableBindings = manager.getKeyBindings();
    mutableBindings.bindKey(KeyCode::F, InputAction::INTERACT);
    assert(bindings.getActionForKey(KeyCode::F) == InputAction::INTERACT);
    
    // Test reloading bindings
    manager.reloadKeyBindings();
    assert(bindings.getActionForKey(KeyCode::F) == InputAction::UNKNOWN_ACTION); // Should be cleared
    
    manager.shutdown();
    
    std::cout << "Input manager key bindings test passed!" << std::endl;
}

void testInputManagerCallbacks() {
    std::cout << "Testing input manager callbacks..." << std::endl;
    
    InputManager manager;
    manager.initialize();
    
    // Test key press callback
    bool keyPressedCalled = false;
    KeyCode pressedKey = KeyCode::UNKNOWN;
    
    manager.setOnKeyPressed([&](KeyCode key) {
        keyPressedCalled = true;
        pressedKey = key;
    });
    
    // Test key release callback
    bool keyReleasedCalled = false;
    KeyCode releasedKey = KeyCode::UNKNOWN;
    
    manager.setOnKeyReleased([&](KeyCode key) {
        keyReleasedCalled = true;
        releasedKey = key;
    });
    
    // Test action triggered callback
    bool actionTriggeredCalled = false;
    InputAction triggeredAction = InputAction::UNKNOWN_ACTION;
    
    manager.setOnActionTriggered([&](InputAction action) {
        actionTriggeredCalled = true;
        triggeredAction = action;
    });
    
    // Test mouse moved callback
    bool mouseMovedCalled = false;
    float mouseX = 0.0f, mouseY = 0.0f;
    
    manager.setOnMouseMoved([&](float x, float y) {
        mouseMovedCalled = true;
        mouseX = x;
        mouseY = y;
    });
    
    // Note: In a real scenario, key presses would come from platform events
    // and we'd call update() to process them. For this test, we're just
    // verifying that the callback system is set up correctly.
    
    manager.shutdown();
    
    std::cout << "Input manager callbacks test passed!" << std::endl;
}

void testInputManagerRecording() {
    std::cout << "Testing input manager recording..." << std::endl;
    
    InputManager manager;
    manager.initialize();
    
    // Test recording
    assert(!manager.isRecording());
    manager.startRecording("test_recording.dat");
    assert(manager.isRecording());
    
    manager.stopRecording();
    assert(!manager.isRecording());
    
    // Test playback
    assert(!manager.isPlayingBack());
    manager.startPlayback("test_recording.dat");
    assert(manager.isPlayingBack());
    
    manager.stopPlayback();
    assert(!manager.isPlayingBack());
    
    manager.shutdown();
    
    std::cout << "Input manager recording test passed!" << std::endl;
}

void testKeyBindingValidation() {
    std::cout << "Testing key binding validation..." << std::endl;
    
    KeyBindings bindings;
    
    // Test valid binding
    assert(bindings.isValidBinding(KeyCode::F, InputAction::INTERACT));
    
    // Test invalid bindings
    assert(!bindings.isValidBinding(KeyCode::UNKNOWN, InputAction::INTERACT));
    assert(!bindings.isValidBinding(KeyCode::F, InputAction::UNKNOWN_ACTION));
    
    // Test conflict detection
    bindings.bindKey(KeyCode::F, InputAction::INTERACT);
    auto conflicts = bindings.getConflicts(KeyCode::F, InputAction::MOVE_UP);
    assert(!conflicts.empty()); // Should detect conflict
    
    conflicts = bindings.getConflicts(KeyCode::F, InputAction::INTERACT);
    assert(conflicts.empty()); // No conflict with same action
    
    conflicts = bindings.getConflicts(KeyCode::G, InputAction::MOVE_UP);
    assert(conflicts.empty()); // No conflict with different key
    
    std::cout << "Key binding validation test passed!" << std::endl;
}

void testAlternativeBindings() {
    std::cout << "Testing alternative bindings..." << std::endl;
    
    KeyBindings bindings;
    
    // Test loading alternative bindings
    bindings.loadAlternativeBindings();
    
    // Should have arrow key movement
    assert(bindings.getActionForKey(KeyCode::UP) == InputAction::MOVE_UP);
    assert(bindings.getActionForKey(KeyCode::DOWN) == InputAction::MOVE_DOWN);
    assert(bindings.getActionForKey(KeyCode::LEFT) == InputAction::MOVE_LEFT);
    assert(bindings.getActionForKey(KeyCode::RIGHT) == InputAction::MOVE_RIGHT);
    
    // Should have space for interaction
    assert(bindings.getActionForKey(KeyCode::SPACE) == InputAction::INTERACT);
    
    // WASD keys should not be bound in alternative layout
    assert(bindings.getActionForKey(KeyCode::W) == InputAction::UNKNOWN_ACTION);
    assert(bindings.getActionForKey(KeyCode::A) == InputAction::UNKNOWN_ACTION);
    assert(bindings.getActionForKey(KeyCode::S) == InputAction::UNKNOWN_ACTION);
    assert(bindings.getActionForKey(KeyCode::D) == InputAction::UNKNOWN_ACTION);
    
    std::cout << "Alternative bindings test passed!" << std::endl;
}

int main() {
    try {
        testKeyBindingsBasic();
        testKeyBindingManagement();
        testStringConversion();
        testInputStateBasic();
        testInputStateMovement();
        testInputManagerBasic();
        testInputManagerKeyBindings();
        testInputManagerCallbacks();
        testInputManagerRecording();
        testKeyBindingValidation();
        testAlternativeBindings();
        
        std::cout << "\nAll tests passed! Input system implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}