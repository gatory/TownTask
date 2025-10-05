#pragma once

#include <raylib.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <memory>

struct BuildingConfig {
    std::string id;
    std::string name;
    Vector2 position;
    Vector2 size;
    std::string spriteFile;
    Color color;
    float interactionRadius;
    std::string type;
    Texture2D texture;
    bool textureLoaded = false;
};

struct CharacterAnimationFrame {
    Rectangle sourceRect;
    Vector2 offset;
};

struct CharacterAnimation {
    std::vector<CharacterAnimationFrame> frames;
    float frameRate;
    bool loop;
    std::string name;
};

struct CharacterConfig {
    std::string spriteSheetPath;
    Vector2 frameSize;
    std::unordered_map<std::string, CharacterAnimation> animations;
    std::string defaultAnimation;
    float scale;
    float collisionRadius;
    Texture2D spriteSheet;
    bool spriteSheetLoaded = false;
};

class AssetManager {
public:
    static AssetManager& getInstance();
    
    // Initialization
    bool initialize();
    void cleanup();
    
    // Map and buildings
    bool loadMapConfig(const std::string& configPath = "assets/config/map_config.json");
    const std::vector<BuildingConfig>& getBuildings() const { return buildings; }
    BuildingConfig* getBuildingById(const std::string& id);
    
    // Character
    bool loadCharacterConfig(const std::string& configPath = "assets/config/character_config.json");
    const CharacterConfig& getCharacterConfig() const { return characterConfig; }
    
    // Texture management
    Texture2D loadTexture(const std::string& path);
    void unloadTexture(const std::string& path);
    
    // Sprite processing utilities
    static Texture2D removeWhiteBackground(const std::string& imagePath, Color backgroundColor = WHITE);
    static Texture2D createSpriteSheet(const std::vector<std::string>& imagePaths, int columns, int rows);
    
    // Map utilities
    Vector2 getMapSize() const { return mapSize; }
    int getTileSize() const { return tileSize; }
    
private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    
    // Configuration data
    std::vector<BuildingConfig> buildings;
    CharacterConfig characterConfig;
    Vector2 mapSize;
    int tileSize;
    
    // Texture cache
    std::unordered_map<std::string, Texture2D> textureCache;
    
    // Helper methods
    bool loadBuildingTextures();
    bool loadCharacterTextures();
    Color parseColor(const nlohmann::json& colorJson);
    Vector2 parseVector2(const nlohmann::json& vectorJson);
};