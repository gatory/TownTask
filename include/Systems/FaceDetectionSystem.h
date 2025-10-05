// #pragma once
// #include <opencv2/opencv.hpp>
// #include <thread>
// #include <atomic>
// #include <mutex>
// #include "raylib.h"

// class FaceDetectionSystem {
// public:
//     FaceDetectionSystem();
//     ~FaceDetectionSystem();
    
//     // Start and stop the face detection thread
//     void Start();
//     void Stop();
    
//     // Check if user is currently away
//     bool IsUserAway() const;
    
//     // Get time user has been away (in seconds)
//     float GetTimeAway() const;
    
//     // Get threshold for triggering angry reaction
//     float GetAwayThreshold() const { return awayThreshold; }
//     void SetAwayThreshold(float threshold) { awayThreshold = threshold; }
    
//     // Reset the away timer
//     void ResetAwayTimer();
    
// private:
//     // Background thread function
//     void DetectionLoop();
    
//     // OpenCV objects
//     cv::VideoCapture camera;
//     cv::CascadeClassifier faceCascade;
    
//     // Thread management
//     std::thread detectionThread;
//     std::atomic<bool> running;
//     std::atomic<bool> shouldStop;
    
//     // Face detection state
//     std::atomic<bool> faceDetected;
//     std::atomic<float> timeAway;
//     float awayThreshold; // seconds before triggering angry reaction
    
//     // Mutex for thread safety
//     mutable std::mutex stateMutex;
    
//     // Last update time
//     double lastUpdateTime;
    
//     // Constants
//     static constexpr float UPDATE_INTERVAL = 0.1f; // Check every 100ms
//     static constexpr int NO_FACE_FRAMES_THRESHOLD = 3; // Frames without face before considering away
//     int noFaceFrameCount;
// };
