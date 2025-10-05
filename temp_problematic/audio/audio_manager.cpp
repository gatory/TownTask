#include "audio_manager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

AudioManager::AudioManager() {
    // Constructor - initialization happens in initialize()
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (initialized) {
        return true;
    }
    
    // Initialize audio device
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        std::cerr << "AudioManager: Failed to initialize audio device" << std::endl;
        return false;
    }
    
    // Set initial volumes
    SetMasterVolume(settings.masterVolume);
    
    initialized = true;
    std::cout << "AudioManager: Initialized successfully" << std::endl;
    
    return true;
}

void AudioManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    // Stop all audio
    stopAllSounds();
    stopMusic();
    
    // Unload all resources
    unloadAllAudio();
    
    // Close audio device
    CloseAudioDevice();
    
    initialized = false;
    std::cout << "AudioManager: Shutdown complete" << std::endl;
}

// Sound effect management
bool AudioManager::loadSound(SoundType type, const std::string& filePath) {
    if (!initialized) {
        std::cerr << "AudioManager: Cannot load sound - not initialized" << std::endl;
        return false;
    }
    
    // Unload existing sound if present
    unloadSound(type);
    
    Sound sound = LoadSound(filePath.c_str());
    if (sound.frameCount == 0) { // Check if sound loaded properly
        std::cerr << "AudioManager: Failed to load sound: " << filePath << std::endl;
        return false;
    }
    
    SoundData soundData;
    soundData.sound = sound;
    soundData.loaded = true;
    soundData.baseVolume = 1.0f;
    
    sounds[type] = soundData;
    
    std::cout << "AudioManager: Loaded sound " << soundTypeToString(type) 
              << " from " << filePath << std::endl;
    
    return true;
}

void AudioManager::playSound(SoundType type, float volume) {
    if (!initialized || !settings.soundEffectsEnabled) {
        return;
    }
    
    auto it = sounds.find(type);
    if (it == sounds.end() || !it->second.loaded) {
        return;
    }
    
    SoundData& soundData = it->second;
    
    // Limit concurrent instances
    if (soundData.playCount >= MAX_SOUND_INSTANCES) {
        return;
    }
    
    float effectiveVolume = calculateEffectiveVolume(volume * soundData.baseVolume, false);
    SetSoundVolume(soundData.sound, effectiveVolume);
    
    PlaySound(soundData.sound);
    soundData.playCount++;
}

void AudioManager::playSoundWithPitch(SoundType type, float pitch, float volume) {
    if (!initialized || !settings.soundEffectsEnabled) {
        return;
    }
    
    auto it = sounds.find(type);
    if (it == sounds.end() || !it->second.loaded) {
        return;
    }
    
    SoundData& soundData = it->second;
    
    float effectiveVolume = calculateEffectiveVolume(volume * soundData.baseVolume, false);
    SetSoundVolume(soundData.sound, effectiveVolume);
    SetSoundPitch(soundData.sound, pitch);
    
    PlaySound(soundData.sound);
    soundData.playCount++;
}

void AudioManager::stopSound(SoundType type) {
    auto it = sounds.find(type);
    if (it != sounds.end() && it->second.loaded) {
        StopSound(it->second.sound);
        it->second.playCount = 0;
    }
}

void AudioManager::stopAllSounds() {
    for (auto& pair : sounds) {
        if (pair.second.loaded) {
            StopSound(pair.second.sound);
            pair.second.playCount = 0;
        }
    }
}

// Music management
bool AudioManager::loadMusic(MusicType type, const std::string& filePath) {
    if (!initialized) {
        std::cerr << "AudioManager: Cannot load music - not initialized" << std::endl;
        return false;
    }
    
    // Unload existing music if present
    unloadMusic(type);
    
    Music music = LoadMusicStream(filePath.c_str());
    if (music.frameCount == 0) { // Check if music loaded properly
        std::cerr << "AudioManager: Failed to load music: " << filePath << std::endl;
        return false;
    }
    
    MusicData musicData;
    musicData.music = music;
    musicData.loaded = true;
    musicData.baseVolume = 1.0f;
    
    this->music[type] = musicData;
    
    std::cout << "AudioManager: Loaded music " << musicTypeToString(type) 
              << " from " << filePath << std::endl;
    
    return true;
}

