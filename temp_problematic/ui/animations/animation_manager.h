#pragma once

#include "sprite_animator.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

class AnimationManager {
public:
    AnimationManager();
    ~AnimationManager();
    
    // Initialization and cleanup
    bool initialize();
    void shutdown();
    
    // Sprite sheet management
    bool loadSpriteSheet(const std::string& name, const std::string& filename);
    void unloadSpriteSheet(const std::string& name);
    bool hasSpriteSheet(const std::string& name) const;
    const Texture2D* getSpriteSheet(const std::string& name) const;
    std::vector<std::string> getSpriteSheetNames() const;
    void clearSpriteSheets();
    
    // Animator management
    std::shared_ptr<SpriteAnimator> createAnimator(const std::string& name, const std::string& spriteSheetName = "");
    void removeAnimator(const std::string& name);
    std::shared_ptr<SpriteAnimator> getAnimator(const std::string& name) const;
    bool hasAnimator(const std::string& name) const;
    std::vector<std::string> getAnimatorNames() const;
    void clearAnimators();
    
    // Global animation control
    void updateAll(float deltaTime);
    void pauseAll();
    void resumeAll();
    void stopAll();
    
    // Animation presets and templates
    void createCharacterAnimations(const std::string& animatorName, const std::string& spriteSheetName,
                                 int frameWidth, int frameHeight);
    void createUIAnimations(const std::string& animatorName, const std::string& spriteSheetName);
    void createEffectAnimations(const std::string& animatorName, const std::string& spriteSheetName);
    
    // Batch loading
    void loadAnimationConfig(const std::string& configPath);
    void saveAnimationConfig(const std::string& configPath) const;
    void loadAnimationsFromJson(const nlohmann::json& json);
    nlohmann::json saveAnimationsToJson() const;
    
    // Resource management
    void preloadAnimations(const std::vector<std::string>& animationNames);
    void unloadUnusedAnimations();
    size_t getMemoryUsage() const;
    void optimizeMemory();
    
    // Debug and diagnostics
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const;
    std::vector<std::string> getDebugInfo() const;
    void logStatus() const;
    
    // Global settings
    void setGlobalPlaybackSpeed(float speed);
    float getGlobalPlaybackSpeed() const;
    void setGlobalFrameDuration(float duration);
    float getGlobalFrameDuration() const;
    
    // Event system
    void setOnAnimationComplete(std::function<void(const std::string&, const std::string&)> callback);
    void setOnAnimatorCreated(std::function<void(const std::string&)> callback);
    void setOnAnimatorRemoved(std::function<void(const std::string&)> callback);
    
private:
    // Core data
    std::unordered_map<std::string, Texture2D> spriteSheets;
    std::unordered_map<std::string, std::shared_ptr<SpriteAnimator>> animators;
    
    // Global settings
    float globalPlaybackSpeed;
    float globalFrameDuration;
    bool debugMode;
    bool initialized;
    
    // Event callbacks
    std::function<void(const std::string&, const std::string&)> onAnimationComplete;
    std::function<void(const std::string&)> onAnimatorCreated;
    std::function<void(const std::string&)> onAnimatorRemoved;
    
    // Internal methods
    Texture2D loadTextureFromFile(const std::string& filename);
    void unloadTexture(const Texture2D& texture);
    void setupAnimatorCallbacks(std::shared_ptr<SpriteAnimator> animator, const std::string& name);
    
    // Callback handlers
    void handleAnimationComplete(const std::string& animatorName, const std::string& animationName);
    
    // Utility methods
    bool isValidAnimatorName(const std::string& name) const;
    bool isValidSpriteSheetName(const std::string& name) const;
    
    // Resource tracking
    struct ResourceStats {
        size_t totalTextures;
        size_t totalAnimators;
        size_t totalAnimations;
        size_t memoryUsage;
    };
    
    ResourceStats getResourceStats() const;
};