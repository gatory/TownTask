#include "audio_integration.h"
#include <iostream>
#include <random>

AudioIntegration::AudioIntegration(std::shared_ptr<AudioManager> audioManager)
    : audioManager(audioManager) {
}

// Task system integration
void AudioIntegration::onTaskCreated() {
    audioManager->playUIClickSound();
}

void AudioIntegration::onTaskCompleted() {
    audioManager->playTaskCompleteSound();
    
    // Add a small XP gain sound after a brief delay
    // In a real implementation, you might use a timer or callback system
    audioManager->playSound(SoundType::XP_GAIN, 0.7f);
}

void AudioIntegration::onTaskDeleted() {
    audioManager->playSound(SoundType::UI_CLICK, 0.5f);
}

void AudioIntegration::onTaskPriorityChanged() {
    audioManager->playUIClickSound();
}

// Pomodoro timer integration
void AudioIntegration::onTimerStarted() {
    audioManager->playSound(SoundType::TIMER_START);
    
    // Switch to focus music if not already playing
    if (!isFocusSession) {
        setupFocusSessionMusic();
        isFocusSession = true;
    }
}

void AudioIntegration::onTimerPaused() {
    audioManager->playSound(SoundType::TIMER_PAUSE);
    audioManager->pauseMusic();
}

void AudioIntegration::onTimerResumed() {
    audioManager->playSound(SoundType::TIMER_RESUME);
    audioManager->resumeMusic();
}

void AudioIntegration::onTimerStopped() {
    audioManager->playSound(SoundType::TIMER_STOP);
    
    // Return to ambient music
    if (isFocusSession) {
        if (!currentBuildingType.empty()) {
            setupBuildingAmbientMusic(currentBuildingType);
        } else {
            audioManager->switchToTownMusic();
        }
        isFocusSession = false;
    }
}

void AudioIntegration::onTimerCompleted() {
    audioManager->playTimerAlertSound();
    
    // Brief celebration music
    setupCelebrationMusic();
    
    // Return to normal music after a few seconds (would need timer system)
    isFocusSession = false;
}

void AudioIntegration::onBreakStarted() {
    audioManager->playSound(SoundType::TIMER_START, 0.8f);
    
    // Switch to more relaxed ambient music
    if (!currentBuildingType.empty()) {
        setupBuildingAmbientMusic(currentBuildingType);
    }
}

void AudioIntegration::onBreakCompleted() {
    audioManager->playSound(SoundType::TIMER_ALERT, 0.9f);
}

// Habit tracking integration
void AudioIntegration::onHabitCheckedIn() {
    audioManager->playSound(SoundType::HABIT_CHECK_IN);
}

void AudioIntegration::onHabitStreakAchieved(int streakCount) {
    audioManager->playHabitStreakSound(streakCount);
    
    // Special effects for milestone streaks
    if (streakCount % 7 == 0) { // Weekly milestones
        audioManager->playSound(SoundType::ACHIEVEMENT, 0.8f);
    }
}

void AudioIntegration::onHabitMilestone(int days) {
    audioManager->playAchievementSound();
    
    // Play celebration music for major milestones
    if (days >= 30) {
        setupCelebrationMusic();
    }
}

// Building system integration
void AudioIntegration::onBuildingEntered(const std::string& buildingType) {
    audioManager->playSound(SoundType::BUILDING_ENTER);
    
    currentBuildingType = buildingType;
    
    // Switch to building-specific ambient music
    if (!isFocusSession) {
        setupBuildingAmbientMusic(buildingType);
    }
}

void AudioIntegration::onBuildingExited() {
    audioManager->playSound(SoundType::BUILDING_EXIT);
    
    currentBuildingType.clear();
    
    // Return to town music
    if (!isFocusSession) {
        audioManager->switchToTownMusic();
    }
}

void AudioIntegration::onBuildingUpgraded(const std::string& buildingType, int newLevel) {
    audioManager->playBuildingUpgradeSound();
    
    // Play achievement sound for max level
    if (newLevel >= 5) {
        audioManager->playAchievementSound();
        setupCelebrationMusic();
    }
}

void AudioIntegration::onDecorationPlaced() {
    audioManager->playSound(SoundType::DECORATION_PLACE);
}

