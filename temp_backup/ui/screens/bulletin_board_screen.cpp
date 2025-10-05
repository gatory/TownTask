#include "bulletin_board_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <chrono>

BulletinBoardScreen::BulletinBoardScreen(TaskEngine& taskEngine, GamificationEngine& gamificationEngine,
                                         InputManager& inputManager, AnimationManager& animationManager)
    : Screen("BulletinBoardScreen")
    , taskEngine(taskEngine)
    , gamificationEngine(gamificationEngine)
    , inputManager(inputManager)
    , animationManager(animationManager)
    , showTaskCreationDialog(false)
    , showTaskEditDialog(false)
    , selectedTaskId(0)
    , taskFilter("")
    , sortBy("priority")
    , showCompletedTasks(false)
    , showTaskDetails(true)
    , showOverdueTasks(true)
    , boardLayout("grid")
    , hoveredButtonIndex(-1)
    , activeInputFieldIndex(-1) {
    
    // Create UI elements
    createButtons();
    createInputFields();
    
    // Initialize task form
    resetTaskForm();
    
    std::cout << "BulletinBoardScreen: Initialized successfully" << std::endl;
}

BulletinBoardScreen::~BulletinBoardScreen() {
    // Cleanup is handled by smart pointers and references
}

// Screen interface implementation
void BulletinBoardScreen::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update tasks
    updateTasks(deltaTime);
    
    // Update animations
    updateAnimations(deltaTime);
    
    // Update UI elements
    updateUI(deltaTime);
    
    // Update button states
    const InputState& input = inputManager.getCurrentState();
    updateButtons(input);
    
    // Update input fields
    updateInputFields(input);
}

void BulletinBoardScreen::render() {
    if (!isActive()) return;
    
    // Render background and bulletin board
    renderBackground();
    renderBulletinBoard();
    
    // Render sticky notes
    renderStickyNotes();
    
    // Render character
    renderCharacter();
    
    // Render filter controls
    renderFilterControls();
    
    // Render task details if enabled
    if (showTaskDetails && selectedTaskId != 0) {
        renderTaskDetails();
    }
    
    // Render dialogs
    if (showTaskCreationDialog) {
        renderTaskCreationDialog();
    }
    
    if (showTaskEditDialog) {
        renderTaskEditDialog();
    }
}

void BulletinBoardScreen::handleInput(const InputState& input) {
    if (!isActive()) return;
    
    // Handle keyboard input
    handleKeyboardInput(input);
    
    // Handle mouse input
    handleMouseInput(input);
    
    // Handle task form input if dialog is open
    if (showTaskCreationDialog || showTaskEditDialog) {
        handleTaskFormInput(input);
    }
}

void BulletinBoardScreen::onEnter() {
    Screen::onEnter();
    
    // Initialize character animator
    characterAnimator = animationManager.createAnimator("bulletin_character", "character_sheet");
    if (characterAnimator) {
        animationManager.createCharacterAnimations("bulletin_character", "character_sheet", 32, 32);
        playCharacterAnimation("idle");
    }
    
    // Initialize paper animator for task animations
    paperAnimator = animationManager.createAnimator("paper_effects", "paper_sheet");
    if (paperAnimator) {
        animationManager.createEffectAnimations("paper_effects", "paper_sheet", 64, 64);
    }
    
    // Refresh tasks and layout
    refreshFilteredTasks();
    layoutStickyNotes();
    
    std::cout << "BulletinBoardScreen: Entered bulletin board" << std::endl;
}

void BulletinBoardScreen::onExit() {
    // Save any pending changes
    if (showTaskCreationDialog || showTaskEditDialog) {
        // Auto-save or prompt user
        showTaskCreationDialog = false;
        showTaskEditDialog = false;
    }
    
    Screen::onExit();
    std::cout << "BulletinBoardScreen: Exited bulletin board" << std::endl;
}

// Task management functionality
void BulletinBoardScreen::createTask(const std::string& title, Task::Priority priority) {
    Task newTask(title, priority);
    uint32_t taskId = taskEngine.createTask(newTask);
    
    // Create sticky note for the new task
    Task createdTask = taskEngine.getTask(taskId);
    createStickyNoteForTask(createdTask);
    
    // Play creation animation
    playTaskCreationAnimation(taskId);
    
    // Award XP for task creation
    gamificationEngine.awardExperience(5, "Created new task");
    
    // Refresh display
    refreshFilteredTasks();
    layoutStickyNotes();
    
    // Trigger callback
    if (onTaskCreated) {
        onTaskCreated(taskId);
    }
    
    std::cout << "BulletinBoardScreen: Created task '" << title << "' with ID " << taskId << std::endl;
}

void BulletinBoardScreen::completeTask(uint32_t taskId) {
    // Get task before completing it
    Task task = taskEngine.getTask(taskId);
    
    // Complete the task
    bool success = taskEngine.completeTask(taskId);
    if (success) {
        // Play completion animation
        playTaskCompletionAnimation(taskId);
        
        // Award XP based on priority
        int xpReward = TASK_COMPLETION_XP;
        switch (task.getPriority()) {
            case Task::Priority::HIGH: xpReward = 20; break;
            case Task::Priority::MEDIUM: xpReward = 15; break;
            case Task::Priority::LOW: xpReward = 10; break;
        }
        
        gamificationEngine.awardExperience(xpReward, "Completed task: " + task.getTitle());
        
        // Remove sticky note after animation
        // (This will be handled in the animation update)
        
        // Trigger callback
        if (onTaskCompleted) {
            onTaskCompleted(taskId, xpReward);
        }
        
        std::cout << "BulletinBoardScreen: Completed task '" << task.getTitle() 
                  << "' and awarded " << xpReward << " XP" << std::endl;
    }
}

