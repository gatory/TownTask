#pragma once

constexpr int UNIT = 32;

namespace GameConfig {
    // Screen dimensions
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 800;
    constexpr int GUI_HEIGHT = 400;
    constexpr int GAME_HEIGHT = 400;

    // Player settings
    constexpr float PLAYER_SPEED = 200.0f;
    constexpr int PLAYER_SIZE = 64;
    constexpr int PLAYER_SPRITE_FRAME_WIDTH = 32;
    constexpr int PLAYER_SPRITE_FRAME_HEIGHT = 32;
    constexpr float SPEECH_BUBBLE_OFFSET_Y = -32.0f;
    
    // AI behavior
    constexpr float AI_WANDER_SPEED = 100.0f;
    constexpr float AI_MIN_MOVE_TIME = 0.5f;
    constexpr float AI_MAX_MOVE_TIME = 2.0f;
    
    // Animation
    constexpr int PLAYER_ANIMATION_FRAMES = 10;
    constexpr float PLAYER_ANIMATION_FRAME_TIME = 0.2f;
    
    // Speech bubble
    constexpr int SPEECH_BUBBLE_SCALE = 2;
    constexpr int SPEECH_BUBBLE_TEXT_OFFSET_X = 8;
    constexpr int SPEECH_BUBBLE_TEXT_OFFSET_Y = 8;
    constexpr int SPEECH_BUBBLE_TEXT_LINE_HEIGHT = 12;
    constexpr int SPEECH_BUBBLE_FONT_SIZE = 8;
}

namespace BuildingConfig {
    // Building dimensions
    constexpr int POMODORO_WIDTH = 100;
    constexpr int POMODORO_HEIGHT = 120;
    constexpr int LIBRARY_WIDTH = 140;
    constexpr int LIBRARY_HEIGHT = 160;
    constexpr int HOUSE_WIDTH = 120;
    constexpr int HOUSE_HEIGHT = 140;
    
    // Door dimensions
    constexpr int DOOR_WIDTH = 30;
    constexpr int DOOR_HEIGHT = 40;
    
    // Interior object sizes
    constexpr int NPC_SIZE = 48;
    constexpr int DESK_WIDTH = 80;
    constexpr int DESK_HEIGHT = 60;
}

namespace SceneID {
    constexpr int MAIN = 0;
    constexpr int POMODORO_INTERIOR = 1;
    constexpr int LIBRARY_INTERIOR = 2;
    constexpr int HOUSE_INTERIOR = 3;
}

namespace AssetPath {
    constexpr const char* PLAYER_IDLE = "assets/Idle.png";
    constexpr const char* SPEECH_BUBBLE = "assets/speech_v2_32x24.png";
    constexpr const char* MAIN_BACKGROUND = "assets/map.png";
    constexpr const char* SHOP_BACKGROUND = "assets/shop-800x449.png";
    constexpr const char* LIBRARY_BACKGROUND = "assets/library-bg.png";
    constexpr const char* HOUSE_BACKGROUND = "assets/house-bg.png";
    // Pixel art building sprites
    constexpr const char* HOUSE_1 = "assets/house1.png";
    constexpr const char* HOUSE_2 = "assets/house2.png";
    constexpr const char* HOUSE_3 = "assets/house3.png";
    constexpr const char* HOUSE_4 = "assets/house4.png";
    constexpr const char* LIBRARIAN_SPRITE = "assets/librarian-idle.png";
    constexpr const char* DESK_SPRITE = "assets/desk.png";
    constexpr const char* DOOR_SPRITE = "assets/door.png";
    // Sound assets
    constexpr const char* SFX_OPEN_DOOR = "assets/opening-door-411632.mp3";
    constexpr const char* SFX_CLOSE_DOOR = "assets/close-door-382723.mp3";
    constexpr const char* SFX_COFFEEAMBIENCE = "assets/white-noise-358382.mp3";
    constexpr const char* SFX_LEVEL_COMPLETE = "assets/game-level-complete-143022.mp3";
    constexpr const char* MAIN_MUSIC = "assets/background.mp3";

}