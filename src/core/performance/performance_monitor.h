#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <memory>

// Performance metrics
struct PerformanceMetrics {
    float frameTime = 0.0f;
    float fps = 0.0f;
    float updateTime = 0.0f;
    float renderTime = 0.0f;
    float inputTime = 0.0f;
    size_t memoryUsage = 0;
    int textureCount = 0;
    size_t notificationCount = 0;
    
    // Averages over time
    float avgFrameTime = 0.0f;
    float avgFps = 0.0f;
    float minFps = 999.0f;
    float maxFps = 0.0f;
};

// Performance timer for measuring specific operations
class PerformanceTimer {
public:
    PerformanceTimer(const std::string& name);
    ~PerformanceTimer();
    
    void start();
    void stop();
    float getElapsedMs() const;
    
private:
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    bool isRunning = false;
};

// Main performance monitoring system
class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    // Lifecycle
    void update(float deltaTime);
    void render();
    
    // Metrics collection
    void recordFrameTime(float frameTime);
    void recordUpdateTime(float updateTime);
    void recordRenderTime(float renderTime);
    void recordInputTime(float inputTime);
    void recordMemoryUsage(size_t bytes);
    void recordTextureCount(int count);
    void recordNotificationCount(size_t count);
    
    // Performance timers
    std::unique_ptr<PerformanceTimer> createTimer(const std::string& name);
    void recordTimerResult(const std::string& name, float timeMs);
    
    // Analysis
    const PerformanceMetrics& getMetrics() const { return currentMetrics; }
    bool isPerformanceGood() const;
    std::vector<std::string> getPerformanceWarnings() const;
    std::vector<std::string> getOptimizationSuggestions() const;
    
    // Configuration
    void setTargetFPS(float fps) { targetFPS = fps; }
    void setMemoryWarningThreshold(size_t bytes) { memoryWarningThreshold = bytes; }
    void enableProfiling(bool enable) { profilingEnabled = enable; }
    void setDisplayPosition(Vector2 position) { displayPosition = position; }
    
    // Display
    void toggleDisplay() { showDisplay = !showDisplay; }
    bool isDisplayVisible() const { return showDisplay; }
    
    // Optimization helpers
    void optimizeTextureUsage();
    void optimizeMemoryUsage();
    void optimizeRenderingPipeline();

private:
    // Current metrics
    PerformanceMetrics currentMetrics;
    
    // Historical data for averaging
    std::vector<float> frameTimeHistory;
    std::vector<float> fpsHistory;
    static constexpr size_t HISTORY_SIZE = 60; // 1 second at 60 FPS
    
    // Timer results
    std::unordered_map<std::string, std::vector<float>> timerResults;
    
    // Configuration
    float targetFPS = 60.0f;
    size_t memoryWarningThreshold = 100 * 1024 * 1024; // 100MB
    bool profilingEnabled = true;
    bool showDisplay = false;
    Vector2 displayPosition = {10, 100};
    
    // Internal methods
    void updateAverages();
    void renderPerformanceDisplay();
    void renderPerformanceGraph();
    void renderTimerResults();
    
    // Optimization methods
    void checkPerformanceThresholds();
    void suggestOptimizations();
    
    // Display helpers
    Color getPerformanceColor(float value, float good, float warning) const;
    void drawMetricBar(const std::string& label, float value, float max, Vector2 position, Vector2 size, Color color);
};

// Global performance monitor instance (raw pointer for simplicity)
extern PerformanceMonitor* g_performanceMonitor;