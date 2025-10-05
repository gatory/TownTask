#include "sprite_animator.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>

// SpriteAnimator implementation
SpriteAnimator::SpriteAnimator() 
    : state(AnimationState::STOPPED), currentFrame(0), frameTimer(0.0f), 
      playbackSpeed(1.0f), animationComplete(false), loopCount(0),
      transitionDuration(0.0f), smoothingEnabled(false), debugMode(false) {
}

SpriteAnimator::SpriteAnimator(const Texture2D& spriteSheet) 
    : SpriteAnimator() {
    setSpriteSheet(spriteSheet);
}

// Sprite sheet management
void SpriteAnimator::setSpriteSheet(const Texture2D& spriteSheet) {
    this->spriteSheet = spriteSheet;
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Set sprite sheet - " << spriteSheet.filename 
                  << " (" << spriteSheet.width << "x" << spriteSheet.height << ")" << std::endl;
    }
}

const Texture2D& SpriteAnimator::getSpriteSheet() const {
    return spriteSheet;
}

bool SpriteAnimator::hasSpriteSheet() const {
    return spriteSheet.isValid();
}

// Animation management
void SpriteAnimator::addAnimation(const std::string& name, const Animation& animation) {
    if (!validateAnimation(animation)) {
        if (debugMode) {
            std::cout << "SpriteAnimator: Invalid animation '" << name << "'" << std::endl;
        }
        return;
    }
    
    animations[name] = animation;
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Added animation '" << name << "' with " 
                  << animation.getFrameCount() << " frames" << std::endl;
    }
}

void SpriteAnimator::removeAnimation(const std::string& name) {
    auto it = animations.find(name);
    if (it != animations.end()) {
        // Stop current animation if it's the one being removed
        if (currentAnimationName == name) {
            stopAnimation();
        }
        
        animations.erase(it);
        
        if (debugMode) {
            std::cout << "SpriteAnimator: Removed animation '" << name << "'" << std::endl;
        }
    }
}

bool SpriteAnimator::hasAnimation(const std::string& name) const {
    return animations.find(name) != animations.end();
}

const SpriteAnimator::Animation* SpriteAnimator::getAnimation(const std::string& name) const {
    auto it = animations.find(name);
    return it != animations.end() ? &it->second : nullptr;
}

std::vector<std::string> SpriteAnimator::getAnimationNames() const {
    std::vector<std::string> names;
    names.reserve(animations.size());
    
    for (const auto& [name, animation] : animations) {
        names.push_back(name);
    }
    
    return names;
}

void SpriteAnimator::clearAnimations() {
    stopAnimation();
    animations.clear();
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Cleared all animations" << std::endl;
    }
}

// Animation playback
void SpriteAnimator::playAnimation(const std::string& name, bool restart) {
    if (!hasAnimation(name)) {
        if (debugMode) {
            std::cout << "SpriteAnimator: Animation '" << name << "' not found" << std::endl;
        }
        return;
    }
    
    std::string oldAnimation = currentAnimationName;
    
    if (currentAnimationName != name || restart) {
        currentAnimationName = name;
        resetAnimation();
        triggerAnimationChangedCallback(oldAnimation);
    }
    
    state = AnimationState::PLAYING;
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Playing animation '" << name << "'" << std::endl;
    }
}

void SpriteAnimator::stopAnimation() {
    state = AnimationState::STOPPED;
    resetAnimation();
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Stopped animation" << std::endl;
    }
}

void SpriteAnimator::pauseAnimation() {
    if (state == AnimationState::PLAYING) {
        state = AnimationState::PAUSED;
        
        if (debugMode) {
            std::cout << "SpriteAnimator: Paused animation" << std::endl;
        }
    }
}

void SpriteAnimator::resumeAnimation() {
    if (state == AnimationState::PAUSED) {
        state = AnimationState::PLAYING;
        
        if (debugMode) {
            std::cout << "SpriteAnimator: Resumed animation" << std::endl;
        }
    }
}

void SpriteAnimator::restartCurrentAnimation() {
    if (!currentAnimationName.empty()) {
        resetAnimation();
        state = AnimationState::PLAYING;
        
        if (debugMode) {
            std::cout << "SpriteAnimator: Restarted animation '" << currentAnimationName << "'" << std::endl;
        }
    }
}

// Animation state queries
SpriteAnimator::AnimationState SpriteAnimator::getState() const {
    return state;
}

bool SpriteAnimator::isPlaying() const {
    return state == AnimationState::PLAYING;
}

bool SpriteAnimator::isPaused() const {
    return state == AnimationState::PAUSED;
}

bool SpriteAnimator::isStopped() const {
    return state == AnimationState::STOPPED;
}

bool SpriteAnimator::isAnimationComplete() const {
    return animationComplete;
}

