#pragma once

#include "screen.h"
#include "../../core/engines/pomodoro_timer.h"
#include "../../core/engines/gamification_engine.h"
#include "../../input/input_manager.h"
#include "../../ui/animations/animation_manager.h"
#include "../../audio/audio_integration.h"
#include <memory>
#include <string>
#include <functional>

class CoffeeShopScreen : public Screen {
public:
    CoffeeShopScreen(PomodoroTimer& pomodoroTimer, GamificationEngine& gamificationEngine, 
                     InputManager& inputManager, AnimationManager& animationManager,
                     std::shared_ptr<AudioIntegration> audioIntegration = nullptr);
    ~CoffeeShopScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Coffee shop specific functionality
    void startPomodoroSession(PomodoroTimer::SessionType sessionType, int customMinutes = 0);
    void pauseTimer();
    void resumeTimer();
    void stopTimer();
    
    // UI state management
    void setShowCustomDurationDialog(bool show);
    bool isShowingCustomDurationDialog() const;
    void setCustomDuration(int minutes);
    int getCustomDuration() const;
    
    // Visual settings
    void setShowTimerDetails(bool show);
    bool isShowingTimerDetails() const;
    void setShowCoffeeRewards(bool show);
    bool isShowingCoffeeRewards() const;
    void setShowSessionHistory(bool show);
    bool isShowingSessionHistory() const;
    
    // Coffee shop atmosphere
    void setCoffeeShopLevel(int level);
    int getCoffeeShopLevel() const;
    void addDecoration(const std::string& decorationId);
    void removeDecoration(const std::string& decorationId);
    std::vector<std::string> getDecorations() const;
    
    // Event callbacks
    void setOnSessionCompleted(std::function<void(PomodoroTimer::SessionType, int)> callback);
    void setOnCoffeeEarned(std::function<void(int)> callback);
    void setOnExitRequested(std::function<void()> callback);

private:
    // Core references
    PomodoroTimer& pomodoroTimer;
    GamificationEngine& gamificationEngine;
    InputManager& inputManager;
    AnimationManager& animationManager;
    std::shared_ptr<AudioIntegration> audioIntegration;
    
    // UI state
    bool showCustomDurationDialog;
    int customDuration;
    bool showTimerDetails;
    bool showCoffeeRewards;
    bool showSessionHistory;
    
    // Coffee shop state
    int coffeeShopLevel;
    std::vector<std::string> decorations;
    
    // Animation state
    std::shared_ptr<SpriteAnimator> characterAnimator;
    std::shared_ptr<SpriteAnimator> coffeeAnimator;
    std::string currentCharacterAnimation;
    
    // UI elements state
    struct TimerDisplay {
        float progressBarWidth;
        float progressBarHeight;
        float progressFillAmount; // 0.0 to 1.0
        std::string timeText;
        std::string sessionTypeText;
        bool isBlinking;
        float blinkTimer;
    } timerDisplay;
    
    struct CoffeeRewardDisplay {
        int currentCoffeeCount;
        int earnedThisSession;
        bool showEarnedAnimation;
        float earnedAnimationTimer;
        std::vector<std::string> recentRewards;
    } coffeeRewardDisplay;
    
    // Button states
    enum class ButtonState { NORMAL, HOVERED, PRESSED };
    struct Button {
        float x, y, width, height;
        std::string text;
        ButtonState state;
        bool enabled;
        std::function<void()> onClick;
        
        Button(float x, float y, float w, float h, const std::string& text)
            : x(x), y(y), width(w), height(h), text(text), state(ButtonState::NORMAL), enabled(true) {}
    };
    
    std::vector<Button> buttons;
    int hoveredButtonIndex;
    
    // Event callbacks
    std::function<void(PomodoroTimer::SessionType, int)> onSessionCompleted;
    std::function<void(int)> onCoffeeEarned;
    std::function<void()> onExitRequested;
    
    // Internal update methods
    void updateTimer(float deltaTime);
    void updateAnimations(float deltaTime);
    void updateUI(float deltaTime);
    void updateButtons(const InputState& input);
    
    // Rendering methods
    void renderBackground();
    void renderCoffeeShopInterior();
    void renderCharacter();
    void renderTimerInterface();
    void renderProgressBar();
    void renderTimerControls();
    void renderCoffeeRewards();
    void renderSessionHistory();
    void renderCustomDurationDialog();
    void renderButtons();
    void renderDecorations();
    
    // UI helper methods
    void createButtons();
    void updateButtonStates(const InputState& input);
    bool isPointInButton(const Button& button, float x, float y) const;
    void handleButtonClick(int buttonIndex);
    
    // Timer event handlers
    void onTimerSessionCompleted(PomodoroTimer::SessionType sessionType);
    void onTimerSessionInterrupted(PomodoroTimer::SessionType sessionType);
    void onTimerTick(int remainingSeconds);
    void onTimerStateChanged(PomodoroTimer::TimerState oldState, PomodoroTimer::TimerState newState);
    
    // Animation helpers
    void playCharacterAnimation(const std::string& animationName);
    void updateCharacterAnimationForTimerState();
    std::string getTimerStateAnimationName() const;
    
    // Coffee reward helpers
    void awardCoffee(int amount);
    void showCoffeeEarnedAnimation();
    void updateCoffeeDisplay();
    
    // UI formatting helpers
    std::string formatTime(int seconds) const;
    std::string getSessionTypeDisplayName(PomodoroTimer::SessionType sessionType) const;
    std::string getTimerStateDisplayText() const;
    
    // Coffee shop visual helpers
    void renderCoffeeShopByLevel();
    void renderDecoration(const std::string& decorationId, float x, float y);
    Color getCoffeeShopAmbientColor() const;
    
    // Input helpers
    void handleKeyboardInput(const InputState& input);
    void handleMouseInput(const InputState& input);
    
    // Constants
    static constexpr float TIMER_PROGRESS_BAR_WIDTH = 400.0f;
    static constexpr float TIMER_PROGRESS_BAR_HEIGHT = 20.0f;
    static constexpr float BUTTON_WIDTH = 120.0f;
    static constexpr float BUTTON_HEIGHT = 40.0f;
    static constexpr float COFFEE_ANIMATION_DURATION = 2.0f;
    static constexpr int DEFAULT_WORK_DURATION = 25;
    static constexpr int DEFAULT_SHORT_BREAK_DURATION = 5;
    static constexpr int DEFAULT_LONG_BREAK_DURATION = 15;
};