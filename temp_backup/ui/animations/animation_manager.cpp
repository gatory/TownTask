#include "animation_manager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

AnimationManager::AnimationManager() 
    : globalPlaybackSpeed(1.0f), globalFrameDuration(0.1f), debugMode(false), initialized(false) {
}

AnimationManager::~AnimationManager() {
    shutdown();
}

// Initialization and cleanup
bool AnimationManager::initialize() {
    if (initialized) {
        return true;
    }
    
    // Initialize any global animation resources here
    initialized = true;
    
    if (debugMode) {
        std::cout << "AnimationManager: Initialized" << std::endl;
    }
    
    return true;
}

void AnimationManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    clearAnimators();
    clearSpriteSheets();
    
    initialized = false;
    
    if (debugMode) {
        std::cout << "AnimationManager: Shutdown" << std::endl;
    }
}

// Sprite sheet management
bool AnimationManager::loadSpriteSheet(const std::string& name, const std::string& filename) {
    if (name.empty() || filename.empty()) {
        return false;
    }
    
    Texture2D texture = loadTextureFromFile(filename);
    if (!texture.isValid()) {
        if (debugMode) {
            std::cout << "AnimationManager: Failed to load sprite sheet '" << name 
                      << "' from " << filename << std::endl;
        }
        return false;
    }
    
    spriteSheets[name] = texture;
    
    if (debugMode) {
        std::cout << "AnimationManager: Loaded sprite sheet '" << name 
                  << "' (" << texture.width << "x" << texture.height << ")" << std::endl;
    }
    
    return true;
}

void AnimationManager::unloadSpriteSheet(const std::string& name) {
    auto it = spriteSheets.find(name);
    if (it != spriteSheets.end()) {
        unloadTexture(it->second);
        spriteSheets.erase(it);
        
        if (debugMode) {
            std::cout << "AnimationManager: Unloaded sprite sheet '" << name << "'" << std::endl;
        }
    }
}

bool AnimationManager::hasSpriteSheet(const std::string& name) const {
    return spriteSheets.find(name) != spriteSheets.end();
}

const Texture2D* AnimationManager::getSpriteSheet(const std::string& name) const {
    auto it = spriteSheets.find(name);
    return it != spriteSheets.end() ? &it->second : nullptr;
}

std::vector<std::string> AnimationManager::getSpriteSheetNames() const {
    std::vector<std::string> names;
    names.reserve(spriteSheets.size());
    
    for (const auto& [name, texture] : spriteSheets) {
        names.push_back(name);
    }
    
    return names;
}

void AnimationManager::clearSpriteSheets() {
    for (auto& [name, texture] : spriteSheets) {
        unloadTexture(texture);
    }
    spriteSheets.clear();
    
    if (debugMode) {
        std::cout << "AnimationManager: Cleared all sprite sheets" << std::endl;
    }
}

// Animator management
std::shared_ptr<SpriteAnimator> AnimationManager::createAnimator(const std::string& name, const std::string& spriteSheetName) {
    if (!isValidAnimatorName(name)) {
        return nullptr;
    }
    
    auto animator = std::make_shared<SpriteAnimator>();
    
    // Set sprite sheet if provided
    if (!spriteSheetName.empty()) {
        const Texture2D* texture = getSpriteSheet(spriteSheetName);
        if (texture) {
            animator->setSpriteSheet(*texture);
        } else if (debugMode) {
            std::cout << "AnimationManager: Warning - sprite sheet '" << spriteSheetName 
                      << "' not found for animator '" << name << "'" << std::endl;
        }
    }
    
    // Apply global settings
    animator->setPlaybackSpeed(globalPlaybackSpeed);
    animator->enableDebugMode(debugMode);
    
    // Setup callbacks
    setupAnimatorCallbacks(animator, name);
    
    animators[name] = animator;
    
    if (onAnimatorCreated) {
        onAnimatorCreated(name);
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Created animator '" << name << "'" << std::endl;
    }
    
    return animator;
}

void AnimationManager::removeAnimator(const std::string& name) {
    auto it = animators.find(name);
    if (it != animators.end()) {
        animators.erase(it);
        
        if (onAnimatorRemoved) {
            onAnimatorRemoved(name);
        }
        
        if (debugMode) {
            std::cout << "AnimationManager: Removed animator '" << name << "'" << std::endl;
        }
    }
}

std::shared_ptr<SpriteAnimator> AnimationManager::getAnimator(const std::string& name) const {
    auto it = animators.find(name);
    return it != animators.end() ? it->second : nullptr;
}

bool AnimationManager::hasAnimator(const std::string& name) const {
    return animators.find(name) != animators.end();
}

std::vector<std::string> AnimationManager::getAnimatorNames() const {
    std::vector<std::string> names;
    names.reserve(animators.size());
    
    for (const auto& [name, animator] : animators) {
        names.push_back(name);
    }
    
    return names;
}

