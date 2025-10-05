#include "Systems/RenderSystem.h"
#include "Core/Constants.h"

void RenderSystem::LoadTextures(World& world) {
    for (Entity e : world.GetEntities()) {
        Sprite* sprite = world.GetSprite(e);
        if (sprite && !sprite->loaded && !sprite->texturePath.empty()) {
            sprite->texture = LoadTexture(sprite->texturePath.c_str());
            sprite->loaded = true;
        }
        
        BuildingInterior* interior = world.GetBuildingInterior(e);
        if (interior && !interior->texturesLoaded) {
            if (!interior->guiBackgroundPath.empty()) {
                interior->guiBackgroundTexture = LoadTexture(interior->guiBackgroundPath.c_str());
            }
            if (!interior->gameBackgroundPath.empty()) {
                interior->gameBackgroundTexture = LoadTexture(interior->gameBackgroundPath.c_str());
            }
            interior->texturesLoaded = true;
        }
    }

    // Try to load a dedicated main background if an entity references AssetPath::MAIN_BACKGROUND
    if (!mainBackgroundLoaded) {
        for (Entity e : world.GetEntities()) {
            Sprite* s = world.GetSprite(e);
            if (!s) continue;
            if (s->texturePath == AssetPath::MAIN_BACKGROUND) {
                // load texture
                mainBackgroundTexture = LoadTexture(s->texturePath.c_str());
                mainBackgroundLoaded = true;
                break;
            }
        }
    }
}

void RenderSystem::UnloadTextures(World& world) {
    for (Entity e : world.GetEntities()) {
        Sprite* sprite = world.GetSprite(e);
        if (sprite && sprite->loaded) {
            UnloadTexture(sprite->texture);
            sprite->loaded = false;
        }
        
        BuildingInterior* interior = world.GetBuildingInterior(e);
        if (interior && interior->texturesLoaded) {
            if (!interior->guiBackgroundPath.empty()) {
                UnloadTexture(interior->guiBackgroundTexture);
            }
            if (!interior->gameBackgroundPath.empty()) {
                UnloadTexture(interior->gameBackgroundTexture);
            }
            interior->texturesLoaded = false;
        }
    }

    if (mainBackgroundLoaded) {
        UnloadTexture(mainBackgroundTexture);
        mainBackgroundLoaded = false;
    }
}

void RenderSystem::RenderBuildingInteriorBackgrounds(World& world, int currentScene, int guiHeight, int gameHeight) {
    for (Entity e : world.GetEntities()) {
        Scene* scene = world.GetScene(e);
        if (!scene || scene->sceneId != currentScene) continue;
        
        BuildingInterior* interior = world.GetBuildingInterior(e);
        if (!interior) continue;
        
        // Draw GUI background (top half)
        if (!interior->guiBackgroundPath.empty() && interior->texturesLoaded) {
            Rectangle source = {0, 0, (float)interior->guiBackgroundTexture.width, (float)interior->guiBackgroundTexture.height};
            Rectangle dest = {0, 0, (float)GameConfig::SCREEN_WIDTH, (float)guiHeight};
            DrawTexturePro(interior->guiBackgroundTexture, source, dest, {0, 0}, 0, WHITE);
        } else {
            DrawRectangle(0, 0, GameConfig::SCREEN_WIDTH, guiHeight, interior->guiBackgroundColor);
        }
        
        // Draw game background (bottom half)
        if (!interior->gameBackgroundPath.empty() && interior->texturesLoaded) {
            Rectangle source = {0, 0, (float)interior->gameBackgroundTexture.width, (float)interior->gameBackgroundTexture.height};
            Rectangle dest = {0, (float)guiHeight, (float)GameConfig::SCREEN_WIDTH, (float)gameHeight};
            DrawTexturePro(interior->gameBackgroundTexture, source, dest, {0, 0}, 0, WHITE);
        } else {
            DrawRectangle(0, guiHeight, GameConfig::SCREEN_WIDTH, gameHeight, interior->gameBackgroundColor);
        }
        
        break;
    }
}

void RenderSystem::RenderMainBackground(World& world) {
    // If we loaded a dedicated main background texture, use it; otherwise fall back to entity sprite
    if (mainBackgroundLoaded) {
        Rectangle source = {0, 0, (float)mainBackgroundTexture.width, (float)mainBackgroundTexture.height};
        Rectangle dest = {0, 0, (float)GameConfig::SCREEN_WIDTH, (float)GameConfig::SCREEN_HEIGHT};
        DrawTexturePro(mainBackgroundTexture, source, dest, {0, 0}, 0, WHITE);
        return;
    }

    for (Entity e : world.GetEntities()) {
        Sprite* sprite = world.GetSprite(e);
        Scene* scene = world.GetScene(e);
        if (!sprite || !scene) continue;
        if (scene->sceneId != SceneID::MAIN) continue;
        if (sprite->texturePath != AssetPath::MAIN_BACKGROUND) continue;

        if (sprite->loaded) {
            Rectangle source = {0, 0, (float)sprite->texture.width, (float)sprite->texture.height};
            Rectangle dest = {0, 0, (float)GameConfig::SCREEN_WIDTH, (float)GameConfig::SCREEN_HEIGHT};
            DrawTexturePro(sprite->texture, source, dest, {0, 0}, 0, WHITE);
        }
        break; // Only draw first matching background
    }
}

