#include "notification_system.h"
#include <iostream>
#include <algorithm>

NotificationSystem::NotificationSystem() 
    : notificationBasePosition({10, 10}), maxNotifications(5) {
    // Constructor
}

NotificationSystem::~NotificationSystem() {
    // Destructor
}

void NotificationSystem::update(float deltaTime) {
    // Update notifications
    updateNotificationAnimations(deltaTime);
    
    // Remove expired notifications
    notifications.erase(
        std::remove_if(notifications.begin(), notifications.end(),
            [](const std::unique_ptr<Notification>& n) {
                return n->timeRemaining <= 0.0f && n->alpha <= 0.0f;
            }),
        notifications.end()
    );
    
    // Update positions
    updateNotificationPositions();
}

void NotificationSystem::render() {
    renderNotifications();
    renderProgressIndicator();
    renderConfirmationDialog();
    renderErrorDialog();
}

void NotificationSystem::showNotification(const std::string& message, NotificationType type, float duration) {
    // Remove oldest notifications if we're at the limit
    while (notifications.size() >= static_cast<size_t>(maxNotifications)) {
        notifications.erase(notifications.begin());
    }
    
    // Add new notification
    auto notification = std::make_unique<Notification>(message, type, duration);
    notifications.push_back(std::move(notification));
    
    std::cout << "Notification: " << message << std::endl;
}

void NotificationSystem::showSuccess(const std::string& message, float duration) {
    showNotification(message, NotificationType::SUCCESS, duration);
}

void NotificationSystem::showWarning(const std::string& message, float duration) {
    showNotification(message, NotificationType::WARNING, duration);
}

void NotificationSystem::showError(const std::string& message, float duration) {
    showNotification(message, NotificationType::ERROR, duration);
}

void NotificationSystem::showAchievement(const std::string& message, float duration) {
    showNotification(message, NotificationType::ACHIEVEMENT, duration);
}

void NotificationSystem::clearAllNotifications() {
    notifications.clear();
}

void NotificationSystem::showProgress(const std::string& message, float progress) {
    progressIndicator.message = message;
    progressIndicator.progress = std::clamp(progress, 0.0f, 1.0f);
    progressIndicator.isVisible = true;
    progressIndicator.isIndeterminate = false;
}

void NotificationSystem::updateProgress(float progress) {
    if (progressIndicator.isVisible) {
        progressIndicator.progress = std::clamp(progress, 0.0f, 1.0f);
    }
}

void NotificationSystem::showIndeterminateProgress(const std::string& message) {
    progressIndicator.message = message;
    progressIndicator.isVisible = true;
    progressIndicator.isIndeterminate = true;
}

void NotificationSystem::hideProgress() {
    progressIndicator.isVisible = false;
}

void NotificationSystem::showConfirmation(const std::string& title, const std::string& message,
                                        std::function<void()> onConfirm, std::function<void()> onCancel,
                                        const std::string& confirmText, const std::string& cancelText) {
    confirmationDialog.title = title;
    confirmationDialog.message = message;
    confirmationDialog.confirmText = confirmText;
    confirmationDialog.cancelText = cancelText;
    confirmationDialog.onConfirm = onConfirm;
    confirmationDialog.onCancel = onCancel;
    confirmationDialog.isVisible = true;
}

void NotificationSystem::hideConfirmation() {
    confirmationDialog.isVisible = false;
}

void NotificationSystem::showErrorDialog(const std::string& title, const std::string& message, 
                                       const std::string& details, std::function<void()> onClose) {
    errorDialog.title = title;
    errorDialog.message = message;
    errorDialog.details = details;
    errorDialog.onClose = onClose;
    errorDialog.isVisible = true;
    errorDialog.showDetails = !details.empty();
}

void NotificationSystem::hideErrorDialog() {
    errorDialog.isVisible = false;
}

bool NotificationSystem::handleInput() {
    bool handled = false;
    
    if (confirmationDialog.isVisible) {
        handled = handleConfirmationInput();
    }
    
    if (!handled && errorDialog.isVisible) {
        handled = handleErrorDialogInput();
    }
    
    return handled;
}

bool NotificationSystem::hasActiveDialogs() const {
    return confirmationDialog.isVisible || errorDialog.isVisible;
}

bool NotificationSystem::hasActiveNotifications() const {
    return !notifications.empty() || progressIndicator.isVisible;
}