void AnimationManager::clearAnimators() {
    animators.clear();
    
    if (debugMode) {
        std::cout << "AnimationManager: Cleared all animators" << std::endl;
    }
}

// Global animation control
void AnimationManager::updateAll(float deltaTime) {
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->update(deltaTime);
        }
    }
}

void AnimationManager::pauseAll() {
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->pauseAnimation();
        }
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Paused all animations" << std::endl;
    }
}

void AnimationManager::resumeAll() {
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->resumeAnimation();
        }
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Resumed all animations" << std::endl;
    }
}

void AnimationManager::stopAll() {
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->stopAnimation();
        }
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Stopped all animations" << std::endl;
    }
}

// Animation presets and templates
void AnimationManager::createCharacterAnimations(const std::string& animatorName, const std::string& spriteSheetName,
                                                int frameWidth, int frameHeight) {
    auto animator = getAnimator(animatorName);
    if (!animator) {
        animator = createAnimator(animatorName, spriteSheetName);
    }
    
    if (!animator) {
        return;
    }
    
    // Create standard character animations
    // Assuming a typical character sprite sheet layout
    
    // Idle animation (first row, 4 frames)
    auto idleAnim = animator->createAnimation("idle", 0, 0, frameWidth, frameHeight, 4, 0.5f, true);
    animator->addAnimation("idle", idleAnim);
    
    // Walking animations (rows 1-4 for different directions)
    auto walkDownAnim = animator->createAnimation("walk_down", 0, frameHeight, frameWidth, frameHeight, 4, 0.2f, true);
    animator->addAnimation("walk_down", walkDownAnim);
    
    auto walkUpAnim = animator->createAnimation("walk_up", 0, frameHeight * 2, frameWidth, frameHeight, 4, 0.2f, true);
    animator->addAnimation("walk_up", walkUpAnim);
    
    auto walkLeftAnim = animator->createAnimation("walk_left", 0, frameHeight * 3, frameWidth, frameHeight, 4, 0.2f, true);
    animator->addAnimation("walk_left", walkLeftAnim);
    
    auto walkRightAnim = animator->createAnimation("walk_right", 0, frameHeight * 4, frameWidth, frameHeight, 4, 0.2f, true);
    animator->addAnimation("walk_right", walkRightAnim);
    
    // Interaction animation (row 5, 2 frames)
    auto interactAnim = animator->createAnimation("interact", 0, frameHeight * 5, frameWidth, frameHeight, 2, 0.3f, false);
    animator->addAnimation("interact", interactAnim);
    
    if (debugMode) {
        std::cout << "AnimationManager: Created character animations for '" << animatorName << "'" << std::endl;
    }
}

void AnimationManager::createUIAnimations(const std::string& animatorName, const std::string& spriteSheetName) {
    auto animator = getAnimator(animatorName);
    if (!animator) {
        animator = createAnimator(animatorName, spriteSheetName);
    }
    
    if (!animator) {
        return;
    }
    
    // Create common UI animations
    // Button hover effect
    auto buttonHoverAnim = animator->createAnimation("button_hover", 0, 0, 64, 32, 3, 0.1f, true);
    animator->addAnimation("button_hover", buttonHoverAnim);
    
    // Loading spinner
    auto loadingAnim = animator->createAnimation("loading", 0, 32, 32, 32, 8, 0.1f, true);
    animator->addAnimation("loading", loadingAnim);
    
    // Notification popup
    auto notificationAnim = animator->createAnimation("notification", 0, 64, 128, 64, 6, 0.15f, false);
    animator->addAnimation("notification", notificationAnim);
    
    if (debugMode) {
        std::cout << "AnimationManager: Created UI animations for '" << animatorName << "'" << std::endl;
    }
}

void AnimationManager::createEffectAnimations(const std::string& animatorName, const std::string& spriteSheetName) {
    auto animator = getAnimator(animatorName);
    if (!animator) {
        animator = createAnimator(animatorName, spriteSheetName);
    }
    
    if (!animator) {
        return;
    }
    
    // Create effect animations
    // Sparkle effect
    auto sparkleAnim = animator->createAnimation("sparkle", 0, 0, 32, 32, 8, 0.08f, false);
    animator->addAnimation("sparkle", sparkleAnim);
    
    // Explosion effect
    auto explosionAnim = animator->createAnimation("explosion", 0, 32, 64, 64, 12, 0.06f, false);
    animator->addAnimation("explosion", explosionAnim);
    
    // Smoke effect
    auto smokeAnim = animator->createAnimation("smoke", 0, 96, 48, 48, 10, 0.1f, true);
    animator->addAnimation("smoke", smokeAnim);
    
    if (debugMode) {
        std::cout << "AnimationManager: Created effect animations for '" << animatorName << "'" << std::endl;
    }
}

