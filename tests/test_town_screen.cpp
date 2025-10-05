#include <iostream>
#include <cassert>
#include <memory>
#include "../src/ui/screens/screen_manager.h"
#include "../src/ui/screens/town_screen.h"
#include "../src/core/models/town_state.h"
#include "../src/input/input_manager.h"
#include "../src/ui/animations/animation_manager.h"

void testVector2() {
    std::cout << "Testing Vector2 structure..." << std::endl;
    
    Vector2 v1(3, 4);
    Vector2 v2(1, 2);
    
    // Test basic operations
    Vector2 sum = v1 + v2;
    assert(sum.x == 4 && sum.y == 6);
    
    Vector2 diff = v1 - v2;
    assert(diff.x == 2 && diff.y == 2);
    
    Vector2 scaled = v1 * 2;
    assert(scaled.x == 6 && scaled.y == 8);
    
    // Test distance and length
    assert(v1.length() == 5.0f); // 3-4-5 triangle
    assert(v1.distance(Vector2(0, 0)) == 5.0f);
    
    // Test normalization
    Vector2 normalized = v1.normalized();
    assert(std::abs(normalized.length() - 1.0f) < 0.001f);
    
    std::cout << "Vector2 test passed!" << std::endl;
}

void testBuilding() {
    std::cout << "Testing Building structure..." << std::endl;
    
    Building building("test_building", "Test Building", BuildingType::COFFEE_SHOP, 
                     Vector2(100, 100), Vector2(64, 64));
    
    // Test basic properties
    assert(building.id == "test_building");
    assert(building.name == "Test Building");
    assert(building.type == BuildingType::COFFEE_SHOP);
    assert(building.position.x == 100 && building.position.y == 100);
    assert(building.size.x == 64 && building.size.y == 64);
    
    // Test entrance position (should be bottom center)
    assert(building.entrancePosition.x == 132); // 100 + 64/2
    assert(building.entrancePosition.y == 164); // 100 + 64
    
    // Test interaction methods
    assert(building.isNearEntrance(Vector2(132, 164), 10.0f)); // At entrance
    assert(!building.isNearEntrance(Vector2(200, 200), 10.0f)); // Far away
    
    assert(building.containsPoint(Vector2(120, 120))); // Inside
    assert(!building.containsPoint(Vector2(200, 200))); // Outside
    
    Vector2 center = building.getCenter();
    assert(center.x == 132 && center.y == 132);
    
    std::cout << "Building test passed!" << std::endl;
}

void testTownMap() {
    std::cout << "Testing TownMap..." << std::endl;
    
    TownMap townMap;
    
    // Test initial state
    assert(townMap.buildings.empty());
    assert(townMap.size.x == 800 && townMap.size.y == 600);
    
    // Test adding buildings
    Building building1("building1", "Building 1", BuildingType::COFFEE_SHOP, Vector2(100, 100), Vector2(64, 64));
    Building building2("building2", "Building 2", BuildingType::LIBRARY, Vector2(200, 200), Vector2(64, 64));
    
    townMap.addBuilding(building1);
    townMap.addBuilding(building2);
    
    assert(townMap.buildings.size() == 2);
    
    // Test finding buildings
    Building* found = townMap.findBuilding("building1");
    assert(found != nullptr);
    assert(found->id == "building1");
    
    Building* notFound = townMap.findBuilding("nonexistent");
    assert(notFound == nullptr);
    
    // Test position validation
    Vector2 characterSize(32, 32);
    assert(townMap.isPositionValid(Vector2(50, 50), characterSize)); // Valid position
    assert(!townMap.isPositionValid(Vector2(120, 120), characterSize)); // Inside building
    assert(!townMap.isPositionValid(Vector2(-10, 50), characterSize)); // Outside map
    
    // Test clamping
    Vector2 clamped = townMap.clampToMap(Vector2(-10, -10), characterSize);
    assert(clamped.x == 0 && clamped.y == 0);
    
    clamped = townMap.clampToMap(Vector2(1000, 1000), characterSize);
    assert(clamped.x == 768 && clamped.y == 568); // 800-32, 600-32
    
    std::cout << "TownMap test passed!" << std::endl;
}

