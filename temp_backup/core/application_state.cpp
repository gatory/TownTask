#include "application_state.h"
#include "../ui/screens/town_screen.h"
#include "../ui/screens/coffee_shop_screen.h"
#include "../ui/screens/bulletin_board_screen.h"
#include "../ui/screens/library_screen.h"
#include "../ui/screens/gym_screen.h"
#include "../ui/screens/settings_screen.h"
#include <iostream>

ApplicationState::ApplicationState() {
    // Constructor - initialization happens in initialize()
}

ApplicationState::~ApplicationState() {
    shutdown();
}

bool ApplicationState::initialize() {
    if (initialized) {
        return true;
    }
    
    std::cout << "ApplicationState: Initializing all systems..." << std::endl;
    
    // Initialize systems in dependency order
    if (!initializeCoreSystem()) {
        std::cerr << "ApplicationState: Failed to initialize core systems" << std::endl;
        return false;
    }
    
    if (!initializeWindowSystem()) {
        std::cerr << "ApplicationState: Failed to initialize window system" << std::endl;
        return false;
    }
    
    if (!initializeAudioSystem()) {
        std::cerr << "ApplicationState: Failed to initialize audio system" << std::endl;
        return false;
    }
    
    if (!initializeUISystem()) {
        std::cerr << "ApplicationState: Failed to initialize UI system" << std::endl;
        return false;
    }
    
    if (!initializePersistence()) {
        std::cerr << "ApplicationState: Failed to initialize persistence" << std::endl;
        return false;
    }
    
    // Set up system integration
    setupSystemIntegration();
    
    // Create all screens
    createAllScreens();
    
    // Set up screen transitions
    setupScreenTransitions();
    
    // Load settings and game state
    loadSettings();
    loadGame();
    
    initialized = true;
    std::cout << "ApplicationState: All systems initialized successfully" << std::endl;
    
    return true;
}

void ApplicationState::shutdown() {
    if (!initialized) {
        return;
    }
    
    std::cout << "ApplicationState: Shutting down all systems..." << std::endl;
    
    // Save current state
    saveSettings();
    saveGame();
    
    // Shutdown systems in reverse order
    screenManager.reset();
    
    // Audio systems
    audioIntegration.reset();
    audioManager.reset();
    
    // Window systems
    desktopOverlay.reset();
    windowManager.reset();
    
    // UI systems
    animationManager.reset();
    inputManager.reset();
    
    // Core game systems
    buildingUpgradeSystem.reset();
    gamificationEngine.reset();
    noteSystem.reset();
    habitTracker.reset();
    pomodoroTimer.reset();
    taskEngine.reset();
    townState.reset();
    
    // Persistence
    saveManager.reset();
    
    initialized = false;
    std::cout << "ApplicationState: Shutdown complete" << std::endl;
}

// Application lifecycle
void ApplicationState::update(float deltaTime) {
    if (!initialized) return;
    
    // Update core systems
    taskEngine->update(deltaTime);
    pomodoroTimer->update(deltaTime);
    habitTracker->update(deltaTime);
    gamificationEngine->update(deltaTime);
    buildingUpgradeSystem->update(deltaTime);
    
    // Update UI systems
    inputManager->update();
    animationManager->update(deltaTime);
    screenManager->update(deltaTime);
    
    // Update window and audio systems
    windowManager->update();
    if (desktopOverlay) {
        desktopOverlay->update(deltaTime);
    }
    if (audioManager) {
        audioManager->update(deltaTime);
    }
    
    // Update data flow between systems
    updateDataFlow();
    
    // Auto-save periodically
    static float autoSaveTimer = 0.0f;
    autoSaveTimer += deltaTime;
    if (autoSaveTimer >= AUTO_SAVE_INTERVAL) {
        saveGame();
        autoSaveTimer = 0.0f;
    }
}

void ApplicationState::render() {
    if (!initialized) return;
    
    windowManager->beginDrawing();
    
    // Render current screen
    screenManager->render();
    
    // Render desktop overlay elements if in desktop mode
    if (desktopOverlay && desktopOverlay->isDesktopModeEnabled()) {
        desktopOverlay->renderDesktopModeIndicator();
        desktopOverlay->renderBuildingOverlays();
    }
    
    // Render debug info if enabled
    if (debugMode) {
        renderDebugInfo();
    }
    
    windowManager->endDrawing();
}

