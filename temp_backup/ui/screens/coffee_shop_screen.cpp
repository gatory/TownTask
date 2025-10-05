#include "coffee_shop_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

CoffeeShopScreen::CoffeeShopScreen(PomodoroTimer& pomodoroTimer, GamificationEngine& gamificationEngine,
                                   InputManager& inputManager, AnimationManager& animationManager,
                                   std::shared_ptr<AudioIntegration> audioIntegration)
    : Screen("CoffeeShopScreen")
    , pomodoroTimer(pomodoroTimer)
    , gamificationEngine(gamificationEngine)
    , inputManager(inputManager)
    , animationManager(animationManager)
    , audioIntegration(audioIntegration)
    , showCustomDurationDialog(false)
    , customDuration(DEFAULT_WORK_DURATION)
    , showTimerDetails(true)
    , showCoffeeRewards(true)
    , showSessionHistory(false)
    , coffeeShopLevel(1)
    , hoveredButtonIndex(-1) {
    
    // Initialize timer display
    timerDisplay.progressBarWidth = TIMER_PROGRESS_BAR_WIDTH;
    timerDisplay.progressBarHeight = TIMER_PROGRESS_BAR_HEIGHT;
    timerDisplay.progressFillAmount = 0.0f;
    timerDisplay.timeText = "25:00";
    timerDisplay.sessionTypeText = "Ready to Focus";
    timerDisplay.isBlinking = false;
    timerDisplay.blinkTimer = 0.0f;
    
    // Initialize coffee reward display
    coffeeRewardDisplay.currentCoffeeCount = 0;
    coffeeRewardDisplay.earnedThisSession = 0;
    coffeeRewardDisplay.showEarnedAnimation = false;
    coffeeRewardDisplay.earnedAnimationTimer = 0.0f;
    
    // Set up timer callbacks
    pomodoroTimer.setOnSessionCompleted([this](PomodoroTimer::SessionType sessionType) {
        onTimerSessionCompleted(sessionType);
    });
    
    pomodoroTimer.setOnSessionInterrupted([this](PomodoroTimer::SessionType sessionType) {
        onTimerSessionInterrupted(sessionType);
    });
    
    pomodoroTimer.setOnTick([this](int remainingSeconds) {
        onTimerTick(remainingSeconds);
    });
    
    pomodoroTimer.setOnStateChanged([this](PomodoroTimer::TimerState oldState, PomodoroTimer::TimerState newState) {
        onTimerStateChanged(oldState, newState);
    });
    
    // Create UI buttons
    createButtons();
    
    std::cout << "CoffeeShopScreen: Initialized successfully" << std::endl;
}

CoffeeShopScreen::~CoffeeShopScreen() {
    // Cleanup is handled by smart pointers and references
}

// Screen interface implementation
void CoffeeShopScreen::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update timer
    updateTimer(deltaTime);
    
    // Update animations
    updateAnimations(deltaTime);
    
    // Update UI elements
    updateUI(deltaTime);
    
    // Update button states
    const InputState& input = inputManager.getCurrentState();
    updateButtons(input);
}

void CoffeeShopScreen::render() {
    if (!isActive()) return;
    
    // Render background and coffee shop interior
    renderBackground();
    renderCoffeeShopInterior();
    
    // Render character
    renderCharacter();
    
    // Render timer interface
    renderTimerInterface();
    
    // Render coffee rewards
    if (showCoffeeRewards) {
        renderCoffeeRewards();
    }
    
    // Render session history
    if (showSessionHistory) {
        renderSessionHistory();
    }
    
    // Render custom duration dialog if shown
    if (showCustomDurationDialog) {
        renderCustomDurationDialog();
    }
    
    // Render decorations
    renderDecorations();
}

void CoffeeShopScreen::handleInput(const InputState& input) {
    if (!isActive()) return;
    
    // Handle keyboard input
    handleKeyboardInput(input);
    
    // Handle mouse input
    handleMouseInput(input);
    
    // Handle button interactions
    updateButtonStates(input);
}

void CoffeeShopScreen::onEnter() {
    Screen::onEnter();
    
    // Audio feedback for entering building
    if (audioIntegration) {
        audioIntegration->onBuildingEntered("coffee_shop");
    }
    
    // Initialize character animator
    characterAnimator = animationManager.createAnimator("coffee_character", "character_sheet");
    if (characterAnimator) {
        animationManager.createCharacterAnimations("coffee_character", "character_sheet", 32, 32);
        playCharacterAnimation("idle");
    }
    
    // Initialize coffee animator
    coffeeAnimator = animationManager.createAnimator("coffee_effects", "coffee_sheet");
    if (coffeeAnimator) {
        animationManager.createEffectAnimations("coffee_effects", "coffee_sheet", 64, 64);
    }
    
    // Update coffee count from gamification engine
    coffeeRewardDisplay.currentCoffeeCount = gamificationEngine.getCoffeeTokens();
    
    std::cout << "CoffeeShopScreen: Entered coffee shop" << std::endl;
}