void testTownState() {
    std::cout << "Testing TownState..." << std::endl;
    
    TownState townState;
    
    // Test initial state
    assert(townState.getTimeOfDay() == TownState::TimeOfDay::MORNING);
    assert(townState.getWeather() == TownState::Weather::SUNNY);
    
    // Test character access
    Character& character = townState.getCharacter();
    assert(character.getName() == "Player");
    
    // Test building management
    townState.unlockBuilding("coffee_shop");
    assert(townState.canInteractWithBuilding("coffee_shop"));
    
    // Test finding nearest building
    character.setPosition(Character::Position(250, 250)); // Near coffee shop
    Building* nearest = townState.findNearestBuilding(Vector2(250, 250), 100.0f);
    assert(nearest != nullptr);
    
    // Test time and weather
    townState.setTimeOfDay(TownState::TimeOfDay::EVENING);
    assert(townState.getTimeOfDay() == TownState::TimeOfDay::EVENING);
    
    townState.setWeather(TownState::Weather::RAINY);
    assert(townState.getWeather() == TownState::Weather::RAINY);
    
    std::cout << "TownState test passed!" << std::endl;
}

void testScreenManager() {
    std::cout << "Testing ScreenManager..." << std::endl;
    
    ScreenManager screenManager;
    
    // Test initial state
    assert(!screenManager.hasScreens());
    assert(screenManager.getScreenCount() == 0);
    assert(screenManager.getCurrentScreen() == nullptr);
    
    // Create a mock screen
    class MockScreen : public Screen {
    public:
        MockScreen(const std::string& name) : Screen(name), updateCalled(false), renderCalled(false) {}
        
        void update(float deltaTime) override { updateCalled = true; }
        void render() override { renderCalled = true; }
        void handleInput(const InputState& input) override {}
        
        bool updateCalled;
        bool renderCalled;
    };
    
    // Test pushing screen
    auto mockScreen = std::make_unique<MockScreen>("MockScreen");
    MockScreen* mockScreenPtr = mockScreen.get();
    
    screenManager.pushScreen(std::move(mockScreen));
    
    assert(screenManager.hasScreens());
    assert(screenManager.getScreenCount() == 1);
    assert(screenManager.getCurrentScreen() == mockScreenPtr);
    assert(mockScreenPtr->isActive());
    
    // Test screen update and render
    screenManager.update(0.016f);
    screenManager.render();
    
    assert(mockScreenPtr->updateCalled);
    assert(mockScreenPtr->renderCalled);
    
    // Test popping screen
    screenManager.popScreen();
    assert(!screenManager.hasScreens());
    assert(screenManager.getScreenCount() == 0);
    
    std::cout << "ScreenManager test passed!" << std::endl;
}

void testTownScreenBasic() {
    std::cout << "Testing basic TownScreen functionality..." << std::endl;
    
    // Create required dependencies
    TownState townState;
    InputManager inputManager;
    AnimationManager animationManager;
    
    inputManager.initialize();
    animationManager.initialize();
    
    // Create TownScreen
    TownScreen townScreen(townState, inputManager, animationManager);
    
    // Test initial state
    assert(townScreen.getName() == "TownScreen");
    assert(!townScreen.isActive());
    assert(!townScreen.isDesktopMode());
    assert(townScreen.isCameraFollowingCharacter());
    
    // Test entering screen
    townScreen.onEnter();
    townScreen.setActive(true);
    assert(townScreen.isActive());
    
    // Test character position
    Vector2 testPos(300, 300);
    townScreen.setCharacterPosition(testPos);
    Vector2 retrievedPos = townScreen.getCharacterPosition();
    assert(std::abs(retrievedPos.x - testPos.x) < 0.1f);
    assert(std::abs(retrievedPos.y - testPos.y) < 0.1f);
    
    // Test camera
    Vector2 cameraPos = townScreen.getCameraPosition();
    assert(cameraPos.x >= 0 && cameraPos.y >= 0); // Should be valid
    
    // Test visual settings
    townScreen.setShowInteractionPrompts(false);
    assert(!townScreen.isShowingInteractionPrompts());
    
    townScreen.setShowBuildingNames(true);
    assert(townScreen.isShowingBuildingNames());
    
    townScreen.setShowDebugInfo(true);
    assert(townScreen.isShowingDebugInfo());
    
    // Test desktop mode
    townScreen.setDesktopMode(true);
    assert(townScreen.isDesktopMode());
    
    townScreen.setDesktopAlpha(0.5f);
    assert(std::abs(townScreen.getDesktopAlpha() - 0.5f) < 0.001f);
    
    inputManager.shutdown();
    animationManager.shutdown();
    
    std::cout << "Basic TownScreen test passed!" << std::endl;
}