// Batch loading
void AnimationManager::loadAnimationConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        if (debugMode) {
            std::cout << "AnimationManager: Could not open config file: " << configPath << std::endl;
        }
        return;
    }
    
    try {
        nlohmann::json json;
        file >> json;
        loadAnimationsFromJson(json);
    } catch (const std::exception& e) {
        if (debugMode) {
            std::cout << "AnimationManager: Error loading config: " << e.what() << std::endl;
        }
    }
}

void AnimationManager::saveAnimationConfig(const std::string& configPath) const {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        if (debugMode) {
            std::cout << "AnimationManager: Could not save config file: " << configPath << std::endl;
        }
        return;
    }
    
    nlohmann::json json = saveAnimationsToJson();
    file << json.dump(2);
}

void AnimationManager::loadAnimationsFromJson(const nlohmann::json& json) {
    // Load sprite sheets
    if (json.contains("spriteSheets")) {
        for (const auto& sheetData : json["spriteSheets"]) {
            if (sheetData.contains("name") && sheetData.contains("filename")) {
                loadSpriteSheet(sheetData["name"], sheetData["filename"]);
            }
        }
    }
    
    // Load animators
    if (json.contains("animators")) {
        for (const auto& animatorData : json["animators"]) {
            if (animatorData.contains("name")) {
                std::string name = animatorData["name"];
                std::string spriteSheet = animatorData.value("spriteSheet", "");
                
                auto animator = createAnimator(name, spriteSheet);
                if (animator && animatorData.contains("animations")) {
                    animator->loadAnimationsFromJson(animatorData);
                }
            }
        }
    }
    
    // Load global settings
    if (json.contains("globalSettings")) {
        const auto& settings = json["globalSettings"];
        globalPlaybackSpeed = settings.value("playbackSpeed", 1.0f);
        globalFrameDuration = settings.value("frameDuration", 0.1f);
    }
}

nlohmann::json AnimationManager::saveAnimationsToJson() const {
    nlohmann::json json;
    
    // Save sprite sheets
    nlohmann::json spriteSheetArray = nlohmann::json::array();
    for (const auto& [name, texture] : spriteSheets) {
        nlohmann::json sheetData;
        sheetData["name"] = name;
        sheetData["filename"] = texture.filename;
        sheetData["width"] = texture.width;
        sheetData["height"] = texture.height;
        spriteSheetArray.push_back(sheetData);
    }
    json["spriteSheets"] = spriteSheetArray;
    
    // Save animators
    nlohmann::json animatorArray = nlohmann::json::array();
    for (const auto& [name, animator] : animators) {
        if (animator) {
            nlohmann::json animatorData = animator->saveAnimationsToJson();
            animatorData["name"] = name;
            animatorArray.push_back(animatorData);
        }
    }
    json["animators"] = animatorArray;
    
    // Save global settings
    json["globalSettings"] = {
        {"playbackSpeed", globalPlaybackSpeed},
        {"frameDuration", globalFrameDuration}
    };
    
    return json;
}

// Resource management
void AnimationManager::preloadAnimations(const std::vector<std::string>& animationNames) {
    // Implementation would preload specific animations into memory
    // For now, this is a placeholder
    
    if (debugMode) {
        std::cout << "AnimationManager: Preloaded " << animationNames.size() << " animations" << std::endl;
    }
}

void AnimationManager::unloadUnusedAnimations() {
    // Implementation would unload animations that haven't been used recently
    // For now, this is a placeholder
    
    if (debugMode) {
        std::cout << "AnimationManager: Unloaded unused animations" << std::endl;
    }
}

size_t AnimationManager::getMemoryUsage() const {
    size_t usage = 0;
    
    // Calculate sprite sheet memory usage
    for (const auto& [name, texture] : spriteSheets) {
        usage += texture.width * texture.height * 4; // Assuming RGBA
    }
    
    // Add animator memory usage (rough estimate)
    usage += animators.size() * sizeof(SpriteAnimator);
    
    return usage;
}

void AnimationManager::optimizeMemory() {
    unloadUnusedAnimations();
    
    if (debugMode) {
        std::cout << "AnimationManager: Optimized memory usage" << std::endl;
    }
}

// Debug and diagnostics
void AnimationManager::enableDebugMode(bool enable) {
    debugMode = enable;
    
    // Apply to all existing animators
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->enableDebugMode(enable);
        }
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Debug mode " << (enable ? "enabled" : "disabled") << std::endl;
    }
}

bool AnimationManager::isDebugModeEnabled() const {
    return debugMode;
}