void AudioManager::playMusic(MusicType type, bool loop) {
    if (!initialized || !settings.musicEnabled) {
        return;
    }
    
    auto it = music.find(type);
    if (it == music.end() || !it->second.loaded) {
        return;
    }
    
    // Stop current music if different
    if (currentMusic != type) {
        stopMusic();
    }
    
    MusicData& musicData = it->second;
    
    musicData.music.looping = loop;
    applyVolumeToMusic(type);
    
    PlayMusicStream(musicData.music);
    musicData.isPlaying = true;
    musicData.isPaused = false;
    currentMusic = type;
}

void AudioManager::stopMusic() {
    auto it = music.find(currentMusic);
    if (it != music.end() && it->second.loaded && it->second.isPlaying) {
        StopMusicStream(it->second.music);
        it->second.isPlaying = false;
        it->second.isPaused = false;
    }
}

void AudioManager::pauseMusic() {
    auto it = music.find(currentMusic);
    if (it != music.end() && it->second.loaded && it->second.isPlaying && !it->second.isPaused) {
        PauseMusicStream(it->second.music);
        it->second.isPaused = true;
    }
}

void AudioManager::resumeMusic() {
    auto it = music.find(currentMusic);
    if (it != music.end() && it->second.loaded && it->second.isPlaying && it->second.isPaused) {
        ResumeMusicStream(it->second.music);
        it->second.isPaused = false;
    }
}

void AudioManager::fadeInMusic(MusicType type, float duration) {
    if (duration < 0) {
        duration = settings.fadeInDuration;
    }
    
    fadeState.active = true;
    fadeState.duration = duration;
    fadeState.elapsed = 0.0f;
    fadeState.startVolume = 0.0f;
    fadeState.targetVolume = settings.musicVolume;
    fadeState.targetMusic = type;
    fadeState.isCrossfade = false;
    
    // Start playing at zero volume
    playMusic(type);
    auto it = music.find(type);
    if (it != music.end() && it->second.loaded) {
        SetMusicVolume(it->second.music, 0.0f);
    }
}

void AudioManager::fadeOutMusic(float duration) {
    if (duration < 0) {
        duration = settings.fadeOutDuration;
    }
    
    auto it = music.find(currentMusic);
    if (it == music.end() || !it->second.loaded || !it->second.isPlaying) {
        return;
    }
    
    fadeState.active = true;
    fadeState.duration = duration;
    fadeState.elapsed = 0.0f;
    fadeState.startVolume = settings.musicVolume;
    fadeState.targetVolume = 0.0f;
    fadeState.isCrossfade = false;
}

void AudioManager::crossfadeMusic(MusicType newType, float duration) {
    if (currentMusic == newType) {
        return;
    }
    
    fadeState.active = true;
    fadeState.duration = duration;
    fadeState.elapsed = 0.0f;
    fadeState.startVolume = settings.musicVolume;
    fadeState.targetVolume = settings.musicVolume;
    fadeState.targetMusic = newType;
    fadeState.isCrossfade = true;
}

// Volume and settings
void AudioManager::setMasterVolume(float volume) {
    settings.masterVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    SetMasterVolume(settings.masterVolume);
    
    // Update all currently playing audio
    for (auto& pair : music) {
        if (pair.second.loaded && pair.second.isPlaying) {
            applyVolumeToMusic(pair.first);
        }
    }
}