// Current animation info
std::string SpriteAnimator::getCurrentAnimationName() const {
    return currentAnimationName;
}

int SpriteAnimator::getCurrentFrame() const {
    return currentFrame;
}

int SpriteAnimator::getTotalFrames() const {
    const Animation* anim = getCurrentAnimation();
    return anim ? anim->getFrameCount() : 0;
}

float SpriteAnimator::getCurrentTime() const {
    const Animation* anim = getCurrentAnimation();
    if (!anim) return 0.0f;
    
    return currentFrame * anim->frameDuration + frameTimer;
}

float SpriteAnimator::getTotalTime() const {
    const Animation* anim = getCurrentAnimation();
    return anim ? anim->getTotalDuration() : 0.0f;
}

float SpriteAnimator::getProgress() const {
    float totalTime = getTotalTime();
    if (totalTime <= 0.0f) return 0.0f;
    
    return std::min(1.0f, getCurrentTime() / totalTime);
}

// Frame access
Rectangle SpriteAnimator::getCurrentFrameRect() const {
    const Animation* anim = getCurrentAnimation();
    if (!anim || !isValidFrame(currentFrame)) {
        return Rectangle();
    }
    
    return anim->frames[currentFrame];
}

Rectangle SpriteAnimator::getFrameRect(int frameIndex) const {
    const Animation* anim = getCurrentAnimation();
    if (!anim || frameIndex < 0 || frameIndex >= anim->getFrameCount()) {
        return Rectangle();
    }
    
    return anim->frames[frameIndex];
}

Rectangle SpriteAnimator::getFrameRect(const std::string& animationName, int frameIndex) const {
    const Animation* anim = getAnimation(animationName);
    if (!anim || frameIndex < 0 || frameIndex >= anim->getFrameCount()) {
        return Rectangle();
    }
    
    return anim->frames[frameIndex];
}

// Update and rendering
void SpriteAnimator::update(float deltaTime) {
    if (state == AnimationState::PLAYING) {
        updateAnimation(deltaTime * playbackSpeed);
    }
}

void SpriteAnimator::setFrame(int frameIndex) {
    const Animation* anim = getCurrentAnimation();
    if (!anim) return;
    
    currentFrame = std::clamp(frameIndex, 0, anim->getFrameCount() - 1);
    frameTimer = 0.0f;
    
    triggerFrameChangedCallback();
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Set frame to " << currentFrame << std::endl;
    }
}

void SpriteAnimator::setTime(float time) {
    const Animation* anim = getCurrentAnimation();
    if (!anim) return;
    
    time = std::max(0.0f, time);
    
    if (anim->frameDuration > 0.0f) {
        int newFrame = static_cast<int>(time / anim->frameDuration);
        float remainder = std::fmod(time, anim->frameDuration);
        
        if (anim->loop) {
            newFrame = newFrame % anim->getFrameCount();
        } else {
            newFrame = std::min(newFrame, anim->getFrameCount() - 1);
        }
        
        if (newFrame != currentFrame) {
            currentFrame = newFrame;
            triggerFrameChangedCallback();
        }
        
        frameTimer = remainder;
    }
}

// Animation speed control
void SpriteAnimator::setPlaybackSpeed(float speed) {
    playbackSpeed = std::max(0.0f, speed);
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Set playback speed to " << playbackSpeed << std::endl;
    }
}

float SpriteAnimator::getPlaybackSpeed() const {
    return playbackSpeed;
}

void SpriteAnimator::setFrameDuration(float duration) {
    if (!currentAnimationName.empty()) {
        setFrameDuration(currentAnimationName, duration);
    }
}

void SpriteAnimator::setFrameDuration(const std::string& animationName, float duration) {
    auto it = animations.find(animationName);
    if (it != animations.end() && duration > 0.0f) {
        it->second.frameDuration = duration;
        
        if (debugMode) {
            std::cout << "SpriteAnimator: Set frame duration for '" << animationName 
                      << "' to " << duration << std::endl;
        }
    }
}

// Animation creation helpers
SpriteAnimator::Animation SpriteAnimator::createAnimation(const std::string& name, int startX, int startY,
                                                        int frameWidth, int frameHeight, int frameCount,
                                                        float frameDuration, bool loop, int columns) {
    if (columns <= 0) {
        columns = frameCount; // Single row by default
    }
    
    std::vector<Rectangle> frames;
    frames.reserve(frameCount);
    
    for (int i = 0; i < frameCount; i++) {
        int col = i % columns;
        int row = i / columns;
        
        float x = startX + col * frameWidth;
        float y = startY + row * frameHeight;
        
        frames.emplace_back(x, y, frameWidth, frameHeight);
    }
    
    Animation animation(frames, frameDuration, loop, name);
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Created animation '" << name << "' with " 
                  << frameCount << " frames" << std::endl;
    }
    
    return animation;
}

