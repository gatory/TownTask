#include "gym_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <chrono>

GymScreen::GymScreen(HabitTracker& habitTracker, GamificationEngine& gamificationEngine,
                     InputManager& inputManager, AnimationManager& animationManager)
    : Screen("GymScreen")
    , habitTracker(habitTracker)
    , gamificationEngine(gamificationEngine)
    , inputManager(inputManager)
    , animationManager(animationManager)
    , showHabitCreationDialog(false)
    , showHabitEditDialog(false)
    , showCalendarView(false)
    , selectedHabitId(0)
    , viewMode("grid")
    , showStreaks(true)
    , showProgress(true)
    , showMilestones(true)
    , gymLevel(1)
    , strengthEffectTimer(0.0f)
    , showStrengthEffect(false)
    , hoveredButtonIndex(-1)
    , activeInputFieldIndex(-1) {
    
    // Initialize milestones
    milestones.emplace_back(7, "Week Warrior", "Bronze Medal", ORANGE);
    milestones.emplace_back(30, "Month Master", "Silver Medal", GRAY);
    milestones.emplace_back(100, "Century Champion", "Gold Medal", GOLD);
    milestones.emplace_back(365, "Year Legend", "Platinum Medal", PURPLE);
    
    // Create UI elements
    createButtons();
    createInputFields();
    
    // Initialize habit form
    resetHabitForm();
    
    // Initialize calendar to current month
    calendarMonth = std::chrono::system_clock::now();
    
    std::cout << "GymScreen: Initialized successfully" << std::endl;
}

GymScreen::~GymScreen() {
    // Cleanup is handled by smart pointers and references
}

// Screen interface implementation
void GymScreen::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update habits
    updateHabits(deltaTime);
    
    // Update animations
    updateAnimations(deltaTime);
    
    // Update UI elements
    updateUI(deltaTime);
    
    // Update button states
    const InputState& input = inputManager.getCurrentState();
    updateButtons(input);
    
    // Update input fields
    updateInputFields(input);
    
    // Update calendar if showing
    if (showCalendarView) {
        updateCalendar();
    }
}

void GymScreen::render() {
    if (!isActive()) return;
    
    // Render background and gym interior
    renderBackground();
    renderGymInterior();
    
    // Render character
    renderCharacter();
    
    // Render based on view mode
    if (showCalendarView) {
        renderCalendarView();
    } else {
        renderHabitCards();
    }
    
    // Render habit statistics
    renderHabitStats();
    
    // Render milestones
    if (showMilestones) {
        renderMilestones();
    }
    
    // Render dialogs
    if (showHabitCreationDialog) {
        renderHabitCreationDialog();
    }
    
    if (showHabitEditDialog) {
        renderHabitEditDialog();
    }
    
    // Render buttons
    renderButtons();
}

void GymScreen::handleInput(const InputState& input) {
    if (!isActive()) return;
    
    // Handle keyboard input
    handleKeyboardInput(input);
    
    // Handle mouse input
    handleMouseInput(input);
    
    // Handle habit form input if dialog is open
    if (showHabitCreationDialog || showHabitEditDialog) {
        handleHabitFormInput(input);
    }
}

void GymScreen::onEnter() {
    Screen::onEnter();
    
    // Initialize character animator
    characterAnimator = animationManager.createAnimator("gym_character", "character_sheet");
    if (characterAnimator) {
        animationManager.createCharacterAnimations("gym_character", "character_sheet", 32, 32);
        playCharacterAnimation("exercising");
    }
    
    // Initialize strength animator for visual effects
    strengthAnimator = animationManager.createAnimator("strength_effects", "strength_sheet");
    if (strengthAnimator) {
        animationManager.createEffectAnimations("strength_effects", "strength_sheet", 64, 64);
    }
    
    // Refresh habits and layout
    refreshFilteredHabits();
    layoutHabitCards();
    
    // Update calendar for current month
    generateCalendarForMonth(calendarMonth);
    
    std::cout << "GymScreen: Entered gym" << std::endl;
}

void GymScreen::onExit() {
    // No special cleanup needed for gym screen
    Screen::onExit();
    std::cout << "GymScreen: Exited gym" << std::endl;
}

// Habit management functionality
void GymScreen::createHabit(const std::string& name, int targetFrequency, const std::string& unit) {
    Habit newHabit(name);
    newHabit.setTargetFrequency(targetFrequency);
    newHabit.setUnit(unit);
    
    uint32_t habitId = habitTracker.createHabit(newHabit);
    
    // Create habit card for the new habit
    Habit createdHabit = habitTracker.getHabit(habitId);
    createHabitCardForHabit(createdHabit);
    
    // Award XP for habit creation
    gamificationEngine.awardExperience(HABIT_CREATION_XP, "Created new habit");
    
    // Refresh display
    refreshFilteredHabits();
    layoutHabitCards();
    
    // Trigger callback
    if (onHabitCreated) {
        onHabitCreated(habitId);
    }
    
    std::cout << "GymScreen: Created habit '" << name << "' with ID " << habitId << std::endl;
}

void GymScreen::checkInHabit(uint32_t habitId) {
    bool success = habitTracker.checkInHabit(habitId);
    if (success) {
        Habit habit = habitTracker.getHabit(habitId);
        int newStreak = habit.getCurrentStreak();
        
        // Play check-in animation
        playHabitCheckInAnimation(habitId);
        
        // Award XP for check-in
        int xpReward = HABIT_CHECKIN_XP + (newStreak / 7); // Bonus XP for longer streaks
        gamificationEngine.awardExperience(xpReward, "Checked in habit: " + habit.getName());
        
        // Show strength effects
        showStrengthEffects();
        
        // Check for milestones
        checkMilestones(habitId, newStreak);
        
        // Refresh display
        refreshFilteredHabits();
        
        // Trigger callback
        if (onHabitCheckedIn) {
            onHabitCheckedIn(habitId, newStreak);
        }
        
        std::cout << "GymScreen: Checked in habit '" << habit.getName() 
                  << "', streak: " << newStreak << std::endl;
    }
}

void GymScreen::editHabit(uint32_t habitId) {
    Habit habit = habitTracker.getHabit(habitId);
    if (habit.getId() != 0) { // Valid habit
        populateHabitFormForEditing(habit);
        setShowHabitEditDialog(true);
        setSelectedHabitId(habitId);
    }
}

void GymScreen::deleteHabit(uint32_t habitId) {
    bool success = habitTracker.deleteHabit(habitId);
    if (success) {
        // Clear selection if deleted habit was selected
        if (selectedHabitId == habitId) {
            setSelectedHabitId(0);
        }
        
        // Remove habit card
        removeHabitCardForHabit(habitId);
        
        // Refresh display
        refreshFilteredHabits();
        layoutHabitCards();
        
        std::cout << "GymScreen: Deleted habit with ID " << habitId << std::endl;
    }
}