void BulletinBoardScreen::deleteTask(uint32_t taskId) {
    bool success = taskEngine.deleteTask(taskId);
    if (success) {
        // Remove sticky note
        removeStickyNoteForTask(taskId);
        
        // Refresh display
        refreshFilteredTasks();
        layoutStickyNotes();
        
        std::cout << "BulletinBoardScreen: Deleted task with ID " << taskId << std::endl;
    }
}

void BulletinBoardScreen::editTask(uint32_t taskId) {
    Task task = taskEngine.getTask(taskId);
    if (task.getId() != 0) { // Valid task
        populateTaskFormForEditing(task);
        setShowTaskEditDialog(true);
        setSelectedTaskId(taskId);
    }
}

// Task filtering and sorting
void BulletinBoardScreen::setTaskFilter(const std::string& filter) {
    taskFilter = filter;
    refreshFilteredTasks();
    layoutStickyNotes();
}

std::string BulletinBoardScreen::getTaskFilter() const {
    return taskFilter;
}

void BulletinBoardScreen::setSortBy(const std::string& sortBy) {
    this->sortBy = sortBy;
    refreshFilteredTasks();
    layoutStickyNotes();
}

std::string BulletinBoardScreen::getSortBy() const {
    return sortBy;
}

void BulletinBoardScreen::setShowCompletedTasks(bool show) {
    showCompletedTasks = show;
    refreshFilteredTasks();
    layoutStickyNotes();
}

bool BulletinBoardScreen::isShowingCompletedTasks() const {
    return showCompletedTasks;
}

// UI state management
void BulletinBoardScreen::setShowTaskCreationDialog(bool show) {
    showTaskCreationDialog = show;
    if (show) {
        resetTaskForm();
        activeInputFieldIndex = 0; // Focus on title field
    }
}

bool BulletinBoardScreen::isShowingTaskCreationDialog() const {
    return showTaskCreationDialog;
}

void BulletinBoardScreen::setShowTaskEditDialog(bool show) {
    showTaskEditDialog = show;
    if (show) {
        activeInputFieldIndex = 0; // Focus on title field
    }
}

bool BulletinBoardScreen::isShowingTaskEditDialog() const {
    return showTaskEditDialog;
}

void BulletinBoardScreen::setSelectedTaskId(uint32_t taskId) {
    selectedTaskId = taskId;
}

uint32_t BulletinBoardScreen::getSelectedTaskId() const {
    return selectedTaskId;
}

// Visual settings
void BulletinBoardScreen::setShowTaskDetails(bool show) {
    showTaskDetails = show;
}

bool BulletinBoardScreen::isShowingTaskDetails() const {
    return showTaskDetails;
}

void BulletinBoardScreen::setShowOverdueTasks(bool show) {
    showOverdueTasks = show;
    refreshFilteredTasks();
    layoutStickyNotes();
}

bool BulletinBoardScreen::isShowingOverdueTasks() const {
    return showOverdueTasks;
}

void BulletinBoardScreen::setBoardLayout(const std::string& layout) {
    boardLayout = layout;
    layoutStickyNotes();
}

std::string BulletinBoardScreen::getBoardLayout() const {
    return boardLayout;
}

// Event callbacks
void BulletinBoardScreen::setOnTaskCompleted(std::function<void(uint32_t, int)> callback) {
    onTaskCompleted = callback;
}

void BulletinBoardScreen::setOnTaskCreated(std::function<void(uint32_t)> callback) {
    onTaskCreated = callback;
}

void BulletinBoardScreen::setOnExitRequested(std::function<void()> callback) {
    onExitRequested = callback;
}// Inte
rnal update methods
void BulletinBoardScreen::updateTasks(float deltaTime) {
    // Check for task updates from the engine
    // This could include new tasks, completed tasks, etc.
    
    // Update sticky notes if tasks have changed
    // For now, we'll refresh periodically
    static float refreshTimer = 0.0f;
    refreshTimer += deltaTime;
    if (refreshTimer >= 1.0f) { // Refresh every second
        refreshFilteredTasks();
        refreshTimer = 0.0f;
    }
}

void BulletinBoardScreen::updateAnimations(float deltaTime) {
    // Update character animator
    if (characterAnimator) {
        characterAnimator->update(deltaTime);
    }
    
    // Update paper animator
    if (paperAnimator) {
        paperAnimator->update(deltaTime);
    }
    
    // Update task animations
    updateTaskAnimations(deltaTime);
}

void BulletinBoardScreen::updateUI(float deltaTime) {
    // Update any UI animations or state changes
    // This could include button hover effects, etc.
}

