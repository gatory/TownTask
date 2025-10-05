#include <gtest/gtest.h>
#include "../src/audio/audio_manager.h"
#include <thread>
#include <chrono>

class AudioManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        audioManager = std::make_unique<AudioManager>();
        // Note: We won't actually initialize audio in tests to avoid hardware dependencies
    }
    
    void TearDown() override {
        audioManager.reset();
    }
    
    std::unique_ptr<AudioManager> audioManager;
};

TEST_F(AudioManagerTest, InitializationTest) {
    // Test that AudioManager can be created
    EXPECT_FALSE(audioManager->isInitialized());
    
    // Test settings defaults
    const auto& settings = audioManager->getSettings();
    EXPECT_FLOAT_EQ(settings.masterVolume, 1.0f);
    EXPECT_FLOAT_EQ(settings.soundEffectVolume, 0.8f);
    EXPECT_FLOAT_EQ(settings.musicVolume, 0.6f);
    EXPECT_TRUE(settings.soundEffectsEnabled);
    EXPECT_TRUE(settings.musicEnabled);
}

TEST_F(AudioManagerTest, VolumeControlTest) {
    // Test volume setting
    audioManager->setMasterVolume(0.5f);
    EXPECT_FLOAT_EQ(audioManager->getMasterVolume(), 0.5f);
    
    audioManager->setSoundEffectVolume(0.7f);
    EXPECT_FLOAT_EQ(audioManager->getSoundEffectVolume(), 0.7f);
    
    audioManager->setMusicVolume(0.3f);
    EXPECT_FLOAT_EQ(audioManager->getMusicVolume(), 0.3f);
    
    // Test volume clamping
    audioManager->setMasterVolume(1.5f);
    EXPECT_FLOAT_EQ(audioManager->getMasterVolume(), 1.0f);
    
    audioManager->setMasterVolume(-0.5f);
    EXPECT_FLOAT_EQ(audioManager->getMasterVolume(), 0.0f);
}

TEST_F(AudioManagerTest, EnableDisableTest) {
    // Test sound effects enable/disable
    audioManager->enableSoundEffects(false);
    EXPECT_FALSE(audioManager->areSoundEffectsEnabled());
    
    audioManager->enableSoundEffects(true);
    EXPECT_TRUE(audioManager->areSoundEffectsEnabled());
    
    // Test music enable/disable
    audioManager->enableMusic(false);
    EXPECT_FALSE(audioManager->isMusicEnabled());
    
    audioManager->enableMusic(true);
    EXPECT_TRUE(audioManager->isMusicEnabled());
}

TEST_F(AudioManagerTest, SettingsTest) {
    AudioSettings newSettings;
    newSettings.masterVolume = 0.8f;
    newSettings.soundEffectVolume = 0.9f;
    newSettings.musicVolume = 0.4f;
    newSettings.soundEffectsEnabled = false;
    newSettings.musicEnabled = false;
    newSettings.muteWhenInactive = false;
    
    audioManager->applySettings(newSettings);
    
    const auto& appliedSettings = audioManager->getSettings();
    EXPECT_FLOAT_EQ(appliedSettings.masterVolume, 0.8f);
    EXPECT_FLOAT_EQ(appliedSettings.soundEffectVolume, 0.9f);
    EXPECT_FLOAT_EQ(appliedSettings.musicVolume, 0.4f);
    EXPECT_FALSE(appliedSettings.soundEffectsEnabled);
    EXPECT_FALSE(appliedSettings.musicEnabled);
    EXPECT_FALSE(appliedSettings.muteWhenInactive);
}

TEST_F(AudioManagerTest, ConvenienceMethodsTest) {
    // Test that convenience methods don't crash
    audioManager->playTaskCompleteSound();
    audioManager->playTimerAlertSound();
    audioManager->playHabitStreakSound(5);
    audioManager->playBuildingUpgradeSound();
    audioManager->playAchievementSound();
    audioManager->playUIClickSound();
    audioManager->playUIHoverSound();
    
    // Test music switching
    audioManager->switchToTownMusic();
    audioManager->switchToFocusMusic();
    audioManager->switchToCelebrationMusic();
    audioManager->switchToBuildingMusic(MusicType::COFFEE_SHOP_AMBIENT);
}

TEST_F(AudioManagerTest, ApplicationActiveTest) {
    // Test application active state
    audioManager->setApplicationActive(false);
    audioManager->setApplicationActive(true);
    
    // Should not crash
}

TEST_F(AudioManagerTest, UpdateTest) {
    // Test update method
    audioManager->update(0.016f); // 60 FPS
    audioManager->update(0.033f); // 30 FPS
    
    // Should not crash
}

TEST_F(AudioManagerTest, CallbackTest) {
    bool musicFinishedCalled = false;
    bool soundFinishedCalled = false;
    
    audioManager->setOnMusicFinished([&](MusicType type) {
        musicFinishedCalled = true;
    });
    
    audioManager->setOnSoundFinished([&](SoundType type) {
        soundFinishedCalled = true;
    });
    
    // Callbacks are set (we can't easily test them being called without actual audio)
    EXPECT_FALSE(musicFinishedCalled);
    EXPECT_FALSE(soundFinishedCalled);
}

// Integration test that would work with actual audio files
TEST_F(AudioManagerTest, DISABLED_IntegrationTest) {
    // This test is disabled because it requires actual audio files and hardware
    // In a real scenario, you would:
    // 1. Initialize the audio manager
    // 2. Load test audio files
    // 3. Play sounds and music
    // 4. Test volume changes
    // 5. Test fading
    
    /*
    ASSERT_TRUE(audioManager->initialize());
    
    // Load test files (would need actual audio files)
    ASSERT_TRUE(audioManager->loadSound(SoundType::UI_CLICK, "assets/sounds/click.wav"));
    ASSERT_TRUE(audioManager->loadMusic(MusicType::TOWN_AMBIENT, "assets/music/town.ogg"));
    
    // Test playing
    audioManager->playSound(SoundType::UI_CLICK);
    audioManager->playMusic(MusicType::TOWN_AMBIENT);
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test volume changes
    audioManager->setMasterVolume(0.5f);
    
    // Test fading
    audioManager->fadeOutMusic(1.0f);
    
    // Update for fade
    for (int i = 0; i < 60; ++i) {
        audioManager->update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    */
}