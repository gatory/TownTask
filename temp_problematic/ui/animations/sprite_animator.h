#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

// Simple Rectangle structure for sprite frames
struct Rectangle {
    float x, y, width, height;
    
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
};

// Simple Texture2D structure for sprite sheets
struct Texture2D {
    unsigned int id;
    int width, height;
    std::string filename;
    
    Texture2D() : id(0), width(0), height(0) {}
    Texture2D(unsigned int id, int w, int h, const std::string& file) 
        : id(id), width(w), height(h), filename(file) {}
    
    bool isValid() const { return id != 0 && width > 0 && height > 0; }
};

class SpriteAnimator {
public:
    // Animation structure
    struct Animation {
        std::vector<Rectangle> frames;
        float frameDuration;
        bool loop;
        std::string name;
        
        Animation() : frameDuration(0.1f), loop(true) {}
        Animation(const std::vector<Rectangle>& frames, float duration, bool loop = true, const std::string& name = "")
            : frames(frames), frameDuration(duration), loop(loop), name(name) {}
        
        bool isValid() const { return !frames.empty() && frameDuration > 0.0f; }
        int getFrameCount() const { return static_cast<int>(frames.size()); }
        float getTotalDuration() const { return frameDuration * frames.size(); }
    };
    
    // Animation state
    enum class AnimationState {
        STOPPED,
        PLAYING,
        PAUSED,
        COMPLETED
    };
    
    // Constructor
    SpriteAnimator();
    SpriteAnimator(const Texture2D& spriteSheet);
    
    // Sprite sheet management
    void setSpriteSheet(const Texture2D& spriteSheet);
    const Texture2D& getSpriteSheet() const;
    bool hasSpriteSheet() const;
    
    // Animation management
    void addAnimation(const std::string& name, const Animation& animation);
    void removeAnimation(const std::string& name);
    bool hasAnimation(const std::string& name) const;
    const Animation* getAnimation(const std::string& name) const;
    std::vector<std::string> getAnimationNames() const;
    void clearAnimations();
    
    // Animation playback
    void playAnimation(const std::string& name, bool restart = false);
    void stopAnimation();
    void pauseAnimation();
    void resumeAnimation();
    void restartCurrentAnimation();
    
    // Animation state queries
    AnimationState getState() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;
    bool isAnimationComplete() const;
    
    // Current animation info
    std::string getCurrentAnimationName() const;
    int getCurrentFrame() const;
    int getTotalFrames() const;
    float getCurrentTime() const;
    float getTotalTime() const;
    float getProgress() const; // 0.0 to 1.0
    
    // Frame access
    Rectangle getCurrentFrameRect() const;
    Rectangle getFrameRect(int frameIndex) const;
    Rectangle getFrameRect(const std::string& animationName, int frameIndex) const;
    
    // Update and rendering
    void update(float deltaTime);
    void setFrame(int frameIndex);
    void setTime(float time);
    
    // Animation speed control
    void setPlaybackSpeed(float speed);
    float getPlaybackSpeed() const;
    void setFrameDuration(float duration);
    void setFrameDuration(const std::string& animationName, float duration);
    
    // Animation creation helpers
    Animation createAnimation(const std::string& name, int startX, int startY, 
                            int frameWidth, int frameHeight, int frameCount, 
                            float frameDuration, bool loop = true, int columns = 0);
    Animation createAnimationFromGrid(const std::string& name, int gridX, int gridY,
                                    int frameWidth, int frameHeight, int frameCount,
                                    float frameDuration, bool loop = true);
    
    // Batch animation loading
    void loadAnimationsFromConfig(const std::string& configPath);
    void saveAnimationsToConfig(const std::string& configPath) const;
    void loadAnimationsFromJson(const nlohmann::json& json);
    nlohmann::json saveAnimationsToJson() const;
    
    // Event callbacks
    void setOnAnimationComplete(std::function<void(const std::string&)> callback);
    void setOnAnimationLoop(std::function<void(const std::string&, int)> callback);
    void setOnFrameChanged(std::function<void(const std::string&, int)> callback);
    void setOnAnimationChanged(std::function<void(const std::string&, const std::string&)> callback);
    
    // Debug and utilities
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const;
    std::vector<std::string> getDebugInfo() const;
    void logAnimationState() const;
    
    // Animation blending and transitions (advanced features)
    void setTransitionDuration(float duration);
    float getTransitionDuration() const;
    void enableSmoothing(bool enable);
    bool isSmoothingEnabled() const;
    
private:
    // Core data
    Texture2D spriteSheet;
    std::unordered_map<std::string, Animation> animations;
    
    // Current animation state
    std::string currentAnimationName;
    AnimationState state;
    int currentFrame;
    float frameTimer;
    float playbackSpeed;
    bool animationComplete;
    int loopCount;
    
    // Transition and smoothing
    float transitionDuration;
    bool smoothingEnabled;
    
    // Debug
    bool debugMode;
    
    // Event callbacks
    std::function<void(const std::string&)> onAnimationComplete;
    std::function<void(const std::string&, int)> onAnimationLoop;
    std::function<void(const std::string&, int)> onFrameChanged;
    std::function<void(const std::string&, const std::string&)> onAnimationChanged;
    
    // Internal methods
    void updateAnimation(float deltaTime);
    void advanceFrame();
    void resetAnimation();
    bool isValidFrame(int frameIndex) const;
    const Animation* getCurrentAnimation() const;
    
    // Callback triggers
    void triggerAnimationCompleteCallback();
    void triggerAnimationLoopCallback();
    void triggerFrameChangedCallback();
    void triggerAnimationChangedCallback(const std::string& oldAnimation);
    
    // Utility methods
    Rectangle calculateFrameRect(int x, int y, int width, int height) const;
    std::vector<Rectangle> generateFrameGrid(int startX, int startY, int frameWidth, 
                                           int frameHeight, int frameCount, int columns) const;
    
    // Validation
    bool validateAnimation(const Animation& animation) const;
    bool validateFrameIndex(int frameIndex) const;
};