void ApplicationState::handleInput() {
    if (!initialized) return;
    
    const InputState& input = inputManager->getCurrentInput();
    
    // Handle global shortcuts first
    if (input.isKeyDown(KEY_LEFT_CONTROL)) {
        // Ctrl+D for desktop mode toggle
        if (input.isKeyJustPressed(KEY_D)) {
            toggleDesktopMode();
            return;
        }
        
        // Ctrl+S for save
        if (input.isKeyJustPressed(KEY_S)) {
            saveGame();
            return;
        }
        
        // Ctrl+L for load
        if (input.isKeyJustPressed(KEY_L)) {
            loadGame();
            return;
        }
    }
    
    // Pass input to current screen
    screenManager->handleInput(input);
}

bool ApplicationState::shouldClose() const {
    return windowManager && windowManager->shouldClose();
}

// Screen management
void ApplicationState::switchToTownScreen() {
    auto townScreen = createTownScreen();
    if (townScreen) {
        screenManager->replaceScreen(std::move(townScreen));
        currentBuildingId.clear();
    }
}

void ApplicationState::switchToCoffeeShopScreen() {
    auto coffeeShopScreen = createCoffeeShopScreen();
    if (coffeeShopScreen) {
        screenManager->pushScreen(std::move(coffeeShopScreen));
        currentBuildingId = "coffee_shop";
    }
}

void ApplicationState::switchToBulletinBoardScreen() {
    auto bulletinBoardScreen = createBulletinBoardScreen();
    if (bulletinBoardScreen) {
        screenManager->pushScreen(std::move(bulletinBoardScreen));
        currentBuildingId = "bulletin_board";
    }
}

void ApplicationState::switchToLibraryScreen() {
    auto libraryScreen = createLibraryScreen();
    if (libraryScreen) {
        screenManager->pushScreen(std::move(libraryScreen));
        currentBuildingId = "library";
    }
}

void ApplicationState::switchToGymScreen() {
    auto gymScreen = createGymScreen();
    if (gymScreen) {
        screenManager->pushScreen(std::move(gymScreen));
        currentBuildingId = "gym";
    }
}

void ApplicationState::switchToHomeScreen() {
    // Home screen would be created here when implemented
    std::cout << "ApplicationState: Home screen not yet implemented" << std::endl;
}

void ApplicationState::switchToSettingsScreen() {
    auto settingsScreen = createSettingsScreen();
    if (settingsScreen) {
        screenManager->pushScreen(std::move(settingsScreen));
    }
}

void ApplicationState::returnToPreviousScreen() {
    screenManager->popScreen();
    
    // Clear building ID if returning to town
    Screen* currentScreen = screenManager->getCurrentScreen();
    if (currentScreen && currentScreen->getName() == "TownScreen") {
        currentBuildingId.clear();
    }
}

// Building interactions
void ApplicationState::enterBuilding(const std::string& buildingId) {
    std::cout << "ApplicationState: Entering building " << buildingId << std::endl;
    
    // Audio feedback
    if (audioIntegration) {
        audioIntegration->onBuildingEntered(buildingId);
    }
    
    // Switch to appropriate screen
    if (buildingId == "coffee_shop") {
        switchToCoffeeShopScreen();
    } else if (buildingId == "bulletin_board") {
        switchToBulletinBoardScreen();
    } else if (buildingId == "library") {
        switchToLibraryScreen();
    } else if (buildingId == "gym") {
        switchToGymScreen();
    } else if (buildingId == "home") {
        switchToHomeScreen();
    } else {
        std::cerr << "ApplicationState: Unknown building ID: " << buildingId << std::endl;
    }
}

void ApplicationState::exitCurrentBuilding() {
    if (!isInBuilding()) {
        return;
    }
    
    std::cout << "ApplicationState: Exiting building " << currentBuildingId << std::endl;
    
    // Audio feedback
    if (audioIntegration) {
        audioIntegration->onBuildingExited();
    }
    
    // Return to town screen
    switchToTownScreen();
}

