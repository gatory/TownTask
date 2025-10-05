#include "library_screen.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <chrono>

LibraryScreen::LibraryScreen(NoteEngine& noteEngine, GamificationEngine& gamificationEngine,
                             InputManager& inputManager, AnimationManager& animationManager)
    : Screen("LibraryScreen")
    , noteEngine(noteEngine)
    , gamificationEngine(gamificationEngine)
    , inputManager(inputManager)
    , animationManager(animationManager)
    , showNoteEditor(true)
    , showNoteList(true)
    , selectedNoteId(0)
    , viewMode("split")
    , searchQuery("")
    , tagFilter("")
    , categoryFilter("")
    , sortBy("modified")
    , showTags(true)
    , showCategories(true)
    , showTimestamps(true)
    , fontSize(DEFAULT_FONT_SIZE)
    , noteListScrollOffset(0.0f)
    , hoveredButtonIndex(-1)
    , activeTextAreaIndex(-1) {
    
    // Create UI elements
    createButtons();
    createTextAreas();
    
    // Initialize note editor
    noteEditorData = NoteEditorData();
    
    std::cout << "LibraryScreen: Initialized successfully" << std::endl;
}

LibraryScreen::~LibraryScreen() {
    // Cleanup is handled by smart pointers and references
}

// Screen interface implementation
void LibraryScreen::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update notes
    updateNotes(deltaTime);
    
    // Update animations
    updateAnimations(deltaTime);
    
    // Update UI elements
    updateUI(deltaTime);
    
    // Update button states
    const InputState& input = inputManager.getCurrentState();
    updateButtons(input);
    
    // Update text areas
    updateTextAreas(input);
}

void LibraryScreen::render() {
    if (!isActive()) return;
    
    // Render background and library interior
    renderBackground();
    renderLibraryInterior();
    
    // Render character
    renderCharacter();
    
    // Render based on view mode
    if (viewMode == "split") {
        renderSplitView();
    } else if (viewMode == "list") {
        renderNoteList();
    } else if (viewMode == "editor") {
        renderNoteEditor();
    }
    
    // Render search bar
    renderSearchBar();
    
    // Render tags and categories
    renderTagsAndCategories();
    
    // Render buttons
    renderButtons();
}

void LibraryScreen::handleInput(const InputState& input) {
    if (!isActive()) return;
    
    // Handle keyboard input
    handleKeyboardInput(input);
    
    // Handle mouse input
    handleMouseInput(input);
    
    // Handle text input for active text areas
    if (activeTextAreaIndex >= 0) {
        handleTextInput(input);
    }
    
    // Handle scrolling
    handleScrolling(input);
}

void LibraryScreen::onEnter() {
    Screen::onEnter();
    
    // Initialize character animator
    characterAnimator = animationManager.createAnimator("library_character", "character_sheet");
    if (characterAnimator) {
        animationManager.createCharacterAnimations("library_character", "character_sheet", 32, 32);
        playCharacterAnimation("reading");
    }
    
    // Initialize book animator for note animations
    bookAnimator = animationManager.createAnimator("book_effects", "book_sheet");
    if (bookAnimator) {
        animationManager.createEffectAnimations("book_effects", "book_sheet", 64, 64);
    }
    
    // Refresh notes and layout
    refreshFilteredNotes();
    updateAvailableTagsAndCategories();
    layoutNoteList();
    
    std::cout << "LibraryScreen: Entered library" << std::endl;
}

void LibraryScreen::onExit() {
    // Save any unsaved changes
    if (noteEditorData.hasUnsavedChanges) {
        saveCurrentNote();
    }
    
    Screen::onExit();
    std::cout << "LibraryScreen: Exited library" << std::endl;
}

// Note management functionality
void LibraryScreen::createNote(const std::string& title, const std::string& content, const std::vector<std::string>& tags) {
    Note newNote(title, content);
    
    // Add tags
    for (const std::string& tag : tags) {
        newNote.addTag(tag);
    }
    
    uint32_t noteId = noteEngine.createNote(newNote);
    
    // Award XP for note creation
    gamificationEngine.awardExperience(5, "Created new note");
    
    // Refresh display
    refreshFilteredNotes();
    updateAvailableTagsAndCategories();
    layoutNoteList();
    
    // Select the new note
    selectNote(noteId);
    
    // Trigger callback
    if (onNoteCreated) {
        onNoteCreated(noteId);
    }
    
    std::cout << "LibraryScreen: Created note '" << title << "' with ID " << noteId << std::endl;
}

void LibraryScreen::editNote(uint32_t noteId) {
    Note note = noteEngine.getNote(noteId);
    if (note.getId() != 0) { // Valid note
        loadNoteIntoEditor(note);
        setSelectedNoteId(noteId);
        setShowNoteEditor(true);
    }
}

void LibraryScreen::deleteNote(uint32_t noteId) {
    bool success = noteEngine.deleteNote(noteId);
    if (success) {
        // Clear selection if deleted note was selected
        if (selectedNoteId == noteId) {
            setSelectedNoteId(0);
            noteEditorData = NoteEditorData();
        }
        
        // Refresh display
        refreshFilteredNotes();
        updateAvailableTagsAndCategories();
        layoutNoteList();
        
        // Trigger callback
        if (onNoteDeleted) {
            onNoteDeleted(noteId);
        }
        
        std::cout << "LibraryScreen: Deleted note with ID " << noteId << std::endl;
    }
}

void LibraryScreen::duplicateNote(uint32_t noteId) {
    Note originalNote = noteEngine.getNote(noteId);
    if (originalNote.getId() != 0) {
        // Create a copy with modified title
        std::string newTitle = originalNote.getTitle() + " (Copy)";
        createNote(newTitle, originalNote.getContent(), originalNote.getTags());
    }
}

