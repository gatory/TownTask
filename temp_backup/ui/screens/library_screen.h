#pragma once

#include "screen.h"
#include "../../core/engines/note_engine.h"
#include "../../core/engines/gamification_engine.h"
#include "../../input/input_manager.h"
#include "../../ui/animations/animation_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>

class LibraryScreen : public Screen {
public:
    LibraryScreen(NoteEngine& noteEngine, GamificationEngine& gamificationEngine,
                  InputManager& inputManager, AnimationManager& animationManager);
    ~LibraryScreen() override;
    
    // Screen interface
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    void onEnter() override;
    void onExit() override;
    
    // Note management functionality
    void createNote(const std::string& title, const std::string& content = "", const std::vector<std::string>& tags = {});
    void editNote(uint32_t noteId);
    void deleteNote(uint32_t noteId);
    void duplicateNote(uint32_t noteId);
    
    // Note organization
    void addTagToNote(uint32_t noteId, const std::string& tag);
    void removeTagFromNote(uint32_t noteId, const std::string& tag);
    void setNoteCategory(uint32_t noteId, const std::string& category);
    
    // Search and filtering
    void setSearchQuery(const std::string& query);
    std::string getSearchQuery() const;
    void setTagFilter(const std::string& tag);
    std::string getTagFilter() const;
    void setCategoryFilter(const std::string& category);
    std::string getCategoryFilter() const;
    void setSortBy(const std::string& sortBy); // "title", "created", "modified", "category"
    std::string getSortBy() const;
    
    // UI state management
    void setShowNoteEditor(bool show);
    bool isShowingNoteEditor() const;
    void setShowNoteList(bool show);
    bool isShowingNoteList() const;
    void setSelectedNoteId(uint32_t noteId);
    uint32_t getSelectedNoteId() const;
    void setViewMode(const std::string& mode); // "split", "list", "editor"
    std::string getViewMode() const;
    
    // Visual settings
    void setShowTags(bool show);
    bool isShowingTags() const;
    void setShowCategories(bool show);
    bool isShowingCategories() const;
    void setShowTimestamps(bool show);
    bool isShowingTimestamps() const;
    void setFontSize(int size);
    int getFontSize() const;
    
    // Event callbacks
    void setOnNoteCreated(std::function<void(uint32_t)> callback);
    void setOnNoteModified(std::function<void(uint32_t)> callback);
    void setOnNoteDeleted(std::function<void(uint32_t)> callback);
    void setOnExitRequested(std::function<void()> callback);

private:
    // Core references
    NoteEngine& noteEngine;
    GamificationEngine& gamificationEngine;
    InputManager& inputManager;
    AnimationManager& animationManager;
    
    // UI state
    bool showNoteEditor;
    bool showNoteList;
    uint32_t selectedNoteId;
    std::string viewMode;
    std::string searchQuery;
    std::string tagFilter;
    std::string categoryFilter;
    std::string sortBy;
    
    // Visual settings
    bool showTags;
    bool showCategories;
    bool showTimestamps;
    int fontSize;
    
    // Note editing state
    struct NoteEditorData {
        std::string title;
        std::string content;
        std::vector<std::string> tags;
        std::string category;
        bool isEditing;
        uint32_t editingNoteId;
        bool hasUnsavedChanges;
        
        NoteEditorData() : isEditing(false), editingNoteId(0), hasUnsavedChanges(false) {}
    } noteEditorData;
    
    // Animation state
    std::shared_ptr<SpriteAnimator> characterAnimator;
    std::shared_ptr<SpriteAnimator> bookAnimator;
    std::string currentCharacterAnimation;
    
    // Note display state
    struct NoteListItem {
        uint32_t noteId;
        float y;
        float height;
        bool isSelected;
        bool isHovered;
        
        NoteListItem(uint32_t id, float y) 
            : noteId(id), y(y), height(60), isSelected(false), isHovered(false) {}
    };
    
    std::vector<NoteListItem> noteListItems;
    std::vector<Note> filteredNotes;
    float noteListScrollOffset;
    
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
    