void BulletinBoardScreen::updateStickyNotes() {
    // Ensure we have sticky notes for all filtered tasks
    for (const Task& task : filteredTasks) {
        if (!findStickyNoteForTask(task.getId())) {
            createStickyNoteForTask(task);
        }
    }
    
    // Remove sticky notes for tasks that are no longer in filtered list
    stickyNotes.erase(
        std::remove_if(stickyNotes.begin(), stickyNotes.end(),
            [this](const StickyNote& note) {
                return std::find_if(filteredTasks.begin(), filteredTasks.end(),
                    [&note](const Task& task) { return task.getId() == note.taskId; }) == filteredTasks.end();
            }),
        stickyNotes.end()
    );
}

void BulletinBoardScreen::updateButtons(const InputState& input) {
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

void BulletinBoardScreen::updateInputFields(const InputState& input) {
    // Handle input field focus and text input
    // This will be implemented in handleTaskFormInput
}

// Rendering methods
void BulletinBoardScreen::renderBackground() {
    // Draw bulletin board background
    ClearBackground(Fade(BROWN, 0.3f));
    
    // Draw wood texture pattern
    const int tileSize = 32;
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color woodColor = ((x + y) / tileSize) % 2 == 0 ? BROWN : Fade(BROWN, 0.9f);
            DrawRectangle(x, y, tileSize, tileSize, woodColor);
        }
    }
}

void BulletinBoardScreen::renderBulletinBoard() {
    // Draw the main bulletin board
    int boardX = 100;
    int boardY = 50;
    int boardWidth = 824;
    int boardHeight = 600;
    
    // Board background
    DrawRectangle(boardX, boardY, boardWidth, boardHeight, BEIGE);
    DrawRectangleLines(boardX, boardY, boardWidth, boardHeight, DARKBROWN);
    
    // Board frame
    DrawRectangle(boardX - 10, boardY - 10, boardWidth + 20, boardHeight + 20, DARKBROWN);
    DrawRectangle(boardX, boardY, boardWidth, boardHeight, BEIGE);
    
    // Cork texture (simple dots pattern)
    for (int x = boardX + 20; x < boardX + boardWidth - 20; x += 40) {
        for (int y = boardY + 20; y < boardY + boardHeight - 20; y += 40) {
            DrawCircle(x, y, 2, Fade(BROWN, 0.3f));
        }
    }
    
    // Title
    const char* title = "Task Board";
    int titleWidth = MeasureText(title, 32);
    DrawText(title, boardX + boardWidth/2 - titleWidth/2, boardY - 45, 32, DARKBROWN);
}

void BulletinBoardScreen::renderStickyNotes() {
    for (const StickyNote& note : stickyNotes) {
        // Find corresponding task
        auto taskIt = std::find_if(filteredTasks.begin(), filteredTasks.end(),
            [&note](const Task& task) { return task.getId() == note.taskId; });
        
        if (taskIt != filteredTasks.end()) {
            renderStickyNote(note, *taskIt);
        }
    }
}

void BulletinBoardScreen::renderStickyNote(const StickyNote& note, const Task& task) {
    // Calculate animation effects
    float scale = 1.0f;
    float alpha = 1.0f;
    float rotation = 0.0f;
    
    if (note.isAnimating) {
        float progress = note.animationTimer / ANIMATION_DURATION;
        
        if (note.animationType == "appear") {
            scale = 0.5f + 0.5f * progress;
            alpha = progress;
        } else if (note.animationType == "complete") {
            scale = 1.0f + 0.2f * sin(progress * PI * 4);
            alpha = 1.0f - progress;
        } else if (note.animationType == "tear") {
            rotation = progress * 15.0f; // Slight rotation
            alpha = 1.0f - progress;
        }
    }
    
    // Apply selection highlight
    if (task.getId() == selectedTaskId) {
        DrawRectangle((int)(note.x - 5), (int)(note.y - 5), 
                     (int)(note.width + 10), (int)(note.height + 10), 
                     Fade(YELLOW, 0.5f));
    }
    
    // Get colors based on task properties
    Color backgroundColor = getTaskBackgroundColor(task);
    Color borderColor = getPriorityColor(task.getPriority());
    
    // Apply animation effects
    backgroundColor = Fade(backgroundColor, alpha);
    borderColor = Fade(borderColor, alpha);
    
    // Draw sticky note shadow
    DrawRectangle((int)(note.x + 3), (int)(note.y + 3), 
                 (int)(note.width * scale), (int)(note.height * scale), 
                 Fade(BLACK, 0.2f * alpha));
    
    // Draw sticky note background
    DrawRectangle((int)note.x, (int)note.y, 
                 (int)(note.width * scale), (int)(note.height * scale), 
                 backgroundColor);
    
    // Draw priority border
    int borderWidth = 3;
    switch (task.getPriority()) {
        case Task::Priority::HIGH:
            borderWidth = 5;
            break;
        case Task::Priority::MEDIUM:
            borderWidth = 3;
            break;
        case Task::Priority::LOW:
            borderWidth = 2;
            break;
    }
    
    DrawRectangleLines((int)note.x, (int)note.y, 
                      (int)(note.width * scale), (int)(note.height * scale), 
                      borderColor);
    
    // Draw overdue indicator
    if (isTaskOverdue(task)) {
        DrawTriangle(
            {note.x + note.width * scale - 20, note.y},
            {note.x + note.width * scale, note.y},
            {note.x + note.width * scale, note.y + 20},
            RED
        );
        DrawText("!", (int)(note.x + note.width * scale - 12), (int)(note.y + 2), 12, WHITE);
    }
    
    // Draw task content
    int textX = (int)(note.x + 10);
    int textY = (int)(note.y + 10);
    int textWidth = (int)(note.width * scale - 20);
    
    // Task title (truncated if necessary)
    std::string title = task.getTitle();
    if (title.length() > 20) {
        title = title.substr(0, 17) + "...";
    }
    DrawText(title.c_str(), textX, textY, 14, Fade(BLACK, alpha));
    
    // Priority indicator
    std::string priorityText = getPriorityDisplayName(task.getPriority());
    DrawText(priorityText.c_str(), textX, textY + 20, 10, Fade(borderColor, alpha));
    
    // Due date (if set)
    if (task.getDueDate() != std::chrono::system_clock::time_point{}) {
        std::string dueDateText = formatDueDate(task.getDueDate());
        DrawText(dueDateText.c_str(), textX, textY + 35, 10, Fade(DARKGRAY, alpha));
    }
    
    // Status
    std::string statusText = task.getStatus() == Task::Status::COMPLETED ? "✓ Done" : "○ Pending";
    Color statusColor = task.getStatus() == Task::Status::COMPLETED ? GREEN : GRAY;
    DrawText(statusText.c_str(), textX, (int)(note.y + note.height * scale - 25), 10, Fade(statusColor, alpha));
    
    // Subtask count (if any)
    if (!task.getSubtasks().empty()) {
        std::string subtaskText = std::to_string(task.getSubtasks().size()) + " subtasks";
        DrawText(subtaskText.c_str(), textX, (int)(note.y + note.height * scale - 40), 9, Fade(BLUE, alpha));
    }
}