void CoffeeShopScreen::onExit() {
    // Audio feedback for exiting building
    if (audioIntegration) {
        audioIntegration->onBuildingExited();
    }
    
    // Stop any running timer when exiting
    if (pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING) {
        pomodoroTimer.pause();
    }
    
    Screen::onExit();
    std::cout << "CoffeeShopScreen: Exited coffee shop" << std::endl;
}

// Coffee shop specific functionality
void CoffeeShopScreen::startPomodoroSession(PomodoroTimer::SessionType sessionType, int customMinutes) {
    if (customMinutes > 0) {
        pomodoroTimer.start(sessionType, customMinutes);
    } else {
        pomodoroTimer.start(sessionType);
    }
    
    // Audio feedback for timer start
    if (audioIntegration) {
        audioIntegration->onTimerStarted();
    }
    
    // Reset earned coffee for this session
    coffeeRewardDisplay.earnedThisSession = 0;
    
    // Update character animation
    updateCharacterAnimationForTimerState();
    
    std::cout << "CoffeeShopScreen: Started " << getSessionTypeDisplayName(sessionType) 
              << " session" << std::endl;
}

void CoffeeShopScreen::pauseTimer() {
    pomodoroTimer.pause();
    
    // Audio feedback for timer pause
    if (audioIntegration) {
        audioIntegration->onTimerPaused();
    }
    
    updateCharacterAnimationForTimerState();
}

void CoffeeShopScreen::resumeTimer() {
    pomodoroTimer.resume();
    
    // Audio feedback for timer resume
    if (audioIntegration) {
        audioIntegration->onTimerResumed();
    }
    
    updateCharacterAnimationForTimerState();
}

void CoffeeShopScreen::stopTimer() {
    pomodoroTimer.stop();
    
    // Audio feedback for timer stop
    if (audioIntegration) {
        audioIntegration->onTimerStopped();
    }
    
    updateCharacterAnimationForTimerState();
}

// UI state management
void CoffeeShopScreen::setShowCustomDurationDialog(bool show) {
    showCustomDurationDialog = show;
}

bool CoffeeShopScreen::isShowingCustomDurationDialog() const {
    return showCustomDurationDialog;
}

void CoffeeShopScreen::setCustomDuration(int minutes) {
    customDuration = std::max(1, std::min(120, minutes)); // Clamp between 1 and 120 minutes
}

int CoffeeShopScreen::getCustomDuration() const {
    return customDuration;
}

// Visual settings
void CoffeeShopScreen::setShowTimerDetails(bool show) {
    showTimerDetails = show;
}

bool CoffeeShopScreen::isShowingTimerDetails() const {
    return showTimerDetails;
}

void CoffeeShopScreen::setShowCoffeeRewards(bool show) {
    showCoffeeRewards = show;
}

bool CoffeeShopScreen::isShowingCoffeeRewards() const {
    return showCoffeeRewards;
}

void CoffeeShopScreen::setShowSessionHistory(bool show) {
    showSessionHistory = show;
}

bool CoffeeShopScreen::isShowingSessionHistory() const {
    return showSessionHistory;
}

// Coffee shop atmosphere
void CoffeeShopScreen::setCoffeeShopLevel(int level) {
    coffeeShopLevel = std::max(1, std::min(5, level)); // Clamp between 1 and 5
}

int CoffeeShopScreen::getCoffeeShopLevel() const {
    return coffeeShopLevel;
}

void CoffeeShopScreen::addDecoration(const std::string& decorationId) {
    if (std::find(decorations.begin(), decorations.end(), decorationId) == decorations.end()) {
        decorations.push_back(decorationId);
    }
}

void CoffeeShopScreen::removeDecoration(const std::string& decorationId) {
    decorations.erase(std::remove(decorations.begin(), decorations.end(), decorationId), decorations.end());
}

std::vector<std::string> CoffeeShopScreen::getDecorations() const {
    return decorations;
}

// Event callbacks
void CoffeeShopScreen::setOnSessionCompleted(std::function<void(PomodoroTimer::SessionType, int)> callback) {
    onSessionCompleted = callback;
}

void CoffeeShopScreen::setOnCoffeeEarned(std::function<void(int)> callback) {
    onCoffeeEarned = callback;
}

void CoffeeShopScreen::setOnExitRequested(std::function<void()> callback) {
    onExitRequested = callback;
}
//
 Internal update methods