void AudioManager::setSoundEffectVolume(float volume) {
    settings.soundEffectVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

void AudioManager::setMusicVolume(float volume) {
    settings.musicVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    
    // Update currently playing music
    for (auto& pair : music) {
        if (pair.second.loaded && pair.second.isPlaying) {
            applyVolumeToMusic(pair.first);
        }
    }
}

void AudioManager::enableSoundEffects(bool enabled) {
    settings.soundEffectsEnabled = enabled;
    if (!enabled) {
        stopAllSounds();
    }
}

void AudioManager::enableMusic(bool enabled) {
    settings.musicEnabled = enabled;
    if (!enabled) {
        stopMusic();
    }
}

void AudioManager::applySettings(const AudioSettings& newSettings) {
    settings = newSettings;
    
    setMasterVolume(settings.masterVolume);
    setSoundEffectVolume(settings.soundEffectVolume);
    setMusicVolume(settings.musicVolume);
    
    if (!settings.soundEffectsEnabled) {
        stopAllSounds();
    }
    
    if (!settings.musicEnabled) {
        stopMusic();
    }
}

void AudioManager::update(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    updateFading(deltaTime);
    updateMusicStreaming();
    updateSoundCleanup();
}

void AudioManager::setApplicationActive(bool active) {
    applicationActive = active;
    
    if (settings.muteWhenInactive) {
        if (active) {
            // Restore volumes
            setMasterVolume(settings.masterVolume);
        } else {
            // Mute audio
            SetMasterVolume(0.0f);
        }
    }
}

// Convenience methods for common game events
void AudioManager::playTaskCompleteSound() {
    playSound(SoundType::TASK_COMPLETE);
}

void AudioManager::playTimerAlertSound() {
    playSound(SoundType::TIMER_ALERT, 0.9f);
}

void AudioManager::playHabitStreakSound(int streakCount) {
    // Play different variations based on streak count
    float pitch = 1.0f + (streakCount % 7) * 0.1f; // Musical scale-like progression
    playSoundWithPitch(SoundType::HABIT_STREAK, pitch, 0.8f);
}

void AudioManager::playBuildingUpgradeSound() {
    playSound(SoundType::BUILDING_UPGRADE, 0.9f);
}

void AudioManager::playAchievementSound() {
    playSound(SoundType::ACHIEVEMENT, 1.0f);
}

void AudioManager::playUIClickSound() {
    playSound(SoundType::UI_CLICK, 0.6f);
}

void AudioManager::playUIHoverSound() {
    playSound(SoundType::UI_HOVER, 0.4f);
}

// Music context switching
void AudioManager::switchToBuildingMusic(MusicType buildingType) {
    if (currentMusic != buildingType) {
        crossfadeMusic(buildingType, 1.5f);
    }
}

void AudioManager::switchToTownMusic() {
    switchToBuildingMusic(MusicType::TOWN_AMBIENT);
}

void AudioManager::switchToFocusMusic() {
    switchToBuildingMusic(MusicType::FOCUS_SESSION);
}

void AudioManager::switchToCelebrationMusic() {
    switchToBuildingMusic(MusicType::CELEBRATION);
}

// Event callbacks
void AudioManager::setOnMusicFinished(std::function<void(MusicType)> callback) {
    onMusicFinished = callback;
}

void AudioManager::setOnSoundFinished(std::function<void(SoundType)> callback) {
    onSoundFinished = callback;
}

// Private helper methods
void AudioManager::updateFading(float deltaTime) {
    if (!fadeState.active) {
        return;
    }
    
    fadeState.elapsed += deltaTime;
    float progress = std::min(fadeState.elapsed / fadeState.duration, 1.0f);
    
    if (fadeState.isCrossfade) {
        // Crossfade: fade out current, fade in new
        if (progress < 0.5f) {
            // First half: fade out current
            float fadeOutProgress = progress * 2.0f;
            float volume = fadeState.startVolume * (1.0f - fadeOutProgress);
            
            auto it = music.find(currentMusic);
            if (it != music.end() && it->second.loaded) {
                SetMusicVolume(it->second.music, calculateEffectiveVolume(volume, true));
            }
        } else {
            // Second half: fade in new
            if (currentMusic != fadeState.targetMusic) {
                playMusic(fadeState.targetMusic);
            }
            
            float fadeInProgress = (progress - 0.5f) * 2.0f;
            float volume = fadeState.targetVolume * fadeInProgress;
            
            auto it = music.find(fadeState.targetMusic);
            if (it != music.end() && it->second.loaded) {
                SetMusicVolume(it->second.music, calculateEffectiveVolume(volume, true));
            }
        }
    } else {
        // Simple fade in or out
        float volume = fadeState.startVolume + (fadeState.targetVolume - fadeState.startVolume) * progress;
        
        MusicType targetType = (fadeState.targetVolume > fadeState.startVolume) ? 
                              fadeState.targetMusic : currentMusic;
        
        auto it = music.find(targetType);
        if (it != music.end() && it->second.loaded) {
            SetMusicVolume(it->second.music, calculateEffectiveVolume(volume, true));
        }
    }
    
    // Check if fade is complete
    if (progress >= 1.0f) {
        fadeState.active = false;
        
        if (fadeState.targetVolume == 0.0f) {
            // Fade out complete - stop music
            stopMusic();
        } else if (fadeState.isCrossfade || fadeState.targetMusic != currentMusic) {
            // Ensure target music is playing
            currentMusic = fadeState.targetMusic;
        }
    }
}

void AudioManager::updateMusicStreaming() {
    // Update music stream for currently playing music
    auto it = music.find(currentMusic);
    if (it != music.end() && it->second.loaded && it->second.isPlaying) {
        UpdateMusicStream(it->second.music);
        
        // Check if music finished
        if (!IsMusicStreamPlaying(it->second.music) && !it->second.isPaused) {
            it->second.isPlaying = false;
            
            if (onMusicFinished) {
                onMusicFinished(currentMusic);
            }
        }
    }
}

void AudioManager::updateSoundCleanup() {
    // Clean up finished sounds
    for (auto& pair : sounds) {
        if (pair.second.loaded && pair.second.playCount > 0) {
            if (!IsSoundPlaying(pair.second.sound)) {
                pair.second.playCount = std::max(0, pair.second.playCount - 1);
            }
        }
    }
}

float AudioManager::calculateEffectiveVolume(float baseVolume, bool isMusic) const {
    float typeVolume = isMusic ? settings.musicVolume : settings.soundEffectVolume;
    return std::clamp(baseVolume * typeVolume * settings.masterVolume, MIN_VOLUME, MAX_VOLUME);
}

void AudioManager::applyVolumeToMusic(MusicType type) {
    auto it = music.find(type);
    if (it != music.end() && it->second.loaded) {
        float effectiveVolume = calculateEffectiveVolume(it->second.baseVolume, true);
        SetMusicVolume(it->second.music, effectiveVolume);
    }
}

void AudioManager::applyVolumeToSound(SoundType type) {
    auto it = sounds.find(type);
    if (it != sounds.end() && it->second.loaded) {
        float effectiveVolume = calculateEffectiveVolume(it->second.baseVolume, false);
        SetSoundVolume(it->second.sound, effectiveVolume);
    }
}

// Resource management
void AudioManager::unloadSound(SoundType type) {
    auto it = sounds.find(type);
    if (it != sounds.end() && it->second.loaded) {
        UnloadSound(it->second.sound);
        sounds.erase(it);
    }
}

void AudioManager::unloadMusic(MusicType type) {
    auto it = music.find(type);
    if (it != music.end() && it->second.loaded) {
        if (it->second.isPlaying) {
            StopMusicStream(it->second.music);
        }
        UnloadMusicStream(it->second.music);
        music.erase(it);
    }
}

void AudioManager::unloadAllAudio() {
    // Unload all sounds
    for (auto& pair : sounds) {
        if (pair.second.loaded) {
            UnloadSound(pair.second.sound);
        }
    }
    sounds.clear();
    
    // Unload all music
    for (auto& pair : music) {
        if (pair.second.loaded) {
            if (pair.second.isPlaying) {
                StopMusicStream(pair.second.music);
            }
            UnloadMusicStream(pair.second.music);
        }
    }
    music.clear();
}

// Utility methods
std::string AudioManager::soundTypeToString(SoundType type) const {
    switch (type) {
        case SoundType::TASK_COMPLETE: return "TaskComplete";
        case SoundType::TIMER_START: return "TimerStart";
        case SoundType::TIMER_PAUSE: return "TimerPause";
        case SoundType::TIMER_RESUME: return "TimerResume";
        case SoundType::TIMER_STOP: return "TimerStop";
        case SoundType::TIMER_ALERT: return "TimerAlert";
        case SoundType::HABIT_CHECK_IN: return "HabitCheckIn";
        case SoundType::HABIT_STREAK: return "HabitStreak";
        case SoundType::BUILDING_ENTER: return "BuildingEnter";
        case SoundType::BUILDING_EXIT: return "BuildingExit";
        case SoundType::BUILDING_UPGRADE: return "BuildingUpgrade";
        case SoundType::DECORATION_PLACE: return "DecorationPlace";
        case SoundType::UI_CLICK: return "UIClick";
        case SoundType::UI_HOVER: return "UIHover";
        case SoundType::NOTIFICATION: return "Notification";
        case SoundType::ACHIEVEMENT: return "Achievement";
        case SoundType::COFFEE_REWARD: return "CoffeeReward";
        case SoundType::XP_GAIN: return "XPGain";
        default: return "Unknown";
    }
}

std::string AudioManager::musicTypeToString(MusicType type) const {
    switch (type) {
        case MusicType::TOWN_AMBIENT: return "TownAmbient";
        case MusicType::COFFEE_SHOP_AMBIENT: return "CoffeeShopAmbient";
        case MusicType::LIBRARY_AMBIENT: return "LibraryAmbient";
        case MusicType::GYM_AMBIENT: return "GymAmbient";
        case MusicType::HOME_AMBIENT: return "HomeAmbient";
        case MusicType::BULLETIN_BOARD_AMBIENT: return "BulletinBoardAmbient";
        case MusicType::FOCUS_SESSION: return "FocusSession";
        case MusicType::CELEBRATION: return "Celebration";
        default: return "Unknown";
    }
}

// Settings persistence
void AudioManager::saveSettings(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "AudioManager: Failed to save settings to " << filePath << std::endl;
        return;
    }
    
    file << "masterVolume=" << settings.masterVolume << std::endl;
    file << "soundEffectVolume=" << settings.soundEffectVolume << std::endl;
    file << "musicVolume=" << settings.musicVolume << std::endl;
    file << "soundEffectsEnabled=" << (settings.soundEffectsEnabled ? 1 : 0) << std::endl;
    file << "musicEnabled=" << (settings.musicEnabled ? 1 : 0) << std::endl;
    file << "muteWhenInactive=" << (settings.muteWhenInactive ? 1 : 0) << std::endl;
    file << "fadeInDuration=" << settings.fadeInDuration << std::endl;
    file << "fadeOutDuration=" << settings.fadeOutDuration << std::endl;
    file << "maxConcurrentSounds=" << settings.maxConcurrentSounds << std::endl;
    
    file.close();
    std::cout << "AudioManager: Settings saved to " << filePath << std::endl;
}

bool AudioManager::loadSettings(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "AudioManager: Settings file not found, using defaults" << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "masterVolume") {
            settings.masterVolume = std::stof(value);
        } else if (key == "soundEffectVolume") {
            settings.soundEffectVolume = std::stof(value);
        } else if (key == "musicVolume") {
            settings.musicVolume = std::stof(value);
        } else if (key == "soundEffectsEnabled") {
            settings.soundEffectsEnabled = (std::stoi(value) != 0);
        } else if (key == "musicEnabled") {
            settings.musicEnabled = (std::stoi(value) != 0);
        } else if (key == "muteWhenInactive") {
            settings.muteWhenInactive = (std::stoi(value) != 0);
        } else if (key == "fadeInDuration") {
            settings.fadeInDuration = std::stof(value);
        } else if (key == "fadeOutDuration") {
            settings.fadeOutDuration = std::stof(value);
        } else if (key == "maxConcurrentSounds") {
            settings.maxConcurrentSounds = std::stoi(value);
        }
    }
    
    file.close();
    
    // Apply loaded settings
    applySettings(settings);
    
    std::cout << "AudioManager: Settings loaded from " << filePath << std::endl;
    return true;
}