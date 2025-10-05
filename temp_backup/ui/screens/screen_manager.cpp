#include "screen_manager.h"
#include <iostream>
#include <algorithm>

ScreenManager::ScreenManager() 
    : transitionType(TransitionType::NONE), transitionDuration(0.3f), 
      transitionTimer(0.0f), transitioning(false), debugMode(false) {
}

ScreenManager::~ScreenManager() {
    clearAllScreens();
}

// Screen stack management
void ScreenManager::pushScreen(std::unique_ptr<Screen> screen) {
    if (!screen || !isValidScreen(screen.get())) {
        if (debugMode) {
            std::cout << "ScreenManager: Invalid screen provided to pushScreen" << std::endl;
        }
        return;
    }
    
    std::string screenName = screen->getName();
    std::string previousScreenName;
    
    // Pause current screen if it exists
    if (!screens.empty()) {
        Screen* currentScreen = screens.top().get();
        previousScreenName = currentScreen->getName();
        pauseScreen(currentScreen);
    }
    
    // Add new screen
    screens.push(std::move(screen));
    Screen* newScreen = screens.top().get();
    
    if (transitionType != TransitionType::NONE) {
        startTransition(nullptr); // No new screen for push transition
    } else {
        enterScreen(newScreen);
    }
    
    triggerScreenPushedCallback(screenName);
    triggerScreenChangedCallback(previousScreenName, screenName);
    
    if (debugMode) {
        std::cout << "ScreenManager: Pushed screen '" << screenName << "'" << std::endl;
    }
}

void ScreenManager::popScreen() {
    if (screens.empty()) {
        if (debugMode) {
            std::cout << "ScreenManager: Cannot pop screen - stack is empty" << std::endl;
        }
        return;
    }
    
    Screen* currentScreen = screens.top().get();
    std::string screenName = currentScreen->getName();
    
    exitScreen(currentScreen);
    screens.pop();
    
    std::string newScreenName;
    
    // Resume previous screen if it exists
    if (!screens.empty()) {
        Screen* previousScreen = screens.top().get();
        newScreenName = previousScreen->getName();
        resumeScreen(previousScreen);
    }
    
    triggerScreenPoppedCallback(screenName);
    triggerScreenChangedCallback(screenName, newScreenName);
    
    if (debugMode) {
        std::cout << "ScreenManager: Popped screen '" << screenName << "'" << std::endl;
    }
}

void ScreenManager::replaceScreen(std::unique_ptr<Screen> screen) {
    if (!screen || !isValidScreen(screen.get())) {
        if (debugMode) {
            std::cout << "ScreenManager: Invalid screen provided to replaceScreen" << std::endl;
        }
        return;
    }
    
    std::string newScreenName = screen->getName();
    std::string oldScreenName;
    
    // Exit current screen if it exists
    if (!screens.empty()) {
        Screen* currentScreen = screens.top().get();
        oldScreenName = currentScreen->getName();
        exitScreen(currentScreen);
        screens.pop();
    }
    
    // Add new screen
    screens.push(std::move(screen));
    Screen* newScreen = screens.top().get();
    
    if (transitionType != TransitionType::NONE) {
        startTransition(nullptr);
    } else {
        enterScreen(newScreen);
    }
    
    triggerScreenChangedCallback(oldScreenName, newScreenName);
    
    if (debugMode) {
        std::cout << "ScreenManager: Replaced screen '" << oldScreenName 
                  << "' with '" << newScreenName << "'" << std::endl;
    }
}

void ScreenManager::clearAllScreens() {
    while (!screens.empty()) {
        popScreen();
    }
    
    if (debugMode) {
        std::cout << "ScreenManager: Cleared all screens" << std::endl;
    }
}

// Screen queries
Screen* ScreenManager::getCurrentScreen() const {
    return screens.empty() ? nullptr : screens.top().get();
}

Screen* ScreenManager::getPreviousScreen() const {
    if (screens.size() < 2) {
        return nullptr;
    }
    
    // This is tricky with std::stack - we'd need to copy the stack to access the second element
    // For now, return nullptr as this would require a more complex implementation
    return nullptr;
}

size_t ScreenManager::getScreenCount() const {
    return screens.size();
}

bool ScreenManager::hasScreens() const {
    return !screens.empty();
}

std::vector<std::string> ScreenManager::getScreenNames() const {
    std::vector<std::string> names;
    auto allScreens = getAllScreens();
    
    for (Screen* screen : allScreens) {
        if (screen) {
            names.push_back(screen->getName());
        }
    }
    
    return names;
}

