#pragma once

#include <raylib.h>
#include <string>
#include <functional>

enum class WindowMode {
    WINDOWED,           // Normal windowed mode
    DESKTOP_OVERLAY     // Transparent overlay mode
};

struct WindowSettings {
    // Window properties
    int width = 1024;
    int height = 768;
    std::string title = "TaskTown";
    bool resizable = true;
    bool vsync = true;
    int targetFPS = 60;
    
    // Desktop overlay properties
    float overlayAlpha = 0.8f;          // Transparency level in overlay mode
    bool clickThrough = true;           // Allow clicks to pass through empty areas
    bool alwaysOnTop = true;            // Keep window on top in overlay mode
    bool showInTaskbar = true;          // Show in taskbar (windowed mode only)
    
    // Screen boundary settings
    int screenPadding = 10;             // Pixels from screen edge
    bool clampToScreen = true;          // Keep character within screen bounds
    
    // State persistence
    bool saveWindowState = true;        // Save window position and mode
    std::string stateFilePath = "window_state.json";
};

struct WindowState {
    WindowMode mode = WindowMode::WINDOWED;
    int x = 100;
    int y = 100;
    int width = 1024;
    int height = 768;
    bool isMaximized = false;
    bool isMinimized = false;
    
    // Desktop overlay specific state
    float overlayAlpha = 0.8f;
    bool clickThroughEnabled = true;
    
    // Screen information
    int screenWidth = 0;
    int screenHeight = 0;
    int monitorIndex = 0;
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    
    // Initialization and cleanup
    bool initialize(const WindowSettings& settings);
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    // Window mode management
    bool setWindowMode(WindowMode mode);
    WindowMode getCurrentMode() const { return currentState.mode; }
    bool isDesktopOverlayMode() const { return currentState.mode == WindowMode::DESKTOP_OVERLAY; }
    
    // Window properties
    void setWindowSize(int width, int height);
    void setWindowPosition(int x, int y);
    void setWindowTitle(const std::string& title);
    Vector2 getWindowSize() const;
    Vector2 getWindowPosition() const;
    
    // Desktop overlay specific
    void setOverlayAlpha(float alpha);
    float getOverlayAlpha() const { return currentState.overlayAlpha; }
    void setClickThrough(bool enabled);
    bool isClickThroughEnabled() const { return currentState.clickThroughEnabled; }
    void setAlwaysOnTop(bool enabled);
    
    // Screen boundary management
    Vector2 clampPositionToScreen(const Vector2& position, const Vector2& size) const;
    bool isPositionOnScreen(const Vector2& position, const Vector2& size) const;
    Vector2 getScreenSize() const;
    Vector2 getScreenCenter() const;
    int getCurrentMonitor() const;
    
    // Window state management
    void saveWindowState() const;
    bool loadWindowState();
    void resetToDefaults();
    const WindowState& getWindowState() const { return currentState; }
    
    // Event handling
    void update();
    bool shouldClose() const;
    void handleWindowResize();
    void handleWindowMove();
    
    // Rendering support
    void beginDrawing();
    void endDrawing();
    void clearBackground(Color color = BLANK);
    
    // Desktop overlay rendering helpers
    void beginOverlayRendering();
    void endOverlayRendering();
    void renderOverlayBackground();
    
    // Utility methods
    bool isWindowFocused() const;
    bool isWindowMinimized() const;
    bool isWindowMaximized() const;
    void minimizeWindow();
    void maximizeWindow();
    void restoreWindow();
    
    // Event callbacks
    void setOnModeChanged(std::function<void(WindowMode, WindowMode)> callback);
    void setOnWindowResized(std::function<void(int, int)> callback);
    void setOnWindowMoved(std::function<void(int, int)> callback);
    void setOnWindowFocusChanged(std::function<void(bool)> callback);
    
    // Platform-specific functionality
    void enableTransparency();
    void disableTransparency();
    void setWindowFlags(unsigned int flags);
    void clearWindowFlags(unsigned int flags);
    
private:
    // Core state
    bool initialized = false;
    WindowSettings settings;
    WindowState currentState;
    WindowState previousState;
    
    // Platform-specific handles (would be implemented per platform)
    void* platformWindowHandle = nullptr;
    
    // Event callbacks
    std::function<void(WindowMode, WindowMode)> onModeChanged;
    std::function<void(int, int)> onWindowResized;
    std::function<void(int, int)> onWindowMoved;
    std::function<void(bool)> onWindowFocusChanged;
    
    // Internal state tracking
    bool windowStateChanged = false;
    float lastSaveTime = 0.0f;
    static constexpr float SAVE_DELAY = 1.0f; // Delay before saving state changes
    
    // Internal methods
    bool createWindow();
    bool createOverlayWindow();
    bool createNormalWindow();
    void updateWindowState();
    void applyWindowSettings();
    void setupOverlayMode();
    void setupWindowedMode();
    
    // Platform-specific implementations
    bool setupTransparency();
    bool setupClickThrough();
    bool setupAlwaysOnTop();
    void updateScreenInfo();
    
    // State persistence
    void saveStateToFile() const;
    bool loadStateFromFile();
    
    // Utility
    Vector2 getMonitorSize(int monitor) const;
    Vector2 getMonitorPosition(int monitor) const;
    int getMonitorCount() const;
    
    // Constants
    static constexpr float MIN_ALPHA = 0.1f;
    static constexpr float MAX_ALPHA = 1.0f;
    static constexpr int MIN_WINDOW_WIDTH = 400;
    static constexpr int MIN_WINDOW_HEIGHT = 300;
};