void CoffeeShopScreen::updateTimer(float deltaTime) {
    // Update the pomodoro timer
    pomodoroTimer.update();
    
    // Update timer display
    timerDisplay.progressFillAmount = pomodoroTimer.getProgress();
    timerDisplay.timeText = formatTime(pomodoroTimer.getRemainingTimeSeconds());
    timerDisplay.sessionTypeText = getSessionTypeDisplayName(pomodoroTimer.getCurrentSessionType());
    
    // Update blinking effect for completed sessions
    if (pomodoroTimer.getState() == PomodoroTimer::TimerState::COMPLETED) {
        timerDisplay.isBlinking = true;
        timerDisplay.blinkTimer += deltaTime;
        if (timerDisplay.blinkTimer >= 1.0f) {
            timerDisplay.blinkTimer = 0.0f;
        }
    } else {
        timerDisplay.isBlinking = false;
        timerDisplay.blinkTimer = 0.0f;
    }
}

void CoffeeShopScreen::updateAnimations(float deltaTime) {
    // Update character animator
    if (characterAnimator) {
        characterAnimator->update(deltaTime);
    }
    
    // Update coffee animator
    if (coffeeAnimator) {
        coffeeAnimator->update(deltaTime);
    }
    
    // Update coffee earned animation
    if (coffeeRewardDisplay.showEarnedAnimation) {
        coffeeRewardDisplay.earnedAnimationTimer += deltaTime;
        if (coffeeRewardDisplay.earnedAnimationTimer >= COFFEE_ANIMATION_DURATION) {
            coffeeRewardDisplay.showEarnedAnimation = false;
            coffeeRewardDisplay.earnedAnimationTimer = 0.0f;
        }
    }
}

void CoffeeShopScreen::updateUI(float deltaTime) {
    // Update coffee count from gamification engine
    coffeeRewardDisplay.currentCoffeeCount = gamificationEngine.getCoffeeTokens();
    
    // Update coffee shop level based on upgrades
    // This would typically come from the gamification engine or town state
    // For now, we'll keep it simple
}

void CoffeeShopScreen::updateButtons(const InputState& input) {
    // This will be called from handleInput, so we don't need to duplicate logic here
}

// Rendering methods
void CoffeeShopScreen::renderBackground() {
    // Draw coffee shop background color
    Color bgColor = getCoffeeShopAmbientColor();
    ClearBackground(bgColor);
    
    // Draw a simple wood floor pattern
    const int tileSize = 32;
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color floorColor = ((x + y) / tileSize) % 2 == 0 ? BROWN : Fade(BROWN, 0.8f);
            DrawRectangle(x, y, tileSize, tileSize, floorColor);
        }
    }
}

void CoffeeShopScreen::renderCoffeeShopInterior() {
    renderCoffeeShopByLevel();
    
    // Draw basic coffee shop elements
    // Counter
    DrawRectangle(50, 600, 300, 100, DARKBROWN);
    DrawRectangleLines(50, 600, 300, 100, BLACK);
    
    // Coffee machine
    DrawRectangle(80, 620, 60, 60, GRAY);
    DrawRectangleLines(80, 620, 60, 60, BLACK);
    DrawText("Coffee", 85, 635, 12, WHITE);
    DrawText("Machine", 85, 650, 12, WHITE);
    
    // Tables and chairs (simple rectangles for now)
    DrawRectangle(400, 500, 80, 80, BROWN);
    DrawRectangleLines(400, 500, 80, 80, BLACK);
    
    DrawRectangle(600, 400, 80, 80, BROWN);
    DrawRectangleLines(600, 400, 80, 80, BLACK);
}

void CoffeeShopScreen::renderCharacter() {
    // Draw character at a fixed position in the coffee shop
    Vector2 characterPos = {200, 450};
    
    // Draw character as a simple colored circle (placeholder)
    Color characterColor = BLUE;
    
    // Change color based on timer state
    switch (pomodoroTimer.getState()) {
        case PomodoroTimer::TimerState::RUNNING:
            characterColor = GREEN; // Focused
            break;
        case PomodoroTimer::TimerState::PAUSED:
            characterColor = YELLOW; // Paused
            break;
        case PomodoroTimer::TimerState::COMPLETED:
            characterColor = GOLD; // Completed
            break;
        default:
            characterColor = BLUE; // Default
            break;
    }
    
    // Draw character
    DrawCircle((int)characterPos.x, (int)characterPos.y, 16, characterColor);
    DrawCircleLines((int)characterPos.x, (int)characterPos.y, 16, BLACK);
    
    // Draw focus indicator when timer is running
    if (pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING) {
        // Draw concentration lines around character
        for (int i = 0; i < 8; i++) {
            float angle = (i * 45.0f) * DEG2RAD;
            float startRadius = 20.0f;
            float endRadius = 30.0f;
            Vector2 start = {characterPos.x + cos(angle) * startRadius, characterPos.y + sin(angle) * startRadius};
            Vector2 end = {characterPos.x + cos(angle) * endRadius, characterPos.y + sin(angle) * endRadius};
            DrawLineV(start, end, YELLOW);
        }
    }
}