// Screen finding
Screen* ScreenManager::findScreen(const std::string& name) const {
    auto allScreens = getAllScreens();
    
    for (Screen* screen : allScreens) {
        if (screen && screen->getName() == name) {
            return screen;
        }
    }
    
    return nullptr;
}

bool ScreenManager::hasScreen(const std::string& name) const {
    return findScreen(name) != nullptr;
}

// Core update loop
void ScreenManager::update(float deltaTime) {
    if (transitioning) {
        updateTransition(deltaTime);
        return;
    }
    
    if (screens.empty()) {
        return;
    }
    
    // Update current screen
    Screen* currentScreen = screens.top().get();
    if (currentScreen && (currentScreen->isActive() || currentScreen->shouldUpdateWhenPaused())) {
        currentScreen->update(deltaTime);
    }
    
    // Update paused screens that should update when paused
    auto allScreens = getAllScreens();
    for (Screen* screen : allScreens) {
        if (screen && screen != currentScreen && screen->isPaused() && screen->shouldUpdateWhenPaused()) {
            screen->update(deltaTime);
        }
    }
}

void ScreenManager::render() {
    if (screens.empty()) {
        return;
    }
    
    // Render all screens that should be rendered
    auto allScreens = getAllScreens();
    
    for (Screen* screen : allScreens) {
        if (screen && (screen->isActive() || (screen->isPaused() && screen->shouldRenderWhenPaused()))) {
            screen->render();
        }
    }
    
    // Render transition effects if transitioning
    if (transitioning) {
        // Transition rendering would be implemented here
        // For now, this is a placeholder
    }
}

void ScreenManager::handleInput(const InputState& input) {
    if (transitioning || screens.empty()) {
        return;
    }
    
    // Only the current active screen handles input
    Screen* currentScreen = screens.top().get();
    if (currentScreen && currentScreen->isActive()) {
        currentScreen->handleInput(input);
    }
}

// Screen state management
void ScreenManager::pauseCurrentScreen() {
    if (!screens.empty()) {
        Screen* currentScreen = screens.top().get();
        pauseScreen(currentScreen);
    }
}

void ScreenManager::resumeCurrentScreen() {
    if (!screens.empty()) {
        Screen* currentScreen = screens.top().get();
        resumeScreen(currentScreen);
    }
}

bool ScreenManager::isCurrentScreenPaused() const {
    if (screens.empty()) {
        return false;
    }
    
    Screen* currentScreen = screens.top().get();
    return currentScreen && currentScreen->isPaused();
}

// Transition effects
void ScreenManager::setTransitionType(TransitionType type) {
    transitionType = type;
    
    if (debugMode) {
        std::cout << "ScreenManager: Set transition type to " << static_cast<int>(type) << std::endl;
    }
}

ScreenManager::TransitionType ScreenManager::getTransitionType() const {
    return transitionType;
}

void ScreenManager::setTransitionDuration(float duration) {
    transitionDuration = std::max(0.0f, duration);
    
    if (debugMode) {
        std::cout << "ScreenManager: Set transition duration to " << transitionDuration << std::endl;
    }
}

float ScreenManager::getTransitionDuration() const {
    return transitionDuration;
}

bool ScreenManager::isTransitioning() const {
    return transitioning;
}

// Event callbacks
void ScreenManager::setOnScreenPushed(std::function<void(const std::string&)> callback) {
    onScreenPushed = std::move(callback);
}

void ScreenManager::setOnScreenPopped(std::function<void(const std::string&)> callback) {
    onScreenPopped = std::move(callback);
}

void ScreenManager::setOnScreenChanged(std::function<void(const std::string&, const std::string&)> callback) {
    onScreenChanged = std::move(callback);
}

void ScreenManager::setOnTransitionComplete(std::function<void()> callback) {
    onTransitionComplete = std::move(callback);
}

// Debug and diagnostics
void ScreenManager::enableDebugMode(bool enable) {
    debugMode = enable;
    
    if (debugMode) {
        std::cout << "ScreenManager: Debug mode " << (enable ? "enabled" : "disabled") << std::endl;
    }
}

bool ScreenManager::isDebugModeEnabled() const {
    return debugMode;
}