void BulletinBoardScreen::renderTaskCreationDialog() {
    renderTaskFormDialog("Create New Task", false);
}

void BulletinBoardScreen::renderTaskEditDialog() {
    renderTaskFormDialog("Edit Task", true);
}

void BulletinBoardScreen::renderTaskFormDialog(const std::string& title, bool isEditing) {
    // Modal background
    DrawRectangle(0, 0, 1024, 768, Fade(BLACK, 0.5f));
    
    // Dialog background
    int dialogWidth = 400;
    int dialogHeight = 500;
    int dialogX = 512 - dialogWidth/2;
    int dialogY = 384 - dialogHeight/2;
    
    DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, WHITE);
    DrawRectangleLines(dialogX, dialogY, dialogWidth, dialogHeight, BLACK);
    
    // Title
    int titleWidth = MeasureText(title.c_str(), 20);
    DrawText(title.c_str(), dialogX + dialogWidth/2 - titleWidth/2, dialogY + 20, 20, BLACK);
    
    // Render input fields
    renderInputFields();
    
    // Priority selection
    int priorityY = dialogY + 200;
    DrawText("Priority:", dialogX + 20, priorityY, 14, BLACK);
    
    const char* priorities[] = {"Low", "Medium", "High"};
    Color priorityColors[] = {GREEN, ORANGE, RED};
    
    for (int i = 0; i < 3; i++) {
        int buttonX = dialogX + 20 + i * 80;
        int buttonY = priorityY + 20;
        Color bgColor = (taskFormData.priority == (Task::Priority)i) ? priorityColors[i] : LIGHTGRAY;
        
        DrawRectangle(buttonX, buttonY, 70, 30, bgColor);
        DrawRectangleLines(buttonX, buttonY, 70, 30, BLACK);
        
        int textWidth = MeasureText(priorities[i], 12);
        DrawText(priorities[i], buttonX + 35 - textWidth/2, buttonY + 9, 12, BLACK);
    }
    
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

void BulletinBoardScreen::renderTaskDetails() {
    if (selectedTaskId == 0) return;
    
    // Find the selected task
    auto taskIt = std::find_if(filteredTasks.begin(), filteredTasks.end(),
        [this](const Task& task) { return task.getId() == selectedTaskId; });
    
    if (taskIt == filteredTasks.end()) return;
    
    const Task& task = *taskIt;
    
    // Details panel
    int panelX = 50;
    int panelY = 670;
    int panelWidth = 924;
    int panelHeight = 90;
    
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(WHITE, 0.9f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, BLACK);
    
    // Task details
    int textX = panelX + 10;
    int textY = panelY + 10;
    
    DrawText(("Task: " + task.getTitle()).c_str(), textX, textY, 14, BLACK);
    DrawText(("Priority: " + getPriorityDisplayName(task.getPriority())).c_str(), textX + 300, textY, 14, BLACK);
    
    if (task.getDueDate() != std::chrono::system_clock::time_point{}) {
        DrawText(("Due: " + formatDueDate(task.getDueDate())).c_str(), textX, textY + 20, 12, DARKGRAY);
    }
    
    if (!task.getSubtasks().empty()) {
        DrawText(("Subtasks: " + std::to_string(task.getSubtasks().size())).c_str(), textX + 300, textY + 20, 12, DARKGRAY);
    }
    
    // Action buttons
    DrawText("Actions: [E]dit [D]elete [C]omplete", textX, textY + 40, 12, BLUE);
}

