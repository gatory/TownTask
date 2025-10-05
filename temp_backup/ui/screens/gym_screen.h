#pragma once

#include "screen.h"
#include "../../core/engines/habit_tracker.h"
#include "../../core/engines/gamification_engine.h"
#include "../../input/input_manager.h"
#include "../../ui/animations/animation_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <chrono>

class GymScreen : public Screen {
public:
    GymScreen(HabitTracker& habitTracker, GamificationEngine& gamificationEngine,
              InputManager& inputManager, AnimationManager& animationManager);
    ~GymScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Habit management functionality
    void createHabit(const std::string& name, int targetFrequency = 1, const std::string& unit = "times");
    void checkInHabit(uint32_t habitId);
    void editHabit(uint32_t habitId);
    void deleteHabit(uint32_t habitId);
    void pauseHabit(uint32_t habitId);
    void resumeHabit(uint32_t habitId);
    
    // Habit progress and statistics
    int getHabitStreak(uint32_t habitId) const;
    float getHabitCompletionRate(uint32_t habitId, int days = 30) const;
    std::vector<bool> getHabitCalendarData(uint32_t habitId, int days = 30) const;
    
    // UI state management
    void setShowHabitCreationDialog(bool show);
    bool isShowingHabitCreationDialog() const;
    void setShowHabitEditDialog(bool show);
    bool isShowingHabitEditDialog() const;
    void setShowCalendarView(bool show);
    bool isShowingCalendarView() const;
    void setSelectedHabitId(uint32_t habitId);
    uint32_t getSelectedHabitId() const;
    void setViewMode(const std::string& mode); // "list", "grid", "calendar"
    std::string getViewMode() const;
    
    // Visual settings
    void setShowStreaks(bool show);
    bool isShowingStreaks() const;
    void setShowProgress(bool show);
    bool isShowingProgress() const;
    void setShowMilestones(bool show);
    bool isShowingMilestones() const;
    void setGymLevel(int level);
    int getGymLevel() const;
    
    // Event callbacks
    void setOnHabitCheckedIn(std::function<void(uint32_t, int)> callback); // habitId, newStreak
    void setOnHabitCreated(std::function<void(uint32_t)> callback);
    void setOnMilestoneAchieved(std::function<void(uint32_t, int)> callback); // habitId, milestone
    void setOnExitRequested(std::function<void()> callback);

private:
    // Core references
    HabitTracker& habitTracker;
    GamificationEngine& gamificationEngine;
    InputManager& inputManager;
    AnimationManager& animationManager;
    
    // UI state
    bool showHabitCreationDialog;
    bool showHabitEditDialog;
    bool showCalendarView;
    uint32_t selectedHabitId;
    std::string viewMode;
    
    // Visual settings
    bool showStreaks;
    bool showProgress;
    bool showMilestones;
    int gymLevel;
    
    // Habit creation/editing state
    struct HabitFormData {
        std::string name;
        int targetFrequency;
        std::string unit;
        std::string description;
        bool isEditing;
        uint32_t editingHabitId;
        
        HabitFormData() : targetFrequency(1), unit("times"), isEditing(false), editingHabitId(0) {}
    } habitFormData;
    
    // Animation state
    std::shared_ptr<SpriteAnimator> characterAnimator;
    std::shared_ptr<SpriteAnimator> strengthAnimator;
    std::string currentCharacterAnimation;
    float strengthEffectTimer;
    bool showStrengthEffect;
    
    // Habit display state
    struct HabitCard {
        uint32_t habitId;
        float x, y;
        float width, height;
        Color backgroundColor;
        Color progressColor;
        bool isSelected;
        bool isHovered;
        bool isAnimating;
        float animationTimer;
        std::string animationType; // "checkin", "milestone", "create"
        
        HabitCard(uint32_t id, float x, float y) 
            : habitId(id), x(x), y(y), width(200), height(120), 
              backgroundColor(LIGHTGRAY), progressColor(GREEN),
              isSelected(false), isHovered(false), 
              isAnimating(false), animationTimer(0.0f) {}
    };
    
    std::vector<HabitCard> habitCards;
    std::vector<Habit> filteredHabits;
    
    // Calendar view state
    struct CalendarDay {
        int day;
        bool isCompleted;
        bool isToday;
        bool isInCurrentMonth;
        Color backgroundColor;
        
        CalendarDay(int day) : day(day), isCompleted(false), isToday(false), 
                              isInCurrentMonth(true), backgroundColor(WHITE) {}
    };
    
    std::vector<CalendarDay> calendarDays;
    std::chrono::system_clock::time_point calendarMonth;
    
    // UI elements
    struct Button {
        float x, y, width, height;
        std::string text;
        Color backgroundColor;
        Color textColor;
        bool enabled;
        bool hovered;
        std::function<void()> onClick;
        
        Button(float x, float y, float w, float h, const std::string& text)
            : x(x), y(y), width(w), height(h), text(text), 
              backgroundColor(LIGHTGRAY), textColor(BLACK), 
              enabled(true), hovered(false) {}
    };
    