SpriteAnimator::Animation SpriteAnimator::createAnimationFromGrid(const std::string& name, int gridX, int gridY,
                                                                int frameWidth, int frameHeight, int frameCount,
                                                                float frameDuration, bool loop) {
    return createAnimation(name, gridX * frameWidth, gridY * frameHeight, 
                          frameWidth, frameHeight, frameCount, frameDuration, loop);
}

// Batch animation loading
void SpriteAnimator::loadAnimationsFromConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        if (debugMode) {
            std::cout << "SpriteAnimator: Could not open config file: " << configPath << std::endl;
        }
        return;
    }
    
    try {
        nlohmann::json json;
        file >> json;
        loadAnimationsFromJson(json);
    } catch (const std::exception& e) {
        if (debugMode) {
            std::cout << "SpriteAnimator: Error loading config: " << e.what() << std::endl;
        }
    }
}

void SpriteAnimator::saveAnimationsToConfig(const std::string& configPath) const {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        if (debugMode) {
            std::cout << "SpriteAnimator: Could not save config file: " << configPath << std::endl;
        }
        return;
    }
    
    nlohmann::json json = saveAnimationsToJson();
    file << json.dump(2);
}

void SpriteAnimator::loadAnimationsFromJson(const nlohmann::json& json) {
    if (!json.contains("animations")) {
        return;
    }
    
    for (const auto& animData : json["animations"]) {
        if (!animData.contains("name") || !animData.contains("frames")) {
            continue;
        }
        
        std::string name = animData["name"];
        float frameDuration = animData.value("frameDuration", 0.1f);
        bool loop = animData.value("loop", true);
        
        std::vector<Rectangle> frames;
        for (const auto& frameData : animData["frames"]) {
            if (frameData.contains("x") && frameData.contains("y") && 
                frameData.contains("width") && frameData.contains("height")) {
                
                frames.emplace_back(
                    frameData["x"], frameData["y"],
                    frameData["width"], frameData["height"]
                );
            }
        }
        
        if (!frames.empty()) {
            Animation animation(frames, frameDuration, loop, name);
            addAnimation(name, animation);
        }
    }
}

nlohmann::json SpriteAnimator::saveAnimationsToJson() const {
    nlohmann::json json;
    nlohmann::json animationsArray = nlohmann::json::array();
    
    for (const auto& [name, animation] : animations) {
        nlohmann::json animData;
        animData["name"] = name;
        animData["frameDuration"] = animation.frameDuration;
        animData["loop"] = animation.loop;
        
        nlohmann::json framesArray = nlohmann::json::array();
        for (const auto& frame : animation.frames) {
            nlohmann::json frameData;
            frameData["x"] = frame.x;
            frameData["y"] = frame.y;
            frameData["width"] = frame.width;
            frameData["height"] = frame.height;
            framesArray.push_back(frameData);
        }
        
        animData["frames"] = framesArray;
        animationsArray.push_back(animData);
    }
    
    json["animations"] = animationsArray;
    return json;
}

// Event callbacks
void SpriteAnimator::setOnAnimationComplete(std::function<void(const std::string&)> callback) {
    onAnimationComplete = std::move(callback);
}

void SpriteAnimator::setOnAnimationLoop(std::function<void(const std::string&, int)> callback) {
    onAnimationLoop = std::move(callback);
}

void SpriteAnimator::setOnFrameChanged(std::function<void(const std::string&, int)> callback) {
    onFrameChanged = std::move(callback);
}

void SpriteAnimator::setOnAnimationChanged(std::function<void(const std::string&, const std::string&)> callback) {
    onAnimationChanged = std::move(callback);
}

// Debug and utilities
void SpriteAnimator::enableDebugMode(bool enable) {
    debugMode = enable;
    
    if (debugMode) {
        std::cout << "SpriteAnimator: Debug mode " << (enable ? "enabled" : "disabled") << std::endl;
    }
}

bool SpriteAnimator::isDebugModeEnabled() const {
    return debugMode;
}