void AudioIntegration::onDecorationPurchased() {
    audioManager->playSound(SoundType::COFFEE_REWARD, 0.8f);
}

// Gamification integration
void AudioIntegration::onXPGained(int amount) {
    // Different pitch based on amount
    float pitch = 1.0f + (amount / 100.0f) * 0.3f;
    audioManager->playSoundWithPitch(SoundType::XP_GAIN, pitch, 0.6f);
}

void AudioIntegration::onLevelUp(int newLevel) {
    audioManager->playAchievementSound();
    
    // Brief celebration for level ups
    setupCelebrationMusic();
}

void AudioIntegration::onAchievementUnlocked(const std::string& achievementId) {
    audioManager->playAchievementSound();
    
    // Major achievements get celebration music
    setupCelebrationMusic();
}

void AudioIntegration::onCoffeeTokensEarned(int amount) {
    audioManager->playSound(SoundType::COFFEE_REWARD);
}

// UI integration
void AudioIntegration::onButtonClicked() {
    audioManager->playUIClickSound();
}

void AudioIntegration::onButtonHovered() {
    audioManager->playUIHoverSound();
}

void AudioIntegration::onMenuOpened() {
    audioManager->playUIClickSound();
}

void AudioIntegration::onMenuClosed() {
    audioManager->playSound(SoundType::UI_CLICK, 0.7f);
}

void AudioIntegration::onNotificationShown() {
    audioManager->playSound(SoundType::NOTIFICATION);
}

void AudioIntegration::onErrorOccurred() {
    // Play a subtle error sound (could be a different pitch of UI click)
    audioManager->playSoundWithPitch(SoundType::UI_CLICK, 0.7f, 0.8f);
}

// Screen transition integration
void AudioIntegration::onScreenChanged(const std::string& newScreen) {
    audioManager->playUIClickSound();
    
    // Update music based on screen
    if (newScreen == "town") {
        audioManager->switchToTownMusic();
        currentBuildingType.clear();
    }
}

void AudioIntegration::setupBuildingAmbientMusic(const std::string& buildingType) {
    MusicType musicType = getBuildingMusicType(buildingType);
    audioManager->switchToBuildingMusic(musicType);
}

void AudioIntegration::setupFocusSessionMusic() {
    audioManager->switchToFocusMusic();
}

void AudioIntegration::setupCelebrationMusic() {
    audioManager->switchToCelebrationMusic();
}

// Volume and settings shortcuts
void AudioIntegration::setGameVolume(float volume) {
    audioManager->setMasterVolume(volume);
}

void AudioIntegration::toggleMute() {
    if (isMuted) {
        audioManager->setMasterVolume(previousVolume);
        isMuted = false;
    } else {
        previousVolume = audioManager->getMasterVolume();
        audioManager->setMasterVolume(0.0f);
        isMuted = true;
    }
}

void AudioIntegration::enableGameAudio(bool enabled) {
    audioManager->enableSoundEffects(enabled);
    audioManager->enableMusic(enabled);
}

// Preload audio assets
bool AudioIntegration::preloadEssentialAudio() {
    bool success = true;
    
    // Essential UI sounds
    success &= audioManager->loadSound(SoundType::UI_CLICK, "assets/sounds/ui_click.wav");
    success &= audioManager->loadSound(SoundType::UI_HOVER, "assets/sounds/ui_hover.wav");
    success &= audioManager->loadSound(SoundType::NOTIFICATION, "assets/sounds/notification.wav");
    
    // Essential game sounds
    success &= audioManager->loadSound(SoundType::TASK_COMPLETE, "assets/sounds/task_complete.wav");
    success &= audioManager->loadSound(SoundType::TIMER_ALERT, "assets/sounds/timer_alert.wav");
    success &= audioManager->loadSound(SoundType::ACHIEVEMENT, "assets/sounds/achievement.wav");
    
    // Essential music
    success &= audioManager->loadMusic(MusicType::TOWN_AMBIENT, "assets/music/town_ambient.ogg");
    
    if (success) {
        std::cout << "AudioIntegration: Essential audio loaded successfully" << std::endl;
    } else {
        std::cout << "AudioIntegration: Some essential audio files failed to load" << std::endl;
    }
    
    return success;
}

