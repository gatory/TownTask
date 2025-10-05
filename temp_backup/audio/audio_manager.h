#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

enum class SoundType {
    TASK_COMPLETE,
    TIMER_START,
    TIMER_PAUSE,
    TIMER_RESUME,
    TIMER_STOP,
    TIMER_ALERT,
    HABIT_CHECK_IN,
    HABIT_STREAK,
    BUILDING_ENTER,
    BUILDING_EXIT,
    BUILDING_UPGRADE,
    DECORATION_PLACE,
    UI_CLICK,
    UI_HOVER,
    NOTIFICATION,
    ACHIEVEMENT,
    COFFEE_REWARD,
    XP_GAIN
};

enum class MusicType {
    TOWN_AMBIENT,
    COFFEE_SHOP_AMBIENT,
    LIBRARY_AMBIENT,
    GYM_AMBIENT,
    HOME_AMBIENT,
    BULLETIN_BOARD_AMBIENT,
    FOCUS_SESSION,
    CELEBRATION
};

struct AudioSettings {
    float masterVolume = 1.0f;
    float soundEffectVolume = 0.8f;
    float musicVolume = 0.6f;
    bool soundEffectsEnabled = true;
    bool musicEnabled = true;
    bool muteWhenInactive = true;
    
    // Advanced settings
    float fadeInDuration = 1.0f;
    float fadeOutDuration = 1.0f;
    int maxConcurrentSounds = 8;
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    // Initialization and cleanup
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    // Sound effect management
    bool loadSound(SoundType type, const std::string& filePath);
    void playSound(SoundType type, float volume = 1.0f);
    void playSoundWithPitch(SoundType type, float pitch, float volume = 1.0f);
    void stopSound(SoundType type);
    void stopAllSounds();
    
    // Music management
    bool loadMusic(MusicType type, const std::string& filePath);
    void playMusic(MusicType type, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void fadeInMusic(MusicType type, float duration = -1.0f);
    void fadeOutMusic(float duration = -1.0f);
    void crossfadeMusic(MusicType newType, float duration = 2.0f);
    
    // Volume and settings
    void setMasterVolume(float volume);
    void setSoundEffectVolume(float volume);
    void setMusicVolume(float volume);
    float getMasterVolume() const { return settings.masterVolume; }
    float getSoundEffectVolume() const { return settings.soundEffectVolume; }
    float getMusicVolume() const { return settings.musicVolume; }
    
    void enableSoundEffects(bool enabled);
    void enableMusic(bool enabled);
    bool areSoundEffectsEnabled() const { return settings.soundEffectsEnabled; }
    bool isMusicEnabled() const { return settings.musicEnabled; }
    
    // Settings management
    void applySettings(const AudioSettings& newSettings);
    const AudioSettings& getSettings() const { return settings; }
    void saveSettings(const std::string& filePath) const;
    bool loadSettings(const std::string& filePath);
    
    // Update and state management
    void update(float deltaTime);
    void setApplicationActive(bool active);
    
    // Convenience methods for common game events
    void playTaskCompleteSound();
    void playTimerAlertSound();
    void playHabitStreakSound(int streakCount);
    void playBuildingUpgradeSound();
    void playAchievementSound();
    void playUIClickSound();
    void playUIHoverSound();
    
    // Music context switching
    void switchToBuildingMusic(MusicType buildingType);
    void switchToTownMusic();
    void switchToFocusMusic();
    void switchToCelebrationMusic();
    
    // Event callbacks
    void setOnMusicFinished(std::function<void(MusicType)> callback);
    void setOnSoundFinished(std::function<void(SoundType)> callback);

private:
    struct SoundData {
        Sound sound;
        bool loaded = false;
        float baseVolume = 1.0f;
        int playCount = 0;
    };
    
    struct MusicData {
        Music music;
        bool loaded = false;
        float baseVolume = 1.0f;
        bool isPlaying = false;
        bool isPaused = false;
    };
    
    struct FadeState {
        bool active = false;
        float duration = 0.0f;
        float elapsed = 0.0f;
        float startVolume = 0.0f;
        float targetVolume = 0.0f;
        MusicType targetMusic = MusicType::TOWN_AMBIENT;
        bool isCrossfade = false;
    };
    
    // Core state
    bool initialized = false;
    AudioSettings settings;
    bool applicationActive = true;
    
    // Audio data
    std::unordered_map<SoundType, SoundData> sounds;
    std::unordered_map<MusicType, MusicData> music;
    MusicType currentMusic = MusicType::TOWN_AMBIENT;
    
    // Fade system
    FadeState fadeState;
    
    // Event callbacks
    std::function<void(MusicType)> onMusicFinished;
    std::function<void(SoundType)> onSoundFinished;
    
    // Internal helpers
    void updateFading(float deltaTime);
    void updateMusicStreaming();
    void updateSoundCleanup();
    float calculateEffectiveVolume(float baseVolume, bool isMusic) const;
    void applyVolumeToMusic(MusicType type);
    void applyVolumeToSound(SoundType type);
    
    // Resource management
    void unloadSound(SoundType type);
    void unloadMusic(MusicType type);
    void unloadAllAudio();
    
    // Utility
    std::string soundTypeToString(SoundType type) const;
    std::string musicTypeToString(MusicType type) const;
    
    // Constants
    static constexpr float MIN_VOLUME = 0.0f;
    static constexpr float MAX_VOLUME = 1.0f;
    static constexpr float DEFAULT_FADE_DURATION = 1.0f;
    static constexpr int MAX_SOUND_INSTANCES = 3;
};