// Note organization
void LibraryScreen::addTagToNote(uint32_t noteId, const std::string& tag) {
    Note note = noteEngine.getNote(noteId);
    if (note.getId() != 0) {
        note.addTag(tag);
        noteEngine.updateNote(note);
        
        // Update editor if this note is being edited
        if (noteEditorData.isEditing && noteEditorData.editingNoteId == noteId) {
            noteEditorData.tags = note.getTags();
        }
        
        refreshFilteredNotes();
        updateAvailableTagsAndCategories();
    }
}

void LibraryScreen::removeTagFromNote(uint32_t noteId, const std::string& tag) {
    Note note = noteEngine.getNote(noteId);
    if (note.getId() != 0) {
        note.removeTag(tag);
        noteEngine.updateNote(note);
        
        // Update editor if this note is being edited
        if (noteEditorData.isEditing && noteEditorData.editingNoteId == noteId) {
            noteEditorData.tags = note.getTags();
        }
        
        refreshFilteredNotes();
        updateAvailableTagsAndCategories();
    }
}

void LibraryScreen::setNoteCategory(uint32_t noteId, const std::string& category) {
    Note note = noteEngine.getNote(noteId);
    if (note.getId() != 0) {
        note.setCategory(category);
        noteEngine.updateNote(note);
        
        // Update editor if this note is being edited
        if (noteEditorData.isEditing && noteEditorData.editingNoteId == noteId) {
            noteEditorData.category = category;
        }
        
        refreshFilteredNotes();
        updateAvailableTagsAndCategories();
    }
}

// Search and filtering
void LibraryScreen::setSearchQuery(const std::string& query) {
    searchQuery = query;
    refreshFilteredNotes();
    layoutNoteList();
}

std::string LibraryScreen::getSearchQuery() const {
    return searchQuery;
}

void LibraryScreen::setTagFilter(const std::string& tag) {
    tagFilter = tag;
    refreshFilteredNotes();
    layoutNoteList();
}

std::string LibraryScreen::getTagFilter() const {
    return tagFilter;
}

void LibraryScreen::setCategoryFilter(const std::string& category) {
    categoryFilter = category;
    refreshFilteredNotes();
    layoutNoteList();
}

std::string LibraryScreen::getCategoryFilter() const {
    return categoryFilter;
}

void LibraryScreen::setSortBy(const std::string& sortBy) {
    this->sortBy = sortBy;
    refreshFilteredNotes();
    layoutNoteList();
}

std::string LibraryScreen::getSortBy() const {
    return sortBy;
}

// UI state management
void LibraryScreen::setShowNoteEditor(bool show) {
    showNoteEditor = show;
    if (show && viewMode == "list") {
        setViewMode("split");
    }
}

bool LibraryScreen::isShowingNoteEditor() const {
    return showNoteEditor;
}

void LibraryScreen::setShowNoteList(bool show) {
    showNoteList = show;
    if (show && viewMode == "editor") {
        setViewMode("split");
    }
}

bool LibraryScreen::isShowingNoteList() const {
    return showNoteList;
}

void LibraryScreen::setSelectedNoteId(uint32_t noteId) {
    selectedNoteId = noteId;
    
    // Update note list selection
    for (auto& item : noteListItems) {
        item.isSelected = (item.noteId == noteId);
    }
}

uint32_t LibraryScreen::getSelectedNoteId() const {
    return selectedNoteId;
}

void LibraryScreen::setViewMode(const std::string& mode) {
    viewMode = mode;
    
    // Update show flags based on view mode
    if (mode == "split") {
        showNoteList = true;
        showNoteEditor = true;
    } else if (mode == "list") {
        showNoteList = true;
        showNoteEditor = false;
    } else if (mode == "editor") {
        showNoteList = false;
        showNoteEditor = true;
    }
}

std::string LibraryScreen::getViewMode() const {
    return viewMode;
}

// Visual settings
void LibraryScreen::setShowTags(bool show) {
    showTags = show;
}

bool LibraryScreen::isShowingTags() const {
    return showTags;
}

void LibraryScreen::setShowCategories(bool show) {
    showCategories = show;
}

bool LibraryScreen::isShowingCategories() const {
    return showCategories;
}

void LibraryScreen::setShowTimestamps(bool show) {
    showTimestamps = show;
}

bool LibraryScreen::isShowingTimestamps() const {
    return showTimestamps;
}

void LibraryScreen::setFontSize(int size) {
    fontSize = std::max(8, std::min(24, size)); // Clamp between 8 and 24
}

int LibraryScreen::getFontSize() const {
    return fontSize;
}

// Event callbacks
void LibraryScreen::setOnNoteCreated(std::function<void(uint32_t)> callback) {
    onNoteCreated = callback;
}

void LibraryScreen::setOnNoteModified(std::function<void(uint32_t)> callback) {
    onNoteModified = callback;
}

void LibraryScreen::setOnNoteDeleted(std::function<void(uint32_t)> callback) {
    onNoteDeleted = callback;
}

void LibraryScreen::setOnExitRequested(std::function<void()> callback) {
    onExitRequested = callback;
}// Int
ernal update methods
void LibraryScreen::updateNotes(float deltaTime) {
    // Check for note updates from the engine
    // This could include new notes, modified notes, etc.
    
    // Update notes if they have changed
    static float refreshTimer = 0.0f;
    refreshTimer += deltaTime;
    if (refreshTimer >= 2.0f) { // Refresh every 2 seconds
        refreshFilteredNotes();
        refreshTimer = 0.0f;
    }
}

