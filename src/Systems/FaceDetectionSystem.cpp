#include "../include/Systems/FaceDetectionSystem.h"
#include <iostream>

FaceDetectionSystem::FaceDetectionSystem() 
    : running(false), shouldStop(false), faceDetected(false), 
      timeAway(0.0f), awayThreshold(5.0f), lastUpdateTime(0.0), 
      noFaceFrameCount(0) {
    
    // Load the Haar Cascade for face detection
    // Try multiple possible locations for the cascade file
    std::vector<std::string> cascadePaths = {
        "haarcascade_frontalface_default.xml",
        "data/haarcascade_frontalface_default.xml",
        "../data/haarcascade_frontalface_default.xml",
        "assets/haarcascade_frontalface_default.xml",
    };
    
    bool cascadeLoaded = false;
    for (const auto& path : cascadePaths) {
        if (faceCascade.load(path)) {
            std::cout << "Face cascade loaded from: " << path << std::endl;
            cascadeLoaded = true;
            break;
        }
    }
    
    if (!cascadeLoaded) {
        std::cerr << "Warning: Could not load face cascade classifier!" << std::endl;
        std::cerr << "Face detection will not work. Place haarcascade_frontalface_default.xml in the data/ folder." << std::endl;
    }
}

FaceDetectionSystem::~FaceDetectionSystem() {
    Stop();
}

void FaceDetectionSystem::Start() {
    if (running) return;
    
    // Open the default camera
    camera.open(0);
    if (!camera.isOpened()) {
        std::cerr << "Warning: Could not open camera for face detection!" << std::endl;
        return;
    }
    
    // Set camera properties for better performance
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 320);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    camera.set(cv::CAP_PROP_FPS, 15);
    
    shouldStop = false;
    running = true;
    lastUpdateTime = GetTime();
    noFaceFrameCount = 0;
    timeAway = 0.0f;
    
    // Start the detection thread
    detectionThread = std::thread(&FaceDetectionSystem::DetectionLoop, this);
    
    std::cout << "Face detection started" << std::endl;
}

void FaceDetectionSystem::Stop() {
    if (!running) return;
    
    shouldStop = true;
    
    if (detectionThread.joinable()) {
        detectionThread.join();
    }
    
    if (camera.isOpened()) {
        camera.release();
    }
    
    running = false;
    std::cout << "Face detection stopped" << std::endl;
}

bool FaceDetectionSystem::IsUserAway() const {
    return timeAway >= awayThreshold;
}

float FaceDetectionSystem::GetTimeAway() const {
    return timeAway;
}

void FaceDetectionSystem::ResetAwayTimer() {
    timeAway = 0.0f;
    noFaceFrameCount = 0;
}

void FaceDetectionSystem::DetectionLoop() {
    cv::Mat frame, grayFrame;
    std::vector<cv::Rect> faces;
    
    while (!shouldStop) {
        // Capture frame
        if (!camera.read(frame)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Convert to grayscale for face detection
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayFrame, grayFrame);
        
        // Detect faces
        faces.clear();
        if (!faceCascade.empty()) {
            faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, cv::Size(30, 30));
        }
        
        // Update face detection state
        bool currentlyDetected = !faces.empty();
        
        if (currentlyDetected) {
            // Face detected - reset counters
            faceDetected = true;
            noFaceFrameCount = 0;
            timeAway = 0.0f;
        } else {
            // No face detected - increment counter
            noFaceFrameCount++;
            
            // Only consider the user away after threshold frames
            if (noFaceFrameCount >= NO_FACE_FRAMES_THRESHOLD) {
                faceDetected = false;
                
                // Increment time away
                double currentTime = GetTime();
                double deltaTime = currentTime - lastUpdateTime;
                lastUpdateTime = currentTime;
                
                if (deltaTime > 0 && deltaTime < 1.0) { // Sanity check
                    timeAway = timeAway + static_cast<float>(deltaTime);
                }
            }
        }
        
        // Sleep to maintain roughly the update interval
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(UPDATE_INTERVAL * 1000)));
    }
}
