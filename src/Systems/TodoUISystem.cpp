#include "Systems/TodoUISystem.h"
#include "raygui.h"
#include "Core/Constants.h"
#include "Data/DataManager.h" 

void TodoUISystem::Render(World& world, int currentScene) {
    if (currentScene != SceneID::HOUSE_INTERIOR) return;
    
    for (Entity e : world.GetEntities()) {
        TodoListData* todo = world.GetTodoListData(e);
        if (!todo) continue;
        
        if (todo->isShowingUI) {
            RenderTodoUI(*todo, GameConfig::GUI_HEIGHT);
        }
        break;
    }
}

void TodoUISystem::RenderTodoUI(TodoListData& todo, int guiHeight) {
    // Center the overlay and make it 80% of the screen size
    int width = (int)(GameConfig::SCREEN_WIDTH * 0.8f);
    int height = (int)(GameConfig::SCREEN_HEIGHT * 0.8f);
    int centerX = GameConfig::SCREEN_WIDTH / 2;
    int startY = (GameConfig::SCREEN_HEIGHT - height) / 2;
    
    // Background panel
    DrawRectangle(centerX - width/2, startY, width, height, Fade(BEIGE, 0.95f));
    DrawRectangleLines(centerX - width/2, startY, width, height, DARKBROWN);
    
    // Title
    DrawText("My To-Do List", centerX - 90, startY + 10, 24, DARKBROWN);
    
    // Close button
    if (GuiButton(Rectangle{(float)(centerX + width/2 - 80), (float)(startY + 10), 70, 30}, "Close")) {
        todo.isShowingUI = false;
        todo.isAddingNew = false;
    }
    
    // Add task section
    startY += 50;
    if (!todo.isAddingNew) {
        if (GuiButton(Rectangle{(float)(centerX - width/2 + 20), (float)startY, 150, 35}, "+ Add Task")) {
            todo.isAddingNew = true;
            memset(todo.inputBuffer, 0, sizeof(todo.inputBuffer));
        }
    } else {
        DrawText("New Task:", centerX - width/2 + 20, startY + 8, 16, DARKBROWN);
        GuiTextBox(Rectangle{(float)(centerX - width/2 + 120), (float)startY, 300, 35}, 
                   todo.inputBuffer, 256, true);
        
        if (GuiButton(Rectangle{(float)(centerX - width/2 + 430), (float)startY, 70, 35}, "Add")) {
            if (strlen(todo.inputBuffer) > 0) {
                todo.tasks.push_back(std::string(todo.inputBuffer));
                todo.completed.push_back(false);
                DataManager::SaveTodoList("data/todolist.json", todo.tasks, todo.completed);
                todo.isAddingNew = false;
            }
        }
        
        if (GuiButton(Rectangle{(float)(centerX - width/2 + 510), (float)startY, 70, 35}, "Cancel")) {
            todo.isAddingNew = false;
        }
    }
    
    // Task list
    startY += 50;
    int listHeight = height - 120;
    int itemHeight = 50;
    int visibleItems = listHeight / itemHeight;
    
    DrawText(TextFormat("Tasks (%d total):", (int)todo.tasks.size()), 
             centerX - width/2 + 20, startY, 18, DARKBROWN);
    startY += 30;
    
    int start = todo.scrollOffset;
    int end = std::min(start + visibleItems, (int)todo.tasks.size());
    
    for (int i = start; i < end; i++) {
        int itemY = startY + (i - start) * itemHeight;
        
        // Item background
        Color bgColor = todo.completed[i] ? Fade(GREEN, 0.2f) : Fade(WHITE, 0.5f);
        DrawRectangle(centerX - width/2 + 20, itemY, width - 40, itemHeight - 5, bgColor);
        DrawRectangleLines(centerX - width/2 + 20, itemY, width - 40, itemHeight - 5, DARKBROWN);
        
        // Checkbox
        bool checked = todo.completed[i];
        if (GuiCheckBox(Rectangle{(float)(centerX - width/2 + 30), (float)(itemY + 12), 25, 25}, 
                       "", &checked)) {
            todo.completed[i] = checked;
            DataManager::SaveTodoList("data/todolist.json", todo.tasks, todo.completed);
        }
        
        // Task text (strikethrough if completed)
        const char* taskText = todo.tasks[i].c_str();
        DrawText(taskText, centerX - width/2 + 65, itemY + 15, 16, 
                todo.completed[i] ? GRAY : BLACK);
        
        if (todo.completed[i]) {
            int textWidth = MeasureText(taskText, 16);
            DrawLine(centerX - width/2 + 65, itemY + 23, 
                    centerX - width/2 + 65 + textWidth, itemY + 23, GRAY);
        }
        
        // Delete button
        if (GuiButton(Rectangle{(float)(centerX + width/2 - 100), (float)(itemY + 10), 70, 30}, "Delete")) {
            todo.tasks.erase(todo.tasks.begin() + i);
            todo.completed.erase(todo.completed.begin() + i);
            DataManager::SaveTodoList("data/todolist.json", todo.tasks, todo.completed);
            if (todo.scrollOffset > 0 && i == (int)todo.tasks.size()) {
                todo.scrollOffset--;
            }
        }
    }
    
    // Scroll indicators
    if (todo.tasks.size() > visibleItems) {
        DrawText(TextFormat("%d / %d", start + 1, (int)todo.tasks.size()), 
                centerX - 30, startY + listHeight + 10, 14, DARKBROWN);
        
        if (GuiButton(Rectangle{(float)(centerX - 100), (float)(startY + listHeight + 5), 50, 25}, "Up")) {
            if (todo.scrollOffset > 0) todo.scrollOffset--;
        }
        if (GuiButton(Rectangle{(float)(centerX + 50), (float)(startY + listHeight + 5), 50, 25}, "Down")) {
            if (todo.scrollOffset < (int)todo.tasks.size() - visibleItems) {
                todo.scrollOffset++;
            }
        }
    }
}

void TodoUISystem::HandleInput(World& world) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        for (Entity e : world.GetEntities()) {
            TodoListData* todo = world.GetTodoListData(e);
            if (!todo || !todo->isShowingUI) continue;
            
            todo->scrollOffset -= (int)wheel;
            if (todo->scrollOffset < 0) todo->scrollOffset = 0;
            
            int maxScroll = std::max(0, (int)todo->tasks.size() - 5);
            if (todo->scrollOffset > maxScroll) todo->scrollOffset = maxScroll;
        }
    }
}
