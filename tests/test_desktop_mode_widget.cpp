#include <gtest/gtest.h>
#include "../src/ui/widgets/desktop_mode_widget.h"
#include "../src/ui/window/window_manager.h"
#include "../src/ui/window/desktop_overlay_integration.h"
#include "../src/core/models/town_state.h"
#include <memory>

class DesktopModeWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        windowManager = std::make_shared<WindowManager>();
        townState = std::make_unique<TownState>();
        desktopOverlay = std::make_shared<DesktopOverlayIntegration>(windowManager, *townState);
        
        widget = std::make_unique<DesktopModeWidget>(windowManager, desktopOverlay);
    }
    
    void TearDown() override {
        widget.reset();
        desktopOverlay.reset();
        townState.reset();
        windowManager.reset();
    }
    
    std::shared_ptr<WindowManager> windowManager;
    std::unique_ptr<TownState> townState;
    std::shared_ptr<DesktopOverlayIntegration> desktopOverlay;
    std::unique_ptr<DesktopModeWidget> widget;
};

TEST_F(DesktopModeWidgetTest, InitializationTest) {
    // Test that widget can be created
    EXPECT_TRUE(widget->isVisible());
    EXPECT_TRUE(widget->isEnabled());
    EXPECT_FALSE(widget->isCompactMode());
    EXPECT_TRUE(widget->isShowingLabel());
}

TEST_F(DesktopModeWidgetTest, PositionAndSizeTest) {
    // Test position and size management
    Vector2 newPos = {50, 100};
    widget->setPosition(newPos);
    
    Vector2 currentPos = widget->getPosition();
    EXPECT_FLOAT_EQ(currentPos.x, newPos.x);
    EXPECT_FLOAT_EQ(currentPos.y, newPos.y);
    
    Vector2 newSize = {200, 40};
    widget->setSize(newSize);
    
    Vector2 currentSize = widget->getSize();
    EXPECT_FLOAT_EQ(currentSize.x, newSize.x);
    EXPECT_FLOAT_EQ(currentSize.y, newSize.y);
}

TEST_F(DesktopModeWidgetTest, VisibilityAndStateTest) {
    // Test visibility
    widget->setVisible(false);
    EXPECT_FALSE(widget->isVisible());
    
    widget->setVisible(true);
    EXPECT_TRUE(widget->isVisible());
    
    // Test enabled state
    widget->setEnabled(false);
    EXPECT_FALSE(widget->isEnabled());
    
    widget->setEnabled(true);
    EXPECT_TRUE(widget->isEnabled());
}

TEST_F(DesktopModeWidgetTest, CompactModeTest) {
    // Test compact mode
    widget->setCompactMode(true);
    EXPECT_TRUE(widget->isCompactMode());
    
    Vector2 compactSize = widget->getSize();
    EXPECT_EQ(compactSize.x, 24); // COMPACT_SIZE
    EXPECT_EQ(compactSize.y, 24);
    
    widget->setCompactMode(false);
    EXPECT_FALSE(widget->isCompactMode());
    
    Vector2 fullSize = widget->getSize();
    EXPECT_EQ(fullSize.x, 120);
    EXPECT_EQ(fullSize.y, 30); // FULL_HEIGHT
}

TEST_F(DesktopModeWidgetTest, LabelTest) {
    // Test label visibility
    widget->setShowLabel(false);
    EXPECT_FALSE(widget->isShowingLabel());
    
    widget->setShowLabel(true);
    EXPECT_TRUE(widget->isShowingLabel());
}

TEST_F(DesktopModeWidgetTest, DesktopModeControlTest) {
    // Test desktop mode state detection
    bool initialState = widget->isDesktopModeActive();
    
    // Test toggle (without actual window creation)
    widget->toggleDesktopMode();
    
    // Test explicit setting
    widget->setDesktopModeActive(true);
    widget->setDesktopModeActive(false);
}

TEST_F(DesktopModeWidgetTest, QuickSettingsTest) {
    // Test quick settings panel
    EXPECT_FALSE(widget->isShowingQuickSettings());
    
    widget->showQuickSettings();
    EXPECT_TRUE(widget->isShowingQuickSettings());
    
    widget->hideQuickSettings();
    EXPECT_FALSE(widget->isShowingQuickSettings());
}

TEST_F(DesktopModeWidgetTest, CallbackTest) {
    bool desktopModeChanged = false;
    bool widgetClicked = false;
    
    // Set up callbacks
    widget->setOnDesktopModeChanged([&](bool active) {
        desktopModeChanged = true;
    });
    
    widget->setOnWidgetClicked([&]() {
        widgetClicked = true;
    });
    
    // Callbacks are set (we can't easily test them being called without actual interaction)
    EXPECT_FALSE(desktopModeChanged);
    EXPECT_FALSE(widgetClicked);
}

TEST_F(DesktopModeWidgetTest, UpdateTest) {
    // Test update method (should not crash)
    widget->update(0.016f); // 60 FPS
    widget->update(0.033f); // 30 FPS
}

TEST_F(DesktopModeWidgetTest, InputHandlingTest) {
    // Test input handling (without actual mouse input)
    Vector2 mousePos = {0, 0};
    bool mousePressed = false;
    
    // Should not crash with no input
    widget->handleInput(mousePos, mousePressed);
    
    // Test with mouse over widget
    Vector2 widgetPos = widget->getPosition();
    Vector2 widgetSize = widget->getSize();
    mousePos = {widgetPos.x + widgetSize.x / 2, widgetPos.y + widgetSize.y / 2};
    
    widget->handleInput(mousePos, mousePressed);
    
    // Test with mouse pressed
    mousePressed = true;
    widget->handleInput(mousePos, mousePressed);
}

TEST_F(DesktopModeWidgetTest, RenderTest) {
    // Test render method (should not crash without graphics context)
    // In a real test environment with graphics, this would actually render
    widget->render();
    
    // Test rendering in different states
    widget->setCompactMode(true);
    widget->render();
    
    widget->setCompactMode(false);
    widget->setShowLabel(false);
    widget->render();
    
    widget->showQuickSettings();
    widget->render();
}

// Integration test that would work with actual rendering
TEST_F(DesktopModeWidgetTest, DISABLED_RenderingIntegrationTest) {
    // This test is disabled because it requires actual graphics initialization
    // In a real scenario, you would:
    // 1. Initialize raylib graphics
    // 2. Test widget rendering in different states
    // 3. Test mouse interaction and hover effects
    // 4. Test quick settings panel rendering
    // 5. Test animations and transitions
    
    /*
    InitWindow(400, 300, "Widget Test");
    
    widget->setPosition({50, 50});
    
    // Test rendering loop
    for (int frame = 0; frame < 60; ++frame) {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        // Update widget
        widget->update(1.0f / 60.0f);
        
        // Handle input
        Vector2 mousePos = GetMousePosition();
        bool mousePressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        widget->handleInput(mousePos, mousePressed);
        
        // Render widget
        widget->render();
        
        EndDrawing();
    }
    
    CloseWindow();
    */
}