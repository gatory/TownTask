#include "Systems/LibraryUISystem.h"
#include "raygui.h"
#include "Core/Constants.h"
#include "Data/DataManager.h"

void LibraryUISystem::Render(World& world, int currentScene) {
    if (currentScene != SceneID::LIBRARY_INTERIOR) return;

    // Find library data
    for (Entity e : world.GetEntities()) {
        LibraryData* library = world.GetLibraryData(e);
        if (!library) continue;

        if (library->isShowingUI) {
            RenderLibraryUI(*library, GameConfig::GUI_HEIGHT);
        }
    }
}

void LibraryUISystem::RenderLibraryUI(LibraryData& library, int guiHeight) {
    // Center the overlay and make it 80% of the screen size
    int width = (int)(GameConfig::SCREEN_WIDTH * 0.8f);
    int height = (int)(GameConfig::SCREEN_HEIGHT * 0.8f);
    int centerX = GameConfig::SCREEN_WIDTH / 2;
    int startY = (GameConfig::SCREEN_HEIGHT - height) / 2;
    
    // Background panel
    DrawRectangle(centerX - width/2, startY, width, height, Fade(LIGHTGRAY, 0.95f));
    DrawRectangleLines(centerX - width/2, startY, width, height, DARKGRAY);
    
    // Title
    DrawText("My eBook Library", centerX - 100, startY + 10, 24, BLACK);
    
    // Close button
    if (GuiButton(Rectangle{(float)(centerX + width/2 - 80), (float)(startY + 10), 70, 30}, "Close")) {
        library.isShowingUI = false;
    }
    
    // Search bar
    startY += 50;
    DrawText("Search:", centerX - width/2 + 20, startY + 5, 16, BLACK);
    GuiTextBox(Rectangle{(float)(centerX - width/2 + 90), (float)startY, 300, 30}, 
               library.searchBuffer, 128, true);
    
    // Add book button
    if (GuiButton(Rectangle{(float)(centerX + width/2 - 150), (float)startY, 130, 30}, "Add Book")) {
        // In a real implementation, open file dialog
        library.ebookPaths.push_back("path/to/new/book.pdf");
        library.ebookTitles.push_back("New Book");
        DataManager::SaveLibraryData("data/library.json", library.ebookPaths, library.ebookTitles);
    }
    
    // Book list
    startY += 50;
    int listHeight = height - 120;
    int itemHeight = 60;
    int visibleItems = listHeight / itemHeight;
    
    DrawText("Books:", centerX - width/2 + 20, startY, 18, BLACK);
    startY += 30;
    
    // Scroll through books
    int start = library.scrollOffset;
    int end = std::min(start + visibleItems, (int)library.ebookTitles.size());
    
    for (int i = start; i < end; i++) {
        int itemY = startY + (i - start) * itemHeight;
        
        // Item background
        Color bgColor = (i == library.selectedIndex) ? Fade(BLUE, 0.3f) : Fade(WHITE, 0.5f);
        DrawRectangle(centerX - width/2 + 20, itemY, width - 40, itemHeight - 5, bgColor);
        DrawRectangleLines(centerX - width/2 + 20, itemY, width - 40, itemHeight - 5, DARKGRAY);
        
        // Title
        DrawText(library.ebookTitles[i].c_str(), centerX - width/2 + 30, itemY + 10, 16, BLACK);
        
        // Path (truncated)
        std::string path = library.ebookPaths[i];
        if (path.length() > 60) path = path.substr(0, 57) + "...";
        DrawText(path.c_str(), centerX - width/2 + 30, itemY + 30, 12, DARKGRAY);
        
        // Open button
        if (GuiButton(Rectangle{(float)(centerX + width/2 - 120), (float)(itemY + 15), 90, 30}, "Open")) {
            // Open PDF viewer (system default)
            #ifdef _WIN32
            system(("start " + library.ebookPaths[i]).c_str());
            #elif __APPLE__
            system(("open " + library.ebookPaths[i]).c_str());
            #else
            system(("xdg-open " + library.ebookPaths[i]).c_str());
            #endif
        }
    }
    
    // Scroll indicators
    if (library.ebookTitles.size() > visibleItems) {
        DrawText(TextFormat("%d / %d", start + 1, (int)library.ebookTitles.size()), 
                centerX - 30, startY + listHeight + 10, 14, DARKGRAY);
        
        // Scroll buttons
        if (GuiButton(Rectangle{(float)(centerX - 100), (float)(startY + listHeight + 5), 50, 25}, "Up")) {
            if (library.scrollOffset > 0) library.scrollOffset--;
        }
        if (GuiButton(Rectangle{(float)(centerX + 50), (float)(startY + listHeight + 5), 50, 25}, "Down")) {
            if (library.scrollOffset < (int)library.ebookTitles.size() - visibleItems) {
                library.scrollOffset++;
            }
        }
    }
}

void LibraryUISystem::HandleInput(World& world) {
    // Mouse wheel scrolling input
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        for (Entity e : world.GetEntities()) {
            LibraryData* library = world.GetLibraryData(e);
            if (!library || !library->isShowingUI) continue;

            library->scrollOffset -= (int)(wheel);
            if (library->scrollOffset <0) library->scrollOffset = 0;

            int maxScroll = std::max(0, (int)library->ebookTitles.size() - 4);
            if (library->scrollOffset > maxScroll) library->scrollOffset = maxScroll;
        }
    }
}