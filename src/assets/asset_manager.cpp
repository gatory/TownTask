#include "asset_manager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

bool AssetManager::initialize() {
    std::cout << "AssetManager: Initializing..." << std::endl;
    
    // Load configurations
    if (!loadMapConfig()) {
        std::cerr << "AssetManager: Failed to load map config" << std::endl;
        return false;
    }
    
    if (!loadCharacterConfig()) {
        std::cerr << "AssetManager: Failed to load character config" << std::endl;
        return false;
    }
    
    // Load textures
    if (!loadBuildingTextures()) {
        std::cout << "AssetManager: Warning - Some building textures failed to load" << std::endl;
    }
    
    if (!loadCharacterTextures()) {
        std::cout << "AssetManager: Warning - Character textures failed to load" << std::endl;
    }
    
    std::cout << "AssetManager: Initialization complete" << std::endl;
    return true;
}

void AssetManager::cleanup() {
    std::cout << "AssetManager: Cleaning up..." << std::endl;
    
    // Unload all cached textures
    for (auto& [path, texture] : textureCache) {
        UnloadTexture(texture);
    }
    textureCache.clear();
    
    // Unload building textures
    for (auto& building : buildings) {
        if (building.textureLoaded) {
            UnloadTexture(building.texture);
            building.textureLoaded = false;
        }
    }
    
    // Unload character texture
    if (characterConfig.spriteSheetLoaded) {
        UnloadTexture(characterConfig.spriteSheet);
        characterConfig.spriteSheetLoaded = false;
    }
    
    std::cout << "AssetManager: Cleanup complete" << std::endl;
}

bool AssetManager::loadMapConfig(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "AssetManager: Cannot open map config file: " << configPath << std::endl;
            return false;
        }
        
        nlohmann::json config;
        file >> config;
        
        // Parse map settings
        auto mapConfig = config["map"];
        mapSize.x = mapConfig.value("width", 1024);
        mapSize.y = mapConfig.value("height", 768);
        tileSize = mapConfig.value("tileSize", 64);
        
        // Parse buildings
        buildings.clear();
        for (const auto& buildingJson : config["buildings"]) {
            BuildingConfig building;
            building.id = buildingJson["id"];
            building.name = buildingJson["name"];
            building.position = parseVector2(buildingJson["position"]);
            building.size = parseVector2(buildingJson["size"]);
            building.spriteFile = buildingJson["spriteFile"];
            building.color = parseColor(buildingJson["color"]);
            building.interactionRadius = buildingJson.value("interactionRadius", 50.0f);
            building.type = buildingJson["type"];
            
            buildings.push_back(building);
        }
        
        std::cout << "AssetManager: Loaded " << buildings.size() << " buildings from config" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: Error parsing map config: " << e.what() << std::endl;
        return false;
    }
}

bool AssetManager::loadCharacterConfig(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "AssetManager: Cannot open character config file: " << configPath << std::endl;
            return false;
        }
        
        nlohmann::json config;
        file >> config;
        
        auto charConfig = config["character"];
        characterConfig.spriteSheetPath = charConfig["spriteSheet"];
        characterConfig.frameSize = parseVector2(charConfig["frameSize"]);
        characterConfig.defaultAnimation = charConfig["defaultAnimation"];
        characterConfig.scale = charConfig["scale"];
        characterConfig.collisionRadius = charConfig["collisionRadius"];
        
        // Parse animations
        characterConfig.animations.clear();
        for (const auto& [animName, animData] : charConfig["animations"].items()) {
            CharacterAnimation animation;
            animation.name = animName;
            animation.frameRate = animData["frameRate"];
            animation.loop = animData["loop"];
            
            // Create frames from frame indices
            for (int frameIndex : animData["frames"]) {
                CharacterAnimationFrame frame;
                // Calculate source rectangle based on frame index and frame size
                int framesPerRow = 4; // Assuming 4 frames per row in sprite sheet
                int row = frameIndex / framesPerRow;
                int col = frameIndex % framesPerRow;
                
                frame.sourceRect = {
                    col * characterConfig.frameSize.x,
                    row * characterConfig.frameSize.y,
                    characterConfig.frameSize.x,
                    characterConfig.frameSize.y
                };
                frame.offset = {0, 0}; // No offset by default
                
                animation.frames.push_back(frame);
            }
            
            characterConfig.animations[animName] = animation;
        }
        
        std::cout << "AssetManager: Loaded character config with " << characterConfig.animations.size() << " animations" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: Error parsing character config: " << e.what() << std::endl;
        return false;
    }
}

BuildingConfig* AssetManager::getBuildingById(const std::string& id) {
    for (auto& building : buildings) {
        if (building.id == id) {
            return &building;
        }
    }
    return nullptr;
}

Texture2D AssetManager::loadTexture(const std::string& path) {
    // Check cache first
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }
    
    // Load texture
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id == 0) {
        std::cerr << "AssetManager: Failed to load texture: " << path << std::endl;
        // Return a default 1x1 white texture
        Image whiteImage = GenImageColor(1, 1, WHITE);
        texture = LoadTextureFromImage(whiteImage);
        UnloadImage(whiteImage);
    } else {
        std::cout << "AssetManager: Loaded texture: " << path << std::endl;
    }
    
    // Cache the texture
    textureCache[path] = texture;
    return texture;
}