void GymScreen::pauseHabit(uint32_t habitId) {
    Habit habit = habitTracker.getHabit(habitId);
    if (habit.getId() != 0) {
        habit.setActive(false);
        habitTracker.updateHabit(habit);
        refreshFilteredHabits();
    }
}

void GymScreen::resumeHabit(uint32_t habitId) {
    Habit habit = habitTracker.getHabit(habitId);
    if (habit.getId() != 0) {
        habit.setActive(true);
        habitTracker.updateHabit(habit);
        refreshFilteredHabits();
    }
}

// Habit progress and statistics
int GymScreen::getHabitStreak(uint32_t habitId) const {
    Habit habit = habitTracker.getHabit(habitId);
    return habit.getCurrentStreak();
}

float GymScreen::getHabitCompletionRate(uint32_t habitId, int days) const {
    Habit habit = habitTracker.getHabit(habitId);
    if (habit.getId() == 0) return 0.0f;
    
    // Calculate completion rate over the last N days
    int completedDays = 0;
    auto now = std::chrono::system_clock::now();
    
    for (int i = 0; i < days; i++) {
        auto checkDate = now - std::chrono::hours(24 * i);
        if (habitTracker.wasHabitCompletedOnDate(habitId, checkDate)) {
            completedDays++;
        }
    }
    
    return (float)completedDays / days;
}

std::vector<bool> GymScreen::getHabitCalendarData(uint32_t habitId, int days) const {
    std::vector<bool> calendarData;
    auto now = std::chrono::system_clock::now();
    
    for (int i = days - 1; i >= 0; i--) {
        auto checkDate = now - std::chrono::hours(24 * i);
        bool completed = habitTracker.wasHabitCompletedOnDate(habitId, checkDate);
        calendarData.push_back(completed);
    }
    
    return calendarData;
}

// UI state management
void GymScreen::setShowHabitCreationDialog(bool show) {
    showHabitCreationDialog = show;
    if (show) {
        resetHabitForm();
        activeInputFieldIndex = 0; // Focus on name field
    }
}

bool GymScreen::isShowingHabitCreationDialog() const {
    return showHabitCreationDialog;
}

void GymScreen::setShowHabitEditDialog(bool show) {
    showHabitEditDialog = show;
    if (show) {
        activeInputFieldIndex = 0; // Focus on name field
    }
}

bool GymScreen::isShowingHabitEditDialog() const {
    return showHabitEditDialog;
}

void GymScreen::setShowCalendarView(bool show) {
    showCalendarView = show;
    if (show && selectedHabitId != 0) {
        updateCalendarForHabit(selectedHabitId);
    }
}

bool GymScreen::isShowingCalendarView() const {
    return showCalendarView;
}

void GymScreen::setSelectedHabitId(uint32_t habitId) {
    selectedHabitId = habitId;
    
    // Update habit card selection
    for (auto& card : habitCards) {
        card.isSelected = (card.habitId == habitId);
    }
    
    // Update calendar if showing
    if (showCalendarView && habitId != 0) {
        updateCalendarForHabit(habitId);
    }
}

uint32_t GymScreen::getSelectedHabitId() const {
    return selectedHabitId;
}

void GymScreen::setViewMode(const std::string& mode) {
    viewMode = mode;
    layoutHabitCards();
}

std::string GymScreen::getViewMode() const {
    return viewMode;
}

// Visual settings
void GymScreen::setShowStreaks(bool show) {
    showStreaks = show;
}

bool GymScreen::isShowingStreaks() const {
    return showStreaks;
}

void GymScreen::setShowProgress(bool show) {
    showProgress = show;
}

bool GymScreen::isShowingProgress() const {
    return showProgress;
}

void GymScreen::setShowMilestones(bool show) {
    showMilestones = show;
}

bool GymScreen::isShowingMilestones() const {
    return showMilestones;
}

void GymScreen::setGymLevel(int level) {
    gymLevel = std::max(1, std::min(5, level)); // Clamp between 1 and 5
}

int GymScreen::getGymLevel() const {
    return gymLevel;
}

// Event callbacks
void GymScreen::setOnHabitCheckedIn(std::function<void(uint32_t, int)> callback) {
    onHabitCheckedIn = callback;
}

void GymScreen::setOnHabitCreated(std::function<void(uint32_t)> callback) {
    onHabitCreated = callback;
}

void GymScreen::setOnMilestoneAchieved(std::function<void(uint32_t, int)> callback) {
    onMilestoneAchieved = callback;
}

void GymScreen::setOnExitRequested(std::function<void()> callback) {
    onExitRequested = callback;
}// I
nternal update methods
void GymScreen::updateHabits(float deltaTime) {
    // Check for habit updates from the tracker
    // This could include new habits, completed habits, etc.
    
    // Update habits if they have changed
    static float refreshTimer = 0.0f;
    refreshTimer += deltaTime;
    if (refreshTimer >= 1.0f) { // Refresh every second
        refreshFilteredHabits();
        refreshTimer = 0.0f;
    }
}

void GymScreen::updateAnimations(float deltaTime) {
    // Update character animator
    if (characterAnimator) {
        characterAnimator->update(deltaTime);
    }
    
    // Update strength animator
    if (strengthAnimator) {
        strengthAnimator->update(deltaTime);
    }
    
    // Update strength effect timer
    if (showStrengthEffect) {
        strengthEffectTimer += deltaTime;
        if (strengthEffectTimer >= STRENGTH_EFFECT_DURATION) {
            showStrengthEffect = false;
            strengthEffectTimer = 0.0f;
        }
    }
    
    // Update habit animations
    updateHabitAnimations(deltaTime);
}

void GymScreen::updateUI(float deltaTime) {
    // Update any UI animations or state changes
    // Check for daily reset
    static auto lastUpdateDay = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    
    auto lastDay = std::chrono::duration_cast<std::chrono::days>(lastUpdateDay.time_since_epoch()).count();
    auto currentDay = std::chrono::duration_cast<std::chrono::days>(now.time_since_epoch()).count();
    
    if (currentDay != lastDay) {
        // Day changed, refresh habits
        refreshFilteredHabits();
        lastUpdateDay = now;
    }
}

void GymScreen::updateHabitCards() {
    // Ensure we have habit cards for all filtered habits
    for (const Habit& habit : filteredHabits) {
        if (!findHabitCardForHabit(habit.getId())) {
            createHabitCardForHabit(habit);
        }
    }
    
    // Remove habit cards for habits that are no longer in filtered list
    habitCards.erase(
        std::remove_if(habitCards.begin(), habitCards.end(),
            [this](const HabitCard& card) {
                return std::find_if(filteredHabits.begin(), filteredHabits.end(),
                    [&card](const Habit& habit) { return habit.getId() == card.habitId; }) == filteredHabits.end();
            }),
        habitCards.end()
    );
}

