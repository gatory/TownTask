#include "window_manager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#ifdef PLATFORM_DESKTOP
    #if defined(_WIN32)
        #include <windows.h>
        #include <dwmapi.h>
        #pragma comment(lib, "dwmapi.lib")
    #elif defined(__APPLE__)
        #include <ApplicationServices/ApplicationServices.h>
    #elif defined(__linux__)
        #include <X11/Xlib.h>
        #include <X11/extensions/shape.h>
    #endif
#endif

WindowManager::WindowManager() {
    // Initialize default state
    currentState.screenWidth = GetMonitorWidth(0);
    currentState.screenHeight = GetMonitorHeight(0);
}

WindowManager::~WindowManager() {
    shutdown();
}

bool WindowManager::initialize(const WindowSettings& settings) {
    if (initialized) {
        return true;
    }
    
    this->settings = settings;
    
    // Load previous window state if available
    if (settings.saveWindowState) {
        loadWindowState();
    }
    
    // Update screen information
    updateScreenInfo();
    
    // Create the window based on current mode
    if (!createWindow()) {
        std::cerr << "WindowManager: Failed to create window" << std::endl;
        return false;
    }
    
    // Apply initial settings
    applyWindowSettings();
    
    initialized = true;
    std::cout << "WindowManager: Initialized successfully in " 
              << (currentState.mode == WindowMode::DESKTOP_OVERLAY ? "overlay" : "windowed") 
              << " mode" << std::endl;
    
    return true;
}

void WindowManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    // Save current state
    if (settings.saveWindowState) {
        saveWindowState();
    }
    
    // Close window
    if (IsWindowReady()) {
        CloseWindow();
    }
    
    initialized = false;
    std::cout << "WindowManager: Shutdown complete" << std::endl;
}

// Window mode management
bool WindowManager::setWindowMode(WindowMode mode) {
    if (!initialized || currentState.mode == mode) {
        return true;
    }
    
    WindowMode previousMode = currentState.mode;
    
    // Save current window state before switching
    if (IsWindowReady()) {
        Vector2 pos = GetWindowPosition();
        Vector2 size = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        
        currentState.x = (int)pos.x;
        currentState.y = (int)pos.y;
        currentState.width = (int)size.x;
        currentState.height = (int)size.y;
    }
    
    currentState.mode = mode;
    
    // Close current window
    if (IsWindowReady()) {
        CloseWindow();
    }
    
    // Create new window with new mode
    if (!createWindow()) {
        std::cerr << "WindowManager: Failed to switch to " 
                  << (mode == WindowMode::DESKTOP_OVERLAY ? "overlay" : "windowed") 
                  << " mode" << std::endl;
        
        // Try to restore previous mode
        currentState.mode = previousMode;
        createWindow();
        return false;
    }
    
    // Apply settings for new mode
    applyWindowSettings();
    
    // Trigger callback
    if (onModeChanged) {
        onModeChanged(previousMode, mode);
    }
    
    std::cout << "WindowManager: Switched to " 
              << (mode == WindowMode::DESKTOP_OVERLAY ? "overlay" : "windowed") 
              << " mode" << std::endl;
    
    return true;
}

// Window properties
void WindowManager::setWindowSize(int width, int height) {
    if (!initialized) return;
    
    width = std::max(width, MIN_WINDOW_WIDTH);
    height = std::max(height, MIN_WINDOW_HEIGHT);
    
    SetWindowSize(width, height);
    
    currentState.width = width;
    currentState.height = height;
    windowStateChanged = true;
}

void WindowManager::setWindowPosition(int x, int y) {
    if (!initialized) return;
    
    SetWindowPosition(x, y);
    
    currentState.x = x;
    currentState.y = y;
    windowStateChanged = true;
}

void WindowManager::setWindowTitle(const std::string& title) {
    if (!initialized) return;
    
    SetWindowTitle(title.c_str());
}

Vector2 WindowManager::getWindowSize() const {
    if (!initialized) return {(float)settings.width, (float)settings.height};
    
    return {(float)GetScreenWidth(), (float)GetScreenHeight()};
}

Vector2 WindowManager::getWindowPosition() const {
    if (!initialized) return {(float)currentState.x, (float)currentState.y};
    
    return GetWindowPosition();
}

// Desktop overlay specific
void WindowManager::setOverlayAlpha(float alpha) {
    currentState.overlayAlpha = std::clamp(alpha, MIN_ALPHA, MAX_ALPHA);
    
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        // Apply transparency immediately if in overlay mode
        setupTransparency();
    }
    
    windowStateChanged = true;
}

void WindowManager::setClickThrough(bool enabled) {
    currentState.clickThroughEnabled = enabled;
    
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        setupClickThrough();
    }
    
    windowStateChanged = true;
}