std::vector<std::string> SpriteAnimator::getDebugInfo() const {
    std::vector<std::string> info;
    
    info.push_back("=== SpriteAnimator Debug Info ===");
    info.push_back("Sprite Sheet: " + spriteSheet.filename + " (" + 
                   std::to_string(spriteSheet.width) + "x" + std::to_string(spriteSheet.height) + ")");
    info.push_back("Total Animations: " + std::to_string(animations.size()));
    info.push_back("Current Animation: " + currentAnimationName);
    info.push_back("State: " + std::to_string(static_cast<int>(state)));
    info.push_back("Current Frame: " + std::to_string(currentFrame) + "/" + std::to_string(getTotalFrames()));
    info.push_back("Frame Timer: " + std::to_string(frameTimer));
    info.push_back("Playback Speed: " + std::to_string(playbackSpeed));
    info.push_back("Loop Count: " + std::to_string(loopCount));
    info.push_back("Animation Complete: " + std::string(animationComplete ? "Yes" : "No"));
    
    info.push_back("\n=== Available Animations ===");
    for (const auto& [name, animation] : animations) {
        info.push_back(name + ": " + std::to_string(animation.getFrameCount()) + " frames, " +
                      std::to_string(animation.frameDuration) + "s/frame, " +
                      (animation.loop ? "looping" : "non-looping"));
    }
    
    return info;
}

void SpriteAnimator::logAnimationState() const {
    if (!debugMode) return;
    
    auto debugInfo = getDebugInfo();
    for (const auto& line : debugInfo) {
        std::cout << line << std::endl;
    }
    std::cout << "---" << std::endl;
}

// Animation blending and transitions
void SpriteAnimator::setTransitionDuration(float duration) {
    transitionDuration = std::max(0.0f, duration);
}

float SpriteAnimator::getTransitionDuration() const {
    return transitionDuration;
}

void SpriteAnimator::enableSmoothing(bool enable) {
    smoothingEnabled = enable;
}

bool SpriteAnimator::isSmoothingEnabled() const {
    return smoothingEnabled;
}

// Private methods
void SpriteAnimator::updateAnimation(float deltaTime) {
    const Animation* anim = getCurrentAnimation();
    if (!anim) return;
    
    frameTimer += deltaTime;
    
    while (frameTimer >= anim->frameDuration) {
        frameTimer -= anim->frameDuration;
        advanceFrame();
    }
}

void SpriteAnimator::advanceFrame() {
    const Animation* anim = getCurrentAnimation();
    if (!anim) return;
    
    int oldFrame = currentFrame;
    currentFrame++;
    
    if (currentFrame >= anim->getFrameCount()) {
        if (anim->loop) {
            currentFrame = 0;
            loopCount++;
            triggerAnimationLoopCallback();
        } else {
            currentFrame = anim->getFrameCount() - 1;
            state = AnimationState::COMPLETED;
            animationComplete = true;
            triggerAnimationCompleteCallback();
        }
    }
    
    if (oldFrame != currentFrame) {
        triggerFrameChangedCallback();
    }
}

void SpriteAnimator::resetAnimation() {
    currentFrame = 0;
    frameTimer = 0.0f;
    animationComplete = false;
    loopCount = 0;
}

bool SpriteAnimator::isValidFrame(int frameIndex) const {
    const Animation* anim = getCurrentAnimation();
    return anim && frameIndex >= 0 && frameIndex < anim->getFrameCount();
}

const SpriteAnimator::Animation* SpriteAnimator::getCurrentAnimation() const {
    return getAnimation(currentAnimationName);
}

// Callback triggers
void SpriteAnimator::triggerAnimationCompleteCallback() {
    if (onAnimationComplete) {
        onAnimationComplete(currentAnimationName);
    }
}

void SpriteAnimator::triggerAnimationLoopCallback() {
    if (onAnimationLoop) {
        onAnimationLoop(currentAnimationName, loopCount);
    }
}

void SpriteAnimator::triggerFrameChangedCallback() {
    if (onFrameChanged) {
        onFrameChanged(currentAnimationName, currentFrame);
    }
}

void SpriteAnimator::triggerAnimationChangedCallback(const std::string& oldAnimation) {
    if (onAnimationChanged) {
        onAnimationChanged(oldAnimation, currentAnimationName);
    }
}

// Utility methods
Rectangle SpriteAnimator::calculateFrameRect(int x, int y, int width, int height) const {
    return Rectangle(static_cast<float>(x), static_cast<float>(y), 
                    static_cast<float>(width), static_cast<float>(height));
}

std::vector<Rectangle> SpriteAnimator::generateFrameGrid(int startX, int startY, int frameWidth,
                                                       int frameHeight, int frameCount, int columns) const {
    std::vector<Rectangle> frames;
    frames.reserve(frameCount);
    
    for (int i = 0; i < frameCount; i++) {
        int col = i % columns;
        int row = i / columns;
        
        float x = startX + col * frameWidth;
        float y = startY + row * frameHeight;
        
        frames.emplace_back(x, y, frameWidth, frameHeight);
    }
    
    return frames;
}

// Validation
bool SpriteAnimator::validateAnimation(const Animation& animation) const {
    return animation.isValid();
}

bool SpriteAnimator::validateFrameIndex(int frameIndex) const {
    return isValidFrame(frameIndex);
}