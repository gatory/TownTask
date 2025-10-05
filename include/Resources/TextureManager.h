#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>

class TextureManager {
private:
    std::unordered_map<std::string, Texture2D> textures;
    
public:
    ~TextureManager();
    
    Texture2D& Load(const std::string& path);
    bool IsLoaded(const std::string& path) const;
    void Unload(const std::string& path);
    void UnloadAll();
};