    // Text input areas
    struct TextArea {
        float x, y, width, height;
        std::string label;
        std::string* valuePtr;
        bool isActive;
        bool isMultiline;
        int maxLength;
        float scrollOffset;
        int cursorPosition;
        
        TextArea(float x, float y, float w, float h, const std::string& label, std::string* valuePtr)
            : x(x), y(y), width(w), height(h), label(label), valuePtr(valuePtr),
              isActive(false), isMultiline(false), maxLength(1000), 
              scrollOffset(0), cursorPosition(0) {}
    };
    
    std::vector<TextArea> textAreas;
    int activeTextAreaIndex;
    
    // Tag management
    std::vector<std::string> availableTags;
    std::vector<std::string> availableCategories;
    
    // Event callbacks
    std::function<void(uint32_t)> onNoteCreated;
    std::function<void(uint32_t)> onNoteModified;
    std::function<void(uint32_t)> onNoteDeleted;
    std::function<void()> onExitRequested;
    
    // Internal update methods
    void updateNotes(float deltaTime);
    void updateAnimations(float deltaTime);
    void updateUI(float deltaTime);
    void updateNoteList();
    void updateButtons(const InputState& input);
    void updateTextAreas(const InputState& input);
    
    // Rendering methods
    void renderBackground();
    void renderLibraryInterior();
    void renderCharacter();
    void renderNoteList();
    void renderNoteEditor();
    void renderSplitView();
    void renderSearchBar();
    void renderTagsAndCategories();
    void renderButtons();
    void renderTextAreas();
    void renderNoteListItem(const NoteListItem& item, const Note& note);
    
    // Note management helpers
    void refreshFilteredNotes();
    void sortNotes();
    bool passesFilter(const Note& note) const;
    void layoutNoteList();
    void selectNote(uint32_t noteId);
    void loadNoteIntoEditor(const Note& note);
    void saveCurrentNote();
    void createNewNote();
    void discardChanges();
    
    // Search and filter helpers
    bool matchesSearchQuery(const Note& note) const;
    void updateAvailableTagsAndCategories();
    std::vector<std::string> parseTagsFromString(const std::string& tagString) const;
    std::string tagsToString(const std::vector<std::string>& tags) const;
    
    // UI helpers
    void createButtons();
    void createTextAreas();
    void handleButtonClick(int buttonIndex);
    bool isPointInButton(const Button& button, float x, float y) const;
    bool isPointInTextArea(const TextArea& area, float x, float y) const;
    bool isPointInNoteListItem(const NoteListItem& item, float x, float y) const;
    
    // Animation helpers
    void playCharacterAnimation(const std::string& animationName);
    void updateCharacterAnimationForState();
    std::string getStateAnimationName() const;
    
    // Input handling helpers
    void handleKeyboardInput(const InputState& input);
    void handleMouseInput(const InputState& input);
    void handleTextInput(const InputState& input);
    void handleNoteListClick(float x, float y);
    void handleScrolling(const InputState& input);
    
    // Rendering helpers
    void renderBook(float x, float y, const Note& note);
    void renderTag(const std::string& tag, float x, float y, bool isSelected = false);
    void renderCategory(const std::string& category, float x, float y, bool isSelected = false);
    Color getCategoryColor(const std::string& category) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const;
    std::string truncateText(const std::string& text, int maxLength) const;
    
    // Layout helpers
    void calculateSplitViewLayout();
    void calculateListViewLayout();
    void calculateEditorViewLayout();
    
    // Constants
    static constexpr float NOTE_LIST_ITEM_HEIGHT = 60.0f;
    static constexpr float NOTE_LIST_PADDING = 10.0f;
    static constexpr float BUTTON_WIDTH = 80.0f;
    static constexpr float BUTTON_HEIGHT = 30.0f;
    static constexpr float SEARCH_BAR_HEIGHT = 30.0f;
    static constexpr int DEFAULT_FONT_SIZE = 12;
    static constexpr int MAX_TITLE_LENGTH = 100;
    static constexpr int MAX_CONTENT_LENGTH = 10000;
    static constexpr float SCROLL_SPEED = 20.0f;
};