void RenderSystem::RenderEntities(World& world, int currentScene, int guiHeight) {
    for (Entity e : world.GetEntities()) {
        Scene* scene = world.GetScene(e);
        bool isPlayer = world.HasPlayer(e);

        if (scene && !isPlayer && scene->sceneId != currentScene) continue;

        Position* pos = world.GetPosition(e);
        Sprite* sprite = world.GetSprite(e);
        if (!pos || !sprite) continue;

    // Skip drawing the main background entity here; it's rendered separately stretched to screen
    if (world.HasBuildingInterior(e)) continue;
    if (sprite && sprite->texturePath == AssetPath::MAIN_BACKGROUND) continue;

        Color color = sprite->tint;

        // For interiors, all entity positions are in local game-area coordinates.
        // Add the GUI offset when computing renderY so entities can freely occupy the whole game area.
        float renderY = pos->y;
        if (currentScene != SceneID::MAIN) renderY += (float)guiHeight;
        
        Animation* anim = world.GetAnimation(e);
        if (anim && sprite->loaded) {
            Rectangle source = {
                (float)(anim->currentFrame * anim->frameWidth),
                0,
                (float)anim->frameWidth,
                (float)anim->frameHeight
            };
            Rectangle dest = {
                pos->x,
                renderY,
                (float)sprite->width,
                (float)sprite->height
            };
            DrawTexturePro(sprite->texture, source, dest, {0, 0}, 0, color);
        } else {
            Hitbox* hitbox = world.GetHitbox(e);
            if (hitbox && sprite->texturePath.empty()) {
                DrawRectangle((int)pos->x, (int)renderY, (int)hitbox->width, (int)hitbox->height, color);
            } else if (sprite->loaded) {
                // Draw texture scaled to the sprite width/height using DrawTexturePro
                Rectangle source = {0, 0, (float)sprite->texture.width, (float)sprite->texture.height};
                Rectangle dest = {(float)pos->x, renderY, (float)sprite->width, (float)sprite->height};
                DrawTexturePro(sprite->texture, source, dest, {0, 0}, 0, color);
            }
        }
    }
}

void RenderSystem::RenderSpeechBubbles(World& world, Texture2D speechBubbleTexture) {
    for (Entity e : world.GetEntities()) {
        SpeechBubble* bubble = world.GetSpeechBubble(e);
        Position* pos = world.GetPosition(e);
        if (!bubble || !bubble->active || !pos) continue;
        // For interiors, position is local to game area; add GUI offset when rendering
        Scene* sc = world.GetScene(e);
        float renderY = pos->y;
        if (sc && sc->sceneId != SceneID::MAIN) renderY += (float)GameConfig::GUI_HEIGHT;

        // Use per-entity bubble offset for placement (allows CollisionSystem to set offsets dynamically)
        float bubbleX = pos->x + bubble->offsetX;
        float bubbleY = renderY + bubble->offsetY;
        
        Rectangle source = {0, 0, (float)speechBubbleTexture.width, (float)speechBubbleTexture.height};
        Rectangle dest = {bubbleX, bubbleY, (float)speechBubbleTexture.width * 2, (float)speechBubbleTexture.height * 2};
        DrawTexturePro(speechBubbleTexture, source, dest, {0, 0}, 0, WHITE);
        
        std::string text = bubble->text;
        size_t newlinePos = text.find('\n');
        if (newlinePos != std::string::npos) {
            std::string line1 = text.substr(0, newlinePos);
            std::string line2 = text.substr(newlinePos + 1);
            DrawText(line1.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 8), 8, BLACK);
            DrawText(line2.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 20), 8, BLACK);
        } else {
            DrawText(text.c_str(), (int)(bubbleX + 8), (int)(bubbleY + 8), 8, BLACK);
        }
    }
}

float RenderSystem::CalculateRenderY(Entity e, World& world, int currentScene, int guiHeight) {
    Position* pos = world.GetPosition(e);
    if (!pos) return 0;
    
    float renderY = pos->y;
    
    if (world.HasPlayer(e) && currentScene != SceneID::MAIN) {
        renderY += guiHeight;
    }
    
    return renderY;
}