void BulletinBoardScreen::renderFilterControls() {
    // Filter controls panel
    int controlsX = 50;
    int controlsY = 10;
    int controlsWidth = 924;
    int controlsHeight = 30;
    
    DrawRectangle(controlsX, controlsY, controlsWidth, controlsHeight, Fade(LIGHTGRAY, 0.8f));
    DrawRectangleLines(controlsX, controlsY, controlsWidth, controlsHeight, BLACK);
    
    // Filter text
    DrawText("Filter:", controlsX + 10, controlsY + 8, 14, BLACK);
    DrawText(taskFilter.empty() ? "All Tasks" : taskFilter.c_str(), controlsX + 60, controlsY + 8, 14, DARKBLUE);
    
    // Sort text
    DrawText("Sort:", controlsX + 200, controlsY + 8, 14, BLACK);
    DrawText(sortBy.c_str(), controlsX + 240, controlsY + 8, 14, DARKBLUE);
    
    // Layout text
    DrawText("Layout:", controlsX + 350, controlsY + 8, 14, BLACK);
    DrawText(boardLayout.c_str(), controlsX + 400, controlsY + 8, 14, DARKBLUE);
    
    // Task count
    std::string countText = std::to_string(filteredTasks.size()) + " tasks";
    DrawText(countText.c_str(), controlsX + controlsWidth - 100, controlsY + 8, 14, BLACK);
}