void CoffeeShopScreen::renderTimerInterface() {
    // Main timer display area
    int centerX = 512;
    int timerY = 100;
    
    // Draw timer background panel
    DrawRectangle(centerX - 250, timerY - 50, 500, 200, Fade(BLACK, 0.7f));
    DrawRectangleLines(centerX - 250, timerY - 50, 500, 200, WHITE);
    
    // Draw session type text
    const char* sessionText = timerDisplay.sessionTypeText.c_str();
    int sessionTextWidth = MeasureText(sessionText, 24);
    DrawText(sessionText, centerX - sessionTextWidth/2, timerY - 30, 24, WHITE);
    
    // Draw time display
    const char* timeText = timerDisplay.timeText.c_str();
    int timeTextWidth = MeasureText(timeText, 48);
    Color timeColor = timerDisplay.isBlinking && (int)(timerDisplay.blinkTimer * 2) % 2 ? YELLOW : WHITE;
    DrawText(timeText, centerX - timeTextWidth/2, timerY, 48, timeColor);
    
    // Draw progress bar
    renderProgressBar();
    
    // Draw timer controls
    renderTimerControls();
    
    // Draw timer state text
    std::string stateText = getTimerStateDisplayText();
    int stateTextWidth = MeasureText(stateText.c_str(), 16);
    DrawText(stateText.c_str(), centerX - stateTextWidth/2, timerY + 120, 16, LIGHTGRAY);
}

void CoffeeShopScreen::renderProgressBar() {
    int centerX = 512;
    int progressY = 180;
    
    // Background bar
    DrawRectangle(centerX - (int)timerDisplay.progressBarWidth/2, progressY, 
                 (int)timerDisplay.progressBarWidth, (int)timerDisplay.progressBarHeight, DARKGRAY);
    
    // Progress fill
    int fillWidth = (int)(timerDisplay.progressBarWidth * timerDisplay.progressFillAmount);
    Color fillColor = GREEN;
    
    // Change color based on session type
    switch (pomodoroTimer.getCurrentSessionType()) {
        case PomodoroTimer::SessionType::WORK:
            fillColor = GREEN;
            break;
        case PomodoroTimer::SessionType::SHORT_BREAK:
            fillColor = BLUE;
            break;
        case PomodoroTimer::SessionType::LONG_BREAK:
            fillColor = PURPLE;
            break;
    }
    
    DrawRectangle(centerX - (int)timerDisplay.progressBarWidth/2, progressY, 
                 fillWidth, (int)timerDisplay.progressBarHeight, fillColor);
    
    // Border
    DrawRectangleLines(centerX - (int)timerDisplay.progressBarWidth/2, progressY, 
                      (int)timerDisplay.progressBarWidth, (int)timerDisplay.progressBarHeight, WHITE);
}

void CoffeeShopScreen::renderTimerControls() {
    renderButtons();
}

void CoffeeShopScreen::renderCoffeeRewards() {
    // Coffee rewards panel
    int rewardX = 50;
    int rewardY = 50;
    int panelWidth = 200;
    int panelHeight = 150;
    
    // Background panel
    DrawRectangle(rewardX, rewardY, panelWidth, panelHeight, Fade(BLACK, 0.7f));
    DrawRectangleLines(rewardX, rewardY, panelWidth, panelHeight, ORANGE);
    
    // Title
    DrawText("Coffee Rewards", rewardX + 10, rewardY + 10, 16, ORANGE);
    
    // Current coffee count
    std::string coffeeText = "Coffee: " + std::to_string(coffeeRewardDisplay.currentCoffeeCount);
    DrawText(coffeeText.c_str(), rewardX + 10, rewardY + 35, 14, WHITE);
    
    // Earned this session
    if (coffeeRewardDisplay.earnedThisSession > 0) {
        std::string earnedText = "Earned: +" + std::to_string(coffeeRewardDisplay.earnedThisSession);
        Color earnedColor = coffeeRewardDisplay.showEarnedAnimation ? GOLD : GREEN;
        DrawText(earnedText.c_str(), rewardX + 10, rewardY + 55, 14, earnedColor);
    }
    
    // Completed pomodoros today
    std::string pomodoroText = "Pomodoros: " + std::to_string(pomodoroTimer.getCompletedPomodoros());
    DrawText(pomodoroText.c_str(), rewardX + 10, rewardY + 75, 14, WHITE);
    
    // Coffee animation
    if (coffeeRewardDisplay.showEarnedAnimation) {
        float animProgress = coffeeRewardDisplay.earnedAnimationTimer / COFFEE_ANIMATION_DURATION;
        float scale = 1.0f + sin(animProgress * PI * 4) * 0.3f;
        int coffeeSize = (int)(20 * scale);
        DrawCircle(rewardX + panelWidth - 40, rewardY + 40, coffeeSize, ORANGE);
        DrawText("☕", rewardX + panelWidth - 50, rewardY + 30, 20, BROWN);
    }
}

