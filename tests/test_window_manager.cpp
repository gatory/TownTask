#include <gtest/gtest.h>
#include "../src/ui/window/window_manager.h"
#include <thread>
#include <chrono>

class WindowManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        windowManager = std::make_unique<WindowManager>();
        
        // Set up test settings
        settings.width = 800;
        settings.height = 600;
        settings.title = "Test Window";
        settings.saveWindowState = false; // Don't save state during tests
    }
    
    void TearDown() override {
        windowManager.reset();
    }
    
    std::unique_ptr<WindowManager> windowManager;
    WindowSettings settings;
};

TEST_F(WindowManagerTest, InitializationTest) {
    // Test that WindowManager can be created
    EXPECT_FALSE(windowManager->isInitialized());
    
    // Test default mode
    EXPECT_EQ(windowManager->getCurrentMode(), WindowMode::WINDOWED);
    EXPECT_FALSE(windowManager->isDesktopOverlayMode());
}

TEST_F(WindowManagerTest, WindowSettingsTest) {
    // Test window settings configuration
    settings.overlayAlpha = 0.7f;
    settings.clickThrough = true;
    settings.alwaysOnTop = true;
    settings.screenPadding = 20;
    
    // Settings should be configurable
    EXPECT_FLOAT_EQ(settings.overlayAlpha, 0.7f);
    EXPECT_TRUE(settings.clickThrough);
    EXPECT_TRUE(settings.alwaysOnTop);
    EXPECT_EQ(settings.screenPadding, 20);
}

TEST_F(WindowManagerTest, WindowModeTest) {
    // Test mode switching (without actual window creation)
    EXPECT_EQ(windowManager->getCurrentMode(), WindowMode::WINDOWED);
    
    // Test mode detection
    EXPECT_FALSE(windowManager->isDesktopOverlayMode());
}

TEST_F(WindowManagerTest, OverlayAlphaTest) {
    // Test alpha value setting and clamping
    windowManager->setOverlayAlpha(0.5f);
    EXPECT_FLOAT_EQ(windowManager->getOverlayAlpha(), 0.5f);
    
    // Test clamping to valid range
    windowManager->setOverlayAlpha(1.5f); // Above max
    EXPECT_FLOAT_EQ(windowManager->getOverlayAlpha(), 1.0f);
    
    windowManager->setOverlayAlpha(-0.5f); // Below min
    EXPECT_FLOAT_EQ(windowManager->getOverlayAlpha(), 0.1f); // MIN_ALPHA
}

TEST_F(WindowManagerTest, ClickThroughTest) {
    // Test click-through setting
    windowManager->setClickThrough(true);
    EXPECT_TRUE(windowManager->isClickThroughEnabled());
    
    windowManager->setClickThrough(false);
    EXPECT_FALSE(windowManager->isClickThroughEnabled());
}

TEST_F(WindowManagerTest, ScreenBoundaryTest) {
    // Test screen boundary calculations (using mock screen size)
    Vector2 testPos = {100, 100};
    Vector2 testSize = {50, 50};
    
    // Test position validation
    bool isOnScreen = windowManager->isPositionOnScreen(testPos, testSize);
    // Result depends on actual screen size, but should not crash
    
    // Test position clamping
    Vector2 clampedPos = windowManager->clampPositionToScreen(testPos, testSize);
    EXPECT_GE(clampedPos.x, 0);
    EXPECT_GE(clampedPos.y, 0);
}

TEST_F(WindowManagerTest, WindowStateTest) {
    // Test window state management
    const WindowState& state = windowManager->getWindowState();
    
    // Default state should be reasonable
    EXPECT_EQ(state.mode, WindowMode::WINDOWED);
    EXPECT_GE(state.width, 0);
    EXPECT_GE(state.height, 0);
    EXPECT_GE(state.screenWidth, 0);
    EXPECT_GE(state.screenHeight, 0);
}