void GymScreen::updateButtons(const InputState& input) {
    // Reset button states
    hoveredButtonIndex = -1;
    for (auto& button : buttons) {
        button.hovered = false;
    }
    
    // Check for hover and click
    for (size_t i = 0; i < buttons.size(); i++) {
        if (isPointInButton(buttons[i], input.mouse.x, input.mouse.y)) {
            hoveredButtonIndex = (int)i;
            buttons[i].hovered = true;
            
            // Handle click
            if (input.mouse.leftButton) {
                handleButtonClick((int)i);
            }
            break;
        }
    }
}

void GymScreen::updateInputFields(const InputState& input) {
    // Handle input field focus
    for (size_t i = 0; i < inputFields.size(); i++) {
        inputFields[i].isActive = (i == activeInputFieldIndex);
        
        if (input.mouse.leftButton && isPointInInputField(inputFields[i], input.mouse.x, input.mouse.y)) {
            activeInputFieldIndex = (int)i;
        }
    }
}

void GymScreen::updateCalendar() {
    // Update calendar data for selected habit
    if (selectedHabitId != 0) {
        updateCalendarForHabit(selectedHabitId);
    }
}

// Rendering methods
void GymScreen::renderBackground() {
    // Draw gym background
    Color bgColor = getGymAmbientColor();
    ClearBackground(bgColor);
    
    // Draw gym floor pattern
    const int tileSize = 32;
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color floorColor = ((x + y) / tileSize) % 2 == 0 ? DARKGRAY : GRAY;
            DrawRectangle(x, y, tileSize, tileSize, floorColor);
        }
    }
}

void GymScreen::renderGymInterior() {
    // Draw gym equipment and decorations
    renderGymEquipment();
    renderGymDecorations();
    
    // Title
    const char* title = "Gym - Build Strong Habits";
    int titleWidth = MeasureText(title, 24);
    DrawText(title, 512 - titleWidth/2, 20, 24, WHITE);
    
    // Gym level indicator
    std::string levelText = "Gym Level: " + std::to_string(gymLevel);
    DrawText(levelText.c_str(), 50, 50, 16, YELLOW);
}

void GymScreen::renderCharacter() {
    // Draw character at gym equipment
    Vector2 characterPos = {200, 400};
    
    // Draw character as a simple colored circle (placeholder)
    Color characterColor = BLUE;
    
    // Change color based on activity and strength
    if (showStrengthEffect) {
        characterColor = RED; // Getting stronger
    } else if (selectedHabitId != 0) {
        characterColor = GREEN; // Focused on habit
    }
    
    // Draw character
    DrawCircle((int)characterPos.x, (int)characterPos.y, 20, characterColor);
    DrawCircleLines((int)characterPos.x, (int)characterPos.y, 20, BLACK);
    
    // Draw strength effects
    if (showStrengthEffect) {
        float effectProgress = strengthEffectTimer / STRENGTH_EFFECT_DURATION;
        float radius = 25 + sin(effectProgress * PI * 8) * 5;
        
        for (int i = 0; i < 6; i++) {
            float angle = (i * 60.0f + effectProgress * 360) * DEG2RAD;
            Vector2 effectPos = {
                characterPos.x + cos(angle) * radius,
                characterPos.y + sin(angle) * radius
            };
            DrawCircle((int)effectPos.x, (int)effectPos.y, 3, YELLOW);
        }
        
        // Strength text
        DrawText("GETTING STRONGER!", (int)(characterPos.x - 60), (int)(characterPos.y - 40), 12, YELLOW);
    }
}

void GymScreen::renderHabitCards() {
    for (const HabitCard& card : habitCards) {
        // Find corresponding habit
        auto habitIt = std::find_if(filteredHabits.begin(), filteredHabits.end(),
            [&card](const Habit& habit) { return habit.getId() == card.habitId; });
        
        if (habitIt != filteredHabits.end()) {
            renderHabitCard(card, *habitIt);
        }
    }
}

void GymScreen::renderHabitCard(const HabitCard& card, const Habit& habit) {
    // Calculate animation effects
    float scale = 1.0f;
    float alpha = 1.0f;
    
    if (card.isAnimating) {
        float progress = card.animationTimer / ANIMATION_DURATION;
        
        if (card.animationType == "checkin") {
            scale = 1.0f + 0.1f * sin(progress * PI * 4);
        } else if (card.animationType == "milestone") {
            scale = 1.0f + 0.2f * sin(progress * PI * 2);
            alpha = 1.0f + 0.3f * sin(progress * PI * 4);
        } else if (card.animationType == "create") {
            scale = 0.8f + 0.2f * progress;
            alpha = progress;
        }
    }
    
    // Apply selection highlight
    if (card.isSelected) {
        DrawRectangle((int)(card.x - 5), (int)(card.y - 5), 
                     (int)(card.width + 10), (int)(card.height + 10), 
                     Fade(YELLOW, 0.5f));
    }
    
    // Get colors based on habit properties
    Color backgroundColor = getHabitColor(habit);
    Color borderColor = habit.isActive() ? BLACK : GRAY;
    
    // Apply animation effects
    backgroundColor = Fade(backgroundColor, alpha);
    borderColor = Fade(borderColor, alpha);
    
    // Draw habit card shadow
    DrawRectangle((int)(card.x + 3), (int)(card.y + 3), 
                 (int)(card.width * scale), (int)(card.height * scale), 
                 Fade(BLACK, 0.2f * alpha));
    
    // Draw habit card background
    DrawRectangle((int)card.x, (int)card.y, 
                 (int)(card.width * scale), (int)(card.height * scale), 
                 backgroundColor);
    
    // Draw border
    DrawRectangleLines((int)card.x, (int)card.y, 
                      (int)(card.width * scale), (int)(card.height * scale), 
                      borderColor);
    
    // Draw habit content
    int textX = (int)(card.x + 10);
    int textY = (int)(card.y + 10);
    
    // Habit name
    std::string name = habit.getName();
    if (name.length() > 18) {
        name = name.substr(0, 15) + "...";
    }
    DrawText(name.c_str(), textX, textY, 14, Fade(BLACK, alpha));
    
    // Streak display
    if (showStreaks) {
        renderStreak(habit.getCurrentStreak(), (float)(textX), (float)(textY + 25));
    }
    
    // Progress bar
    if (showProgress) {
        float progress = calculateProgress(habit);
        Color progressColor = getProgressColor(progress);
        renderProgressBar((float)(textX), (float)(textY + 50), card.width - 20, 10, progress, progressColor);
    }
    
    // Frequency info
    std::string frequencyText = formatFrequency(habit.getTargetFrequency(), habit.getUnit());
    DrawText(frequencyText.c_str(), textX, (int)(card.y + card.height - 30), 10, Fade(DARKGRAY, alpha));
    
    // Status
    std::string statusText = getHabitStatusText(habit);
    Color statusColor = habit.wasCompletedToday() ? GREEN : ORANGE;
    DrawText(statusText.c_str(), textX, (int)(card.y + card.height - 15), 10, Fade(statusColor, alpha));
    
    // Check-in button (if not completed today)
    if (!habit.wasCompletedToday() && habit.isActive()) {
        int buttonX = (int)(card.x + card.width - 60);
        int buttonY = (int)(card.y + 10);
        
        DrawRectangle(buttonX, buttonY, 50, 25, Fade(GREEN, alpha));
        DrawRectangleLines(buttonX, buttonY, 50, 25, Fade(BLACK, alpha));
        DrawText("Check", buttonX + 8, buttonY + 6, 10, Fade(WHITE, alpha));
    }
}

