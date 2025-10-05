#include "performance_monitor.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Global performance monitor instance
PerformanceMonitor* g_performanceMonitor = nullptr;

// PerformanceTimer implementation
PerformanceTimer::PerformanceTimer(const std::string& name) : name(name) {
    start();
}

PerformanceTimer::~PerformanceTimer() {
    if (isRunning) {
        stop();
        if (g_performanceMonitor) {
            g_performanceMonitor->recordTimerResult(name, getElapsedMs());
        }
    }
}

void PerformanceTimer::start() {
    startTime = std::chrono::high_resolution_clock::now();
    isRunning = true;
}

void PerformanceTimer::stop() {
    endTime = std::chrono::high_resolution_clock::now();
    isRunning = false;
}

float PerformanceTimer::getElapsedMs() const {
    auto end = isRunning ? std::chrono::high_resolution_clock::now() : endTime;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime);
    return duration.count() / 1000.0f; // Convert to milliseconds
}

// PerformanceMonitor implementation
PerformanceMonitor::PerformanceMonitor() {
    frameTimeHistory.reserve(HISTORY_SIZE);
    fpsHistory.reserve(HISTORY_SIZE);
    
    std::cout << "PerformanceMonitor: Initialized" << std::endl;
}

PerformanceMonitor::~PerformanceMonitor() {
    std::cout << "PerformanceMonitor: Shutdown" << std::endl;
}

void PerformanceMonitor::update(float deltaTime) {
    if (!profilingEnabled) {
        return;
    }
    
    // Record frame time
    recordFrameTime(deltaTime);
    
    // Update averages
    updateAverages();
    
    // Check performance thresholds
    checkPerformanceThresholds();
}

void PerformanceMonitor::render() {
    if (showDisplay) {
        renderPerformanceDisplay();
    }
}

void PerformanceMonitor::recordFrameTime(float frameTime) {
    currentMetrics.frameTime = frameTime;
    currentMetrics.fps = frameTime > 0.0f ? 1.0f / frameTime : 0.0f;
    
    // Add to history
    frameTimeHistory.push_back(frameTime);
    fpsHistory.push_back(currentMetrics.fps);
    
    // Maintain history size
    if (frameTimeHistory.size() > HISTORY_SIZE) {
        frameTimeHistory.erase(frameTimeHistory.begin());
    }
    if (fpsHistory.size() > HISTORY_SIZE) {
        fpsHistory.erase(fpsHistory.begin());
    }
    
    // Update min/max FPS
    currentMetrics.minFps = std::min(currentMetrics.minFps, currentMetrics.fps);
    currentMetrics.maxFps = std::max(currentMetrics.maxFps, currentMetrics.fps);
}

void PerformanceMonitor::recordUpdateTime(float updateTime) {
    currentMetrics.updateTime = updateTime;
}

void PerformanceMonitor::recordRenderTime(float renderTime) {
    currentMetrics.renderTime = renderTime;
}

void PerformanceMonitor::recordInputTime(float inputTime) {
    currentMetrics.inputTime = inputTime;
}

void PerformanceMonitor::recordMemoryUsage(size_t bytes) {
    currentMetrics.memoryUsage = bytes;
}

void PerformanceMonitor::recordTextureCount(int count) {
    currentMetrics.textureCount = count;
}

void PerformanceMonitor::recordNotificationCount(size_t count) {
    currentMetrics.notificationCount = count;
}

std::unique_ptr<PerformanceTimer> PerformanceMonitor::createTimer(const std::string& name) {
    return std::make_unique<PerformanceTimer>(name);
}

void PerformanceMonitor::recordTimerResult(const std::string& name, float timeMs) {
    timerResults[name].push_back(timeMs);
    
    // Maintain reasonable history size for timers
    if (timerResults[name].size() > 100) {
        timerResults[name].erase(timerResults[name].begin());
    }
}

bool PerformanceMonitor::isPerformanceGood() const {
    return currentMetrics.avgFps >= targetFPS * 0.9f && // Within 10% of target FPS
           currentMetrics.frameTime <= (1.0f / targetFPS) * 1.2f && // Frame time not too high
           currentMetrics.memoryUsage < memoryWarningThreshold;
}