void NotificationSystem::renderNotifications() {
    for (const auto& notification : notifications) {
        if (!notification->isVisible || notification->alpha <= 0.0f) {
            continue;
        }
        
        // Background
        Color bgColor = getNotificationBackgroundColor(notification->type);
        bgColor.a = (unsigned char)(bgColor.a * notification->alpha);
        
        DrawRectangleRounded(
            {notification->position.x, notification->position.y, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT},
            0.1f, 8, bgColor
        );
        
        // Border
        Color borderColor = getNotificationColor(notification->type);
        borderColor.a = (unsigned char)(borderColor.a * notification->alpha);
        
        DrawRectangleRoundedLines(
            {notification->position.x, notification->position.y, NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT},
            0.1f, 8, borderColor
        );
        
        // Text
        Color textColor = WHITE;
        textColor.a = (unsigned char)(255 * notification->alpha);
        
        // Wrap text if needed
        int fontSize = 16;
        int maxWidth = (int)(NOTIFICATION_WIDTH - 20);
        
        DrawText(notification->message.c_str(), 
                (int)(notification->position.x + 10), 
                (int)(notification->position.y + 20), 
                fontSize, textColor);
    }
}

void NotificationSystem::renderProgressIndicator() {
    if (!progressIndicator.isVisible) {
        return;
    }
    
    Vector2 screenCenter = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    Vector2 size = {300, 100};
    Rectangle bounds = {screenCenter.x - size.x/2, screenCenter.y - size.y/2, size.x, size.y};
    
    // Background
    DrawRectangleRounded(bounds, 0.1f, 8, Fade(BLACK, 0.8f));
    DrawRectangleRoundedLines(bounds, 0.1f, 8, WHITE);
    
    // Title
    DrawText(progressIndicator.message.c_str(), 
            (int)(bounds.x + 20), (int)(bounds.y + 20), 
            18, WHITE);
    
    if (progressIndicator.isIndeterminate) {
        // Spinning indicator
        static float spinAngle = 0.0f;
        spinAngle += 180.0f * GetFrameTime();
        if (spinAngle > 360.0f) spinAngle -= 360.0f;
        
        Vector2 center = {bounds.x + bounds.width/2, bounds.y + bounds.height - 25};
        DrawRing(center, 8, 12, 0, spinAngle, 32, BLUE);
    } else {
        // Progress bar
        Rectangle progressBounds = {bounds.x + 20, bounds.y + bounds.height - 30, bounds.width - 40, 10};
        
        // Background
        DrawRectangleRounded(progressBounds, 0.5f, 4, DARKGRAY);
        
        // Progress
        Rectangle progressFill = progressBounds;
        progressFill.width *= progressIndicator.progress;
        DrawRectangleRounded(progressFill, 0.5f, 4, BLUE);
        
        // Percentage
        char percentText[16];
        snprintf(percentText, sizeof(percentText), "%.0f%%", progressIndicator.progress * 100);
        int textWidth = MeasureText(percentText, 14);
        DrawText(percentText, 
                (int)(bounds.x + bounds.width/2 - textWidth/2), 
                (int)(bounds.y + bounds.height - 50), 
                14, WHITE);
    }
}

void NotificationSystem::renderConfirmationDialog() {
    if (!confirmationDialog.isVisible) {
        return;
    }
    
    Vector2 size = {400, 200};
    Rectangle bounds = getDialogBounds(size);
    
    // Overlay
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
    
    // Dialog background
    DrawRectangleRounded(bounds, 0.1f, 8, WHITE);
    DrawRectangleRoundedLines(bounds, 0.1f, 8, DARKGRAY);
    
    // Title
    DrawText(confirmationDialog.title.c_str(), 
            (int)(bounds.x + 20), (int)(bounds.y + 20), 
            20, BLACK);
    
    // Message
    DrawText(confirmationDialog.message.c_str(), 
            (int)(bounds.x + 20), (int)(bounds.y + 60), 
            16, DARKGRAY);
    
    // Buttons
    Rectangle confirmBtn = {bounds.x + bounds.width - 180, bounds.y + bounds.height - 50, 80, 30};
    Rectangle cancelBtn = {bounds.x + bounds.width - 90, bounds.y + bounds.height - 50, 80, 30};
    
    // Confirm button
    DrawRectangleRounded(confirmBtn, 0.2f, 4, GREEN);
    int confirmTextWidth = MeasureText(confirmationDialog.confirmText.c_str(), 16);
    DrawText(confirmationDialog.confirmText.c_str(), 
            (int)(confirmBtn.x + confirmBtn.width/2 - confirmTextWidth/2), 
            (int)(confirmBtn.y + 7), 
            16, WHITE);
    
    // Cancel button
    DrawRectangleRounded(cancelBtn, 0.2f, 4, RED);
    int cancelTextWidth = MeasureText(confirmationDialog.cancelText.c_str(), 16);
    DrawText(confirmationDialog.cancelText.c_str(), 
            (int)(cancelBtn.x + cancelBtn.width/2 - cancelTextWidth/2), 
            (int)(cancelBtn.y + 7), 
            16, WHITE);
}