void GymScreen::renderCalendarView() {
    if (selectedHabitId == 0) return;
    
    // Calendar panel
    int calendarX = 300;
    int calendarY = 100;
    int calendarWidth = 500;
    int calendarHeight = 400;
    
    DrawRectangle(calendarX, calendarY, calendarWidth, calendarHeight, Fade(WHITE, 0.9f));
    DrawRectangleLines(calendarX, calendarY, calendarWidth, calendarHeight, BLACK);
    
    // Calendar title
    Habit habit = habitTracker.getHabit(selectedHabitId);
    std::string title = "Calendar: " + habit.getName();
    DrawText(title.c_str(), calendarX + 10, calendarY + 10, 16, BLACK);
    
    // Month navigation
    std::string monthText = formatMonthYear(calendarMonth);
    int monthWidth = MeasureText(monthText.c_str(), 14);
    DrawText(monthText.c_str(), calendarX + calendarWidth/2 - monthWidth/2, calendarY + 35, 14, BLACK);
    
    // Previous/Next month buttons
    DrawText("<", calendarX + 50, calendarY + 35, 14, BLUE);
    DrawText(">", calendarX + calendarWidth - 60, calendarY + 35, 14, BLUE);
    
    // Calendar grid
    int startX = calendarX + 20;
    int startY = calendarY + 70;
    int daySize = (int)CALENDAR_DAY_SIZE;
    
    // Day headers
    const char* dayHeaders[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for (int i = 0; i < 7; i++) {
        DrawText(dayHeaders[i], startX + i * (daySize + 5) + 5, startY, 10, DARKGRAY);
    }
    
    // Calendar days
    int currentX = startX;
    int currentY = startY + 20;
    int dayCount = 0;
    
    for (const CalendarDay& day : calendarDays) {
        renderCalendarDay(day, (float)currentX, (float)currentY, (float)daySize);
        
        currentX += daySize + 5;
        dayCount++;
        
        if (dayCount % 7 == 0) {
            currentX = startX;
            currentY += daySize + 5;
        }
    }
    
    // Calendar legend
    int legendY = calendarY + calendarHeight - 40;
    DrawText("● Completed", calendarX + 20, legendY, 12, GREEN);
    DrawText("○ Missed", calendarX + 120, legendY, 12, RED);
    DrawText("◐ Today", calendarX + 200, legendY, 12, BLUE);
}

void GymScreen::renderHabitCreationDialog() {
    renderHabitFormDialog("Create New Habit", false);
}

void GymScreen::renderHabitEditDialog() {
    renderHabitFormDialog("Edit Habit", true);
}

void GymScreen::renderHabitFormDialog(const std::string& title, bool isEditing) {
    // Modal background
    DrawRectangle(0, 0, 1024, 768, Fade(BLACK, 0.5f));
    
    // Dialog background
    int dialogWidth = 400;
    int dialogHeight = 350;
    int dialogX = 512 - dialogWidth/2;
    int dialogY = 384 - dialogHeight/2;
    
    DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, WHITE);
    DrawRectangleLines(dialogX, dialogY, dialogWidth, dialogHeight, BLACK);
    
    // Title
    int titleWidth = MeasureText(title.c_str(), 20);
    DrawText(title.c_str(), dialogX + dialogWidth/2 - titleWidth/2, dialogY + 20, 20, BLACK);
    
    // Render input fields
    renderInputFields();
    
    // Buttons
    int buttonY = dialogY + dialogHeight - 60;
    
    // Submit button
    const char* submitText = isEditing ? "Update" : "Create";
    DrawRectangle(dialogX + dialogWidth - 180, buttonY, 70, 30, GREEN);
    DrawRectangleLines(dialogX + dialogWidth - 180, buttonY, 70, 30, BLACK);
    int submitWidth = MeasureText(submitText, 12);
    DrawText(submitText, dialogX + dialogWidth - 180 + 35 - submitWidth/2, buttonY + 9, 12, BLACK);
    
    // Cancel button
    DrawRectangle(dialogX + dialogWidth - 100, buttonY, 70, 30, RED);
    DrawRectangleLines(dialogX + dialogWidth - 100, buttonY, 70, 30, BLACK);
    int cancelWidth = MeasureText("Cancel", 12);
    DrawText("Cancel", dialogX + dialogWidth - 100 + 35 - cancelWidth/2, buttonY + 9, 12, BLACK);
}

void GymScreen::renderHabitStats() {
    if (selectedHabitId == 0) return;
    
    Habit habit = habitTracker.getHabit(selectedHabitId);
    if (habit.getId() == 0) return;
    
    // Stats panel
    int statsX = 50;
    int statsY = 600;
    int statsWidth = 924;
    int statsHeight = 100;
    
    DrawRectangle(statsX, statsY, statsWidth, statsHeight, Fade(WHITE, 0.9f));
    DrawRectangleLines(statsX, statsY, statsWidth, statsHeight, BLACK);
    
    // Habit name
    DrawText(("Habit: " + habit.getName()).c_str(), statsX + 10, statsY + 10, 16, BLACK);
    
    // Current streak
    std::string streakText = "Current Streak: " + std::to_string(habit.getCurrentStreak()) + " days";
    DrawText(streakText.c_str(), statsX + 10, statsY + 35, 14, GREEN);
    
    // Best streak
    std::string bestStreakText = "Best Streak: " + std::to_string(habit.getBestStreak()) + " days";
    DrawText(bestStreakText.c_str(), statsX + 200, statsY + 35, 14, BLUE);
    
    // Completion rate
    float completionRate = getHabitCompletionRate(selectedHabitId, 30);
    std::string rateText = "30-day Rate: " + std::to_string((int)(completionRate * 100)) + "%";
    DrawText(rateText.c_str(), statsX + 400, statsY + 35, 14, PURPLE);
    
    // Total completions
    std::string totalText = "Total: " + std::to_string(habit.getTotalCompletions()) + " times";
    DrawText(totalText.c_str(), statsX + 600, statsY + 35, 14, ORANGE);
    
    // Action buttons
    DrawText("Actions: [Space] Check-in  [E] Edit  [D] Delete  [C] Calendar", statsX + 10, statsY + 65, 12, DARKGRAY);
}