std::vector<std::string> PerformanceMonitor::getPerformanceWarnings() const {
    std::vector<std::string> warnings;
    
    if (currentMetrics.avgFps < targetFPS * 0.8f) {
        warnings.push_back("Low FPS detected: " + std::to_string(currentMetrics.avgFps) + " (target: " + std::to_string(targetFPS) + ")");
    }
    
    if (currentMetrics.frameTime > (1.0f / targetFPS) * 1.5f) {
        warnings.push_back("High frame time: " + std::to_string(currentMetrics.frameTime * 1000.0f) + "ms");
    }
    
    if (currentMetrics.memoryUsage > memoryWarningThreshold) {
        warnings.push_back("High memory usage: " + std::to_string(currentMetrics.memoryUsage / (1024 * 1024)) + "MB");
    }
    
    if (currentMetrics.textureCount > 50) {
        warnings.push_back("High texture count: " + std::to_string(currentMetrics.textureCount));
    }
    
    return warnings;
}

std::vector<std::string> PerformanceMonitor::getOptimizationSuggestions() const {
    std::vector<std::string> suggestions;
    
    if (currentMetrics.renderTime > currentMetrics.updateTime * 2.0f) {
        suggestions.push_back("Render time is high - consider optimizing drawing operations");
    }
    
    if (currentMetrics.textureCount > 20) {
        suggestions.push_back("Consider texture atlasing to reduce texture count");
    }
    
    if (currentMetrics.notificationCount > 10) {
        suggestions.push_back("Too many active notifications - consider limiting them");
    }
    
    if (currentMetrics.updateTime > 5.0f) {
        suggestions.push_back("Update time is high - profile game logic");
    }
    
    return suggestions;
}

void PerformanceMonitor::updateAverages() {
    if (!frameTimeHistory.empty()) {
        float sum = 0.0f;
        for (float time : frameTimeHistory) {
            sum += time;
        }
        currentMetrics.avgFrameTime = sum / frameTimeHistory.size();
    }
    
    if (!fpsHistory.empty()) {
        float sum = 0.0f;
        for (float fps : fpsHistory) {
            sum += fps;
        }
        currentMetrics.avgFps = sum / fpsHistory.size();
    }
}

void PerformanceMonitor::renderPerformanceDisplay() {
    Vector2 pos = displayPosition;
    const float lineHeight = 20.0f;
    const float panelWidth = 300.0f;
    const float panelHeight = 400.0f;
    
    // Background panel
    DrawRectangle((int)pos.x - 5, (int)pos.y - 5, (int)panelWidth, (int)panelHeight, Fade(BLACK, 0.8f));
    DrawRectangleLines((int)pos.x - 5, (int)pos.y - 5, (int)panelWidth, (int)panelHeight, WHITE);
    
    // Title
    DrawText("Performance Monitor", (int)pos.x, (int)pos.y, 18, YELLOW);
    pos.y += lineHeight * 1.5f;
    
    // FPS metrics
    Color fpsColor = getPerformanceColor(currentMetrics.avgFps, targetFPS * 0.9f, targetFPS * 0.7f);
    DrawText(("FPS: " + std::to_string((int)currentMetrics.fps) + " (avg: " + std::to_string((int)currentMetrics.avgFps) + ")").c_str(), 
             (int)pos.x, (int)pos.y, 14, fpsColor);
    pos.y += lineHeight;
    
    // Frame time
    Color frameTimeColor = getPerformanceColor(1.0f / currentMetrics.frameTime, targetFPS * 0.9f, targetFPS * 0.7f);
    DrawText(("Frame Time: " + std::to_string(currentMetrics.frameTime * 1000.0f) + "ms").c_str(), 
             (int)pos.x, (int)pos.y, 14, frameTimeColor);
    pos.y += lineHeight;
    
    // Update/Render times
    DrawText(("Update: " + std::to_string(currentMetrics.updateTime) + "ms").c_str(), 
             (int)pos.x, (int)pos.y, 14, WHITE);
    pos.y += lineHeight;
    
    DrawText(("Render: " + std::to_string(currentMetrics.renderTime) + "ms").c_str(), 
             (int)pos.x, (int)pos.y, 14, WHITE);
    pos.y += lineHeight;
    
    // Memory usage
    Color memColor = getPerformanceColor((float)(memoryWarningThreshold - currentMetrics.memoryUsage), 
                                        (float)(memoryWarningThreshold * 0.7f), 
                                        (float)(memoryWarningThreshold * 0.9f));
    DrawText(("Memory: " + std::to_string(currentMetrics.memoryUsage / (1024 * 1024)) + "MB").c_str(), 
             (int)pos.x, (int)pos.y, 14, memColor);
    pos.y += lineHeight;
    
    // Texture count
    DrawText(("Textures: " + std::to_string(currentMetrics.textureCount)).c_str(), 
             (int)pos.x, (int)pos.y, 14, WHITE);
    pos.y += lineHeight;
    
    // Notifications
    DrawText(("Notifications: " + std::to_string(currentMetrics.notificationCount)).c_str(), 
             (int)pos.x, (int)pos.y, 14, WHITE);
    pos.y += lineHeight * 1.5f;
    
    // Performance warnings
    auto warnings = getPerformanceWarnings();
    if (!warnings.empty()) {
        DrawText("Warnings:", (int)pos.x, (int)pos.y, 14, RED);
        pos.y += lineHeight;
        
        for (const auto& warning : warnings) {
            DrawText(("- " + warning).c_str(), (int)pos.x, (int)pos.y, 12, ORANGE);
            pos.y += lineHeight * 0.8f;
        }
    }
    
    // Performance graph
    renderPerformanceGraph();
}