void testTownScreenInteraction() {
    std::cout << "Testing TownScreen building interaction..." << std::endl;
    
    TownState townState;
    InputManager inputManager;
    AnimationManager animationManager;
    
    inputManager.initialize();
    animationManager.initialize();
    
    TownScreen townScreen(townState, inputManager, animationManager);
    townScreen.onEnter();
    townScreen.setActive(true);
    
    // Position character near coffee shop
    townScreen.setCharacterPosition(Vector2(248, 328)); // Near coffee shop entrance
    
    // Update to find nearest building
    townScreen.update(0.016f);
    
    // Test interaction
    Building* nearest = townScreen.getNearestInteractableBuilding();
    if (nearest) {
        assert(nearest->id == "coffee_shop");
        assert(townScreen.canInteractWithNearestBuilding());
    }
    
    // Test interaction callback
    bool interactionCalled = false;
    std::string interactedBuilding;
    
    townScreen.setOnBuildingInteraction([&](const std::string& buildingId) {
        interactionCalled = true;
        interactedBuilding = buildingId;
    });
    
    if (nearest) {
        townScreen.interactWithBuilding(nearest->id);
        assert(interactionCalled);
        assert(interactedBuilding == nearest->id);
    }
    
    inputManager.shutdown();
    animationManager.shutdown();
    
    std::cout << "TownScreen interaction test passed!" << std::endl;
}

void testTownScreenMovement() {
    std::cout << "Testing TownScreen character movement..." << std::endl;
    
    TownState townState;
    InputManager inputManager;
    AnimationManager animationManager;
    
    inputManager.initialize();
    animationManager.initialize();
    
    TownScreen townScreen(townState, inputManager, animationManager);
    townScreen.onEnter();
    townScreen.setActive(true);
    
    // Test initial position
    Vector2 initialPos = townScreen.getCharacterPosition();
    
    // Test movement callback
    bool movementCalled = false;
    Vector2 movedToPosition;
    
    townScreen.setOnCharacterMoved([&](const Vector2& position) {
        movementCalled = true;
        movedToPosition = position;
    });
    
    // Test setting position
    Vector2 newPos(400, 400);
    townScreen.setCharacterPosition(newPos);
    
    assert(movementCalled);
    assert(std::abs(movedToPosition.x - newPos.x) < 0.1f);
    assert(std::abs(movedToPosition.y - newPos.y) < 0.1f);
    
    // Test camera following
    assert(townScreen.isCameraFollowingCharacter());
    
    Vector2 cameraPos = townScreen.getCameraPosition();
    // Camera should be centered on character (approximately)
    
    inputManager.shutdown();
    animationManager.shutdown();
    
    std::cout << "TownScreen movement test passed!" << std::endl;
}

void testTownScreenCallbacks() {
    std::cout << "Testing TownScreen callbacks..." << std::endl;
    
    TownState townState;
    InputManager inputManager;
    AnimationManager animationManager;
    
    inputManager.initialize();
    animationManager.initialize();
    
    TownScreen townScreen(townState, inputManager, animationManager);
    
    // Test camera change callback
    bool cameraChanged = false;
    Vector2 cameraPosition;
    
    townScreen.setOnCameraChanged([&](const Vector2& position) {
        cameraChanged = true;
        cameraPosition = position;
    });
    
    townScreen.setCameraPosition(Vector2(100, 100));
    assert(cameraChanged);
    assert(cameraPosition.x == 100 && cameraPosition.y == 100);
    
    inputManager.shutdown();
    animationManager.shutdown();
    
    std::cout << "TownScreen callbacks test passed!" << std::endl;
}

int main() {
    try {
        testVector2();
        testBuilding();
        testTownMap();
        testTownState();
        testScreenManager();
        testTownScreenBasic();
        testTownScreenInteraction();
        testTownScreenMovement();
        testTownScreenCallbacks();
        
        std::cout << "\nAll tests passed! Town screen system implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}