#include "Systems/RenderSystem.h"
#include "Core/Constants.h"
#include <sstream>
#include <vector>
#include <algorithm>

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
        
        // Wrap text into lines that fit within a max inner width (in pixels)
        const int fontSize = GameConfig::SPEECH_BUBBLE_FONT_SIZE;
        const int lineHeight = GameConfig::SPEECH_BUBBLE_TEXT_LINE_HEIGHT;
        const int textOffsetX = GameConfig::SPEECH_BUBBLE_TEXT_OFFSET_X;
        const int textOffsetY = GameConfig::SPEECH_BUBBLE_TEXT_OFFSET_Y;

        int maxInnerWidth = std::min(320, GameConfig::SCREEN_WIDTH / 3); // clamp to reasonable width

        std::vector<std::string> lines;
        std::string raw = bubble->text;
        // Split by explicit newlines first
        std::istringstream rawStream(raw);
        std::string paragraph;
        while (std::getline(rawStream, paragraph)) {
            // wrap this paragraph by words
            std::istringstream words(paragraph);
            std::string word;
            std::string cur;
            while (words >> word) {
                std::string attempt = cur.empty() ? word : cur + " " + word;
                int w = MeasureText(attempt.c_str(), fontSize);
                if (w <= maxInnerWidth) {
                    cur = attempt;
                } else {
                    if (!cur.empty()) lines.push_back(cur);
                    // if single word longer than maxInnerWidth, push it anyway
                    cur = word;
                    if (MeasureText(cur.c_str(), fontSize) > maxInnerWidth) {
                        lines.push_back(cur);
                        cur.clear();
                    }
                }
            }
            if (!cur.empty()) lines.push_back(cur);
            // preserve blank line as paragraph separator
            if (paragraph.empty()) lines.push_back(std::string());
        }

        if (lines.empty()) lines.push_back(std::string());

        // compute max line width
        int innerWidth = 0;
        for (auto &ln : lines) {
            innerWidth = std::max(innerWidth, MeasureText(ln.c_str(), fontSize));
        }

        int bubbleInnerW = innerWidth;
        int bubbleInnerH = (int)lines.size() * lineHeight;

        float destW = (float)(bubbleInnerW + textOffsetX * 2);
        float destH = (float)(bubbleInnerH + textOffsetY * 2);

        // 9-slice rendering: preserve corners, stretch edges and center
        const int corner = 8; // pixels in source texture for corners
        float srcW = (float)speechBubbleTexture.width;
        float srcH = (float)speechBubbleTexture.height;

        // Source rectangles for 9 patches
        Rectangle srcTopLeft     = {0, 0, (float)corner, (float)corner};
        Rectangle srcTop         = {(float)corner, 0, srcW - 2*corner, (float)corner};
        Rectangle srcTopRight    = {srcW - corner, 0, (float)corner, (float)corner};
        Rectangle srcLeft        = {0, (float)corner, (float)corner, srcH - 2*corner};
        Rectangle srcCenter      = {(float)corner, (float)corner, srcW - 2*corner, srcH - 2*corner};
        Rectangle srcRight       = {srcW - corner, (float)corner, (float)corner, srcH - 2*corner};
        Rectangle srcBottomLeft  = {0, srcH - corner, (float)corner, (float)corner};
        Rectangle srcBottom      = {(float)corner, srcH - corner, srcW - 2*corner, (float)corner};
        Rectangle srcBottomRight = {srcW - corner, srcH - corner, (float)corner, (float)corner};

        // Dest sizes
        float leftW = (float)corner;
        float rightW = (float)corner;
        float topH = (float)corner;
        float bottomH = (float)corner;
        float centerW = destW - leftW - rightW;
        float centerH = destH - topH - bottomH;

        // Draw corners
        DrawTexturePro(speechBubbleTexture, srcTopLeft,     {bubbleX,                     bubbleY,                      leftW, topH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcTopRight,    {bubbleX + leftW + centerW,   bubbleY,                      rightW, topH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcBottomLeft,  {bubbleX,                     bubbleY + topH + centerH,     leftW, bottomH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcBottomRight, {bubbleX + leftW + centerW,   bubbleY + topH + centerH,     rightW, bottomH}, {0,0}, 0, WHITE);

        // Draw edges
        DrawTexturePro(speechBubbleTexture, srcTop,  {bubbleX + leftW,           bubbleY,                centerW, topH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcBottom,{bubbleX + leftW,           bubbleY + topH + centerH,centerW, bottomH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcLeft, {bubbleX,                   bubbleY + topH,          leftW, centerH}, {0,0}, 0, WHITE);
        DrawTexturePro(speechBubbleTexture, srcRight,{bubbleX + leftW + centerW, bubbleY + topH,          rightW, centerH}, {0,0}, 0, WHITE);

        // Draw center
        DrawTexturePro(speechBubbleTexture, srcCenter, {bubbleX + leftW, bubbleY + topH, centerW, centerH}, {0,0}, 0, WHITE);

        // Draw text fully opaque
        for (size_t i = 0; i < lines.size(); ++i) {
            int tx = (int)(bubbleX + textOffsetX);
            int ty = (int)(bubbleY + textOffsetY + i * lineHeight);
            DrawText(lines[i].c_str(), tx, ty, fontSize, BLACK);
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