    std::vector<Button> buttons;
    int hoveredButtonIndex;
    
    // Input fields for habit creation
    struct InputField {
        float x, y, width, height;
        std::string label;
        std::string value;
        bool isActive;
        bool isNumeric;
        int maxLength;
        
        InputField(float x, float y, float w, float h, const std::string& label)
            : x(x), y(y), width(w), height(h), label(label), 
              isActive(false), isNumeric(false), maxLength(50) {}
    };
    
    std::vector<InputField> inputFields;
    int activeInputFieldIndex;
    
    // Milestone tracking
    struct Milestone {
        int days;
        std::string name;
        std::string reward;
        Color color;
        
        Milestone(int d, const std::string& n, const std::string& r, Color c)
            : days(d), name(n), reward(r), color(c) {}
    };
    
    std::vector<Milestone> milestones;
    
    // Event callbacks
    std::function<void(uint32_t, int)> onHabitCheckedIn;
    std::function<void(uint32_t)> onHabitCreated;
    std::function<void(uint32_t, int)> onMilestoneAchieved;
    std::function<void()> onExitRequested;
    
    // Internal update methods
    void updateHabits(float deltaTime);
    void updateAnimations(float deltaTime);
    void updateUI(float deltaTime);
    void updateHabitCards();
    void updateButtons(const InputState& input);
    void updateInputFields(const InputState& input);
    void updateCalendar();
    
    // Rendering methods
    void renderBackground();
    void renderGymInterior();
    void renderCharacter();
    void renderHabitCards();
    void renderHabitCard(const HabitCard& card, const Habit& habit);
    void renderCalendarView();
    void renderHabitCreationDialog();
    void renderHabitEditDialog();
    void renderHabitStats();
    void renderMilestones();
    void renderButtons();
    void renderInputFields();
    
    // Habit management helpers
    void refreshFilteredHabits();
    void layoutHabitCards();
    void createHabitCardForHabit(const Habit& habit);
    void removeHabitCardForHabit(uint32_t habitId);
    HabitCard* findHabitCardForHabit(uint32_t habitId);
    void checkMilestones(uint32_t habitId, int newStreak);
    
    // Animation helpers
    void playHabitCheckInAnimation(uint32_t habitId);
    void playMilestoneAnimation(uint32_t habitId);
    void playCharacterAnimation(const std::string& animationName);
    void updateHabitAnimations(float deltaTime);
    void showStrengthEffects();
    
    // UI helpers
    void createButtons();
    void createInputFields();
    void resetHabitForm();
    void populateHabitFormForEditing(const Habit& habit);
    bool validateHabitForm() const;
    void submitHabitForm();
    void handleButtonClick(int buttonIndex);
    bool isPointInButton(const Button& button, float x, float y) const;
    bool isPointInHabitCard(const HabitCard& card, float x, float y) const;
    bool isPointInInputField(const InputField& field, float x, float y) const;
    
    // Calendar helpers
    void generateCalendarForMonth(const std::chrono::system_clock::time_point& month);
    void updateCalendarForHabit(uint32_t habitId);
    std::string formatMonthYear(const std::chrono::system_clock::time_point& date) const;
    bool isToday(const std::chrono::system_clock::time_point& date) const;
    
    // Progress calculation helpers
    float calculateProgress(const Habit& habit) const;
    Color getProgressColor(float progress) const;
    Color getStreakColor(int streak) const;
    std::string formatFrequency(int frequency, const std::string& unit) const;
    
    // Input handling helpers
    void handleKeyboardInput(const InputState& input);
    void handleMouseInput(const InputState& input);
    void handleHabitFormInput(const InputState& input);
    void handleHabitCardClick(float x, float y);
    void handleCalendarClick(float x, float y);
    
    // Rendering helpers
    void renderProgressBar(float x, float y, float width, float height, float progress, Color color);
    void renderStreak(int streak, float x, float y);
    void renderMilestone(const Milestone& milestone, float x, float y, bool achieved);
    void renderCalendarDay(const CalendarDay& day, float x, float y, float size);
    Color getHabitColor(const Habit& habit) const;
    std::string getHabitStatusText(const Habit& habit) const;
    
    // Gym visual helpers
    void renderGymEquipment();
    void renderGymDecorations();
    void renderStrengthIndicators();
    Color getGymAmbientColor() const;
    
    // Constants
    static constexpr float HABIT_CARD_WIDTH = 200.0f;
    static constexpr float HABIT_CARD_HEIGHT = 120.0f;
    static constexpr float HABIT_CARD_MARGIN = 15.0f;
    static constexpr float BUTTON_WIDTH = 100.0f;
    static constexpr float BUTTON_HEIGHT = 30.0f;
    static constexpr float CALENDAR_DAY_SIZE = 30.0f;
    static constexpr float ANIMATION_DURATION = 1.5f;
    static constexpr float STRENGTH_EFFECT_DURATION = 3.0f;
    static constexpr int MAX_HABIT_CARDS_PER_ROW = 4;
    static constexpr int HABIT_CREATION_XP = 10;
    static constexpr int HABIT_CHECKIN_XP = 5;
};