void NotificationSystem::renderErrorDialog() {
    if (!errorDialog.isVisible) {
        return;
    }
    
    Vector2 size = {500.0f, errorDialog.showDetails ? 350.0f : 200.0f};
    Rectangle bounds = getDialogBounds(size);
    
    // Overlay
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
    
    // Dialog background
    DrawRectangleRounded(bounds, 0.1f, 8, WHITE);
    DrawRectangleRoundedLines(bounds, 0.1f, 8, RED);
    
    // Title
    DrawText(errorDialog.title.c_str(), 
            (int)(bounds.x + 20), (int)(bounds.y + 20), 
            20, RED);
    
    // Message
    DrawText(errorDialog.message.c_str(), 
            (int)(bounds.x + 20), (int)(bounds.y + 60), 
            16, BLACK);
    
    // Details (if available)
    if (errorDialog.showDetails && !errorDialog.details.empty()) {
        DrawText("Details:", 
                (int)(bounds.x + 20), (int)(bounds.y + 100), 
                14, DARKGRAY);
        
        DrawText(errorDialog.details.c_str(), 
                (int)(bounds.x + 20), (int)(bounds.y + 125), 
                12, DARKGRAY);
    }
    
    // Close button
    Rectangle closeBtn = {bounds.x + bounds.width - 100, bounds.y + bounds.height - 50, 80, 30};
    DrawRectangleRounded(closeBtn, 0.2f, 4, BLUE);
    int closeTextWidth = MeasureText("Close", 16);
    DrawText("Close", 
            (int)(closeBtn.x + closeBtn.width/2 - closeTextWidth/2), 
            (int)(closeBtn.y + 7), 
            16, WHITE);
}

Color NotificationSystem::getNotificationColor(NotificationType type) {
    switch (type) {
        case NotificationType::SUCCESS: return GREEN;
        case NotificationType::WARNING: return ORANGE;
        case NotificationType::ERROR: return RED;
        case NotificationType::ACHIEVEMENT: return GOLD;
        case NotificationType::INFO:
        default: return BLUE;
    }
}

Color NotificationSystem::getNotificationBackgroundColor(NotificationType type) {
    Color baseColor = getNotificationColor(type);
    return Fade(baseColor, 0.1f);
}

void NotificationSystem::updateNotificationPositions() {
    float currentY = notificationBasePosition.y;
    
    for (const auto& notification : notifications) {
        notification->position.x = notificationBasePosition.x;
        notification->position.y = currentY;
        currentY += NOTIFICATION_HEIGHT + NOTIFICATION_SPACING;
    }
}

Rectangle NotificationSystem::getDialogBounds(Vector2 size) {
    Vector2 screenCenter = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    return {screenCenter.x - size.x/2, screenCenter.y - size.y/2, size.x, size.y};
}

bool NotificationSystem::handleConfirmationInput() {
    Vector2 size = {400, 200};
    Rectangle bounds = getDialogBounds(size);
    Rectangle confirmBtn = {bounds.x + bounds.width - 180, bounds.y + bounds.height - 50, 80, 30};
    Rectangle cancelBtn = {bounds.x + bounds.width - 90, bounds.y + bounds.height - 50, 80, 30};
    
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, confirmBtn)) {
            if (confirmationDialog.onConfirm) {
                confirmationDialog.onConfirm();
            }
            hideConfirmation();
            return true;
        } else if (CheckCollisionPointRec(mousePos, cancelBtn)) {
            if (confirmationDialog.onCancel) {
                confirmationDialog.onCancel();
            }
            hideConfirmation();
            return true;
        }
    }
    
    // Keyboard shortcuts
    if (IsKeyPressed(KEY_ENTER)) {
        if (confirmationDialog.onConfirm) {
            confirmationDialog.onConfirm();
        }
        hideConfirmation();
        return true;
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        if (confirmationDialog.onCancel) {
            confirmationDialog.onCancel();
        }
        hideConfirmation();
        return true;
    }
    
    return false;
}

bool NotificationSystem::handleErrorDialogInput() {
    Vector2 size = {500.0f, errorDialog.showDetails ? 350.0f : 200.0f};
    Rectangle bounds = getDialogBounds(size);
    Rectangle closeBtn = {bounds.x + bounds.width - 100, bounds.y + bounds.height - 50, 80, 30};
    
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, closeBtn)) {
            if (errorDialog.onClose) {
                errorDialog.onClose();
            }
            hideErrorDialog();
            return true;
        }
    }
    
    // Keyboard shortcuts
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        if (errorDialog.onClose) {
            errorDialog.onClose();
        }
        hideErrorDialog();
        return true;
    }
    
    return false;
}

void NotificationSystem::updateNotificationAnimations(float deltaTime) {
    for (const auto& notification : notifications) {
        // Update time remaining
        notification->timeRemaining -= deltaTime;
        
        // Fade out animation
        if (notification->timeRemaining <= 1.0f) {
            notification->alpha = std::max(0.0f, notification->timeRemaining);
        }
    }
}