void WindowManager::setAlwaysOnTop(bool enabled) {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        setupAlwaysOnTop();
    }
}

// Screen boundary management
Vector2 WindowManager::clampPositionToScreen(const Vector2& position, const Vector2& size) const {
    Vector2 clampedPos = position;
    
    // Get screen bounds
    float screenWidth = (float)currentState.screenWidth;
    float screenHeight = (float)currentState.screenHeight;
    float padding = (float)settings.screenPadding;
    
    // Clamp to screen boundaries
    clampedPos.x = std::clamp(clampedPos.x, padding, screenWidth - size.x - padding);
    clampedPos.y = std::clamp(clampedPos.y, padding, screenHeight - size.y - padding);
    
    return clampedPos;
}

bool WindowManager::isPositionOnScreen(const Vector2& position, const Vector2& size) const {
    float screenWidth = (float)currentState.screenWidth;
    float screenHeight = (float)currentState.screenHeight;
    
    return position.x >= 0 && position.y >= 0 && 
           position.x + size.x <= screenWidth && 
           position.y + size.y <= screenHeight;
}

Vector2 WindowManager::getScreenSize() const {
    return {(float)currentState.screenWidth, (float)currentState.screenHeight};
}

Vector2 WindowManager::getScreenCenter() const {
    return {(float)currentState.screenWidth / 2.0f, (float)currentState.screenHeight / 2.0f};
}

int WindowManager::getCurrentMonitor() const {
    return GetCurrentMonitor();
}

// Window state management
void WindowManager::saveWindowState() const {
    if (!settings.saveWindowState) return;
    
    saveStateToFile();
}

bool WindowManager::loadWindowState() {
    if (!settings.saveWindowState) return false;
    
    return loadStateFromFile();
}

void WindowManager::resetToDefaults() {
    currentState.mode = WindowMode::WINDOWED;
    currentState.x = 100;
    currentState.y = 100;
    currentState.width = settings.width;
    currentState.height = settings.height;
    currentState.overlayAlpha = settings.overlayAlpha;
    currentState.clickThroughEnabled = settings.clickThrough;
    
    windowStateChanged = true;
}

// Event handling
void WindowManager::update() {
    if (!initialized) return;
    
    // Check for window state changes
    if (IsWindowReady()) {
        Vector2 currentPos = GetWindowPosition();
        Vector2 currentSize = getWindowSize();
        
        bool posChanged = (int)currentPos.x != currentState.x || (int)currentPos.y != currentState.y;
        bool sizeChanged = (int)currentSize.x != currentState.width || (int)currentSize.y != currentState.height;
        
        if (posChanged) {
            currentState.x = (int)currentPos.x;
            currentState.y = (int)currentPos.y;
            windowStateChanged = true;
            
            if (onWindowMoved) {
                onWindowMoved(currentState.x, currentState.y);
            }
        }
        
        if (sizeChanged) {
            currentState.width = (int)currentSize.x;
            currentState.height = (int)currentSize.y;
            windowStateChanged = true;
            
            if (onWindowResized) {
                onWindowResized(currentState.width, currentState.height);
            }
        }
    }
    
    // Auto-save state after delay
    if (windowStateChanged && settings.saveWindowState) {
        lastSaveTime += GetFrameTime();
        if (lastSaveTime >= SAVE_DELAY) {
            saveWindowState();
            windowStateChanged = false;
            lastSaveTime = 0.0f;
        }
    }
    
    // Update screen info periodically
    static float screenUpdateTimer = 0.0f;
    screenUpdateTimer += GetFrameTime();
    if (screenUpdateTimer >= 5.0f) { // Update every 5 seconds
        updateScreenInfo();
        screenUpdateTimer = 0.0f;
    }
}

bool WindowManager::shouldClose() const {
    return WindowShouldClose();
}

void WindowManager::handleWindowResize() {
    if (!initialized) return;
    
    Vector2 size = getWindowSize();
    currentState.width = (int)size.x;
    currentState.height = (int)size.y;
    windowStateChanged = true;
}

void WindowManager::handleWindowMove() {
    if (!initialized) return;
    
    Vector2 pos = getWindowPosition();
    currentState.x = (int)pos.x;
    currentState.y = (int)pos.y;
    windowStateChanged = true;
}

// Rendering support
void WindowManager::beginDrawing() {
    BeginDrawing();
    
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        beginOverlayRendering();
    }
}

void WindowManager::endDrawing() {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        endOverlayRendering();
    }
    
    EndDrawing();
}

void WindowManager::clearBackground(Color color) {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        // Use transparent background for overlay mode
        ClearBackground(BLANK);
    } else {
        ClearBackground(color);
    }
}

// Desktop overlay rendering helpers
void WindowManager::beginOverlayRendering() {
    // Set up blending for transparency
    rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_ONE_MINUS_SRC_ALPHA, RLGL_FUNC_ADD);
    rlSetBlendMode(BLEND_ALPHA);
}