void PerformanceMonitor::renderPerformanceGraph() {
    if (fpsHistory.empty()) return;
    
    Vector2 graphPos = {displayPosition.x, displayPosition.y + 250};
    Vector2 graphSize = {280, 80};
    
    // Graph background
    DrawRectangle((int)graphPos.x, (int)graphPos.y, (int)graphSize.x, (int)graphSize.y, Fade(DARKGRAY, 0.5f));
    DrawRectangleLines((int)graphPos.x, (int)graphPos.y, (int)graphSize.x, (int)graphSize.y, WHITE);
    
    // Target FPS line
    float targetY = graphPos.y + graphSize.y - (targetFPS / 120.0f) * graphSize.y;
    DrawLine((int)graphPos.x, (int)targetY, (int)(graphPos.x + graphSize.x), (int)targetY, GREEN);
    
    // FPS history
    if (fpsHistory.size() > 1) {
        for (size_t i = 1; i < fpsHistory.size(); ++i) {
            float x1 = graphPos.x + ((float)(i - 1) / (float)fpsHistory.size()) * graphSize.x;
            float y1 = graphPos.y + graphSize.y - (fpsHistory[i - 1] / 120.0f) * graphSize.y;
            float x2 = graphPos.x + ((float)i / (float)fpsHistory.size()) * graphSize.x;
            float y2 = graphPos.y + graphSize.y - (fpsHistory[i] / 120.0f) * graphSize.y;
            
            Color lineColor = fpsHistory[i] >= targetFPS * 0.9f ? GREEN : 
                             fpsHistory[i] >= targetFPS * 0.7f ? ORANGE : RED;
            
            DrawLine((int)x1, (int)y1, (int)x2, (int)y2, lineColor);
        }
    }
    
    // Graph labels
    DrawText("FPS History", (int)graphPos.x, (int)(graphPos.y - 15), 12, WHITE);
    DrawText("0", (int)graphPos.x, (int)(graphPos.y + graphSize.y + 5), 10, LIGHTGRAY);
    DrawText("120", (int)(graphPos.x + graphSize.x - 20), (int)(graphPos.y + graphSize.y + 5), 10, LIGHTGRAY);
}

Color PerformanceMonitor::getPerformanceColor(float value, float good, float warning) const {
    if (value >= good) return GREEN;
    if (value >= warning) return ORANGE;
    return RED;
}

void PerformanceMonitor::checkPerformanceThresholds() {
    // This could trigger automatic optimizations or warnings
    if (!isPerformanceGood()) {
        // Could log warnings or trigger optimizations
        static float lastWarningTime = 0.0f;
        float currentTime = GetTime();
        
        if (currentTime - lastWarningTime > 5.0f) { // Warn every 5 seconds max
            auto warnings = getPerformanceWarnings();
            for (const auto& warning : warnings) {
                std::cout << "Performance Warning: " << warning << std::endl;
            }
            lastWarningTime = currentTime;
        }
    }
}

void PerformanceMonitor::optimizeTextureUsage() {
    // This would implement texture optimization strategies
    std::cout << "PerformanceMonitor: Optimizing texture usage..." << std::endl;
}

void PerformanceMonitor::optimizeMemoryUsage() {
    // This would implement memory optimization strategies
    std::cout << "PerformanceMonitor: Optimizing memory usage..." << std::endl;
}

void PerformanceMonitor::optimizeRenderingPipeline() {
    // This would implement rendering optimizations
    std::cout << "PerformanceMonitor: Optimizing rendering pipeline..." << std::endl;
}