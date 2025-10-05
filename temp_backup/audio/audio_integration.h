#pragma once

#include "audio_manager.h"
#include <memory>

// Helper class to integrate audio with game systems
class AudioIntegration {
public:
    AudioIntegration(std::shared_ptr<AudioManager> audioManager);
    
    // Task system integration
    void onTaskCreated();
    void onTaskCompleted();
    void onTaskDeleted();
    void onTaskPriorityChanged();
    
    // Pomodoro timer integration
    void onTimerStarted();
    void onTimerPaused();
    void onTimerResumed();
    void onTimerStopped();
    void onTimerCompleted();
    void onBreakStarted();
    void onBreakCompleted();
    
    // Habit tracking integration
    void onHabitCheckedIn();
    void onHabitStreakAchieved(int streakCount);
    void onHabitMilestone(int days);
    
    // Building system integration
    void onBuildingEntered(const std::string& buildingType);
    void onBuildingExited();
    void onBuildingUpgraded(const std::string& buildingType, int newLevel);
    void onDecorationPlaced();
    void onDecorationPurchased();
    
    // Gamification integration
    void onXPGained(int amount);
    void onLevelUp(int newLevel);
    void onAchievementUnlocked(const std::string& achievementId);
    void onCoffeeTokensEarned(int amount);
    
    // UI integration
    void onButtonClicked();
    void onButtonHovered();
    void onMenuOpened();
    void onMenuClosed();
    void onNotificationShown();
    void onErrorOccurred();
    
    // Screen transition integration
    void onScreenChanged(const std::string& newScreen);
    void setupBuildingAmbientMusic(const std::string& buildingType);
    void setupFocusSessionMusic();
    void setupCelebrationMusic();
    
    // Volume and settings shortcuts
    void setGameVolume(float volume);
    void toggleMute();
    void enableGameAudio(bool enabled);
    
    // Preload common audio assets
    bool preloadEssentialAudio();
    bool preloadBuildingAudio();
    bool preloadGameplayAudio();
    
private:
    std::shared_ptr<AudioManager> audioManager;
    
    // State tracking
    std::string currentBuildingType;
    bool isFocusSession = false;
    bool isMuted = false;
    float previousVolume = 1.0f;
    
    // Helper methods
    MusicType getBuildingMusicType(const std::string& buildingType) const;
    void playRandomVariation(SoundType baseType, int variations = 3);
    void scheduleDelayedSound(SoundType type, float delay);
};