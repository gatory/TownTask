#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Notification types
enum class NotificationType {
    INFO,
    SUCCESS,
    WARNING,
    ERROR,
    ACHIEVEMENT
};

// Individual notification
struct Notification {
    std::string message;
    NotificationType type;
    float duration;
    float timeRemaining;
    bool isVisible;
    Vector2 position;
    float alpha;
    
    Notification(const std::string& msg, NotificationType t, float dur = 3.0f)
        : message(msg), type(t), duration(dur), timeRemaining(dur), 
          isVisible(true), position({0, 0}), alpha(1.0f) {}
};

// Progress indicator
struct ProgressIndicator {
    std::string message;
    float progress; // 0.0 to 1.0
    bool isVisible;
    bool isIndeterminate; // Spinning indicator
    
    ProgressIndicator() : message("Loading..."), progress(0.0f), 
                         isVisible(false), isIndeterminate(false) {}
};

// Confirmation dialog
struct ConfirmationDialog {
    std::string title;
    std::string message;
    std::string confirmText;
    std::string cancelText;
    bool isVisible;
    std::function<void()> onConfirm;
    std::function<void()> onCancel;
    
    ConfirmationDialog() : title("Confirm"), message("Are you sure?"), 
                          confirmText("Yes"), cancelText("No"), 
                          isVisible(false) {}
};

// Error dialog
struct ErrorDialog {
    std::string title;
    std::string message;
    std::string details;
    bool isVisible;
    bool showDetails;
    std::function<void()> onClose;
    
    ErrorDialog() : title("Error"), message("An error occurred"), 
                   isVisible(false), showDetails(false) {}
};

// Main notification system
class NotificationSystem {
public:
    NotificationSystem();
    ~NotificationSystem();
    
    // Lifecycle
    void update(float deltaTime);
    void render();
    
    // Notifications
    void showNotification(const std::string& message, NotificationType type = NotificationType::INFO, float duration = 3.0f);
    void showSuccess(const std::string& message, float duration = 3.0f);
    void showWarning(const std::string& message, float duration = 4.0f);
    void showError(const std::string& message, float duration = 5.0f);
    void showAchievement(const std::string& message, float duration = 4.0f);
    void clearAllNotifications();
    
    // Progress indicators
    void showProgress(const std::string& message, float progress = 0.0f);
    void updateProgress(float progress);
    void showIndeterminateProgress(const std::string& message);
    void hideProgress();
    
    // Confirmation dialogs
    void showConfirmation(const std::string& title, const std::string& message,
                         std::function<void()> onConfirm, std::function<void()> onCancel = nullptr,
                         const std::string& confirmText = "Yes", const std::string& cancelText = "No");
    void hideConfirmation();
    
    // Error dialogs
    void showErrorDialog(const std::string& title, const std::string& message, 
                        const std::string& details = "", std::function<void()> onClose = nullptr);
    void hideErrorDialog();
    
    // Input handling
    bool handleInput();
    
    // Status
    bool hasActiveDialogs() const;
    bool hasActiveNotifications() const;
    
    // Configuration
    void setNotificationPosition(Vector2 position) { notificationBasePosition = position; }
    void setMaxNotifications(int max) { maxNotifications = max; }

private:
    // Notification management
    std::vector<std::unique_ptr<Notification>> notifications;
    Vector2 notificationBasePosition;
    int maxNotifications;
    
    // UI components
    ProgressIndicator progressIndicator;
    ConfirmationDialog confirmationDialog;
    ErrorDialog errorDialog;
    
    // Rendering helpers
    void renderNotifications();
    void renderProgressIndicator();
    void renderConfirmationDialog();
    void renderErrorDialog();
    
    // Color helpers
    Color getNotificationColor(NotificationType type);
    Color getNotificationBackgroundColor(NotificationType type);
    
    // Layout helpers
    void updateNotificationPositions();
    Rectangle getDialogBounds(Vector2 size);
    
    // Input helpers
    bool handleConfirmationInput();
    bool handleErrorDialogInput();
    
    // Animation
    void updateNotificationAnimations(float deltaTime);
    
    // Constants
    static constexpr float NOTIFICATION_WIDTH = 300.0f;
    static constexpr float NOTIFICATION_HEIGHT = 60.0f;
    static constexpr float NOTIFICATION_SPACING = 10.0f;
    static constexpr float FADE_SPEED = 2.0f;
};