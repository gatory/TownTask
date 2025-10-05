#pragma once

#include "../../input/input_manager.h"

// Forward declarations
struct InputState;

// Base Screen class for all game screens
class Screen {
public:
    virtual ~Screen() = default;
    
    // Core screen lifecycle methods
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(const InputState& input) = 0;
    
    // Screen transition methods
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPause() {}
    virtual void onResume() {}
    
    // Screen state queries
    virtual bool isActive() const { return active; }
    virtual bool isPaused() const { return paused; }
    virtual std::string getName() const { return screenName; }
    
    // Screen configuration
    virtual bool shouldUpdateWhenPaused() const { return false; }
    virtual bool shouldRenderWhenPaused() const { return true; }
    virtual bool isModal() const { return false; }
    
    // Screen state management (public for testing)
    void setActive(bool isActive) { active = isActive; }
    void setPaused(bool isPaused) { paused = isPaused; }
    void setName(const std::string& name) { screenName = name; }
    
protected:
    Screen(const std::string& name) : screenName(name), active(false), paused(false) {}
    
private:
    std::string screenName;
    bool active;
    bool paused;
    
    friend class ScreenManager;
};