void BulletinBoardScreen::renderButtons() {
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

void BulletinBoardScreen::renderInputFields() {
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
}

void BulletinBoardScreen::renderCharacter() {
    // Draw character at a fixed position in the bulletin board area
    Vector2 characterPos = {50, 400};
    
    // Draw character as a simple colored circle (placeholder)
    Color characterColor = BLUE;
    
    // Change color based on activity
    if (showTaskCreationDialog || showTaskEditDialog) {
        characterColor = YELLOW; // Busy creating/editing
    } else if (selectedTaskId != 0) {
        characterColor = GREEN; // Focused on a task
    }
    
    // Draw character
    DrawCircle((int)characterPos.x, (int)characterPos.y, 16, characterColor);
    DrawCircleLines((int)characterPos.x, (int)characterPos.y, 16, BLACK);
    
    // Draw thought bubble when creating tasks
    if (showTaskCreationDialog || showTaskEditDialog) {
        DrawCircle((int)(characterPos.x + 25), (int)(characterPos.y - 25), 15, WHITE);
        DrawCircleLines((int)(characterPos.x + 25), (int)(characterPos.y - 25), 15, BLACK);
        DrawText("?", (int)(characterPos.x + 20), (int)(characterPos.y - 30), 16, BLACK);
    }
}// 
Task management helpers
void BulletinBoardScreen::refreshFilteredTasks() {
    filteredTasks.clear();
    
    // Get all tasks from the engine
    std::vector<Task> allTasks = taskEngine.getAllTasks();
    
    // Apply filters
    for (const Task& task : allTasks) {
        if (passesFilter(task)) {
            filteredTasks.push_back(task);
        }
    }
    
    // Sort tasks
    sortTasks();
    
    // Update sticky notes
    updateStickyNotes();
}

void BulletinBoardScreen::sortTasks() {
    if (sortBy == "priority") {
        std::sort(filteredTasks.begin(), filteredTasks.end(),
            [](const Task& a, const Task& b) {
                return (int)a.getPriority() > (int)b.getPriority(); // High priority first
            });
    } else if (sortBy == "dueDate") {
        std::sort(filteredTasks.begin(), filteredTasks.end(),
            [](const Task& a, const Task& b) {
                return a.getDueDate() < b.getDueDate();
            });
    } else if (sortBy == "created") {
        std::sort(filteredTasks.begin(), filteredTasks.end(),
            [](const Task& a, const Task& b) {
                return a.getCreatedAt() > b.getCreatedAt(); // Newest first
            });
    } else if (sortBy == "title") {
        std::sort(filteredTasks.begin(), filteredTasks.end(),
            [](const Task& a, const Task& b) {
                return a.getTitle() < b.getTitle();
            });
    }
}

bool BulletinBoardScreen::passesFilter(const Task& task) const {
    // Status filter
    if (!showCompletedTasks && task.getStatus() == Task::Status::COMPLETED) {
        return false;
    }
    
    // Overdue filter
    if (!showOverdueTasks && isTaskOverdue(task)) {
        return false;
    }
    
    // Text filter
    if (!taskFilter.empty()) {
        std::string lowerTitle = task.getTitle();
        std::string lowerFilter = taskFilter;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
        
        if (lowerTitle.find(lowerFilter) == std::string::npos) {
            return false;
        }
    }
    
    return true;
}

void BulletinBoardScreen::layoutStickyNotes() {
    if (boardLayout == "grid") {
        layoutGridView();
    } else if (boardLayout == "list") {
        layoutListView();
    } else if (boardLayout == "kanban") {
        layoutKanbanView();
    }
}

void BulletinBoardScreen::createStickyNoteForTask(const Task& task) {
    Vector2 position = getNextStickyNotePosition();
    StickyNote note(task.getId(), position.x, position.y);
    note.backgroundColor = getTaskBackgroundColor(task);
    note.isAnimating = true;
    note.animationType = "appear";
    note.animationTimer = 0.0f;
    
    stickyNotes.push_back(note);
}

void BulletinBoardScreen::removeStickyNoteForTask(uint32_t taskId) {
    stickyNotes.erase(
        std::remove_if(stickyNotes.begin(), stickyNotes.end(),
            [taskId](const StickyNote& note) { return note.taskId == taskId; }),
        stickyNotes.end()
    );
}

BulletinBoardScreen::StickyNote* BulletinBoardScreen::findStickyNoteForTask(uint32_t taskId) {
    auto it = std::find_if(stickyNotes.begin(), stickyNotes.end(),
        [taskId](const StickyNote& note) { return note.taskId == taskId; });
    
    return (it != stickyNotes.end()) ? &(*it) : nullptr;
}

// Animation helpers
void BulletinBoardScreen::playTaskCompletionAnimation(uint32_t taskId) {
    StickyNote* note = findStickyNoteForTask(taskId);
    if (note) {
        note->isAnimating = true;
        note->animationType = "complete";
        note->animationTimer = 0.0f;
    }
}

void BulletinBoardScreen::playTaskCreationAnimation(uint32_t taskId) {
    StickyNote* note = findStickyNoteForTask(taskId);
    if (note) {
        note->isAnimating = true;
        note->animationType = "appear";
        note->animationTimer = 0.0f;
    }
}

void BulletinBoardScreen::playCharacterAnimation(const std::string& animationName) {
    if (characterAnimator && currentCharacterAnimation != animationName) {
        characterAnimator->playAnimation(animationName);
        currentCharacterAnimation = animationName;
    }
}

void BulletinBoardScreen::updateTaskAnimations(float deltaTime) {
    for (auto& note : stickyNotes) {
        if (note.isAnimating) {
            note.animationTimer += deltaTime;
            
            if (note.animationTimer >= ANIMATION_DURATION) {
                note.isAnimating = false;
                note.animationTimer = 0.0f;
                
                // Handle animation completion
                if (note.animationType == "complete") {
                    // Remove the sticky note after completion animation
                    removeStickyNoteForTask(note.taskId);
                    refreshFilteredTasks();
                    layoutStickyNotes();
                    return; // Exit early since we modified the container
                }
            }
        }
    }
}

// UI helpers
void BulletinBoardScreen::createButtons() {
    buttons.clear();
    
    // Create Task button
    buttons.emplace_back(950, 60, BUTTON_WIDTH, BUTTON_HEIGHT, "New Task");
    buttons.back().backgroundColor = GREEN;
    buttons.back().onClick = [this]() {
        setShowTaskCreationDialog(true);
    };
    
    // Filter buttons
    buttons.emplace_back(950, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "All Tasks");
    buttons.back().onClick = [this]() {
        setTaskFilter("");
    };
    
    buttons.emplace_back(950, 140, BUTTON_WIDTH, BUTTON_HEIGHT, "High Priority");
    buttons.back().backgroundColor = RED;
    buttons.back().onClick = [this]() {
        setTaskFilter(""); // Will be handled by priority filtering logic
        // For now, just refresh to show all tasks
        refreshFilteredTasks();
    };
    
    // Layout buttons
    buttons.emplace_back(950, 200, BUTTON_WIDTH, BUTTON_HEIGHT, "Grid View");
    buttons.back().onClick = [this]() {
        setBoardLayout("grid");
    };
    
    buttons.emplace_back(950, 240, BUTTON_WIDTH, BUTTON_HEIGHT, "List View");
    buttons.back().onClick = [this]() {
        setBoardLayout("list");
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

void BulletinBoardScreen::createInputFields() {
    inputFields.clear();
    
    int dialogX = 512 - 200;
    int dialogY = 384 - 250;
    
    // Title field
    inputFields.emplace_back(dialogX + 20, dialogY + 60, 360, 30, "Task Title:");
    inputFields.back().maxLength = 100;
    
    // Description field
    inputFields.emplace_back(dialogX + 20, dialogY + 120, 360, 60, "Description:");
    inputFields.back().isMultiline = true;
    inputFields.back().maxLength = 500;
    
    // Due date field
    inputFields.emplace_back(dialogX + 20, dialogY + 300, 180, 30, "Due Date (YYYY-MM-DD):");
    inputFields.back().maxLength = 10;
}

void BulletinBoardScreen::resetTaskForm() {
    taskFormData.title.clear();
    taskFormData.priority = Task::Priority::MEDIUM;
    taskFormData.dueDateString.clear();
    taskFormData.description.clear();
    taskFormData.subtasks.clear();
    taskFormData.isEditing = false;
    taskFormData.editingTaskId = 0;
    
    // Reset input fields
    if (inputFields.size() >= 3) {
        inputFields[0].value.clear(); // Title
        inputFields[1].value.clear(); // Description
        inputFields[2].value.clear(); // Due date
    }
}

void BulletinBoardScreen::populateTaskFormForEditing(const Task& task) {
    taskFormData.title = task.getTitle();
    taskFormData.priority = task.getPriority();
    taskFormData.dueDateString = formatDueDate(task.getDueDate());
    taskFormData.description = ""; // Tasks don't have descriptions in our current model
    taskFormData.subtasks.clear(); // Will be populated from task.getSubtasks()
    taskFormData.isEditing = true;
    taskFormData.editingTaskId = task.getId();
    
    // Populate input fields
    if (inputFields.size() >= 3) {
        inputFields[0].value = taskFormData.title;
        inputFields[1].value = taskFormData.description;
        inputFields[2].value = taskFormData.dueDateString;
    }
}

bool BulletinBoardScreen::validateTaskForm() const {
    return !taskFormData.title.empty();
}

void BulletinBoardScreen::submitTaskForm() {
    if (!validateTaskForm()) {
        return;
    }
    
    if (taskFormData.isEditing) {
        // Update existing task
        Task task = taskEngine.getTask(taskFormData.editingTaskId);
        if (task.getId() != 0) {
            task.setTitle(taskFormData.title);
            task.setPriority(taskFormData.priority);
            // Set due date if provided
            // task.setDueDate(...); // Would need date parsing
            
            taskEngine.updateTask(task);
            refreshFilteredTasks();
            layoutStickyNotes();
        }
        setShowTaskEditDialog(false);
    } else {
        // Create new task
        createTask(taskFormData.title, taskFormData.priority);
        setShowTaskCreationDialog(false);
    }
    
    resetTaskForm();
}

void BulletinBoardScreen::handleButtonClick(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < (int)buttons.size()) {
        const Button& button = buttons[buttonIndex];
        if (button.enabled && button.onClick) {
            button.onClick();
        }
    }
}

bool BulletinBoardScreen::isPointInButton(const Button& button, float x, float y) const {
    return x >= button.x && x <= button.x + button.width &&
           y >= button.y && y <= button.y + button.height;
}

bool BulletinBoardScreen::isPointInStickyNote(const StickyNote& note, float x, float y) const {
    return x >= note.x && x <= note.x + note.width &&
           y >= note.y && y <= note.y + note.height;
}

bool BulletinBoardScreen::isPointInInputField(const InputField& field, float x, float y) const {
    return x >= field.x && x <= field.x + field.width &&
           y >= field.y && y <= field.y + field.height;
}

// Task visual helpers
Color BulletinBoardScreen::getPriorityColor(Task::Priority priority) const {
    switch (priority) {
        case Task::Priority::HIGH: return RED;
        case Task::Priority::MEDIUM: return ORANGE;
        case Task::Priority::LOW: return GREEN;
        default: return GRAY;
    }
}

Color BulletinBoardScreen::getTaskBackgroundColor(const Task& task) const {
    if (task.getStatus() == Task::Status::COMPLETED) {
        return LIGHTGREEN;
    } else if (isTaskOverdue(task)) {
        return Fade(RED, 0.3f);
    } else {
        switch (task.getPriority()) {
            case Task::Priority::HIGH: return Fade(RED, 0.2f);
            case Task::Priority::MEDIUM: return YELLOW;
            case Task::Priority::LOW: return Fade(GREEN, 0.2f);
            default: return WHITE;
        }
    }
}

std::string BulletinBoardScreen::getPriorityDisplayName(Task::Priority priority) const {
    switch (priority) {
        case Task::Priority::HIGH: return "High";
        case Task::Priority::MEDIUM: return "Medium";
        case Task::Priority::LOW: return "Low";
        default: return "Unknown";
    }
}

std::string BulletinBoardScreen::formatDueDate(const std::chrono::system_clock::time_point& dueDate) const {
    if (dueDate == std::chrono::system_clock::time_point{}) {
        return "No due date";
    }
    
    auto time_t = std::chrono::system_clock::to_time_t(dueDate);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
    return oss.str();
}

bool BulletinBoardScreen::isTaskOverdue(const Task& task) const {
    if (task.getDueDate() == std::chrono::system_clock::time_point{}) {
        return false; // No due date set
    }
    
    auto now = std::chrono::system_clock::now();
    return task.getDueDate() < now && task.getStatus() != Task::Status::COMPLETED;
}

// Input handling helpers
void BulletinBoardScreen::handleKeyboardInput(const InputState& input) {
    // Handle dialog input
    if (showTaskCreationDialog || showTaskEditDialog) {
        return; // Handled in handleTaskFormInput
    }
    
    // Handle task actions
    if (selectedTaskId != 0) {
        if (input.isKeyJustPressed(KEY_E)) {
            editTask(selectedTaskId);
        } else if (input.isKeyJustPressed(KEY_D)) {
            deleteTask(selectedTaskId);
            setSelectedTaskId(0);
        } else if (input.isKeyJustPressed(KEY_C)) {
            completeTask(selectedTaskId);
            setSelectedTaskId(0);
        }
    }
    
    // General shortcuts
    if (input.isKeyJustPressed(KEY_N)) {
        setShowTaskCreationDialog(true);
    } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
        if (onExitRequested) {
            onExitRequested();
        }
    }
}

void BulletinBoardScreen::handleMouseInput(const InputState& input) {
    // Handle sticky note clicks
    if (input.mouse.leftButton) {
        handleStickyNoteClick(input.mouse.x, input.mouse.y);
    }
}

void BulletinBoardScreen::handleTaskFormInput(const InputState& input) {
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
            if (key >= 32 && key <= 125 && field.value.length() < field.maxLength) {
                field.value += (char)key;
            }
            key = GetCharPressed();
        }
        
        // Handle backspace
        if (input.isKeyJustPressed(KEY_BACKSPACE) && !field.value.empty()) {
            field.value.pop_back();
        }
        
        // Update form data
        if (activeInputFieldIndex == 0) {
            taskFormData.title = field.value;
        } else if (activeInputFieldIndex == 1) {
            taskFormData.description = field.value;
        } else if (activeInputFieldIndex == 2) {
            taskFormData.dueDateString = field.value;
        }
    }
    
    // Handle priority selection
    if (input.mouse.leftButton) {
        int dialogX = 512 - 200;
        int priorityY = 384 - 250 + 200;
        
        for (int i = 0; i < 3; i++) {
            int buttonX = dialogX + 20 + i * 80;
            int buttonY = priorityY + 20;
            
            if (input.mouse.x >= buttonX && input.mouse.x <= buttonX + 70 &&
                input.mouse.y >= buttonY && input.mouse.y <= buttonY + 30) {
                taskFormData.priority = (Task::Priority)i;
            }
        }
        
        // Handle form buttons
        int buttonY = 384 - 250 + 500 - 60;
        
        // Submit button
        if (input.mouse.x >= 512 - 200 + 400 - 180 && input.mouse.x <= 512 - 200 + 400 - 110 &&
            input.mouse.y >= buttonY && input.mouse.y <= buttonY + 30) {
            submitTaskForm();
        }
        
        // Cancel button
        if (input.mouse.x >= 512 - 200 + 400 - 100 && input.mouse.x <= 512 - 200 + 400 - 30 &&
            input.mouse.y >= buttonY && input.mouse.y <= buttonY + 30) {
            setShowTaskCreationDialog(false);
            setShowTaskEditDialog(false);
            resetTaskForm();
        }
    }
    
    // Handle keyboard shortcuts
    if (input.isKeyJustPressed(KEY_ENTER)) {
        submitTaskForm();
    } else if (input.isKeyJustPressed(KEY_ESCAPE)) {
        setShowTaskCreationDialog(false);
        setShowTaskEditDialog(false);
        resetTaskForm();
    } else if (input.isKeyJustPressed(KEY_TAB)) {
        activeInputFieldIndex = (activeInputFieldIndex + 1) % (int)inputFields.size();
    }
}