bool AudioIntegration::preloadBuildingAudio() {
    bool success = true;
    
    // Building sounds
    success &= audioManager->loadSound(SoundType::BUILDING_ENTER, "assets/sounds/building_enter.wav");
    success &= audioManager->loadSound(SoundType::BUILDING_EXIT, "assets/sounds/building_exit.wav");
    success &= audioManager->loadSound(SoundType::BUILDING_UPGRADE, "assets/sounds/building_upgrade.wav");
    success &= audioManager->loadSound(SoundType::DECORATION_PLACE, "assets/sounds/decoration_place.wav");
    
    // Building ambient music
    success &= audioManager->loadMusic(MusicType::COFFEE_SHOP_AMBIENT, "assets/music/coffee_shop_ambient.ogg");
    success &= audioManager->loadMusic(MusicType::LIBRARY_AMBIENT, "assets/music/library_ambient.ogg");
    success &= audioManager->loadMusic(MusicType::GYM_AMBIENT, "assets/music/gym_ambient.ogg");
    success &= audioManager->loadMusic(MusicType::HOME_AMBIENT, "assets/music/home_ambient.ogg");
    success &= audioManager->loadMusic(MusicType::BULLETIN_BOARD_AMBIENT, "assets/music/bulletin_board_ambient.ogg");
    
    if (success) {
        std::cout << "AudioIntegration: Building audio loaded successfully" << std::endl;
    } else {
        std::cout << "AudioIntegration: Some building audio files failed to load" << std::endl;
    }
    
    return success;
}

bool AudioIntegration::preloadGameplayAudio() {
    bool success = true;
    
    // Timer sounds
    success &= audioManager->loadSound(SoundType::TIMER_START, "assets/sounds/timer_start.wav");
    success &= audioManager->loadSound(SoundType::TIMER_PAUSE, "assets/sounds/timer_pause.wav");
    success &= audioManager->loadSound(SoundType::TIMER_RESUME, "assets/sounds/timer_resume.wav");
    success &= audioManager->loadSound(SoundType::TIMER_STOP, "assets/sounds/timer_stop.wav");
    
    // Habit sounds
    success &= audioManager->loadSound(SoundType::HABIT_CHECK_IN, "assets/sounds/habit_checkin.wav");
    success &= audioManager->loadSound(SoundType::HABIT_STREAK, "assets/sounds/habit_streak.wav");
    
    // Gamification sounds
    success &= audioManager->loadSound(SoundType::XP_GAIN, "assets/sounds/xp_gain.wav");
    success &= audioManager->loadSound(SoundType::COFFEE_REWARD, "assets/sounds/coffee_reward.wav");
    
    // Special music
    success &= audioManager->loadMusic(MusicType::FOCUS_SESSION, "assets/music/focus_session.ogg");
    success &= audioManager->loadMusic(MusicType::CELEBRATION, "assets/music/celebration.ogg");
    
    if (success) {
        std::cout << "AudioIntegration: Gameplay audio loaded successfully" << std::endl;
    } else {
        std::cout << "AudioIntegration: Some gameplay audio files failed to load" << std::endl;
    }
    
    return success;
}

// Private helper methods
MusicType AudioIntegration::getBuildingMusicType(const std::string& buildingType) const {
    if (buildingType == "coffee_shop") return MusicType::COFFEE_SHOP_AMBIENT;
    if (buildingType == "library") return MusicType::LIBRARY_AMBIENT;
    if (buildingType == "gym") return MusicType::GYM_AMBIENT;
    if (buildingType == "home") return MusicType::HOME_AMBIENT;
    if (buildingType == "bulletin_board") return MusicType::BULLETIN_BOARD_AMBIENT;
    
    return MusicType::TOWN_AMBIENT; // Default fallback
}

void AudioIntegration::playRandomVariation(SoundType baseType, int variations) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pitchDist(0.9f, 1.1f);
    
    float randomPitch = pitchDist(gen);
    audioManager->playSoundWithPitch(baseType, randomPitch);
}

void AudioIntegration::scheduleDelayedSound(SoundType type, float delay) {
    // In a real implementation, this would use a timer system
    // For now, it's just a placeholder
    std::cout << "AudioIntegration: Would schedule " << (int)type << " after " << delay << "s" << std::endl;
}