void WindowManager::endOverlayRendering() {
    // Reset blending
    rlSetBlendMode(BLEND_ALPHA);
}

void WindowManager::renderOverlayBackground() {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        // Draw a subtle background to make game elements visible
        Color bgColor = Fade(BLACK, 0.1f * currentState.overlayAlpha);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bgColor);
    }
}/
/ Utility methods
bool WindowManager::isWindowFocused() const {
    return IsWindowFocused();
}

bool WindowManager::isWindowMinimized() const {
    return IsWindowMinimized();
}

bool WindowManager::isWindowMaximized() const {
    return IsWindowMaximized();
}

void WindowManager::minimizeWindow() {
    MinimizeWindow();
    currentState.isMinimized = true;
}

void WindowManager::maximizeWindow() {
    MaximizeWindow();
    currentState.isMaximized = true;
}

void WindowManager::restoreWindow() {
    RestoreWindow();
    currentState.isMinimized = false;
    currentState.isMaximized = false;
}

// Event callbacks
void WindowManager::setOnModeChanged(std::function<void(WindowMode, WindowMode)> callback) {
    onModeChanged = callback;
}

void WindowManager::setOnWindowResized(std::function<void(int, int)> callback) {
    onWindowResized = callback;
}

void WindowManager::setOnWindowMoved(std::function<void(int, int)> callback) {
    onWindowMoved = callback;
}

void WindowManager::setOnWindowFocusChanged(std::function<void(bool)> callback) {
    onWindowFocusChanged = callback;
}

// Platform-specific functionality
void WindowManager::enableTransparency() {
    setupTransparency();
}

void WindowManager::disableTransparency() {
    // Reset to opaque
    currentState.overlayAlpha = 1.0f;
    setupTransparency();
}

void WindowManager::setWindowFlags(unsigned int flags) {
    SetWindowState(flags);
}

void WindowManager::clearWindowFlags(unsigned int flags) {
    ClearWindowState(flags);
}

// Private methods
bool WindowManager::createWindow() {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        return createOverlayWindow();
    } else {
        return createNormalWindow();
    }
}

bool WindowManager::createOverlayWindow() {
    // Set window configuration flags for overlay mode
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    
    // Initialize window
    InitWindow(currentState.width, currentState.height, settings.title.c_str());
    
    if (!IsWindowReady()) {
        return false;
    }
    
    // Set up overlay-specific properties
    setupOverlayMode();
    
    return true;
}

bool WindowManager::createNormalWindow() {
    // Set window configuration flags for normal mode
    unsigned int flags = 0;
    
    if (settings.resizable) {
        flags |= FLAG_WINDOW_RESIZABLE;
    }
    
    if (settings.vsync) {
        flags |= FLAG_VSYNC_HINT;
    }
    
    SetConfigFlags(flags);
    
    // Initialize window
    InitWindow(currentState.width, currentState.height, settings.title.c_str());
    
    if (!IsWindowReady()) {
        return false;
    }
    
    // Set up windowed-specific properties
    setupWindowedMode();
    
    return true;
}

void WindowManager::updateWindowState() {
    if (!IsWindowReady()) return;
    
    Vector2 pos = GetWindowPosition();
    Vector2 size = getWindowSize();
    
    currentState.x = (int)pos.x;
    currentState.y = (int)pos.y;
    currentState.width = (int)size.x;
    currentState.height = (int)size.y;
    currentState.isMinimized = IsWindowMinimized();
    currentState.isMaximized = IsWindowMaximized();
}

void WindowManager::applyWindowSettings() {
    if (!IsWindowReady()) return;
    
    // Set target FPS
    SetTargetFPS(settings.targetFPS);
    
    // Set window position and size
    SetWindowPosition(currentState.x, currentState.y);
    SetWindowSize(currentState.width, currentState.height);
    
    // Apply mode-specific settings
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        setupTransparency();
        setupClickThrough();
        setupAlwaysOnTop();
    }
}

void WindowManager::setupOverlayMode() {
    std::cout << "WindowManager: Setting up overlay mode" << std::endl;
    
    // Make window transparent and always on top
    setupTransparency();
    setupAlwaysOnTop();
    
    if (currentState.clickThroughEnabled) {
        setupClickThrough();
    }
}

void WindowManager::setupWindowedMode() {
    std::cout << "WindowManager: Setting up windowed mode" << std::endl;
    
    // Ensure window is opaque and has normal behavior
    currentState.overlayAlpha = 1.0f;
}