void CoffeeShopScreen::renderSessionHistory() {
    // Session history panel (placeholder)
    int historyX = 774;
    int historyY = 50;
    int panelWidth = 200;
    int panelHeight = 300;
    
    DrawRectangle(historyX, historyY, panelWidth, panelHeight, Fade(BLACK, 0.7f));
    DrawRectangleLines(historyX, historyY, panelWidth, panelHeight, BLUE);
    
    DrawText("Session History", historyX + 10, historyY + 10, 16, BLUE);
    DrawText("(Coming Soon)", historyX + 10, historyY + 35, 12, LIGHTGRAY);
}

void CoffeeShopScreen::renderCustomDurationDialog() {
    // Modal dialog for custom duration
    int dialogWidth = 300;
    int dialogHeight = 200;
    int dialogX = 512 - dialogWidth/2;
    int dialogY = 384 - dialogHeight/2;
    
    // Background overlay
    DrawRectangle(0, 0, 1024, 768, Fade(BLACK, 0.5f));
    
    // Dialog background
    DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, DARKGRAY);
    DrawRectangleLines(dialogX, dialogY, dialogWidth, dialogHeight, WHITE);
    
    // Title
    DrawText("Custom Duration", dialogX + 20, dialogY + 20, 20, WHITE);
    
    // Duration input (simplified)
    std::string durationText = std::to_string(customDuration) + " minutes";
    DrawText(durationText.c_str(), dialogX + 20, dialogY + 60, 16, WHITE);
    
    // Instructions
    DrawText("Use +/- keys to adjust", dialogX + 20, dialogY + 90, 12, LIGHTGRAY);
    DrawText("Press ENTER to confirm", dialogX + 20, dialogY + 110, 12, LIGHTGRAY);
    DrawText("Press ESC to cancel", dialogX + 20, dialogY + 130, 12, LIGHTGRAY);
}

void CoffeeShopScreen::renderButtons() {
    for (size_t i = 0; i < buttons.size(); i++) {
        const Button& button = buttons[i];
        
        // Button background color based on state
        Color bgColor = DARKGRAY;
        Color textColor = WHITE;
        
        if (!button.enabled) {
            bgColor = GRAY;
            textColor = DARKGRAY;
        } else {
            switch (button.state) {
                case ButtonState::HOVERED:
                    bgColor = LIGHTGRAY;
                    textColor = BLACK;
                    break;
                case ButtonState::PRESSED:
                    bgColor = WHITE;
                    textColor = BLACK;
                    break;
                default:
                    break;
            }
        }
        
        // Draw button
        DrawRectangle((int)button.x, (int)button.y, (int)button.width, (int)button.height, bgColor);
        DrawRectangleLines((int)button.x, (int)button.y, (int)button.width, (int)button.height, WHITE);
        
        // Draw button text
        int textWidth = MeasureText(button.text.c_str(), 14);
        int textX = (int)(button.x + button.width/2 - textWidth/2);
        int textY = (int)(button.y + button.height/2 - 7);
        DrawText(button.text.c_str(), textX, textY, 14, textColor);
    }
}

void CoffeeShopScreen::renderDecorations() {
    // Render decorations based on coffee shop level and unlocked items
    for (const std::string& decorationId : decorations) {
        // For now, just draw placeholder decorations
        if (decorationId == "plant_01") {
            DrawCircle(150, 300, 15, GREEN);
            DrawRectangle(145, 315, 10, 20, BROWN);
        } else if (decorationId == "painting_01") {
            DrawRectangle(500, 200, 60, 40, GOLD);
            DrawRectangleLines(500, 200, 60, 40, BLACK);
        }
    }
}//
 UI helper methods
