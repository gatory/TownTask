#include "Systems/GUISystem.h"
#include "raygui.h"
#include "Core/Constants.h"
#include <cstdio>

void GUISystem::Render(World& world, int currentScene, PomodoroTimer& pomodoroTimer, int guiHeight) {
    for (Entity e : world.GetEntities()) {
        Scene* scene = world.GetScene(e);
        if (!scene || scene->sceneId != currentScene) continue;
        
        BuildingInterior* interior = world.GetBuildingInterior(e);
        if (!interior) continue;
        
        if (interior->guiType == GUIType::Pomodoro) {
            RenderPomodoroGUI(world, pomodoroTimer, guiHeight);
        }
        break;
    }
}

void GUISystem::RenderPomodoroGUI(World& world, PomodoroTimer& timer, int guiHeight) {
    int centerX = GameConfig::SCREEN_WIDTH / 2;
    int startY = 50;
    
    DrawText("Pomodoro Timer", centerX - 100, startY, 30, BLACK);
    
    startY += 60;
    DrawText("Cycles:", centerX - 150, startY + 8, 20, BLACK);
    
    if (GuiButton({(float)(centerX - 50), (float)startY, 30, 30}, "-")) {
        if (timer.GetCycleCount() > 1 && !timer.IsRunning()) {
            timer.SetCycleCount(timer.GetCycleCount() - 1);
        }
    }
    
    char cycleText[16];
    sprintf(cycleText, "%d", timer.GetCycleCount());
    DrawText(cycleText, centerX - 10, startY + 5, 20, BLACK);
    
    if (GuiButton({(float)(centerX + 40), (float)startY, 30, 30}, "+")) {
        if (timer.GetCycleCount() < PomodoroTimer::MAX_CYCLES && !timer.IsRunning()) {
            timer.SetCycleCount(timer.GetCycleCount() + 1);
        }
    }
    
    startY += 60;
    if (!timer.IsRunning()) {
        if (GuiButton({(float)(centerX - 75), (float)startY, 150, 40}, "Start")) {
            timer.Start();
        }
    }
    
    startY += 60;
    if (GuiButton({(float)(centerX - 75), (float)startY, 150, 40}, "Reset")) {
        timer.Reset();
    }

    // Breakout button: allow user to cancel the pomodoro and free the player to move
    startY += 60;
    if (GuiButton({(float)(centerX - 75), (float)startY, 150, 40}, "Break")) {
        // Unfreeze player(s) and reset timer
        for (Entity e : world.GetEntities()) {
            if (!world.HasPlayer(e)) continue;
            PlayerInput* pi = world.GetPlayerInput(e);
            if (pi) pi->frozen = false;
        }
        timer.Reset();
    }
    
    startY += 80;
    if (timer.IsRunning() || timer.GetState() != PomodoroState::Idle) {
        const char* stateText = "Idle";
        Color stateColor = GRAY;
        if (timer.GetState() == PomodoroState::Study) {
            stateText = "STUDY TIME";
            stateColor = DARKGREEN;
        } else if (timer.GetState() == PomodoroState::Break) {
            stateText = "BREAK TIME";
            stateColor = DARKBLUE;
        }
        
    DrawText(stateText, centerX - 80, startY, 24, stateColor);
        
        startY += 40;
        char cycleProgress[64];
        sprintf(cycleProgress, "Cycle %d / %d", timer.GetCurrentCycle(), timer.GetCycleCount());
    DrawText(cycleProgress, centerX - 60, startY, 18, BLACK);
        
        startY += 40;
        int totalElapsed = timer.GetTotalElapsed();
        int hours = totalElapsed / 3600;
        int minutes = (totalElapsed % 3600) / 60;
        int seconds = totalElapsed % 60;
        
        char timeText[32];
        sprintf(timeText, "%02d:%02d:%02d", hours, minutes, seconds);
        DrawText("Elapsed:", centerX - 100, startY, 20, BLACK);
        DrawText(timeText, centerX - 10, startY, 20, DARKGRAY);
    }
}