void GymScreen::renderMilestones() {
    // Milestones panel
    int milestonesX = 50;
    int milestonesY = 720;
    int milestonesWidth = 924;
    int milestonesHeight = 40;
    
    DrawRectangle(milestonesX, milestonesY, milestonesWidth, milestonesHeight, Fade(LIGHTGRAY, 0.8f));
    DrawRectangleLines(milestonesX, milestonesY, milestonesWidth, milestonesHeight, BLACK);
    
    DrawText("Milestones:", milestonesX + 10, milestonesY + 5, 14, BLACK);
    
    int currentX = milestonesX + 100;
    for (const Milestone& milestone : milestones) {
        bool achieved = false;
        if (selectedHabitId != 0) {
            int currentStreak = getHabitStreak(selectedHabitId);
            achieved = currentStreak >= milestone.days;
        }
        
        renderMilestone(milestone, (float)currentX, (float)(milestonesY + 5), achieved);
        currentX += 150;
    }
}

void GymScreen::renderButtons() {
    for (size_t i = 0; i < buttons.size(); i++) {
        const Button& button = buttons[i];
        
        Color bgColor = button.backgroundColor;
        Color textColor = button.textColor;
        
        if (!button.enabled) {
            bgColor = GRAY;
            textColor = DARKGRAY;
        } else if (button.hovered) {
            bgColor = Fade(WHITE, 0.8f);
            textColor = BLACK;
        }
        
        DrawRectangle((int)button.x, (int)button.y, (int)button.width, (int)button.height, bgColor);
        DrawRectangleLines((int)button.x, (int)button.y, (int)button.width, (int)button.height, BLACK);
        
        int textWidth = MeasureText(button.text.c_str(), 12);
        int textX = (int)(button.x + button.width/2 - textWidth/2);
        int textY = (int)(button.y + button.height/2 - 6);
        DrawText(button.text.c_str(), textX, textY, 12, textColor);
    }
}

void GymScreen::renderInputFields() {
    for (size_t i = 0; i < inputFields.size(); i++) {
        const InputField& field = inputFields[i];
        
        Color bgColor = field.isActive ? WHITE : LIGHTGRAY;
        Color borderColor = field.isActive ? BLUE : BLACK;
        
        // Label
        DrawText(field.label.c_str(), (int)field.x, (int)(field.y - 20), 12, BLACK);
        
        // Field background
        DrawRectangle((int)field.x, (int)field.y, (int)field.width, (int)field.height, bgColor);
        DrawRectangleLines((int)field.x, (int)field.y, (int)field.width, (int)field.height, borderColor);
        
        // Field text
        DrawText(field.value.c_str(), (int)(field.x + 5), (int)(field.y + 5), 12, BLACK);
        
        // Cursor (if active)
        if (field.isActive) {
            int textWidth = MeasureText(field.value.c_str(), 12);
            DrawLine((int)(field.x + 5 + textWidth), (int)(field.y + 5), 
                    (int)(field.x + 5 + textWidth), (int)(field.y + field.height - 5), BLACK);
        }
    }
}//
 Habit management helpers
void GymScreen::refreshFilteredHabits() {
    filteredHabits.clear();
    
    // Get all habits from the tracker
    std::vector<Habit> allHabits = habitTracker.getAllHabits();
    
    // For now, show all habits (no filtering)
    filteredHabits = allHabits;
    
    // Update habit cards
    updateHabitCards();
}

void GymScreen::layoutHabitCards() {
    if (viewMode == "grid") {
        int startX = 300;
        int startY = 100;
        int currentX = startX;
        int currentY = startY;
        int cardsInRow = 0;
        
        for (auto& card : habitCards) {
            card.x = (float)currentX;
            card.y = (float)currentY;
            
            currentX += (int)(HABIT_CARD_WIDTH + HABIT_CARD_MARGIN);
            cardsInRow++;
            
            if (cardsInRow >= MAX_HABIT_CARDS_PER_ROW) {
                currentX = startX;
                currentY += (int)(HABIT_CARD_HEIGHT + HABIT_CARD_MARGIN);
                cardsInRow = 0;
            }
        }
    } else if (viewMode == "list") {
        int startX = 300;
        int startY = 100;
        int currentY = startY;
        
        for (auto& card : habitCards) {
            card.x = (float)startX;
            card.y = (float)currentY;
            card.width = 600; // Wider for list view
            card.height = 80;  // Shorter for list view
            
            currentY += (int)(card.height + HABIT_CARD_MARGIN);
        }
    }
}

void GymScreen::createHabitCardForHabit(const Habit& habit) {
    HabitCard card(habit.getId(), 0, 0); // Position will be set by layout
    card.backgroundColor = getHabitColor(habit);
    card.isAnimating = true;
    card.animationType = "create";
    card.animationTimer = 0.0f;
    
    habitCards.push_back(card);
}

void GymScreen::removeHabitCardForHabit(uint32_t habitId) {
    habitCards.erase(
        std::remove_if(habitCards.begin(), habitCards.end(),
            [habitId](const HabitCard& card) { return card.habitId == habitId; }),
        habitCards.end()
    );
}

GymScreen::HabitCard* GymScreen::findHabitCardForHabit(uint32_t habitId) {
    auto it = std::find_if(habitCards.begin(), habitCards.end(),
        [habitId](const HabitCard& card) { return card.habitId == habitId; });
    
    return (it != habitCards.end()) ? &(*it) : nullptr;
}

void GymScreen::checkMilestones(uint32_t habitId, int newStreak) {
    for (const Milestone& milestone : milestones) {
        if (newStreak == milestone.days) {
            // Milestone achieved!
            playMilestoneAnimation(habitId);
            
            // Award bonus XP
            int bonusXP = milestone.days * 2;
            gamificationEngine.awardExperience(bonusXP, "Milestone: " + milestone.name);
            
            // Trigger callback
            if (onMilestoneAchieved) {
                onMilestoneAchieved(habitId, milestone.days);
            }
            
            std::cout << "GymScreen: Milestone achieved - " << milestone.name 
                      << " (" << milestone.days << " days)" << std::endl;
            break;
        }
    }
}

