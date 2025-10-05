#pragma once

#include "screen.h"
#include "../../core/engines/task_engine.h"
#include "../../core/engines/gamification_engine.h"
#include "../../input/input_manager.h"
#include "../../ui/animations/animation_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>

class BulletinBoardScreen : public Screen {
public:
    BulletinBoardScreen(TaskEngine& taskEngine, GamificationEngine& gamificationEngine,
                        InputManager& inputManager, AnimationManager& animationManager);
    ~BulletinBoardScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Task management functionality
    void createTask(const std::string& title, Task::Priority priority = Task::Priority::MEDIUM);
    void completeTask(uint32_t taskId);
    void deleteTask(uint32_t taskId);
    void editTask(uint32_t taskId);
    
    // Task filtering and sorting
    void setTaskFilter(const std::string& filter);
    std::string getTaskFilter() const;
    void setSortBy(const std::string& sortBy); // "priority", "dueDate", "created", "title"
    std::string getSortBy() const;
    void setShowCompletedTasks(bool show);
    bool isShowingCompletedTasks() const;
    
    // UI state management
    void setShowTaskCreationDialog(bool show);
    bool isShowingTaskCreationDialog() const;
    void setShowTaskEditDialog(bool show);
    bool isShowingTaskEditDialog() const;
    void setSelectedTaskId(uint32_t taskId);
    uint32_t getSelectedTaskId() const;
    
    // Visual settings
    void setShowTaskDetails(bool show);
    bool isShowingTaskDetails() const;
    void setShowOverdueTasks(bool show);
    bool isShowingOverdueTasks() const;
    void setBoardLayout(const std::string& layout); // "grid", "list", "kanban"
    std::string getBoardLayout() const;
    
    // Event callbacks
    void setOnTaskCompleted(std::function<void(uint32_t, int)> callback); // taskId, xpAwarded
    void setOnTaskCreated(std::function<void(uint32_t)> callback);
    void setOnExitRequested(std::function<void()> callback);

private:
    // Core references
    TaskEngine& taskEngine;
    GamificationEngine& gamificationEngine;
    InputManager& inputManager;
    AnimationManager& animationManager;
    
    // UI state
    bool showTaskCreationDialog;
    bool showTaskEditDialog;
    uint32_t selectedTaskId;
    std::string taskFilter;
    std::string sortBy;
    bool showCompletedTasks;
    bool showTaskDetails;
    bool showOverdueTasks;
    std::string boardLayout;
    
    // Task creation/editing state
    struct TaskFormData {
        std::string title;
        Task::Priority priority;
        std::string dueDateString;
        std::string description;
        std::vector<std::string> subtasks;
        bool isEditing;
        uint32_t editingTaskId;
        
        TaskFormData() : priority(Task::Priority::MEDIUM), isEditing(false), editingTaskId(0) {}
    } taskFormData;
    
    // Animation state
    std::shared_ptr<SpriteAnimator> characterAnimator;
    std::shared_ptr<SpriteAnimator> paperAnimator;
    std::string currentCharacterAnimation;
    
    // Task display state
    struct StickyNote {
        uint32_t taskId;
        float x, y;
        float width, height;
        Color backgroundColor;
        Color textColor;
        bool isAnimating;
        float animationTimer;
        std::string animationType; // "appear", "complete", "tear"
        
        StickyNote(uint32_t id, float x, float y) 
            : taskId(id), x(x), y(y), width(200), height(150), 
              backgroundColor(YELLOW), textColor(BLACK), 
              isAnimating(false), animationTimer(0.0f) {}
    };
    
    std::vector<StickyNote> stickyNotes;
    std::vector<Task> filteredTasks;
    
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
    
    // Input fields for task creation
    struct InputField {
        float x, y, width, height;
        std::string label;
        std::string value;
        bool isActive;
        bool isMultiline;
        int maxLength;
        
        InputField(float x, float y, float w, float h, const std::string& label)
            : x(x), y(y), width(w), height(h), label(label), 
              isActive(false), isMultiline(false), maxLength(100) {}
    };
    
    std::vector<InputField> inputFields;
    int activeInputFieldIndex;
    
    // Event callbacks
    std::function<void(uint32_t, int)> onTaskCompleted;
    std::function<void(uint32_t)> onTaskCreated;
    std::function<void()> onExitRequested;
    
    // Internal update methods
    void updateTasks(float deltaTime);
    void updateAnimations(float deltaTime);
    void updateUI(float deltaTime);
    void updateStickyNotes();
    void updateButtons(const InputState& input);
    void updateInputFields(const InputState& input);
    
    // Rendering methods
    void renderBackground();
    void renderBulletinBoard();
    void renderStickyNotes();
    void renderStickyNote(const StickyNote& note, const Task& task);
    void renderTaskCreationDialog();
    void renderTaskEditDialog();
    void renderTaskDetails();
    void renderFilterControls();
    void renderButtons();
    void renderInputFields();
    void renderCharacter();
    
    // Task management helpers
    void refreshFilteredTasks();
    void sortTasks();
    bool passesFilter(const Task& task) const;
    void layoutStickyNotes();
    void createStickyNoteForTask(const Task& task);
    void removeStickyNoteForTask(uint32_t taskId);
    StickyNote* findStickyNoteForTask(uint32_t taskId);
    
    // Animation helpers
    void playTaskCompletionAnimation(uint32_t taskId);
    void playTaskCreationAnimation(uint32_t taskId);
    void playCharacterAnimation(const std::string& animationName);
    void updateTaskAnimations(float deltaTime);
    
    // UI helpers
    void createButtons();
    void createInputFields();
    void resetTaskForm();
    void populateTaskFormForEditing(const Task& task);
    bool validateTaskForm() const;
    void submitTaskForm();
    void handleButtonClick(int buttonIndex);
    bool isPointInButton(const Button& button, float x, float y) const;
    bool isPointInStickyNote(const StickyNote& note, float x, float y) const;
    bool isPointInInputField(const InputField& field, float x, float y) const;
    
    // Task visual helpers
    Color getPriorityColor(Task::Priority priority) const;
    Color getTaskBackgroundColor(const Task& task) const;
    std::string getPriorityDisplayName(Task::Priority priority) const;
    std::string formatDueDate(const std::chrono::system_clock::time_point& dueDate) const;
    bool isTaskOverdue(const Task& task) const;
    
    // Input handling helpers
    void handleKeyboardInput(const InputState& input);
    void handleMouseInput(const InputState& input);
    void handleTaskFormInput(const InputState& input);
    void handleStickyNoteClick(float x, float y);
    
    // Layout helpers
    void layoutGridView();
    void layoutListView();
    void layoutKanbanView();
    Vector2 getNextStickyNotePosition();
    
    // Constants
    static constexpr float STICKY_NOTE_WIDTH = 200.0f;
    static constexpr float STICKY_NOTE_HEIGHT = 150.0f;
    static constexpr float STICKY_NOTE_MARGIN = 20.0f;
    static constexpr float BUTTON_WIDTH = 100.0f;
    static constexpr float BUTTON_HEIGHT = 30.0f;
    static constexpr float ANIMATION_DURATION = 1.0f;
    static constexpr int MAX_STICKY_NOTES_PER_ROW = 4;
    static constexpr int TASK_COMPLETION_XP = 10;
};