void CoffeeShopScreen::createButtons() {
    buttons.clear();
    
    int buttonY = 320;
    int buttonSpacing = 10;
    int startX = 512 - (int)((BUTTON_WIDTH * 4 + buttonSpacing * 3) / 2);
    
    // Work session button
    buttons.emplace_back(startX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Work (25m)");
    buttons.back().onClick = [this]() {
        startPomodoroSession(PomodoroTimer::SessionType::WORK);
    };
    
    // Short break button
    buttons.emplace_back(startX + BUTTON_WIDTH + buttonSpacing, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Break (5m)");
    buttons.back().onClick = [this]() {
        startPomodoroSession(PomodoroTimer::SessionType::SHORT_BREAK);
    };
    
    // Long break button
    buttons.emplace_back(startX + (BUTTON_WIDTH + buttonSpacing) * 2, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Long (15m)");
    buttons.back().onClick = [this]() {
        startPomodoroSession(PomodoroTimer::SessionType::LONG_BREAK);
    };
    
    // Custom duration button
    buttons.emplace_back(startX + (BUTTON_WIDTH + buttonSpacing) * 3, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Custom");
    buttons.back().onClick = [this]() {
        setShowCustomDurationDialog(true);
    };
    
    // Control buttons (second row)
    int controlY = buttonY + BUTTON_HEIGHT + buttonSpacing;
    int controlStartX = 512 - (int)((BUTTON_WIDTH * 3 + buttonSpacing * 2) / 2);
    
    // Pause/Resume button
    buttons.emplace_back(controlStartX, controlY, BUTTON_WIDTH, BUTTON_HEIGHT, "Pause");
    buttons.back().onClick = [this]() {
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING) {
            pauseTimer();
        } else if (pomodoroTimer.getState() == PomodoroTimer::TimerState::PAUSED) {
            resumeTimer();
        }
    };
    
    // Stop button
    buttons.emplace_back(controlStartX + BUTTON_WIDTH + buttonSpacing, controlY, BUTTON_WIDTH, BUTTON_HEIGHT, "Stop");
    buttons.back().onClick = [this]() {
        stopTimer();
    };
    
    // Exit button
    buttons.emplace_back(controlStartX + (BUTTON_WIDTH + buttonSpacing) * 2, controlY, BUTTON_WIDTH, BUTTON_HEIGHT, "Exit");
    buttons.back().onClick = [this]() {
        if (onExitRequested) {
            onExitRequested();
        }
    };
}

void CoffeeShopScreen::updateButtonStates(const InputState& input) {
    // Update button text based on timer state
    if (buttons.size() >= 6) { // Make sure we have enough buttons
        // Update pause/resume button text
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING) {
            buttons[4].text = "Pause";
        } else if (pomodoroTimer.getState() == PomodoroTimer::TimerState::PAUSED) {
            buttons[4].text = "Resume";
        } else {
            buttons[4].text = "Pause";
        }
        
        // Enable/disable buttons based on timer state
        bool timerRunning = pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING;
        bool timerPaused = pomodoroTimer.getState() == PomodoroTimer::TimerState::PAUSED;
        
        // Session buttons (0-3) - disabled when timer is running
        for (int i = 0; i < 4; i++) {
            buttons[i].enabled = !timerRunning && !timerPaused;
        }
        
        // Pause button (4) - enabled when running or paused
        buttons[4].enabled = timerRunning || timerPaused;
        
        // Stop button (5) - enabled when running or paused
        buttons[5].enabled = timerRunning || timerPaused;
        
        // Exit button (6) - always enabled
        buttons[6].enabled = true;
    }
    
    // Handle mouse hover and click
    hoveredButtonIndex = -1;
    
    // Reset all button states
    for (auto& button : buttons) {
        button.state = ButtonState::NORMAL;
    }
    
    // Check for hover and click
    for (size_t i = 0; i < buttons.size(); i++) {
        if (isPointInButton(buttons[i], input.mouse.x, input.mouse.y)) {
            hoveredButtonIndex = (int)i;
            
            if (buttons[i].enabled) {
                if (input.mouse.leftButton) {
                    buttons[i].state = ButtonState::PRESSED;
                } else {
                    buttons[i].state = ButtonState::HOVERED;
                }
                
                // Handle click
                if (input.mouse.leftButton && !buttons[i].onClick) {
                    // Button was just pressed
                    handleButtonClick((int)i);
                }
            }
            break;
        }
    }
}

bool CoffeeShopScreen::isPointInButton(const Button& button, float x, float y) const {
    return x >= button.x && x <= button.x + button.width &&
           y >= button.y && y <= button.y + button.height;
}

void CoffeeShopScreen::handleButtonClick(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < (int)buttons.size()) {
        const Button& button = buttons[buttonIndex];
        if (button.enabled && button.onClick) {
            // Audio feedback for button click
            if (audioIntegration) {
                audioIntegration->onButtonClicked();
            }
            
            button.onClick();
        }
    }
}

// Timer event handlers
void CoffeeShopScreen::onTimerSessionCompleted(PomodoroTimer::SessionType sessionType) {
    std::cout << "CoffeeShopScreen: Session completed - " << getSessionTypeDisplayName(sessionType) << std::endl;
    
    // Audio feedback for timer completion
    if (audioIntegration) {
        audioIntegration->onTimerCompleted();
    }
    
    // Award coffee for work sessions
    if (sessionType == PomodoroTimer::SessionType::WORK) {
        awardCoffee(1);
        
        // Award XP through gamification engine
        gamificationEngine.awardExperience(25, "Completed Pomodoro session");
        
        // Audio feedback for XP gain
        if (audioIntegration) {
            audioIntegration->onXPGained(25);
        }
    }
    
    // Update character animation
    updateCharacterAnimationForTimerState();
    
    // Trigger callback
    if (onSessionCompleted) {
        onSessionCompleted(sessionType, pomodoroTimer.getRemainingTimeSeconds());
    }
}

void CoffeeShopScreen::onTimerSessionInterrupted(PomodoroTimer::SessionType sessionType) {
    std::cout << "CoffeeShopScreen: Session interrupted - " << getSessionTypeDisplayName(sessionType) << std::endl;
    
    // Update character animation to show disappointment
    playCharacterAnimation("disappointed");
    
    // Update character animation
    updateCharacterAnimationForTimerState();
}

void CoffeeShopScreen::onTimerTick(int remainingSeconds) {
    // Update display is handled in updateTimer
    // This could be used for additional tick-based effects
}

void CoffeeShopScreen::onTimerStateChanged(PomodoroTimer::TimerState oldState, PomodoroTimer::TimerState newState) {
    std::cout << "CoffeeShopScreen: Timer state changed from " << (int)oldState << " to " << (int)newState << std::endl;
    
    // Update character animation based on new state
    updateCharacterAnimationForTimerState();
}

// Animation helpers
void CoffeeShopScreen::playCharacterAnimation(const std::string& animationName) {
    if (characterAnimator && currentCharacterAnimation != animationName) {
        characterAnimator->playAnimation(animationName);
        currentCharacterAnimation = animationName;
    }
}

void CoffeeShopScreen::updateCharacterAnimationForTimerState() {
    std::string animationName = getTimerStateAnimationName();
    playCharacterAnimation(animationName);
}

std::string CoffeeShopScreen::getTimerStateAnimationName() const {
    switch (pomodoroTimer.getState()) {
        case PomodoroTimer::TimerState::RUNNING:
            return "focused";
        case PomodoroTimer::TimerState::PAUSED:
            return "idle";
        case PomodoroTimer::TimerState::COMPLETED:
            return "celebrating";
        default:
            return "idle";
    }
}

// Coffee reward helpers
void CoffeeShopScreen::awardCoffee(int amount) {
    coffeeRewardDisplay.earnedThisSession += amount;
    gamificationEngine.awardCoffeeTokens(amount);
    showCoffeeEarnedAnimation();
    
    // Audio feedback for coffee reward
    if (audioIntegration) {
        audioIntegration->onCoffeeTokensEarned(amount);
    }
    
    if (onCoffeeEarned) {
        onCoffeeEarned(amount);
    }
}

void CoffeeShopScreen::showCoffeeEarnedAnimation() {
    coffeeRewardDisplay.showEarnedAnimation = true;
    coffeeRewardDisplay.earnedAnimationTimer = 0.0f;
}

void CoffeeShopScreen::updateCoffeeDisplay() {
    coffeeRewardDisplay.currentCoffeeCount = gamificationEngine.getCoffeeTokens();
}

// UI formatting helpers
std::string CoffeeShopScreen::formatTime(int seconds) const {
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << remainingSeconds;
    return oss.str();
}

std::string CoffeeShopScreen::getSessionTypeDisplayName(PomodoroTimer::SessionType sessionType) const {
    switch (sessionType) {
        case PomodoroTimer::SessionType::WORK:
            return "Focus Session";
        case PomodoroTimer::SessionType::SHORT_BREAK:
            return "Short Break";
        case PomodoroTimer::SessionType::LONG_BREAK:
            return "Long Break";
        default:
            return "Unknown";
    }
}

std::string CoffeeShopScreen::getTimerStateDisplayText() const {
    switch (pomodoroTimer.getState()) {
        case PomodoroTimer::TimerState::STOPPED:
            return "Ready to start";
        case PomodoroTimer::TimerState::RUNNING:
            return "Session in progress";
        case PomodoroTimer::TimerState::PAUSED:
            return "Session paused";
        case PomodoroTimer::TimerState::COMPLETED:
            return "Session completed!";
        default:
            return "";
    }
}

// Coffee shop visual helpers
void CoffeeShopScreen::renderCoffeeShopByLevel() {
    // Render different visual elements based on coffee shop level
    switch (coffeeShopLevel) {
        case 1:
            // Basic coffee shop
            break;
        case 2:
            // Add some decorations
            DrawCircle(100, 200, 20, GREEN); // Plant
            break;
        case 3:
            // More decorations
            DrawCircle(100, 200, 20, GREEN); // Plant
            DrawRectangle(800, 150, 80, 60, GOLD); // Painting
            break;
        case 4:
            // Even more decorations
            DrawCircle(100, 200, 20, GREEN); // Plant
            DrawRectangle(800, 150, 80, 60, GOLD); // Painting
            DrawCircle(900, 300, 15, PINK); // Flowers
            break;
        case 5:
            // Fully upgraded
            DrawCircle(100, 200, 20, GREEN); // Plant
            DrawRectangle(800, 150, 80, 60, GOLD); // Painting
            DrawCircle(900, 300, 15, PINK); // Flowers
            DrawRectangle(50, 100, 100, 80, PURPLE); // Premium coffee machine
            break;
    }
}

void CoffeeShopScreen::renderDecoration(const std::string& decorationId, float x, float y) {
    // Render specific decorations at given positions
    if (decorationId == "plant_01") {
        DrawCircle((int)x, (int)y, 15, GREEN);
        DrawRectangle((int)x - 5, (int)y + 15, 10, 20, BROWN);
    } else if (decorationId == "painting_01") {
        DrawRectangle((int)x, (int)y, 60, 40, GOLD);
        DrawRectangleLines((int)x, (int)y, 60, 40, BLACK);
    } else if (decorationId == "flowers_01") {
        DrawCircle((int)x, (int)y, 12, PINK);
        DrawCircle((int)x - 8, (int)y + 5, 8, RED);
        DrawCircle((int)x + 8, (int)y + 5, 8, YELLOW);
    }
}

Color CoffeeShopScreen::getCoffeeShopAmbientColor() const {
    // Return different ambient colors based on coffee shop level
    switch (coffeeShopLevel) {
        case 1: return Fade(BROWN, 0.3f);
        case 2: return Fade(ORANGE, 0.3f);
        case 3: return Fade(GOLD, 0.3f);
        case 4: return Fade(YELLOW, 0.3f);
        case 5: return Fade(WHITE, 0.3f);
        default: return Fade(BROWN, 0.3f);
    }
}

// Input helpers
void CoffeeShopScreen::handleKeyboardInput(const InputState& input) {
    // Handle custom duration dialog input
    if (showCustomDurationDialog) {
        if (input.isKeyJustPressed(KEY_EQUAL) || input.isKeyJustPressed(KEY_KP_ADD)) {
            setCustomDuration(customDuration + 1);
        } else if (input.isKeyJustPressed(KEY_MINUS) || input.isKeyJustPressed(KEY_KP_SUBTRACT)) {
            setCustomDuration(customDuration - 1);
        } else if (input.isKeyJustPressed(KEY_ENTER) || input.isKeyJustPressed(KEY_KP_ENTER)) {
            setShowCustomDurationDialog(false);
            startPomodoroSession(PomodoroTimer::SessionType::WORK, customDuration);
        } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
            setShowCustomDurationDialog(false);
        }
        return;
    }
    
    // Regular keyboard shortcuts
    if (input.isKeyJustPressed(KEY_SPACE)) {
        // Space to pause/resume
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::RUNNING) {
            pauseTimer();
        } else if (pomodoroTimer.getState() == PomodoroTimer::TimerState::PAUSED) {
            resumeTimer();
        }
    } else if (input.isKeyJustPressed(KEY_S)) {
        // S to stop
        stopTimer();
    } else if (input.isKeyJustPressed(KEY_ONE)) {
        // 1 for work session
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::STOPPED) {
            startPomodoroSession(PomodoroTimer::SessionType::WORK);
        }
    } else if (input.isKeyJustPressed(KEY_TWO)) {
        // 2 for short break
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::STOPPED) {
            startPomodoroSession(PomodoroTimer::SessionType::SHORT_BREAK);
        }
    } else if (input.isKeyJustPressed(KEY_THREE)) {
        // 3 for long break
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::STOPPED) {
            startPomodoroSession(PomodoroTimer::SessionType::LONG_BREAK);
        }
    } else if (input.isKeyJustPressed(KEY_C)) {
        // C for custom duration
        if (pomodoroTimer.getState() == PomodoroTimer::TimerState::STOPPED) {
            setShowCustomDurationDialog(true);
        }
    } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
        // ESC to exit
        if (onExitRequested) {
            onExitRequested();
        }
    }
}

void CoffeeShopScreen::handleMouseInput(const InputState& input) {
    // Mouse input is handled in updateButtonStates
    // This method can be used for additional mouse interactions
}