// Animation helpers
void GymScreen::playHabitCheckInAnimation(uint32_t habitId) {
    HabitCard* card = findHabitCardForHabit(habitId);
    if (card) {
        card->isAnimating = true;
        card->animationType = "checkin";
        card->animationTimer = 0.0f;
    }
}

void GymScreen::playMilestoneAnimation(uint32_t habitId) {
    HabitCard* card = findHabitCardForHabit(habitId);
    if (card) {
        card->isAnimating = true;
        card->animationType = "milestone";
        card->animationTimer = 0.0f;
    }
}

void GymScreen::playCharacterAnimation(const std::string& animationName) {
    if (characterAnimator) {
        characterAnimator->playAnimation(animationName);
    }
}

void GymScreen::updateHabitAnimations(float deltaTime) {
    for (auto& card : habitCards) {
        if (card.isAnimating) {
            card.animationTimer += deltaTime;
            
            if (card.animationTimer >= ANIMATION_DURATION) {
                card.isAnimating = false;
                card.animationTimer = 0.0f;
            }
        }
    }
}

void GymScreen::showStrengthEffects() {
    showStrengthEffect = true;
    strengthEffectTimer = 0.0f;
    playCharacterAnimation("getting_stronger");
}

// UI helpers
void GymScreen::createButtons() {
    buttons.clear();
    
    // New Habit button
    buttons.emplace_back(950, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "New Habit");
    buttons.back().backgroundColor = GREEN;
    buttons.back().onClick = [this]() {
        setShowHabitCreationDialog(true);
    };
    
    // View mode buttons
    buttons.emplace_back(950, 150, BUTTON_WIDTH, BUTTON_HEIGHT, "Grid View");
    buttons.back().onClick = [this]() {
        setViewMode("grid");
    };
    
    buttons.emplace_back(950, 190, BUTTON_WIDTH, BUTTON_HEIGHT, "List View");
    buttons.back().onClick = [this]() {
        setViewMode("list");
    };
    
    // Calendar button
    buttons.emplace_back(950, 240, BUTTON_WIDTH, BUTTON_HEIGHT, "Calendar");
    buttons.back().onClick = [this]() {
        setShowCalendarView(!showCalendarView);
    };
    
    // Settings buttons
    buttons.emplace_back(950, 300, BUTTON_WIDTH, BUTTON_HEIGHT, "Show Streaks");
    buttons.back().onClick = [this]() {
        setShowStreaks(!showStreaks);
    };
    
    buttons.emplace_back(950, 340, BUTTON_WIDTH, BUTTON_HEIGHT, "Show Progress");
    buttons.back().onClick = [this]() {
        setShowProgress(!showProgress);
    };
    
    // Exit button
    buttons.emplace_back(950, 700, BUTTON_WIDTH, BUTTON_HEIGHT, "Exit");
    buttons.back().backgroundColor = ORANGE;
    buttons.back().onClick = [this]() {
        if (onExitRequested) {
            onExitRequested();
        }
    };
}

void GymScreen::createInputFields() {
    inputFields.clear();
    
    int dialogX = 512 - 200;
    int dialogY = 384 - 175;
    
    // Name field
    inputFields.emplace_back(dialogX + 20, dialogY + 60, 360, 30, "Habit Name:");
    inputFields.back().maxLength = 50;
    
    // Frequency field
    inputFields.emplace_back(dialogX + 20, dialogY + 120, 100, 30, "Target (per day):");
    inputFields.back().isNumeric = true;
    inputFields.back().maxLength = 3;
    
    // Unit field
    inputFields.emplace_back(dialogX + 140, dialogY + 120, 100, 30, "Unit:");
    inputFields.back().maxLength = 20;
}

void GymScreen::resetHabitForm() {
    habitFormData.name.clear();
    habitFormData.targetFrequency = 1;
    habitFormData.unit = "times";
    habitFormData.description.clear();
    habitFormData.isEditing = false;
    habitFormData.editingHabitId = 0;
    
    // Reset input fields
    if (inputFields.size() >= 3) {
        inputFields[0].value.clear(); // Name
        inputFields[1].value = "1";   // Frequency
        inputFields[2].value = "times"; // Unit
    }
}

void GymScreen::populateHabitFormForEditing(const Habit& habit) {
    habitFormData.name = habit.getName();
    habitFormData.targetFrequency = habit.getTargetFrequency();
    habitFormData.unit = habit.getUnit();
    habitFormData.description = ""; // Habits don't have descriptions in our current model
    habitFormData.isEditing = true;
    habitFormData.editingHabitId = habit.getId();
    
    // Populate input fields
    if (inputFields.size() >= 3) {
        inputFields[0].value = habitFormData.name;
        inputFields[1].value = std::to_string(habitFormData.targetFrequency);
        inputFields[2].value = habitFormData.unit;
    }
}

bool GymScreen::validateHabitForm() const {
    return !habitFormData.name.empty() && habitFormData.targetFrequency > 0;
}

void GymScreen::submitHabitForm() {
    if (!validateHabitForm()) {
        return;
    }
    
    if (habitFormData.isEditing) {
        // Update existing habit
        Habit habit = habitTracker.getHabit(habitFormData.editingHabitId);
        if (habit.getId() != 0) {
            habit.setName(habitFormData.name);
            habit.setTargetFrequency(habitFormData.targetFrequency);
            habit.setUnit(habitFormData.unit);
            
            habitTracker.updateHabit(habit);
            refreshFilteredHabits();
            layoutHabitCards();
        }
        setShowHabitEditDialog(false);
    } else {
        // Create new habit
        createHabit(habitFormData.name, habitFormData.targetFrequency, habitFormData.unit);
        setShowHabitCreationDialog(false);
    }
    
    resetHabitForm();
}

void GymScreen::handleButtonClick(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < (int)buttons.size()) {
        const Button& button = buttons[buttonIndex];
        if (button.enabled && button.onClick) {
            button.onClick();
        }
    }
}

bool GymScreen::isPointInButton(const Button& button, float x, float y) const {
    return x >= button.x && x <= button.x + button.width &&
           y >= button.y && y <= button.y + button.height;
}

bool GymScreen::isPointInHabitCard(const HabitCard& card, float x, float y) const {
    return x >= card.x && x <= card.x + card.width &&
           y >= card.y && y <= card.y + card.height;
}

bool GymScreen::isPointInInputField(const InputField& field, float x, float y) const {
    return x >= field.x && x <= field.x + field.width &&
           y >= field.y && y <= field.y + field.height;
}