void AssetManager::unloadTexture(const std::string& path) {
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        UnloadTexture(it->second);
        textureCache.erase(it);
    }
}

Texture2D AssetManager::removeWhiteBackground(const std::string& imagePath, Color backgroundColor) {
    Image image = LoadImage(imagePath.c_str());
    if (image.data == nullptr) {
        std::cerr << "AssetManager: Failed to load image for background removal: " << imagePath << std::endl;
        Texture2D emptyTexture = {0};
        return emptyTexture;
    }
    
    // Convert to RGBA format for easier processing
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    // Process pixels to make background transparent
    Color* pixels = (Color*)image.data;
    int pixelCount = image.width * image.height;
    
    for (int i = 0; i < pixelCount; i++) {
        Color pixel = pixels[i];
        
        // Check if pixel is close to background color (with some tolerance)
        int tolerance = 30;
        if (abs(pixel.r - backgroundColor.r) < tolerance &&
            abs(pixel.g - backgroundColor.g) < tolerance &&
            abs(pixel.b - backgroundColor.b) < tolerance) {
            pixels[i] = {0, 0, 0, 0}; // Make transparent
        }
    }
    
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    std::cout << "AssetManager: Processed image with background removal: " << imagePath << std::endl;
    return texture;
}

Texture2D AssetManager::createSpriteSheet(const std::vector<std::string>& imagePaths, int columns, int rows) {
    if (imagePaths.empty()) {
        Texture2D emptyTexture = {0};
        return emptyTexture;
    }
    
    // Load first image to get dimensions
    Image firstImage = LoadImage(imagePaths[0].c_str());
    if (firstImage.data == nullptr) {
        Texture2D emptyTexture = {0};
        return emptyTexture;
    }
    
    int frameWidth = firstImage.width;
    int frameHeight = firstImage.height;
    
    // Create sprite sheet image
    Image spriteSheet = GenImageColor(frameWidth * columns, frameHeight * rows, {0, 0, 0, 0});
    
    // Copy each frame to the sprite sheet
    for (size_t i = 0; i < imagePaths.size() && i < columns * rows; i++) {
        Image frameImage = LoadImage(imagePaths[i].c_str());
        if (frameImage.data != nullptr) {
            int col = i % columns;
            int row = i / columns;
            
            Rectangle destRect = {
                (float)(col * frameWidth),
                (float)(row * frameHeight),
                (float)frameWidth,
                (float)frameHeight
            };
            
            ImageDraw(&spriteSheet, frameImage, 
                     {0, 0, (float)frameWidth, (float)frameHeight}, 
                     destRect, WHITE);
            
            UnloadImage(frameImage);
        }
    }
    
    UnloadImage(firstImage);
    
    Texture2D texture = LoadTextureFromImage(spriteSheet);
    UnloadImage(spriteSheet);
    
    std::cout << "AssetManager: Created sprite sheet from " << imagePaths.size() << " images" << std::endl;
    return texture;
}

bool AssetManager::loadBuildingTextures() {
    bool allLoaded = true;
    
    for (auto& building : buildings) {
        std::string fullPath = "assets/sprites/" + building.spriteFile;
        
        if (std::filesystem::exists(fullPath)) {
            building.texture = loadTexture(fullPath);
            building.textureLoaded = (building.texture.id != 0);
            
            if (!building.textureLoaded) {
                std::cerr << "AssetManager: Failed to load texture for building: " << building.id << std::endl;
                allLoaded = false;
            }
        } else {
            std::cout << "AssetManager: Texture file not found for building " << building.id << ": " << fullPath << std::endl;
            std::cout << "AssetManager: Will use colored rectangle for " << building.name << std::endl;
        }
    }
    
    return allLoaded;
}

bool AssetManager::loadCharacterTextures() {
    std::string fullPath = "assets/sprites/" + characterConfig.spriteSheetPath;
    
    if (std::filesystem::exists(fullPath)) {
        characterConfig.spriteSheet = loadTexture(fullPath);
        characterConfig.spriteSheetLoaded = (characterConfig.spriteSheet.id != 0);
        
        if (!characterConfig.spriteSheetLoaded) {
            std::cerr << "AssetManager: Failed to load character sprite sheet" << std::endl;
            return false;
        }
    } else {
        std::cout << "AssetManager: Character sprite sheet not found: " << fullPath << std::endl;
        std::cout << "AssetManager: Will use simple colored circle for character" << std::endl;
        return false;
    }
    
    return true;
}

Color AssetManager::parseColor(const nlohmann::json& colorJson) {
    return {
        (unsigned char)colorJson.value("r", 255),
        (unsigned char)colorJson.value("g", 255),
        (unsigned char)colorJson.value("b", 255),
        (unsigned char)colorJson.value("a", 255)
    };
}

Vector2 AssetManager::parseVector2(const nlohmann::json& vectorJson) {
    return {
        vectorJson.value("x", 0.0f),
        vectorJson.value("y", 0.0f)
    };
}