void BulletinBoardScreen::handleStickyNoteClick(float x, float y) {
    for (const StickyNote& note : stickyNotes) {
        if (isPointInStickyNote(note, x, y)) {
            setSelectedTaskId(note.taskId);
            break;
        }
    }
}

// Layout helpers
void BulletinBoardScreen::layoutGridView() {
    int startX = 120;
    int startY = 70;
    int currentX = startX;
    int currentY = startY;
    int notesInRow = 0;
    
    for (auto& note : stickyNotes) {
        note.x = (float)currentX;
        note.y = (float)currentY;
        
        currentX += (int)(STICKY_NOTE_WIDTH + STICKY_NOTE_MARGIN);
        notesInRow++;
        
        if (notesInRow >= MAX_STICKY_NOTES_PER_ROW) {
            currentX = startX;
            currentY += (int)(STICKY_NOTE_HEIGHT + STICKY_NOTE_MARGIN);
            notesInRow = 0;
        }
    }
}

void BulletinBoardScreen::layoutListView() {
    int startX = 120;
    int startY = 70;
    int currentY = startY;
    
    for (auto& note : stickyNotes) {
        note.x = (float)startX;
        note.y = (float)currentY;
        note.width = 600; // Wider for list view
        note.height = 80;  // Shorter for list view
        
        currentY += (int)(note.height + STICKY_NOTE_MARGIN);
    }
}