TEST_F(WindowManagerTest, ScreenInfoTest) {
    // Test screen information retrieval
    Vector2 screenSize = windowManager->getScreenSize();
    Vector2 screenCenter = windowManager->getScreenCenter();
    
    EXPECT_GT(screenSize.x, 0);
    EXPECT_GT(screenSize.y, 0);
    EXPECT_FLOAT_EQ(screenCenter.x, screenSize.x / 2.0f);
    EXPECT_FLOAT_EQ(screenCenter.y, screenSize.y / 2.0f);
}

TEST_F(WindowManagerTest, CallbackTest) {
    bool modeChangedCalled = false;
    bool windowResizedCalled = false;
    bool windowMovedCalled = false;
    bool focusChangedCalled = false;
    
    // Set up callbacks
    windowManager->setOnModeChanged([&](WindowMode from, WindowMode to) {
        modeChangedCalled = true;
    });
    
    windowManager->setOnWindowResized([&](int width, int height) {
        windowResizedCalled = true;
    });
    
    windowManager->setOnWindowMoved([&](int x, int y) {
        windowMovedCalled = true;
    });
    
    windowManager->setOnWindowFocusChanged([&](bool focused) {
        focusChangedCalled = true;
    });
    
    // Callbacks are set (we can't easily test them being called without actual windows)
    EXPECT_FALSE(modeChangedCalled);
    EXPECT_FALSE(windowResizedCalled);
    EXPECT_FALSE(windowMovedCalled);
    EXPECT_FALSE(focusChangedCalled);
}

TEST_F(WindowManagerTest, StateResetTest) {
    // Modify some state
    windowManager->setOverlayAlpha(0.3f);
    windowManager->setClickThrough(true);
    
    // Reset to defaults
    windowManager->resetToDefaults();
    
    // Check that defaults are restored
    const WindowState& state = windowManager->getWindowState();
    EXPECT_EQ(state.mode, WindowMode::WINDOWED);
    EXPECT_EQ(state.x, 100);
    EXPECT_EQ(state.y, 100);
}

TEST_F(WindowManagerTest, UpdateTest) {
    // Test update method (should not crash)
    windowManager->update();
    
    // Test should close method
    bool shouldClose = windowManager->shouldClose();
    // Result depends on actual window state, but should not crash
}

// Integration test that would work with actual window creation
TEST_F(WindowManagerTest, DISABLED_IntegrationTest) {
    // This test is disabled because it requires actual window creation
    // In a real scenario, you would:
    // 1. Initialize the window manager
    // 2. Test window creation in both modes
    // 3. Test mode switching
    // 4. Test transparency and click-through
    // 5. Test state persistence
    
    /*
    ASSERT_TRUE(windowManager->initialize(settings));
    EXPECT_TRUE(windowManager->isInitialized());
    
    // Test windowed mode
    EXPECT_EQ(windowManager->getCurrentMode(), WindowMode::WINDOWED);
    
    // Test switching to overlay mode
    ASSERT_TRUE(windowManager->setWindowMode(WindowMode::DESKTOP_OVERLAY));
    EXPECT_EQ(windowManager->getCurrentMode(), WindowMode::DESKTOP_OVERLAY);
    EXPECT_TRUE(windowManager->isDesktopOverlayMode());
    
    // Test transparency
    windowManager->setOverlayAlpha(0.5f);
    EXPECT_FLOAT_EQ(windowManager->getOverlayAlpha(), 0.5f);
    
    // Test click-through
    windowManager->setClickThrough(true);
    EXPECT_TRUE(windowManager->isClickThroughEnabled());
    
    // Test switching back to windowed mode
    ASSERT_TRUE(windowManager->setWindowMode(WindowMode::WINDOWED));
    EXPECT_EQ(windowManager->getCurrentMode(), WindowMode::WINDOWED);
    
    // Test state persistence
    windowManager->saveWindowState();
    
    windowManager->shutdown();
    */
}