// Platform-specific implementations
bool WindowManager::setupTransparency() {
#ifdef PLATFORM_DESKTOP
    #if defined(_WIN32)
        // Windows implementation
        HWND hwnd = GetActiveWindow();
        if (hwnd) {
            LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
            
            BYTE alpha = (BYTE)(currentState.overlayAlpha * 255);
            SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
            
            return true;
        }
    #elif defined(__APPLE__)
        // macOS implementation would go here
        // This is a simplified placeholder
        std::cout << "WindowManager: macOS transparency setup (placeholder)" << std::endl;
        return true;
    #elif defined(__linux__)
        // Linux implementation would go here
        // This is a simplified placeholder
        std::cout << "WindowManager: Linux transparency setup (placeholder)" << std::endl;
        return true;
    #endif
#endif
    
    std::cout << "WindowManager: Transparency setup completed" << std::endl;
    return true;
}

bool WindowManager::setupClickThrough() {
#ifdef PLATFORM_DESKTOP
    #if defined(_WIN32)
        // Windows implementation
        HWND hwnd = GetActiveWindow();
        if (hwnd && currentState.clickThroughEnabled) {
            LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT);
            return true;
        }
    #elif defined(__APPLE__)
        // macOS implementation would go here
        std::cout << "WindowManager: macOS click-through setup (placeholder)" << std::endl;
        return true;
    #elif defined(__linux__)
        // Linux implementation would go here
        std::cout << "WindowManager: Linux click-through setup (placeholder)" << std::endl;
        return true;
    #endif
#endif
    
    std::cout << "WindowManager: Click-through setup completed" << std::endl;
    return true;
}

bool WindowManager::setupAlwaysOnTop() {
    if (currentState.mode == WindowMode::DESKTOP_OVERLAY) {
        SetWindowState(FLAG_WINDOW_TOPMOST);
    } else {
        ClearWindowState(FLAG_WINDOW_TOPMOST);
    }
    
    return true;
}

void WindowManager::updateScreenInfo() {
    int monitor = GetCurrentMonitor();
    currentState.screenWidth = GetMonitorWidth(monitor);
    currentState.screenHeight = GetMonitorHeight(monitor);
    currentState.monitorIndex = monitor;
}

// State persistence
void WindowManager::saveStateToFile() const {
    std::ofstream file(settings.stateFilePath);
    if (!file.is_open()) {
        std::cerr << "WindowManager: Failed to save window state to " << settings.stateFilePath << std::endl;
        return;
    }
    
    // Simple key-value format
    file << "mode=" << (int)currentState.mode << std::endl;
    file << "x=" << currentState.x << std::endl;
    file << "y=" << currentState.y << std::endl;
    file << "width=" << currentState.width << std::endl;
    file << "height=" << currentState.height << std::endl;
    file << "overlayAlpha=" << currentState.overlayAlpha << std::endl;
    file << "clickThrough=" << (currentState.clickThroughEnabled ? 1 : 0) << std::endl;
    file << "isMaximized=" << (currentState.isMaximized ? 1 : 0) << std::endl;
    file << "monitorIndex=" << currentState.monitorIndex << std::endl;
    
    file.close();
    std::cout << "WindowManager: Window state saved" << std::endl;
}

bool WindowManager::loadStateFromFile() {
    std::ifstream file(settings.stateFilePath);
    if (!file.is_open()) {
        std::cout << "WindowManager: No saved window state found, using defaults" << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "mode") {
            currentState.mode = (WindowMode)std::stoi(value);
        } else if (key == "x") {
            currentState.x = std::stoi(value);
        } else if (key == "y") {
            currentState.y = std::stoi(value);
        } else if (key == "width") {
            currentState.width = std::stoi(value);
        } else if (key == "height") {
            currentState.height = std::stoi(value);
        } else if (key == "overlayAlpha") {
            currentState.overlayAlpha = std::stof(value);
        } else if (key == "clickThrough") {
            currentState.clickThroughEnabled = (std::stoi(value) != 0);
        } else if (key == "isMaximized") {
            currentState.isMaximized = (std::stoi(value) != 0);
        } else if (key == "monitorIndex") {
            currentState.monitorIndex = std::stoi(value);
        }
    }
    
    file.close();
    
    // Validate loaded values
    currentState.width = std::max(currentState.width, MIN_WINDOW_WIDTH);
    currentState.height = std::max(currentState.height, MIN_WINDOW_HEIGHT);
    currentState.overlayAlpha = std::clamp(currentState.overlayAlpha, MIN_ALPHA, MAX_ALPHA);
    
    std::cout << "WindowManager: Window state loaded" << std::endl;
    return true;
}

// Utility
Vector2 WindowManager::getMonitorSize(int monitor) const {
    return {(float)GetMonitorWidth(monitor), (float)GetMonitorHeight(monitor)};
}

Vector2 WindowManager::getMonitorPosition(int monitor) const {
    return GetMonitorPosition(monitor);
}

int WindowManager::getMonitorCount() const {
    return GetMonitorCount();
}