void LibraryScreen::updateAnimations(float deltaTime) {
    // Update character animator
    if (characterAnimator) {
        characterAnimator->update(deltaTime);
    }
    
    // Update book animator
    if (bookAnimator) {
        bookAnimator->update(deltaTime);
    }
    
    // Update character animation based on current state
    updateCharacterAnimationForState();
}

void LibraryScreen::updateUI(float deltaTime) {
    // Update any UI animations or state changes
    // Auto-save notes periodically
    if (noteEditorData.hasUnsavedChanges) {
        static float autoSaveTimer = 0.0f;
        autoSaveTimer += deltaTime;
        if (autoSaveTimer >= 5.0f) { // Auto-save every 5 seconds
            saveCurrentNote();
            autoSaveTimer = 0.0f;
        }
    }
}

void LibraryScreen::updateNoteList() {
    // Update note list items based on filtered notes
    noteListItems.clear();
    
    float currentY = NOTE_LIST_PADDING;
    for (const Note& note : filteredNotes) {
        noteListItems.emplace_back(note.getId(), currentY);
        noteListItems.back().isSelected = (note.getId() == selectedNoteId);
        currentY += NOTE_LIST_ITEM_HEIGHT + NOTE_LIST_PADDING;
    }
}

void LibraryScreen::updateButtons(const InputState& input) {
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

void LibraryScreen::updateTextAreas(const InputState& input) {
    // Handle text area focus
    for (size_t i = 0; i < textAreas.size(); i++) {
        textAreas[i].isActive = (i == activeTextAreaIndex);
        
        if (input.mouse.leftButton && isPointInTextArea(textAreas[i], input.mouse.x, input.mouse.y)) {
            activeTextAreaIndex = (int)i;
        }
    }
}

// Rendering methods
void LibraryScreen::renderBackground() {
    // Draw library background
    ClearBackground(Fade(DARKBROWN, 0.2f));
    
    // Draw wooden floor pattern
    const int tileSize = 32;
    for (int x = 0; x < 1024; x += tileSize) {
        for (int y = 0; y < 768; y += tileSize) {
            Color floorColor = ((x + y) / tileSize) % 2 == 0 ? BROWN : Fade(BROWN, 0.8f);
            DrawRectangle(x, y, tileSize, tileSize, floorColor);
        }
    }
}

void LibraryScreen::renderLibraryInterior() {
    // Draw bookshelves
    DrawRectangle(50, 100, 30, 500, DARKBROWN);
    DrawRectangle(944, 100, 30, 500, DARKBROWN);
    
    // Draw books on shelves
    for (int shelf = 0; shelf < 5; shelf++) {
        int shelfY = 120 + shelf * 100;
        for (int book = 0; book < 8; book++) {
            int bookX = 55 + book * 3;
            Color bookColor = (Color){(unsigned char)(100 + book * 20), (unsigned char)(50 + shelf * 30), (unsigned char)(150 + book * 10), 255};
            DrawRectangle(bookX, shelfY, 2, 80, bookColor);
        }
        
        // Right shelf
        for (int book = 0; book < 8; book++) {
            int bookX = 949 + book * 3;
            Color bookColor = (Color){(unsigned char)(150 + book * 15), (unsigned char)(100 + shelf * 20), (unsigned char)(80 + book * 15), 255};
            DrawRectangle(bookX, shelfY, 2, 80, bookColor);
        }
    }
    
    // Draw reading desk
    DrawRectangle(400, 600, 224, 100, BROWN);
    DrawRectangleLines(400, 600, 224, 100, BLACK);
    
    // Draw lamp
    DrawCircle(512, 580, 20, YELLOW);
    DrawRectangle(507, 580, 10, 40, DARKGRAY);
    
    // Title
    const char* title = "Library - Knowledge Repository";
    int titleWidth = MeasureText(title, 24);
    DrawText(title, 512 - titleWidth/2, 20, 24, DARKBROWN);
}

void LibraryScreen::renderCharacter() {
    // Draw character at the reading desk
    Vector2 characterPos = {350, 550};
    
    // Draw character as a simple colored circle (placeholder)
    Color characterColor = BLUE;
    
    // Change color based on activity
    if (noteEditorData.isEditing) {
        characterColor = GREEN; // Writing
    } else if (selectedNoteId != 0) {
        characterColor = PURPLE; // Reading
    }
    
    // Draw character
    DrawCircle((int)characterPos.x, (int)characterPos.y, 16, characterColor);
    DrawCircleLines((int)characterPos.x, (int)characterPos.y, 16, BLACK);
    
    // Draw book in character's hands when reading/writing
    if (selectedNoteId != 0 || noteEditorData.isEditing) {
        DrawRectangle((int)(characterPos.x - 8), (int)(characterPos.y + 10), 16, 12, WHITE);
        DrawRectangleLines((int)(characterPos.x - 8), (int)(characterPos.y + 10), 16, 12, BLACK);
    }
}

void LibraryScreen::renderNoteList() {
    // Note list panel
    int listX = 100;
    int listY = 100;
    int listWidth = viewMode == "split" ? 300 : 824;
    int listHeight = 450;
    
    // Background
    DrawRectangle(listX, listY, listWidth, listHeight, Fade(WHITE, 0.9f));
    DrawRectangleLines(listX, listY, listWidth, listHeight, BLACK);
    
    // Title
    DrawText("Notes", listX + 10, listY + 10, 16, BLACK);
    
    // Note count
    std::string countText = std::to_string(filteredNotes.size()) + " notes";
    DrawText(countText.c_str(), listX + listWidth - 80, listY + 10, 12, DARKGRAY);
    
    // Render note list items
    int itemY = listY + 40;
    for (size_t i = 0; i < noteListItems.size(); i++) {
        const NoteListItem& item = noteListItems[i];
        
        // Skip items that are scrolled out of view
        if (item.y - noteListScrollOffset < -NOTE_LIST_ITEM_HEIGHT || 
            item.y - noteListScrollOffset > listHeight) {
            continue;
        }
        
        // Find corresponding note
        auto noteIt = std::find_if(filteredNotes.begin(), filteredNotes.end(),
            [&item](const Note& note) { return note.getId() == item.noteId; });
        
        if (noteIt != filteredNotes.end()) {
            renderNoteListItem(item, *noteIt);
        }
    }
    
    // Scroll indicator
    if (noteListItems.size() * (NOTE_LIST_ITEM_HEIGHT + NOTE_LIST_PADDING) > listHeight) {
        float scrollBarHeight = (float)listHeight * listHeight / (noteListItems.size() * (NOTE_LIST_ITEM_HEIGHT + NOTE_LIST_PADDING));
        float scrollBarY = listY + (noteListScrollOffset / (noteListItems.size() * (NOTE_LIST_ITEM_HEIGHT + NOTE_LIST_PADDING))) * listHeight;
        
        DrawRectangle(listX + listWidth - 10, (int)scrollBarY, 8, (int)scrollBarHeight, GRAY);
    }
}

void LibraryScreen::renderNoteEditor() {
    if (!showNoteEditor) return;
    
    // Editor panel
    int editorX = viewMode == "split" ? 420 : 100;
    int editorY = 100;
    int editorWidth = viewMode == "split" ? 504 : 824;
    int editorHeight = 450;
    
    // Background
    DrawRectangle(editorX, editorY, editorWidth, editorHeight, Fade(WHITE, 0.95f));
    DrawRectangleLines(editorX, editorY, editorWidth, editorHeight, BLACK);
    
    if (selectedNoteId == 0 && !noteEditorData.isEditing) {
        // No note selected
        const char* message = "Select a note to view or create a new one";
        int messageWidth = MeasureText(message, 16);
        DrawText(message, editorX + editorWidth/2 - messageWidth/2, editorY + editorHeight/2, 16, GRAY);
        return;
    }
    
    // Editor title
    const char* editorTitle = noteEditorData.isEditing ? "Editing Note" : "Note Viewer";
    DrawText(editorTitle, editorX + 10, editorY + 10, 16, BLACK);
    
    // Unsaved changes indicator
    if (noteEditorData.hasUnsavedChanges) {
        DrawText("*", editorX + editorWidth - 20, editorY + 10, 16, RED);
    }
    
    // Render text areas (title and content)
    renderTextAreas();
    
    // Tags display
    if (showTags && !noteEditorData.tags.empty()) {
        int tagY = editorY + editorHeight - 60;
        DrawText("Tags:", editorX + 10, tagY, 12, DARKGRAY);
        
        int tagX = editorX + 50;
        for (const std::string& tag : noteEditorData.tags) {
            renderTag(tag, (float)tagX, (float)tagY);
            tagX += MeasureText(tag.c_str(), 10) + 20;
        }
    }
    
    // Category display
    if (showCategories && !noteEditorData.category.empty()) {
        int categoryY = editorY + editorHeight - 40;
        DrawText("Category:", editorX + 10, categoryY, 12, DARKGRAY);
        renderCategory(noteEditorData.category, (float)(editorX + 80), (float)categoryY);
    }
}

void LibraryScreen::renderSplitView() {
    renderNoteList();
    renderNoteEditor();
    
    // Divider line
    DrawLine(410, 100, 410, 550, GRAY);
}

void LibraryScreen::renderSearchBar() {
    // Search bar
    int searchX = 100;
    int searchY = 60;
    int searchWidth = 300;
    int searchHeight = (int)SEARCH_BAR_HEIGHT;
    
    DrawRectangle(searchX, searchY, searchWidth, searchHeight, WHITE);
    DrawRectangleLines(searchX, searchY, searchWidth, searchHeight, GRAY);
    
    // Search icon
    DrawText("🔍", searchX + 5, searchY + 5, 16, GRAY);
    
    // Search text
    std::string displayText = searchQuery.empty() ? "Search notes..." : searchQuery;
    Color textColor = searchQuery.empty() ? LIGHTGRAY : BLACK;
    DrawText(displayText.c_str(), searchX + 25, searchY + 8, 12, textColor);
    
    // Filter indicators
    int filterX = searchX + searchWidth + 20;
    if (!tagFilter.empty()) {
        std::string tagText = "Tag: " + tagFilter;
        DrawText(tagText.c_str(), filterX, searchY + 8, 12, BLUE);
        filterX += MeasureText(tagText.c_str(), 12) + 20;
    }
    
    if (!categoryFilter.empty()) {
        std::string categoryText = "Category: " + categoryFilter;
        DrawText(categoryText.c_str(), filterX, searchY + 8, 12, GREEN);
    }
}

void LibraryScreen::renderTagsAndCategories() {
    if (!showTags && !showCategories) return;
    
    // Tags and categories panel
    int panelX = 100;
    int panelY = 570;
    int panelWidth = 824;
    int panelHeight = 80;
    
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(LIGHTGRAY, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GRAY);
    
    int currentX = panelX + 10;
    int currentY = panelY + 10;
    
    // Available tags
    if (showTags) {
        DrawText("Tags:", currentX, currentY, 12, BLACK);
        currentX += 50;
        
        for (const std::string& tag : availableTags) {
            bool isSelected = (tag == tagFilter);
            renderTag(tag, (float)currentX, (float)currentY, isSelected);
            currentX += MeasureText(tag.c_str(), 10) + 25;
            
            if (currentX > panelX + panelWidth - 100) {
                currentX = panelX + 10;
                currentY += 20;
            }
        }
    }
    
    // Available categories
    if (showCategories && currentY < panelY + panelHeight - 20) {
        currentX = panelX + 10;
        currentY += 25;
        
        DrawText("Categories:", currentX, currentY, 12, BLACK);
        currentX += 80;
        
        for (const std::string& category : availableCategories) {
            bool isSelected = (category == categoryFilter);
            renderCategory(category, (float)currentX, (float)currentY, isSelected);
            currentX += MeasureText(category.c_str(), 10) + 25;
            
            if (currentX > panelX + panelWidth - 100) {
                break; // No more space
            }
        }
    }
}

void LibraryScreen::renderButtons() {
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

void LibraryScreen::renderTextAreas() {
    for (size_t i = 0; i < textAreas.size(); i++) {
        const TextArea& area = textAreas[i];
        
        Color bgColor = area.isActive ? WHITE : LIGHTGRAY;
        Color borderColor = area.isActive ? BLUE : BLACK;
        
        // Label
        DrawText(area.label.c_str(), (int)area.x, (int)(area.y - 20), 12, BLACK);
        
        // Area background
        DrawRectangle((int)area.x, (int)area.y, (int)area.width, (int)area.height, bgColor);
        DrawRectangleLines((int)area.x, (int)area.y, (int)area.width, (int)area.height, borderColor);
        
        // Area text
        if (area.valuePtr) {
            std::string displayText = *area.valuePtr;
            if (area.isMultiline) {
                // Simple multiline text rendering
                std::istringstream iss(displayText);
                std::string line;
                int lineY = (int)(area.y + 5);
                while (std::getline(iss, line) && lineY < area.y + area.height - 15) {
                    DrawText(line.c_str(), (int)(area.x + 5), lineY, fontSize, BLACK);
                    lineY += fontSize + 2;
                }
            } else {
                DrawText(displayText.c_str(), (int)(area.x + 5), (int)(area.y + 5), fontSize, BLACK);
            }
        }
        
        // Cursor (if active)
        if (area.isActive && area.valuePtr) {
            int textWidth = MeasureText(area.valuePtr->c_str(), fontSize);
            DrawLine((int)(area.x + 5 + textWidth), (int)(area.y + 5), 
                    (int)(area.x + 5 + textWidth), (int)(area.y + area.height - 5), BLACK);
        }
    }
}

void LibraryScreen::renderNoteListItem(const NoteListItem& item, const Note& note) {
    int listX = 100;
    int listWidth = viewMode == "split" ? 300 : 824;
    
    float itemY = item.y - noteListScrollOffset + 100 + 40; // Adjust for list position and header
    
    // Background
    Color bgColor = item.isSelected ? Fade(BLUE, 0.3f) : (item.isHovered ? Fade(GRAY, 0.2f) : WHITE);
    DrawRectangle(listX + 5, (int)itemY, listWidth - 10, (int)item.height, bgColor);
    DrawRectangleLines(listX + 5, (int)itemY, listWidth - 10, (int)item.height, GRAY);
    
    // Note title
    std::string title = truncateText(note.getTitle(), 30);
    DrawText(title.c_str(), listX + 10, (int)(itemY + 5), 14, BLACK);
    
    // Note preview (first line of content)
    std::string preview = truncateText(note.getContent(), 40);
    DrawText(preview.c_str(), listX + 10, (int)(itemY + 25), 10, DARKGRAY);
    
    // Timestamp
    if (showTimestamps) {
        std::string timestamp = formatTimestamp(note.getModifiedAt());
        int timestampWidth = MeasureText(timestamp.c_str(), 9);
        DrawText(timestamp.c_str(), listX + listWidth - timestampWidth - 15, (int)(itemY + 5), 9, GRAY);
    }
    
    // Tags indicator
    if (showTags && !note.getTags().empty()) {
        std::string tagCount = std::to_string(note.getTags().size()) + " tags";
        DrawText(tagCount.c_str(), listX + 10, (int)(itemY + 45), 8, BLUE);
    }
    
    // Category indicator
    if (showCategories && !note.getCategory().empty()) {
        Color categoryColor = getCategoryColor(note.getCategory());
        DrawCircle(listX + listWidth - 25, (int)(itemY + 30), 5, categoryColor);
    }
}// Note m
anagement helpers
void LibraryScreen::refreshFilteredNotes() {
    filteredNotes.clear();
    
    // Get all notes from the engine
    std::vector<Note> allNotes = noteEngine.getAllNotes();
    
    // Apply filters
    for (const Note& note : allNotes) {
        if (passesFilter(note)) {
            filteredNotes.push_back(note);
        }
    }
    
    // Sort notes
    sortNotes();
    
    // Update note list
    updateNoteList();
}

void LibraryScreen::sortNotes() {
    if (sortBy == "title") {
        std::sort(filteredNotes.begin(), filteredNotes.end(),
            [](const Note& a, const Note& b) {
                return a.getTitle() < b.getTitle();
            });
    } else if (sortBy == "created") {
        std::sort(filteredNotes.begin(), filteredNotes.end(),
            [](const Note& a, const Note& b) {
                return a.getCreatedAt() > b.getCreatedAt(); // Newest first
            });
    } else if (sortBy == "modified") {
        std::sort(filteredNotes.begin(), filteredNotes.end(),
            [](const Note& a, const Note& b) {
                return a.getModifiedAt() > b.getModifiedAt(); // Most recently modified first
            });
    } else if (sortBy == "category") {
        std::sort(filteredNotes.begin(), filteredNotes.end(),
            [](const Note& a, const Note& b) {
                return a.getCategory() < b.getCategory();
            });
    }
}

bool LibraryScreen::passesFilter(const Note& note) const {
    // Search query filter
    if (!searchQuery.empty() && !matchesSearchQuery(note)) {
        return false;
    }
    
    // Tag filter
    if (!tagFilter.empty()) {
        const std::vector<std::string>& tags = note.getTags();
        if (std::find(tags.begin(), tags.end(), tagFilter) == tags.end()) {
            return false;
        }
    }
    
    // Category filter
    if (!categoryFilter.empty() && note.getCategory() != categoryFilter) {
        return false;
    }
    
    return true;
}

void LibraryScreen::layoutNoteList() {
    updateNoteList();
}

void LibraryScreen::selectNote(uint32_t noteId) {
    setSelectedNoteId(noteId);
    
    if (noteId != 0) {
        Note note = noteEngine.getNote(noteId);
        if (note.getId() != 0) {
            loadNoteIntoEditor(note);
        }
    }
}

void LibraryScreen::loadNoteIntoEditor(const Note& note) {
    noteEditorData.title = note.getTitle();
    noteEditorData.content = note.getContent();
    noteEditorData.tags = note.getTags();
    noteEditorData.category = note.getCategory();
    noteEditorData.isEditing = true;
    noteEditorData.editingNoteId = note.getId();
    noteEditorData.hasUnsavedChanges = false;
    
    // Update text areas
    if (textAreas.size() >= 2) {
        textAreas[0].valuePtr = &noteEditorData.title;
        textAreas[1].valuePtr = &noteEditorData.content;
    }
}

void LibraryScreen::saveCurrentNote() {
    if (!noteEditorData.isEditing || !noteEditorData.hasUnsavedChanges) {
        return;
    }
    
    Note note = noteEngine.getNote(noteEditorData.editingNoteId);
    if (note.getId() != 0) {
        note.setTitle(noteEditorData.title);
        note.setContent(noteEditorData.content);
        note.setCategory(noteEditorData.category);
        
        // Update tags
        note.clearTags();
        for (const std::string& tag : noteEditorData.tags) {
            note.addTag(tag);
        }
        
        noteEngine.updateNote(note);
        noteEditorData.hasUnsavedChanges = false;
        
        // Award XP for note modification
        gamificationEngine.awardExperience(3, "Modified note");
        
        // Refresh display
        refreshFilteredNotes();
        updateAvailableTagsAndCategories();
        
        // Trigger callback
        if (onNoteModified) {
            onNoteModified(note.getId());
        }
        
        std::cout << "LibraryScreen: Saved note '" << note.getTitle() << "'" << std::endl;
    }
}

void LibraryScreen::createNewNote() {
    createNote("New Note", "", {});
}

void LibraryScreen::discardChanges() {
    if (noteEditorData.isEditing && selectedNoteId != 0) {
        // Reload the note from the engine
        Note note = noteEngine.getNote(selectedNoteId);
        if (note.getId() != 0) {
            loadNoteIntoEditor(note);
        }
    } else {
        // Clear editor
        noteEditorData = NoteEditorData();
        setSelectedNoteId(0);
    }
}

// Search and filter helpers
bool LibraryScreen::matchesSearchQuery(const Note& note) const {
    std::string lowerQuery = searchQuery;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Search in title
    std::string lowerTitle = note.getTitle();
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
    if (lowerTitle.find(lowerQuery) != std::string::npos) {
        return true;
    }
    
    // Search in content
    std::string lowerContent = note.getContent();
    std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
    if (lowerContent.find(lowerQuery) != std::string::npos) {
        return true;
    }
    
    // Search in tags
    for (const std::string& tag : note.getTags()) {
        std::string lowerTag = tag;
        std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
        if (lowerTag.find(lowerQuery) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

void LibraryScreen::updateAvailableTagsAndCategories() {
    availableTags.clear();
    availableCategories.clear();
    
    std::vector<Note> allNotes = noteEngine.getAllNotes();
    
    for (const Note& note : allNotes) {
        // Collect tags
        for (const std::string& tag : note.getTags()) {
            if (std::find(availableTags.begin(), availableTags.end(), tag) == availableTags.end()) {
                availableTags.push_back(tag);
            }
        }
        
        // Collect categories
        if (!note.getCategory().empty()) {
            if (std::find(availableCategories.begin(), availableCategories.end(), note.getCategory()) == availableCategories.end()) {
                availableCategories.push_back(note.getCategory());
            }
        }
    }
    
    // Sort for consistent display
    std::sort(availableTags.begin(), availableTags.end());
    std::sort(availableCategories.begin(), availableCategories.end());
}

std::vector<std::string> LibraryScreen::parseTagsFromString(const std::string& tagString) const {
    std::vector<std::string> tags;
    std::istringstream iss(tagString);
    std::string tag;
    
    while (std::getline(iss, tag, ',')) {
        // Trim whitespace
        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        
        if (!tag.empty()) {
            tags.push_back(tag);
        }
    }
    
    return tags;
}

std::string LibraryScreen::tagsToString(const std::vector<std::string>& tags) const {
    std::string result;
    for (size_t i = 0; i < tags.size(); i++) {
        if (i > 0) result += ", ";
        result += tags[i];
    }
    return result;
}

// UI helpers
void LibraryScreen::createButtons() {
    buttons.clear();
    
    // New Note button
    buttons.emplace_back(950, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "New Note");
    buttons.back().backgroundColor = GREEN;
    buttons.back().onClick = [this]() {
        createNewNote();
    };
    
    // Save button
    buttons.emplace_back(950, 140, BUTTON_WIDTH, BUTTON_HEIGHT, "Save");
    buttons.back().backgroundColor = BLUE;
    buttons.back().onClick = [this]() {
        saveCurrentNote();
    };
    
    // Delete button
    buttons.emplace_back(950, 180, BUTTON_WIDTH, BUTTON_HEIGHT, "Delete");
    buttons.back().backgroundColor = RED;
    buttons.back().onClick = [this]() {
        if (selectedNoteId != 0) {
            deleteNote(selectedNoteId);
        }
    };
    
    // Duplicate button
    buttons.emplace_back(950, 220, BUTTON_WIDTH, BUTTON_HEIGHT, "Duplicate");
    buttons.back().onClick = [this]() {
        if (selectedNoteId != 0) {
            duplicateNote(selectedNoteId);
        }
    };
    
    // View mode buttons
    buttons.emplace_back(950, 280, BUTTON_WIDTH, BUTTON_HEIGHT, "Split View");
    buttons.back().onClick = [this]() {
        setViewMode("split");
    };
    
    buttons.emplace_back(950, 320, BUTTON_WIDTH, BUTTON_HEIGHT, "List View");
    buttons.back().onClick = [this]() {
        setViewMode("list");
    };
    
    buttons.emplace_back(950, 360, BUTTON_WIDTH, BUTTON_HEIGHT, "Editor View");
    buttons.back().onClick = [this]() {
        setViewMode("editor");
    };
    
    // Clear filters button
    buttons.emplace_back(950, 420, BUTTON_WIDTH, BUTTON_HEIGHT, "Clear Filters");
    buttons.back().onClick = [this]() {
        setSearchQuery("");
        setTagFilter("");
        setCategoryFilter("");
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

void LibraryScreen::createTextAreas() {
    textAreas.clear();
    
    int editorX = viewMode == "split" ? 420 : 100;
    int editorY = 100;
    
    // Title area
    textAreas.emplace_back(editorX + 10, editorY + 40, 400, 30, "Title:", &noteEditorData.title);
    textAreas.back().maxLength = MAX_TITLE_LENGTH;
    
    // Content area
    textAreas.emplace_back(editorX + 10, editorY + 100, 400, 300, "Content:", &noteEditorData.content);
    textAreas.back().isMultiline = true;
    textAreas.back().maxLength = MAX_CONTENT_LENGTH;
}

void LibraryScreen::handleButtonClick(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < (int)buttons.size()) {
        const Button& button = buttons[buttonIndex];
        if (button.enabled && button.onClick) {
            button.onClick();
        }
    }
}

bool LibraryScreen::isPointInButton(const Button& button, float x, float y) const {
    return x >= button.x && x <= button.x + button.width &&
           y >= button.y && y <= button.y + button.height;
}

bool LibraryScreen::isPointInTextArea(const TextArea& area, float x, float y) const {
    return x >= area.x && x <= area.x + area.width &&
           y >= area.y && y <= area.y + area.height;
}

bool LibraryScreen::isPointInNoteListItem(const NoteListItem& item, float x, float y) const {
    int listX = 100;
    int listWidth = viewMode == "split" ? 300 : 824;
    float itemY = item.y - noteListScrollOffset + 100 + 40;
    
    return x >= listX + 5 && x <= listX + listWidth - 5 &&
           y >= itemY && y <= itemY + item.height;
}

// Animation helpers
void LibraryScreen::playCharacterAnimation(const std::string& animationName) {
    if (characterAnimator && currentCharacterAnimation != animationName) {
        characterAnimator->playAnimation(animationName);
        currentCharacterAnimation = animationName;
    }
}

void LibraryScreen::updateCharacterAnimationForState() {
    std::string animationName = getStateAnimationName();
    playCharacterAnimation(animationName);
}

std::string LibraryScreen::getStateAnimationName() const {
    if (noteEditorData.isEditing && noteEditorData.hasUnsavedChanges) {
        return "writing";
    } else if (selectedNoteId != 0) {
        return "reading";
    } else {
        return "idle";
    }
}

// Input handling helpers
void LibraryScreen::handleKeyboardInput(const InputState& input) {
    // Handle shortcuts
    if (input.controlPressed) {
        if (input.isKeyJustPressed(KEY_N)) {
            createNewNote();
        } else if (input.isKeyJustPressed(KEY_S)) {
            saveCurrentNote();
        } else if (input.isKeyJustPressed(KEY_F)) {
            // Focus search bar (simplified)
            activeTextAreaIndex = -1;
        }
    }
    
    // Handle view mode shortcuts
    if (input.isKeyJustPressed(KEY_ONE)) {
        setViewMode("split");
    } else if (input.isKeyJustPressed(KEY_TWO)) {
        setViewMode("list");
    } else if (input.isKeyJustPressed(KEY_THREE)) {
        setViewMode("editor");
    }
    
    // Handle navigation
    if (input.isKeyJustPressed(KEY_UP) && !filteredNotes.empty()) {
        // Select previous note
        auto it = std::find_if(filteredNotes.begin(), filteredNotes.end(),
            [this](const Note& note) { return note.getId() == selectedNoteId; });
        
        if (it != filteredNotes.begin()) {
            --it;
            selectNote(it->getId());
        }
    } else if (input.isKeyJustPressed(KEY_DOWN) && !filteredNotes.empty()) {
        // Select next note
        auto it = std::find_if(filteredNotes.begin(), filteredNotes.end(),
            [this](const Note& note) { return note.getId() == selectedNoteId; });
        
        if (it != filteredNotes.end() && ++it != filteredNotes.end()) {
            selectNote(it->getId());
        }
    }
    
    // Handle exit
    if (input.isKeyJustPressed(KEY_ESCAPE)) {
        if (noteEditorData.hasUnsavedChanges) {
            // Ask to save changes (simplified - just save)
            saveCurrentNote();
        }
        
        if (onExitRequested) {
            onExitRequested();
        }
    }
}

void LibraryScreen::handleMouseInput(const InputState& input) {
    // Handle note list clicks
    if (input.mouse.leftButton) {
        handleNoteListClick(input.mouse.x, input.mouse.y);
    }
}

void LibraryScreen::handleTextInput(const InputState& input) {
    if (activeTextAreaIndex < 0 || activeTextAreaIndex >= (int)textAreas.size()) {
        return;
    }
    
    TextArea& area = textAreas[activeTextAreaIndex];
    if (!area.valuePtr) return;
    
    // Handle character input
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125 && area.valuePtr->length() < area.maxLength) {
            *area.valuePtr += (char)key;
            noteEditorData.hasUnsavedChanges = true;
        }
        key = GetCharPressed();
    }
    
    // Handle backspace
    if (input.isKeyJustPressed(KEY_BACKSPACE) && !area.valuePtr->empty()) {
        area.valuePtr->pop_back();
        noteEditorData.hasUnsavedChanges = true;
    }
    
    // Handle enter for multiline
    if (area.isMultiline && input.isKeyJustPressed(KEY_ENTER)) {
        *area.valuePtr += '\n';
        noteEditorData.hasUnsavedChanges = true;
    }
    
    // Handle tab to switch between areas
    if (input.isKeyJustPressed(KEY_TAB)) {
        activeTextAreaIndex = (activeTextAreaIndex + 1) % (int)textAreas.size();
    }
}

void LibraryScreen::handleNoteListClick(float x, float y) {
    for (const NoteListItem& item : noteListItems) {
        if (isPointInNoteListItem(item, x, y)) {
            selectNote(item.noteId);
            break;
        }
    }
}

void LibraryScreen::handleScrolling(const InputState& input) {
    // Handle mouse wheel scrolling for note list
    if (input.mouse.scrollY != 0) {
        noteListScrollOffset -= input.mouse.scrollY * SCROLL_SPEED;
        
        // Clamp scroll offset
        float maxScroll = std::max(0.0f, noteListItems.size() * (NOTE_LIST_ITEM_HEIGHT + NOTE_LIST_PADDING) - 400);
        noteListScrollOffset = std::max(0.0f, std::min(noteListScrollOffset, maxScroll));
    }
}

// Rendering helpers
void LibraryScreen::renderBook(float x, float y, const Note& note) {
    // Draw a simple book representation
    Color bookColor = getCategoryColor(note.getCategory());
    DrawRectangle((int)x, (int)y, 20, 30, bookColor);
    DrawRectangleLines((int)x, (int)y, 20, 30, BLACK);
    
    // Book spine
    DrawLine((int)(x + 3), (int)y, (int)(x + 3), (int)(y + 30), BLACK);
}

void LibraryScreen::renderTag(const std::string& tag, float x, float y, bool isSelected) {
    Color bgColor = isSelected ? BLUE : LIGHTGRAY;
    Color textColor = isSelected ? WHITE : BLACK;
    
    int textWidth = MeasureText(tag.c_str(), 10);
    DrawRectangle((int)x, (int)y, textWidth + 10, 15, bgColor);
    DrawRectangleLines((int)x, (int)y, textWidth + 10, 15, BLACK);
    DrawText(tag.c_str(), (int)(x + 5), (int)(y + 2), 10, textColor);
}

void LibraryScreen::renderCategory(const std::string& category, float x, float y, bool isSelected) {
    Color bgColor = isSelected ? getCategoryColor(category) : Fade(getCategoryColor(category), 0.5f);
    Color textColor = isSelected ? WHITE : BLACK;
    
    int textWidth = MeasureText(category.c_str(), 10);
    DrawRectangle((int)x, (int)y, textWidth + 10, 15, bgColor);
    DrawRectangleLines((int)x, (int)y, textWidth + 10, 15, BLACK);
    DrawText(category.c_str(), (int)(x + 5), (int)(y + 2), 10, textColor);
}

Color LibraryScreen::getCategoryColor(const std::string& category) const {
    if (category.empty()) return GRAY;
    
    // Simple hash-based color assignment
    uint32_t hash = 0;
    for (char c : category) {
        hash = hash * 31 + c;
    }
    
    Color colors[] = {RED, GREEN, BLUE, ORANGE, PURPLE, PINK, YELLOW, BROWN};
    return colors[hash % 8];
}

std::string LibraryScreen::formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%m/%d %H:%M");
    return oss.str();
}

std::string LibraryScreen::truncateText(const std::string& text, int maxLength) const {
    if (text.length() <= maxLength) {
        return text;
    }
    return text.substr(0, maxLength - 3) + "...";
}

// Layout helpers (simplified implementations)
void LibraryScreen::calculateSplitViewLayout() {
    createTextAreas(); // Recreate with correct positions
}

void LibraryScreen::calculateListViewLayout() {
    createTextAreas(); // Recreate with correct positions
}

void LibraryScreen::calculateEditorViewLayout() {
    createTextAreas(); // Recreate with correct positions
}