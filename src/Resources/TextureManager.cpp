#include "../include/Resources/TextureManager.h"
#include <iostream>

TextureManager::~TextureManager() {
    UnloadAll();
}

Texture2D& TextureManager::Load(const std::string& path) {
    // Check if already loaded
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }
    
    // Load new texture
    Texture2D texture = LoadTexture(path.c_str());
    
    // Check if loading failed
    if (texture.id == 0) {
        std::cerr << "Warning: Failed to load texture: " << path << std::endl;
    }
    
    // Store and return
    textures[path] = texture;
    return textures[path];
}

bool TextureManager::IsLoaded(const std::string& path) const {
    return textures.find(path) != textures.end();
}

void TextureManager::Unload(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        UnloadTexture(it->second);
        textures.erase(it);
    }
}

void TextureManager::UnloadAll() {
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
    textures.clear();
}