#pragma once

#include "screen.h"
#include <memory>
#include <stack>
#include <vector>
#include <functional>

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();
    
    // Screen stack management
    void pushScreen(std::unique_ptr<Screen> screen);
    void popScreen();
    void replaceScreen(std::unique_ptr<Screen> screen);
    void clearAllScreens();
    
    // Screen queries
    Screen* getCurrentScreen() const;
    Screen* getPreviousScreen() const;
    size_t getScreenCount() const;
    bool hasScreens() const;
    std::vector<std::string> getScreenNames() const;
    
    // Screen finding
    Screen* findScreen(const std::string& name) const;
    bool hasScreen(const std::string& name) const;
    
    // Core update loop
    void update(float deltaTime);
    void render();
    void handleInput(const InputState& input);
    
    // Screen state management
    void pauseCurrentScreen();
    void resumeCurrentScreen();
    bool isCurrentScreenPaused() const;
    
    // Transition effects
    enum class TransitionType {
        NONE,
        FADE,
        SLIDE_LEFT,
        SLIDE_RIGHT,
        SLIDE_UP,
        SLIDE_DOWN
    };
    
    void setTransitionType(TransitionType type);
    TransitionType getTransitionType() const;
    void setTransitionDuration(float duration);
    float getTransitionDuration() const;
    bool isTransitioning() const;
    
    // Event callbacks
    void setOnScreenPushed(std::function<void(const std::string&)> callback);
    void setOnScreenPopped(std::function<void(const std::string&)> callback);
    void setOnScreenChanged(std::function<void(const std::string&, const std::string&)> callback);
    void setOnTransitionComplete(std::function<void()> callback);
    
    // Debug and diagnostics
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const;
    std::vector<std::string> getDebugInfo() const;
    void logScreenStack() const;
    
private:
    // Screen stack
    std::stack<std::unique_ptr<Screen>> screens;
    
    // Transition state
    TransitionType transitionType;
    float transitionDuration;
    float transitionTimer;
    bool transitioning;
    std::unique_ptr<Screen> transitioningScreen;
    
    // Debug
    bool debugMode;
    
    // Event callbacks
    std::function<void(const std::string&)> onScreenPushed;
    std::function<void(const std::string&)> onScreenPopped;
    std::function<void(const std::string&, const std::string&)> onScreenChanged;
    std::function<void()> onTransitionComplete;
    
    // Internal methods
    void startTransition(std::unique_ptr<Screen> newScreen);
    void updateTransition(float deltaTime);
    void completeTransition();
    
    // Screen lifecycle management
    void enterScreen(Screen* screen);
    void exitScreen(Screen* screen);
    void pauseScreen(Screen* screen);
    void resumeScreen(Screen* screen);
    
    // Callback triggers
    void triggerScreenPushedCallback(const std::string& screenName);
    void triggerScreenPoppedCallback(const std::string& screenName);
    void triggerScreenChangedCallback(const std::string& oldScreen, const std::string& newScreen);
    void triggerTransitionCompleteCallback();
    
    // Utility methods
    std::vector<Screen*> getAllScreens() const;
    float calculateTransitionProgress() const;
    
    // Validation
    bool isValidScreen(Screen* screen) const;
};