std::vector<std::string> AnimationManager::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== AnimationManager Debug Info ===");
    info.push_back("Initialized: " + std::string(initialized ? "Yes" : "No"));
    info.push_back("Sprite Sheets: " + std::to_string(spriteSheets.size()));
    info.push_back("Animators: " + std::to_string(animators.size()));
    info.push_back("Global Playback Speed: " + std::to_string(globalPlaybackSpeed));
    info.push_back("Global Frame Duration: " + std::to_string(globalFrameDuration));
    info.push_back("Memory Usage: " + std::to_string(getMemoryUsage()) + " bytes");
    
    info.push_back("\n=== Sprite Sheets ===");
    for (const auto& [name, texture] : spriteSheets) {
        info.push_back(name + ": " + texture.filename + " (" + 
                      std::to_string(texture.width) + "x" + std::to_string(texture.height) + ")");
    }
    
    info.push_back("\n=== Animators ===");
    for (const auto& [name, animator] : animators) {
        if (animator) {
            info.push_back(name + ": " + std::to_string(animator->getAnimationNames().size()) + " animations");
        }
    }
    
    return info;
}

void AnimationManager::logStatus() const {
    if (!debugMode) return;
    
    auto debugInfo = getDebugInfo();
    for (const auto& line : debugInfo) {
        std::cout << line << std::endl;
    }
    std::cout << "---" << std::endl;
}

// Global settings
void AnimationManager::setGlobalPlaybackSpeed(float speed) {
    globalPlaybackSpeed = std::max(0.0f, speed);
    
    // Apply to all existing animators
    for (auto& [name, animator] : animators) {
        if (animator) {
            animator->setPlaybackSpeed(globalPlaybackSpeed);
        }
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Set global playback speed to " << globalPlaybackSpeed << std::endl;
    }
}

float AnimationManager::getGlobalPlaybackSpeed() const {
    return globalPlaybackSpeed;
}

void AnimationManager::setGlobalFrameDuration(float duration) {
    globalFrameDuration = std::max(0.01f, duration);
    
    if (debugMode) {
        std::cout << "AnimationManager: Set global frame duration to " << globalFrameDuration << std::endl;
    }
}

float AnimationManager::getGlobalFrameDuration() const {
    return globalFrameDuration;
}

// Event system
void AnimationManager::setOnAnimationComplete(std::function<void(const std::string&, const std::string&)> callback) {
    onAnimationComplete = std::move(callback);
}

void AnimationManager::setOnAnimatorCreated(std::function<void(const std::string&)> callback) {
    onAnimatorCreated = std::move(callback);
}

void AnimationManager::setOnAnimatorRemoved(std::function<void(const std::string&)> callback) {
    onAnimatorRemoved = std::move(callback);
}

// Private methods
Texture2D AnimationManager::loadTextureFromFile(const std::string& filename) {
    // This is a placeholder implementation
    // In a real implementation, this would use a graphics library like Raylib, SDL, or OpenGL
    // to load the texture from file
    
    // For now, create a dummy texture with some reasonable defaults
    static unsigned int nextId = 1;
    
    // Simulate loading a texture
    if (!filename.empty()) {
        return Texture2D(nextId++, 256, 256, filename); // Dummy 256x256 texture
    }
    
    return Texture2D(); // Invalid texture
}

void AnimationManager::unloadTexture(const Texture2D& texture) {
    // This is a placeholder implementation
    // In a real implementation, this would free the graphics memory
    
    if (debugMode && texture.isValid()) {
        std::cout << "AnimationManager: Unloaded texture " << texture.filename << std::endl;
    }
}

void AnimationManager::setupAnimatorCallbacks(std::shared_ptr<SpriteAnimator> animator, const std::string& name) {
    if (!animator) return;
    
    // Set up animation complete callback
    animator->setOnAnimationComplete([this, name](const std::string& animationName) {
        handleAnimationComplete(name, animationName);
    });
}

void AnimationManager::handleAnimationComplete(const std::string& animatorName, const std::string& animationName) {
    if (onAnimationComplete) {
        onAnimationComplete(animatorName, animationName);
    }
    
    if (debugMode) {
        std::cout << "AnimationManager: Animation '" << animationName 
                  << "' completed in animator '" << animatorName << "'" << std::endl;
    }
}

bool AnimationManager::isValidAnimatorName(const std::string& name) const {
    return !name.empty() && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") == std::string::npos;
}

bool AnimationManager::isValidSpriteSheetName(const std::string& name) const {
    return isValidAnimatorName(name);
}

AnimationManager::ResourceStats AnimationManager::getResourceStats() const {
    ResourceStats stats;
    stats.totalTextures = spriteSheets.size();
    stats.totalAnimators = animators.size();
    stats.totalAnimations = 0;
    stats.memoryUsage = getMemoryUsage();
    
    for (const auto& [name, animator] : animators) {
        if (animator) {
            stats.totalAnimations += animator->getAnimationNames().size();
        }
    }
    
    return stats;
}