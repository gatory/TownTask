#pragma once
#include "Core/World.h"

class TodoUISystem {
public:
    void Render(World& world, int currentScene);
    void HandleInput(World& world);
    
private:
    void RenderTodoUI(TodoListData& todo, int guiHeight);
};