// Game state management
void ApplicationState::saveGame() {
    if (!saveManager) {
        std::cerr << "ApplicationState: SaveManager not available" << std::endl;
        return;
    }
    
    try {
        // Collect data from all systems
        nlohmann::json gameData;
        
        gameData["townState"] = townState->toJson();
        gameData["character"] = townState->getCharacter().toJson();
        gameData["taskEngine"] = taskEngine->toJson();
        gameData["pomodoroTimer"] = pomodoroTimer->toJson();
        gameData["habitTracker"] = habitTracker->toJson();
        gameData["gamificationEngine"] = gamificationEngine->toJson();
        gameData["buildingUpgradeSystem"] = buildingUpgradeSystem->toJson();
        
        // Save application state
        gameData["currentBuildingId"] = currentBuildingId;
        gameData["currentScreen"] = screenManager->getCurrentScreen() ? 
                                   screenManager->getCurrentScreen()->getName() : "";
        
        // Save to file
        bool success = saveManager->saveGame(SAVE_FILE_PATH, gameData);
        if (success) {
            std::cout << "ApplicationState: Game saved successfully" << std::endl;
        } else {
            std::cerr << "ApplicationState: Failed to save game" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "ApplicationState: Save error: " << e.what() << std::endl;
    }
}

bool ApplicationState::loadGame() {
    if (!saveManager) {
        std::cerr << "ApplicationState: SaveManager not available" << std::endl;
        return false;
    }
    
    try {
        nlohmann::json gameData;
        bool success = saveManager->loadGame(SAVE_FILE_PATH, gameData);
        
        if (!success) {
            std::cout << "ApplicationState: No save file found, starting new game" << std::endl;
            newGame();
            return true;
        }
        
        // Load data into all systems
        if (gameData.contains("townState")) {
            townState->loadFromJson(gameData["townState"]);
        }
        
        if (gameData.contains("character")) {
            Character character;
            character.fromJson(gameData["character"]);
            townState->setCharacter(character);
        }
        
        if (gameData.contains("taskEngine")) {
            taskEngine->fromJson(gameData["taskEngine"]);
        }
        
        if (gameData.contains("pomodoroTimer")) {
            pomodoroTimer->fromJson(gameData["pomodoroTimer"]);
        }
        
        if (gameData.contains("habitTracker")) {
            habitTracker->fromJson(gameData["habitTracker"]);
        }
        
        if (gameData.contains("gamificationEngine")) {
            gamificationEngine->fromJson(gameData["gamificationEngine"]);
        }
        
        if (gameData.contains("buildingUpgradeSystem")) {
            buildingUpgradeSystem->fromJson(gameData["buildingUpgradeSystem"]);
        }
        
        // Restore application state
        if (gameData.contains("currentBuildingId")) {
            currentBuildingId = gameData["currentBuildingId"];
        }
        
        // Restore appropriate screen
        std::string savedScreen = gameData.value("currentScreen", "TownScreen");
        if (savedScreen == "CoffeeShopScreen" && !currentBuildingId.empty()) {
            switchToCoffeeShopScreen();
        } else if (savedScreen == "BulletinBoardScreen" && !currentBuildingId.empty()) {
            switchToBulletinBoardScreen();
        } else if (savedScreen == "LibraryScreen" && !currentBuildingId.empty()) {
            switchToLibraryScreen();
        } else if (savedScreen == "GymScreen" && !currentBuildingId.empty()) {
            switchToGymScreen();
        } else {
            switchToTownScreen();
        }
        
        gameLoaded = true;
        std::cout << "ApplicationState: Game loaded successfully" << std::endl;
        
        if (onGameStateChanged) {
            onGameStateChanged();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ApplicationState: Load error: " << e.what() << std::endl;
        newGame();
        return false;
    }
}

void ApplicationState::newGame() {
    std::cout << "ApplicationState: Starting new game" << std::endl;
    
    // Reset all systems to initial state
    townState = std::make_unique<TownState>();
    taskEngine = std::make_unique<TaskEngine>();
    pomodoroTimer = std::make_unique<PomodoroTimer>();
    habitTracker = std::make_unique<HabitTracker>();
    noteSystem = std::make_unique<NoteSystem>();
    gamificationEngine = std::make_unique<GamificationEngine>();
    buildingUpgradeSystem = std::make_unique<BuildingUpgradeSystem>(*townState, *gamificationEngine);
    
    // Re-setup system integration
    setupSystemIntegration();
    
    // Start at town screen
    switchToTownScreen();
    
    currentBuildingId.clear();
    gameLoaded = true;
    
    if (onGameStateChanged) {
        onGameStateChanged();
    }
}

void ApplicationState::resetGame() {
    std::cout << "ApplicationState: Resetting game to defaults" << std::endl;
    
    // Clear save file
    if (saveManager) {
        // This would delete the save file in a real implementation
    }
    
    // Start new game
    newGame();
}

// Settings management
void ApplicationState::applySettings() {
    // Apply settings to all systems
    if (windowManager) {
        // Window settings would be applied here
    }
    
    if (audioManager) {
        // Audio settings would be applied here
    }
    
    if (desktopOverlay) {
        // Desktop overlay settings would be applied here
    }
    
    std::cout << "ApplicationState: Settings applied to all systems" << std::endl;
}

void ApplicationState::saveSettings() {
    std::cout << "ApplicationState: Saving settings..." << std::endl;
    
    if (windowManager) {
        windowManager->saveWindowState();
    }
    
    if (audioManager) {
        audioManager->saveSettings("audio_settings.txt");
    }
    
    if (desktopOverlay) {
        desktopOverlay->saveDesktopSettings();
    }
}

void ApplicationState::loadSettings() {
    std::cout << "ApplicationState: Loading settings..." << std::endl;
    
    if (windowManager) {
        windowManager->loadWindowState();
    }
    
    if (audioManager) {
        audioManager->loadSettings("audio_settings.txt");
    }
    
    if (desktopOverlay) {
        desktopOverlay->loadDesktopSettings();
    }
}

void ApplicationState::resetSettings() {
    std::cout << "ApplicationState: Resetting all settings to defaults" << std::endl;
    
    if (windowManager) {
        windowManager->resetToDefaults();
    }
    
    if (audioManager) {
        AudioSettings defaultSettings;
        audioManager->applySettings(defaultSettings);
    }
    
    if (desktopOverlay) {
        DesktopOverlayIntegration::DesktopSettings defaultSettings;
        desktopOverlay->applyDesktopSettings(defaultSettings);
    }
}

// Desktop mode integration
void ApplicationState::toggleDesktopMode() {
    if (desktopOverlay) {
        desktopOverlay->toggleDesktopMode();
    }
}

bool ApplicationState::isDesktopModeEnabled() const {
    return desktopOverlay && desktopOverlay->isDesktopModeEnabled();
}

// Event callbacks
void ApplicationState::setOnApplicationExit(std::function<void()> callback) {
    onApplicationExit = callback;
}

void ApplicationState::setOnGameStateChanged(std::function<void()> callback) {
    onGameStateChanged = callback;
}

// Debug and diagnostics
void ApplicationState::enableDebugMode(bool enable) {
    debugMode = enable;
    
    if (screenManager) {
        screenManager->enableDebugMode(enable);
    }
    
    std::cout << "ApplicationState: Debug mode " << (enable ? "enabled" : "disabled") << std::endl;
}

std::vector<std::string> ApplicationState::getSystemStatus() const {
    std::vector<std::string> status;
    
    status.push_back("=== TaskTown System Status ===");
    status.push_back("Initialized: " + std::string(initialized ? "YES" : "NO"));
    status.push_back("Debug Mode: " + std::string(debugMode ? "YES" : "NO"));
    status.push_back("Game Loaded: " + std::string(gameLoaded ? "YES" : "NO"));
    status.push_back("Current Building: " + (currentBuildingId.empty() ? "None" : currentBuildingId));
    
    if (screenManager) {
        Screen* currentScreen = screenManager->getCurrentScreen();
        status.push_back("Current Screen: " + (currentScreen ? currentScreen->getName() : "None"));
        status.push_back("Screen Count: " + std::to_string(screenManager->getScreenCount()));
    }
    
    if (desktopOverlay) {
        status.push_back("Desktop Mode: " + std::string(desktopOverlay->isDesktopModeEnabled() ? "YES" : "NO"));
    }
    
    if (audioManager) {
        status.push_back("Audio Initialized: " + std::string(audioManager->isInitialized() ? "YES" : "NO"));
        status.push_back("Sound Effects: " + std::string(audioManager->areSoundEffectsEnabled() ? "ON" : "OFF"));
        status.push_back("Music: " + std::string(audioManager->isMusicEnabled() ? "ON" : "OFF"));
    }
    
    if (gamificationEngine) {
        status.push_back("Coffee Tokens: " + std::to_string(gamificationEngine->getCoffeeTokens()));
        status.push_back("Total XP: " + std::to_string(gamificationEngine->getTotalExperience()));
    }
    
    return status;
}

void ApplicationState::logSystemState() const {
    std::vector<std::string> status = getSystemStatus();
    for (const std::string& line : status) {
        std::cout << line << std::endl;
    }
}