std::vector<std::string> ScreenManager::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== ScreenManager Debug Info ===");
    info.push_back("Screen Count: " + std::to_string(screens.size()));
    info.push_back("Transitioning: " + std::string(transitioning ? "Yes" : "No"));
    info.push_back("Transition Type: " + std::to_string(static_cast<int>(transitionType)));
    info.push_back("Transition Duration: " + std::to_string(transitionDuration));
    
    if (transitioning) {
        info.push_back("Transition Progress: " + std::to_string(calculateTransitionProgress()));
    }
    
    info.push_back("\n=== Screen Stack (top to bottom) ===");
    auto allScreens = getAllScreens();
    for (size_t i = 0; i < allScreens.size(); i++) {
        Screen* screen = allScreens[i];
        if (screen) {
            std::string status = screen->isActive() ? "Active" : (screen->isPaused() ? "Paused" : "Inactive");
            info.push_back(std::to_string(i) + ": " + screen->getName() + " (" + status + ")");
        }
    }
    
    return info;
}

void ScreenManager::logScreenStack() const {
    if (!debugMode) return;
    
    auto debugInfo = getDebugInfo();
    for (const auto& line : debugInfo) {
        std::cout << line << std::endl;
    }
    std::cout << "---" << std::endl;
}

// Private methods
void ScreenManager::startTransition(std::unique_ptr<Screen> newScreen) {
    transitioning = true;
    transitionTimer = 0.0f;
    transitioningScreen = std::move(newScreen);
    
    if (debugMode) {
        std::cout << "ScreenManager: Started transition" << std::endl;
    }
}

void ScreenManager::updateTransition(float deltaTime) {
    transitionTimer += deltaTime;
    
    if (transitionTimer >= transitionDuration) {
        completeTransition();
    }
}

void ScreenManager::completeTransition() {
    transitioning = false;
    transitionTimer = 0.0f;
    
    if (transitioningScreen) {
        enterScreen(transitioningScreen.get());
        transitioningScreen.reset();
    }
    
    triggerTransitionCompleteCallback();
    
    if (debugMode) {
        std::cout << "ScreenManager: Completed transition" << std::endl;
    }
}

// Screen lifecycle management
void ScreenManager::enterScreen(Screen* screen) {
    if (screen) {
        screen->setActive(true);
        screen->setPaused(false);
        screen->onEnter();
        
        if (debugMode) {
            std::cout << "ScreenManager: Entered screen '" << screen->getName() << "'" << std::endl;
        }
    }
}

void ScreenManager::exitScreen(Screen* screen) {
    if (screen) {
        screen->onExit();
        screen->setActive(false);
        screen->setPaused(false);
        
        if (debugMode) {
            std::cout << "ScreenManager: Exited screen '" << screen->getName() << "'" << std::endl;
        }
    }
}

void ScreenManager::pauseScreen(Screen* screen) {
    if (screen && screen->isActive()) {
        screen->setPaused(true);
        screen->setActive(false);
        screen->onPause();
        
        if (debugMode) {
            std::cout << "ScreenManager: Paused screen '" << screen->getName() << "'" << std::endl;
        }
    }
}

void ScreenManager::resumeScreen(Screen* screen) {
    if (screen && screen->isPaused()) {
        screen->setPaused(false);
        screen->setActive(true);
        screen->onResume();
        
        if (debugMode) {
            std::cout << "ScreenManager: Resumed screen '" << screen->getName() << "'" << std::endl;
        }
    }
}

// Callback triggers
void ScreenManager::triggerScreenPushedCallback(const std::string& screenName) {
    if (onScreenPushed) {
        onScreenPushed(screenName);
    }
}

void ScreenManager::triggerScreenPoppedCallback(const std::string& screenName) {
    if (onScreenPopped) {
        onScreenPopped(screenName);
    }
}

void ScreenManager::triggerScreenChangedCallback(const std::string& oldScreen, const std::string& newScreen) {
    if (onScreenChanged) {
        onScreenChanged(oldScreen, newScreen);
    }
}

void ScreenManager::triggerTransitionCompleteCallback() {
    if (onTransitionComplete) {
        onTransitionComplete();
    }
}

// Utility methods
std::vector<Screen*> ScreenManager::getAllScreens() const {
    std::vector<Screen*> allScreens;
    
    // Since we can't copy the stack of unique_ptrs, we'll just return the current screen
    // This is a limitation of using std::stack with unique_ptr
    if (!screens.empty()) {
        allScreens.push_back(screens.top().get());
    }
    
    return allScreens;
}

float ScreenManager::calculateTransitionProgress() const {
    if (transitionDuration <= 0.0f) {
        return 1.0f;
    }
    
    return std::min(1.0f, transitionTimer / transitionDuration);
}

// Validation
bool ScreenManager::isValidScreen(Screen* screen) const {
    return screen != nullptr && !screen->getName().empty();
}