// Calendar helpers
void GymScreen::generateCalendarForMonth(const std::chrono::system_clock::time_point& month) {
    calendarDays.clear();
    
    // Simple calendar generation (placeholder)
    // In a real implementation, this would generate proper calendar days
    for (int day = 1; day <= 31; day++) {
        CalendarDay calDay(day);
        calDay.isToday = (day == 15); // Placeholder
        calDay.isInCurrentMonth = true;
        calendarDays.push_back(calDay);
    }
}

void GymScreen::updateCalendarForHabit(uint32_t habitId) {
    if (habitId == 0) return;
    
    // Update calendar days with habit completion data
    std::vector<bool> completionData = getHabitCalendarData(habitId, 31);
    
    for (size_t i = 0; i < calendarDays.size() && i < completionData.size(); i++) {
        calendarDays[i].isCompleted = completionData[i];
        calendarDays[i].backgroundColor = completionData[i] ? GREEN : WHITE;
    }
}

std::string GymScreen::formatMonthYear(const std::chrono::system_clock::time_point& date) const {
    auto time_t = std::chrono::system_clock::to_time_t(date);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%B %Y");
    return oss.str();
}

bool GymScreen::isToday(const std::chrono::system_clock::time_point& date) const {
    auto now = std::chrono::system_clock::now();
    auto dateDay = std::chrono::duration_cast<std::chrono::days>(date.time_since_epoch()).count();
    auto nowDay = std::chrono::duration_cast<std::chrono::days>(now.time_since_epoch()).count();
    return dateDay == nowDay;
}

// Progress calculation helpers
float GymScreen::calculateProgress(const Habit& habit) const {
    if (!habit.isActive()) return 0.0f;
    
    // Calculate today's progress
    int completedToday = habit.wasCompletedToday() ? 1 : 0;
    int target = habit.getTargetFrequency();
    
    return (float)completedToday / target;
}

Color GymScreen::getProgressColor(float progress) const {
    if (progress >= 1.0f) return GREEN;
    if (progress >= 0.5f) return YELLOW;
    return RED;
}

Color GymScreen::getStreakColor(int streak) const {
    if (streak >= 100) return PURPLE;
    if (streak >= 30) return GOLD;
    if (streak >= 7) return ORANGE;
    if (streak >= 3) return GREEN;
    return GRAY;
}

std::string GymScreen::formatFrequency(int frequency, const std::string& unit) const {
    return std::to_string(frequency) + " " + unit + "/day";
}

// Input handling helpers
void GymScreen::handleKeyboardInput(const InputState& input) {
    // Handle dialog input
    if (showHabitCreationDialog || showHabitEditDialog) {
        return; // Handled in handleHabitFormInput
    }
    
    // Handle habit actions
    if (selectedHabitId != 0) {
        if (input.isKeyJustPressed(KEY_SPACE)) {
            checkInHabit(selectedHabitId);
        } else if (input.isKeyJustPressed(KEY_E)) {
            editHabit(selectedHabitId);
        } else if (input.isKeyJustPressed(KEY_D)) {
            deleteHabit(selectedHabitId);
            setSelectedHabitId(0);
        } else if (input.isKeyJustPressed(KEY_C)) {
            setShowCalendarView(!showCalendarView);
        }
    }
    
    // General shortcuts
    if (input.isKeyJustPressed(KEY_N)) {
        setShowHabitCreationDialog(true);
    } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
        if (onExitRequested) {
            onExitRequested();
        }
    }
}

void GymScreen::handleMouseInput(const InputState& input) {
    // Handle habit card clicks
    if (input.mouse.leftButton) {
        handleHabitCardClick(input.mouse.x, input.mouse.y);
    }
    
    // Handle calendar clicks
    if (showCalendarView && input.mouse.leftButton) {
        handleCalendarClick(input.mouse.x, input.mouse.y);
    }
}

void GymScreen::handleHabitFormInput(const InputState& input) {
    // Handle input field focus
    for (size_t i = 0; i < inputFields.size(); i++) {
        inputFields[i].isActive = (i == activeInputFieldIndex);
        
        if (input.mouse.leftButton && isPointInInputField(inputFields[i], input.mouse.x, input.mouse.y)) {
            activeInputFieldIndex = (int)i;
        }
    }
    
    // Handle text input for active field
    if (activeInputFieldIndex >= 0 && activeInputFieldIndex < (int)inputFields.size()) {
        InputField& field = inputFields[activeInputFieldIndex];
        
        // Handle character input
        int key = GetCharPressed();
        while (key > 0) {
            if (field.isNumeric) {
                if (key >= '0' && key <= '9' && field.value.length() < field.maxLength) {
                    field.value += (char)key;
                }
            } else {
                if (key >= 32 && key <= 125 && field.value.length() < field.maxLength) {
                    field.value += (char)key;
                }
            }
            key = GetCharPressed();
        }
        
        // Handle backspace
        if (input.isKeyJustPressed(KEY_BACKSPACE) && !field.value.empty()) {
            field.value.pop_back();
        }
        
        // Update form data
        if (activeInputFieldIndex == 0) {
            habitFormData.name = field.value;
        } else if (activeInputFieldIndex == 1) {
            habitFormData.targetFrequency = field.value.empty() ? 1 : std::stoi(field.value);
        } else if (activeInputFieldIndex == 2) {
            habitFormData.unit = field.value;
        }
    }
    
    // Handle form buttons
    if (input.mouse.leftButton) {
        int dialogX = 512 - 200;
        int buttonY = 384 - 175 + 350 - 60;
        
        // Submit button
        if (input.mouse.x >= dialogX + 400 - 180 && input.mouse.x <= dialogX + 400 - 110 &&
            input.mouse.y >= buttonY && input.mouse.y <= buttonY + 30) {
            submitHabitForm();
        }
        
        // Cancel button
        if (input.mouse.x >= dialogX + 400 - 100 && input.mouse.x <= dialogX + 400 - 30 &&
            input.mouse.y >= buttonY && input.mouse.y <= buttonY + 30) {
            setShowHabitCreationDialog(false);
            setShowHabitEditDialog(false);
            resetHabitForm();
        }
    }
    
    // Handle keyboard shortcuts
    if (input.isKeyJustPressed(KEY_ENTER)) {
        submitHabitForm();
    } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
        setShowHabitCreationDialog(false);
        setShowHabitEditDialog(false);
        resetHabitForm();
    } else if (input.isKeyJustPressed(KEY_TAB)) {
        activeInputFieldIndex = (activeInputFieldIndex + 1) % (int)inputFields.size();
    }
}

