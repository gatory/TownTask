# include <iostream>
# include <raylib.h>
# include <vector>

void drawBackground(Rectangle rec);

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    // Initialize window
    InitWindow(screenWidth, screenHeight, "TownTasker");

    SetTargetFPS(60);
    
    Rectangle rec = {0, 0, 100, 100};


    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_D)) {
            rec.x += 2.0f;
        }
        if (IsKeyDown(KEY_A)) {
            rec.x -= 2.0f;
        }
        if (IsKeyDown(KEY_D)) {
            rec.x += 2.0f;
        }
        if (IsKeyDown(KEY_D)) {
            rec.x += 2.0f;
        }
        BeginDrawing();

        drawBackground(rec);

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}

void drawBackground(Rectangle rec) {
    ClearBackground(RAYWHITE);
    DrawRectangleRec(rec, RED);
}