void BulletinBoardScreen::layoutKanbanView() {
    // Simple 3-column kanban: To Do, In Progress, Done
    int columnWidth = 250;
    int columnX[] = {120, 390, 660};
    int columnY[] = {70, 70, 70};
    
    for (auto& note : stickyNotes) {
        // Find corresponding task to determine column
        auto taskIt = std::find_if(filteredTasks.begin(), filteredTasks.end(),
            [&note](const Task& task) { return task.getId() == note.taskId; });
        
        if (taskIt != filteredTasks.end()) {
            int column = 0; // Default to "To Do"
            
            if (taskIt->getStatus() == Task::Status::COMPLETED) {
                column = 2; // "Done"
            } else if (taskIt->getStatus() == Task::Status::IN_PROGRESS) {
                column = 1; // "In Progress"
            }
            
            note.x = (float)columnX[column];
            note.y = (float)columnY[column];
            note.width = (float)(columnWidth - 20);
            
            columnY[column] += (int)(note.height + STICKY_NOTE_MARGIN);
        }
    }
}

Vector2 BulletinBoardScreen::getNextStickyNotePosition() {
    // Simple positioning - will be overridden by layout
    return {120.0f, 70.0f};
}

// Add the missing renderTaskFormDialog method
void BulletinBoardScreen::renderTaskFormDialog(const std::string& title, bool isEditing) {
    // This method was referenced but not implemented - adding it here
    // The implementation is already in renderTaskCreationDialog and renderTaskEditDialog
    // This is a helper method to avoid code duplication
}