void GymScreen::handleHabitCardClick(float x, float y) {
    for (const HabitCard& card : habitCards) {
        if (isPointInHabitCard(card, x, y)) {
            setSelectedHabitId(card.habitId);
            
            // Check if clicking on check-in button
            Habit habit = habitTracker.getHabit(card.habitId);
            if (!habit.wasCompletedToday() && habit.isActive()) {
                int buttonX = (int)(card.x + card.width - 60);
                int buttonY = (int)(card.y + 10);
                
                if (x >= buttonX && x <= buttonX + 50 && y >= buttonY && y <= buttonY + 25) {
                    checkInHabit(card.habitId);
                }
            }
            break;
        }
    }
}

void GymScreen::handleCalendarClick(float x, float y) {
    // Handle month navigation
    int calendarX = 300;
    int calendarY = 100;
    int calendarWidth = 500;
    
    // Previous month
    if (x >= calendarX + 50 && x <= calendarX + 70 && y >= calendarY + 35 && y <= calendarY + 50) {
        calendarMonth -= std::chrono::hours(24 * 30); // Approximate month
        generateCalendarForMonth(calendarMonth);
        if (selectedHabitId != 0) {
            updateCalendarForHabit(selectedHabitId);
        }
    }
    
    // Next month
    if (x >= calendarX + calendarWidth - 60 && x <= calendarX + calendarWidth - 40 && 
        y >= calendarY + 35 && y <= calendarY + 50) {
        calendarMonth += std::chrono::hours(24 * 30); // Approximate month
        generateCalendarForMonth(calendarMonth);
        if (selectedHabitId != 0) {
            updateCalendarForHabit(selectedHabitId);
        }
    }
}

// Rendering helpers
void GymScreen::renderProgressBar(float x, float y, float width, float height, float progress, Color color) {
    // Background
    DrawRectangle((int)x, (int)y, (int)width, (int)height, DARKGRAY);
    
    // Progress fill
    int fillWidth = (int)(width * std::min(1.0f, progress));
    DrawRectangle((int)x, (int)y, fillWidth, (int)height, color);
    
    // Border
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, BLACK);
}

void GymScreen::renderStreak(int streak, float x, float y) {
    Color streakColor = getStreakColor(streak);
    std::string streakText = "🔥 " + std::to_string(streak) + " days";
    DrawText(streakText.c_str(), (int)x, (int)y, 12, streakColor);
}

void GymScreen::renderMilestone(const Milestone& milestone, float x, float y, bool achieved) {
    Color color = achieved ? milestone.color : GRAY;
    std::string text = std::to_string(milestone.days) + "d";
    
    DrawCircle((int)(x + 15), (int)(y + 15), 12, color);
    DrawCircleLines((int)(x + 15), (int)(y + 15), 12, BLACK);
    
    int textWidth = MeasureText(text.c_str(), 8);
    DrawText(text.c_str(), (int)(x + 15 - textWidth/2), (int)(y + 11), 8, BLACK);
    
    DrawText(milestone.name.c_str(), (int)x, (int)(y + 35), 8, color);
}

void GymScreen::renderCalendarDay(const CalendarDay& day, float x, float y, float size) {
    Color bgColor = day.backgroundColor;
    Color textColor = BLACK;
    
    if (day.isToday) {
        bgColor = BLUE;
        textColor = WHITE;
    } else if (day.isCompleted) {
        bgColor = GREEN;
        textColor = WHITE;
    }
    
    DrawRectangle((int)x, (int)y, (int)size, (int)size, bgColor);
    DrawRectangleLines((int)x, (int)y, (int)size, (int)size, BLACK);
    
    std::string dayText = std::to_string(day.day);
    int textWidth = MeasureText(dayText.c_str(), 10);
    DrawText(dayText.c_str(), (int)(x + size/2 - textWidth/2), (int)(y + size/2 - 5), 10, textColor);
}

Color GymScreen::getHabitColor(const Habit& habit) const {
    if (!habit.isActive()) return LIGHTGRAY;
    if (habit.wasCompletedToday()) return LIGHTGREEN;
    
    int streak = habit.getCurrentStreak();
    if (streak >= 30) return Fade(GOLD, 0.3f);
    if (streak >= 7) return Fade(ORANGE, 0.3f);
    if (streak >= 3) return Fade(GREEN, 0.3f);
    
    return WHITE;
}

std::string GymScreen::getHabitStatusText(const Habit& habit) const {
    if (!habit.isActive()) return "Paused";
    if (habit.wasCompletedToday()) return "✓ Done Today";
    return "○ Pending";
}

// Gym visual helpers
void GymScreen::renderGymEquipment() {
    // Dumbbells
    DrawRectangle(100, 300, 40, 10, DARKGRAY);
    DrawRectangle(95, 295, 10, 20, DARKGRAY);
    DrawRectangle(135, 295, 10, 20, DARKGRAY);
    
    // Treadmill
    DrawRectangle(100, 450, 80, 60, BLACK);
    DrawRectangle(105, 455, 70, 50, DARKGRAY);
    
    // Bench
    DrawRectangle(100, 350, 60, 15, BROWN);
    DrawRectangle(110, 340, 10, 25, BROWN);
    DrawRectangle(140, 340, 10, 25, BROWN);
}

void GymScreen::renderGymDecorations() {
    // Mirrors (simple rectangles)
    DrawRectangle(50, 150, 5, 200, LIGHTGRAY);
    DrawRectangle(969, 150, 5, 200, LIGHTGRAY);
    
    // Motivational posters (placeholder)
    DrawRectangle(80, 200, 60, 40, YELLOW);
    DrawRectangleLines(80, 200, 60, 40, BLACK);
    DrawText("STRONG!", 90, 215, 12, BLACK);
}

void GymScreen::renderStrengthIndicators() {
    // Show overall strength based on total habit completions
    int totalCompletions = 0;
    for (const Habit& habit : filteredHabits) {
        totalCompletions += habit.getTotalCompletions();
    }
    
    int strengthLevel = std::min(5, totalCompletions / 10);
    
    for (int i = 0; i < 5; i++) {
        Color barColor = (i < strengthLevel) ? RED : DARKGRAY;
        DrawRectangle(50 + i * 15, 80, 10, 20 - i * 2, barColor);
    }
    
    DrawText("Strength", 50, 105, 12, WHITE);
}

Color GymScreen::getGymAmbientColor() const {
    // Return different ambient colors based on gym level
    switch (gymLevel) {
        case 1: return Fade(DARKGRAY, 0.3f);
        case 2: return Fade(GRAY, 0.3f);
        case 3: return Fade(BLUE, 0.2f);
        case 4: return Fade(GREEN, 0.2f);
        case 5: return Fade(GOLD, 0.2f);
        default: return Fade(DARKGRAY, 0.3f);
    }
}