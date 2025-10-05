#include "building_renderer.h"
#include <algorithm>
#include <cmath>
#include <random>

BuildingRenderer::BuildingRenderer(BuildingUpgradeSystem& upgradeSystem)
    : upgradeSystem(upgradeSystem), buildingScale(1.0f), showUpgradeEffects(true), showDecorations(true) {
}

// Main rendering
void BuildingRenderer::renderBuilding(const Building& building, const Vector2& screenPosition) {
    // Initialize visuals if needed
    if (buildingVisuals.find(building.id) == buildingVisuals.end()) {
        initializeBuildingVisuals(building.id);
    }
    
    // Update visuals if building has changed
    if (upgradeSystem.hasBuildingChanged(building.id)) {
        updateBuildingVisuals(building.id);
        upgradeSystem.markBuildingAsUpdated(building.id);
    }
    
    const BuildingVisuals& visuals = buildingVisuals[building.id];
    
    // Render based on building type
    switch (building.type) {
        case BuildingType::COFFEE_SHOP:
            renderCoffeeShop(building, screenPosition, visuals);
            break;
        case BuildingType::LIBRARY:
            renderLibrary(building, screenPosition, visuals);
            break;
        case BuildingType::GYM:
            renderGym(building, screenPosition, visuals);
            break;
        case BuildingType::BULLETIN_BOARD:
            renderBulletinBoard(building, screenPosition, visuals);
            break;
        case BuildingType::HOME:
            renderHome(building, screenPosition, visuals);
            break;
        default:
            renderBasicBuilding(building, screenPosition, visuals);
            break;
    }
    
    // Render decorations
    if (showDecorations) {
        renderDecorations(building, screenPosition);
    }
    
    // Render upgrade effects
    if (showUpgradeEffects) {
        renderUpgradeEffects(building, screenPosition);
    }
    
    // Render level indicator
    renderLevelIndicator(building, screenPosition);
    
    // Render building name
    renderBuildingName(building, screenPosition);
}

void BuildingRenderer::renderBuildingWithUpgrades(const Building& building, const Vector2& screenPosition) {
    renderBuilding(building, screenPosition);
}

void BuildingRenderer::renderUpgradeEffects(const Building& building, const Vector2& screenPosition) {
    if (!showUpgradeEffects) return;
    
    auto it = buildingVisuals.find(building.id);
    if (it == buildingVisuals.end()) return;
    
    const BuildingVisuals& visuals = it->second;
    
    // Render upgrade glow
    if (visuals.showUpgradeGlow) {
        float intensity = sin(visuals.upgradeGlowTimer * 4.0f) * 0.5f + 0.5f;
        renderUpgradeGlow(screenPosition, building.size, intensity);
    }
    
    // Render particles
    for (const UpgradeParticle& particle : visuals.particles) {
        float alpha = particle.life / particle.maxLife;
        Color particleColor = Fade(particle.color, alpha);
        
        if (particle.type == "sparkle") {
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size, particleColor);
        } else if (particle.type == "star") {
            // Draw a simple star shape
            Vector2 center = particle.position;
            float radius = particle.size;
            for (int i = 0; i < 5; i++) {
                float angle1 = (i * 72.0f) * DEG2RAD;
                float angle2 = ((i + 1) * 72.0f) * DEG2RAD;
                Vector2 p1 = {center.x + cos(angle1) * radius, center.y + sin(angle1) * radius};
                Vector2 p2 = {center.x + cos(angle2) * radius, center.y + sin(angle2) * radius};
                DrawLineV(center, p1, particleColor);
                DrawLineV(p1, p2, particleColor);
            }
        } else if (particle.type == "glow") {
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size * 2, Fade(particleColor, alpha * 0.3f));
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size, particleColor);
        }
    }
}

// Update system
void BuildingRenderer::update(float deltaTime) {
    // Update all building visuals
    for (auto& pair : buildingVisuals) {
        BuildingVisuals& visuals = pair.second;
        
        // Update upgrade glow
        if (visuals.showUpgradeGlow) {
            visuals.upgradeGlowTimer += deltaTime;
            if (visuals.upgradeGlowTimer >= UPGRADE_GLOW_DURATION) {
                visuals.showUpgradeGlow = false;
                visuals.upgradeGlowTimer = 0.0f;
            }
        }
        
        // Update particles
        for (auto& particle : visuals.particles) {
            updateParticle(particle, deltaTime);
        }
        
        // Remove expired particles
        visuals.particles.erase(
            std::remove_if(visuals.particles.begin(), visuals.particles.end(),
                [](const UpgradeParticle& p) { return p.life <= 0.0f; }),
            visuals.particles.end()
        );
    }
    
    // Update global particles
    updateParticles(deltaTime);
}

void BuildingRenderer::updateBuildingVisuals(const std::string& buildingId) {
    updateBuildingColors(buildingId);
    updateVisualElements(buildingId);
}

void BuildingRenderer::triggerUpgradeAnimation(const std::string& buildingId) {
    auto it = buildingVisuals.find(buildingId);
    if (it != buildingVisuals.end()) {
        BuildingVisuals& visuals = it->second;
        visuals.showUpgradeGlow = true;
        visuals.upgradeGlowTimer = 0.0f;
        
        // Add upgrade particles
        // Find building position (this is simplified - in real implementation you'd pass the position)
        Vector2 buildingPos = {400, 300}; // Placeholder
        addUpgradeParticles(buildingPos, 15);
    }
}

void BuildingRenderer::triggerDecorationAnimation(const std::string& buildingId, const Vector2& position) {
    addDecorationParticles(position, 8);
}

// Visual customization
void BuildingRenderer::setBuildingScale(float scale) {
    buildingScale = std::max(0.1f, std::min(3.0f, scale));
}

float BuildingRenderer::getBuildingScale() const {
    return buildingScale;
}

void BuildingRenderer::setShowUpgradeEffects(bool show) {
    showUpgradeEffects = show;
}

bool BuildingRenderer::isShowingUpgradeEffects() const {
    return showUpgradeEffects;
}

void BuildingRenderer::setShowDecorations(bool show) {
    showDecorations = show;
}

bool BuildingRenderer::isShowingDecorations() const {
    return showDecorations;
}

// Particle system
void BuildingRenderer::addUpgradeParticles(const Vector2& position, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 particlePos = {
            position.x + (rand() % 40 - 20),
            position.y + (rand() % 40 - 20)
        };
        Vector2 velocity = getRandomVelocity(50.0f);
        Color color = getRandomUpgradeColor();
        std::string type = (i % 3 == 0) ? "star" : ((i % 3 == 1) ? "sparkle" : "glow");
        
        globalParticles.emplace_back(particlePos, velocity, color, PARTICLE_LIFETIME, type);
    }
}

void BuildingRenderer::addDecorationParticles(const Vector2& position, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 particlePos = {
            position.x + (rand() % 20 - 10),
            position.y + (rand() % 20 - 10)
        };
        Vector2 velocity = getRandomVelocity(30.0f);
        Color color = getRandomDecorationColor();
        
        globalParticles.emplace_back(particlePos, velocity, color, DECORATION_PARTICLE_LIFETIME, "sparkle");
    }
}

void BuildingRenderer::updateParticles(float deltaTime) {
    for (auto& particle : globalParticles) {
        updateParticle(particle, deltaTime);
    }
    
    // Remove expired particles
    globalParticles.erase(
        std::remove_if(globalParticles.begin(), globalParticles.end(),
            [](const UpgradeParticle& p) { return p.life <= 0.0f; }),
        globalParticles.end()
    );
}

void BuildingRenderer::renderParticles() {
    for (const UpgradeParticle& particle : globalParticles) {
        float alpha = particle.life / particle.maxLife;
        Color particleColor = Fade(particle.color, alpha);
        
        if (particle.type == "sparkle") {
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size, particleColor);
        } else if (particle.type == "star") {
            // Simple star rendering
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size, particleColor);
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size * 0.5f, WHITE);
        } else if (particle.type == "glow") {
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size * 2, Fade(particleColor, alpha * 0.2f));
            DrawCircle((int)particle.position.x, (int)particle.position.y, particle.size, particleColor);
        }
    }
}

// Building-specific rendering
void BuildingRenderer::renderCoffeeShop(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    // Base structure
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "peaked");
    drawBuildingWindows(position, scaledSize, visuals.accentColor, building.level);
    drawBuildingDoor(position, scaledSize, BROWN);
    
    // Level-specific features
    if (building.level >= 2) {
        // Outdoor seating
        Vector2 tablePos = {position.x + scaledSize.x + 10, position.y + scaledSize.y - 20};
        DrawRectangle((int)tablePos.x, (int)tablePos.y, 15, 15, BROWN);
    }
    
    if (building.level >= 3) {
        // Coffee sign
        Vector2 signPos = {position.x + scaledSize.x/2 - 20, position.y - 15};
        DrawRectangle((int)signPos.x, (int)signPos.y, 40, 10, visuals.accentColor);
        DrawText("COFFEE", (int)signPos.x + 2, (int)signPos.y + 1, 8, BLACK);
    }
    
    if (building.level >= 4) {
        // Awning
        Vector2 awningPos = {position.x - 5, position.y + 20};
        DrawRectangle((int)awningPos.x, (int)awningPos.y, (int)scaledSize.x + 10, 8, visuals.secondaryColor);
    }
    
    if (building.level >= 5) {
        // Garden area
        Vector2 gardenPos = {position.x - 10, position.y + scaledSize.y + 5};
        DrawCircle((int)gardenPos.x + 10, (int)gardenPos.y + 10, 8, GREEN);
        DrawCircle((int)gardenPos.x + 30, (int)gardenPos.y + 8, 6, DARKGREEN);
    }
}

void BuildingRenderer::renderLibrary(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    // Base structure
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "flat");
    drawBuildingWindows(position, scaledSize, visuals.accentColor, building.level);
    drawBuildingDoor(position, scaledSize, DARKBROWN);
    
    // Level-specific features
    if (building.level >= 2) {
        // Reading garden
        Vector2 gardenPos = {position.x - 15, position.y + scaledSize.y + 5};
        DrawRectangle((int)gardenPos.x, (int)gardenPos.y, 20, 15, GREEN);
    }
    
    if (building.level >= 3) {
        // Study alcoves (small extensions)
        Vector2 alcovePos = {position.x + scaledSize.x, position.y + 20};
        DrawRectangle((int)alcovePos.x, (int)alcovePos.y, 15, 30, visuals.primaryColor);
    }
    
    if (building.level >= 4) {
        // Tower addition
        Vector2 towerPos = {position.x + scaledSize.x/2 - 10, position.y - 20};
        DrawRectangle((int)towerPos.x, (int)towerPos.y, 20, 20, visuals.secondaryColor);
        drawBuildingRoof(towerPos, {20, 20}, visuals.accentColor, "peaked");
    }
    
    if (building.level >= 5) {
        // Observatory dome
        Vector2 domePos = {position.x + scaledSize.x/2, position.y - 25};
        DrawCircle((int)domePos.x, (int)domePos.y, 12, visuals.accentColor);
    }
}

void BuildingRenderer::renderGym(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    // Base structure
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "flat");
    drawBuildingWindows(position, scaledSize, visuals.accentColor, building.level);
    drawBuildingDoor(position, scaledSize, DARKGRAY);
    
    // Level-specific features
    if (building.level >= 2) {
        // Equipment visible through windows
        Vector2 equipPos = {position.x + 10, position.y + 30};
        DrawRectangle((int)equipPos.x, (int)equipPos.y, 8, 15, DARKGRAY);
        DrawRectangle((int)equipPos.x + 15, (int)equipPos.y + 5, 12, 8, GRAY);
    }
    
    if (building.level >= 3) {
        // Training yard
        Vector2 yardPos = {position.x + scaledSize.x + 5, position.y + 20};
        DrawRectangle((int)yardPos.x, (int)yardPos.y, 25, 40, Fade(BROWN, 0.5f));
        DrawRectangleLines((int)yardPos.x, (int)yardPos.y, 25, 40, BLACK);
    }
    
    if (building.level >= 4) {
        // Sports equipment outside
        Vector2 equipPos = {position.x - 10, position.y + scaledSize.y - 15};
        DrawCircle((int)equipPos.x, (int)equipPos.y, 5, ORANGE); // Basketball
    }
    
    if (building.level >= 5) {
        // Championship banners
        for (int i = 0; i < 3; i++) {
            Vector2 bannerPos = {position.x + 10 + i * 15, position.y - 10};
            DrawRectangle((int)bannerPos.x, (int)bannerPos.y, 8, 15, GOLD);
        }
    }
}

void BuildingRenderer::renderBulletinBoard(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    // Base structure (simpler for bulletin board)
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "peaked");
    
    // The board itself
    Vector2 boardPos = {position.x + 10, position.y + 20};
    Vector2 boardSize = {scaledSize.x - 20, scaledSize.y - 30};
    DrawRectangle((int)boardPos.x, (int)boardPos.y, (int)boardSize.x, (int)boardSize.y, BEIGE);
    DrawRectangleLines((int)boardPos.x, (int)boardPos.y, (int)boardSize.x, (int)boardSize.y, BROWN);
    
    // Level-specific features
    if (building.level >= 2) {
        // Multiple boards
        Vector2 board2Pos = {position.x + scaledSize.x + 5, position.y + 20};
        DrawRectangle((int)board2Pos.x, (int)board2Pos.y, 20, (int)boardSize.y, BEIGE);
        DrawRectangleLines((int)board2Pos.x, (int)board2Pos.y, 20, (int)boardSize.y, BROWN);
    }
    
    if (building.level >= 3) {
        // Digital display elements
        Vector2 screenPos = {position.x + 5, position.y + 10};
        DrawRectangle((int)screenPos.x, (int)screenPos.y, 15, 10, BLACK);
        DrawRectangle((int)screenPos.x + 2, (int)screenPos.y + 2, 11, 6, BLUE);
    }
}

void BuildingRenderer::renderHome(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    // Base structure
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "peaked");
    drawBuildingWindows(position, scaledSize, visuals.accentColor, building.level);
    drawBuildingDoor(position, scaledSize, BROWN);
    
    // Level-specific features
    if (building.level >= 2) {
        // Garden
        Vector2 gardenPos = {position.x - 10, position.y + scaledSize.y + 2};
        for (int i = 0; i < 3; i++) {
            DrawCircle((int)gardenPos.x + i * 8, (int)gardenPos.y + 5, 3, GREEN);
        }
    }
    
    if (building.level >= 3) {
        // Porch
        Vector2 porchPos = {position.x - 5, position.y + scaledSize.y - 10};
        DrawRectangle((int)porchPos.x, (int)porchPos.y, (int)scaledSize.x + 10, 10, visuals.secondaryColor);
    }
    
    if (building.level >= 4) {
        // Chimney
        Vector2 chimneyPos = {position.x + scaledSize.x - 15, position.y - 15};
        DrawRectangle((int)chimneyPos.x, (int)chimneyPos.y, 8, 20, DARKGRAY);
    }
    
    if (building.level >= 5) {
        // Decorative elements
        Vector2 decorPos = {position.x + scaledSize.x/2, position.y - 10};
        DrawCircle((int)decorPos.x, (int)decorPos.y, 5, GOLD);
    }
}// Deco
ration rendering
void BuildingRenderer::renderDecorations(const Building& building, const Vector2& position) {
    std::vector<std::string> decorations = upgradeSystem.getBuildingDecorations(building.id);
    
    int decorationIndex = 0;
    for (const std::string& decorationId : decorations) {
        // Simple positioning - in a real implementation, decorations would have stored positions
        Vector2 decorationPos = {
            position.x + (decorationIndex % 3) * 20,
            position.y + building.size.y + 5 + (decorationIndex / 3) * 15
        };
        
        renderDecoration(decorationId, decorationPos);
        decorationIndex++;
    }
}

void BuildingRenderer::renderDecoration(const std::string& decorationId, const Vector2& position) {
    // Simple decoration rendering based on ID
    if (decorationId == "coffee_plant") {
        DrawCircle((int)position.x, (int)position.y, 8, GREEN);
        DrawRectangle((int)position.x - 2, (int)position.y + 8, 4, 10, BROWN);
    } else if (decorationId == "reading_lamp") {
        DrawRectangle((int)position.x, (int)position.y, 3, 15, DARKGRAY);
        DrawCircle((int)position.x + 1, (int)position.y - 5, 6, YELLOW);
    } else if (decorationId == "motivational_poster") {
        DrawRectangle((int)position.x, (int)position.y, 12, 16, WHITE);
        DrawRectangleLines((int)position.x, (int)position.y, 12, 16, BLACK);
        DrawText("GO!", (int)position.x + 2, (int)position.y + 6, 8, RED);
    } else if (decorationId == "espresso_machine") {
        DrawRectangle((int)position.x, (int)position.y, 10, 12, DARKGRAY);
        DrawCircle((int)position.x + 5, (int)position.y - 2, 3, GRAY);
    } else {
        // Generic decoration
        DrawRectangle((int)position.x, (int)position.y, 8, 8, PURPLE);
    }
}

// Visual effects
void BuildingRenderer::renderUpgradeGlow(const Vector2& position, const Vector2& size, float intensity) {
    Color glowColor = Fade(GOLD, intensity * 0.3f);
    
    // Draw multiple glow layers
    for (int i = 0; i < 3; i++) {
        float expansion = (i + 1) * 5.0f * intensity;
        DrawRectangle(
            (int)(position.x - expansion), 
            (int)(position.y - expansion),
            (int)(size.x + expansion * 2), 
            (int)(size.y + expansion * 2),
            Fade(glowColor, 0.1f / (i + 1))
        );
    }
}

void BuildingRenderer::renderLevelIndicator(const Building& building, const Vector2& position) {
    if (building.level <= 1) return;
    
    // Draw level indicator
    Vector2 indicatorPos = {position.x + building.size.x - 20, position.y - 15};
    
    // Background circle
    DrawCircle((int)indicatorPos.x, (int)indicatorPos.y, 10, GOLD);
    DrawCircleLines((int)indicatorPos.x, (int)indicatorPos.y, 10, BLACK);
    
    // Level number
    std::string levelText = std::to_string(building.level);
    int textWidth = MeasureText(levelText.c_str(), 12);
    DrawText(levelText.c_str(), 
             (int)indicatorPos.x - textWidth/2, 
             (int)indicatorPos.y - 6, 
             12, BLACK);
}

void BuildingRenderer::renderBuildingName(const Building& building, const Vector2& position) {
    // Render building name below the building
    Vector2 namePos = {position.x + building.size.x/2, position.y + building.size.y + 5};
    
    const char* name = building.name.c_str();
    int textWidth = MeasureText(name, 10);
    
    // Background for text
    DrawRectangle((int)namePos.x - textWidth/2 - 2, (int)namePos.y - 1, textWidth + 4, 12, Fade(BLACK, 0.7f));
    
    // Text
    DrawText(name, (int)namePos.x - textWidth/2, (int)namePos.y, 10, WHITE);
}

// Helper methods
void BuildingRenderer::initializeBuildingVisuals(const std::string& buildingId) {
    BuildingVisuals visuals;
    updateBuildingColors(buildingId);
    updateVisualElements(buildingId);
    buildingVisuals[buildingId] = visuals;
}

void BuildingRenderer::updateBuildingColors(const std::string& buildingId) {
    auto it = buildingVisuals.find(buildingId);
    if (it == buildingVisuals.end()) {
        buildingVisuals[buildingId] = BuildingVisuals();
        it = buildingVisuals.find(buildingId);
    }
    
    BuildingUpgradeSystem::BuildingTheme theme = upgradeSystem.getBuildingTheme(buildingId);
    it->second.primaryColor = theme.primaryColor;
    it->second.secondaryColor = theme.secondaryColor;
    it->second.accentColor = theme.accentColor;
}

void BuildingRenderer::updateVisualElements(const std::string& buildingId) {
    auto it = buildingVisuals.find(buildingId);
    if (it != buildingVisuals.end()) {
        it->second.visualElements = upgradeSystem.getBuildingVisualElements(buildingId);
    }
}

// Rendering helpers
void BuildingRenderer::renderBasicBuilding(const Building& building, const Vector2& position, const BuildingVisuals& visuals) {
    Vector2 scaledSize = {building.size.x * buildingScale, building.size.y * buildingScale};
    
    drawBuildingBase(position, scaledSize, visuals.primaryColor);
    drawBuildingRoof(position, scaledSize, visuals.secondaryColor, "flat");
    drawBuildingWindows(position, scaledSize, visuals.accentColor, building.level);
    drawBuildingDoor(position, scaledSize, BROWN);
}

void BuildingRenderer::renderBuildingLevel(const Building& building, const Vector2& position, int level) {
    // This method can be used to render level-specific variations
    // Implementation depends on specific building type
}

void BuildingRenderer::renderVisualElement(const std::string& element, const Vector2& position, const Vector2& size, const BuildingVisuals& visuals) {
    // Render specific visual elements based on upgrade level
    if (element == "improved_exterior") {
        drawBuildingTrim(position, size, visuals.accentColor);
    } else if (element == "better_windows") {
        // Enhanced window rendering is handled in drawBuildingWindows
    } else if (element == "decorative_trim") {
        drawBuildingTrim(position, size, GOLD);
    }
    // Add more visual elements as needed
}

// Particle helpers
Vector2 BuildingRenderer::getRandomVelocity(float speed) const {
    float angle = (rand() % 360) * DEG2RAD;
    return {cos(angle) * speed, sin(angle) * speed};
}

Color BuildingRenderer::getRandomUpgradeColor() const {
    Color colors[] = {GOLD, YELLOW, ORANGE, WHITE, PURPLE};
    return colors[rand() % 5];
}

Color BuildingRenderer::getRandomDecorationColor() const {
    Color colors[] = {GREEN, BLUE, PINK, PURPLE, ORANGE};
    return colors[rand() % 5];
}

void BuildingRenderer::updateParticle(UpgradeParticle& particle, float deltaTime) {
    particle.life -= deltaTime;
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;
    
    // Apply gravity to some particles
    if (particle.type == "sparkle") {
        particle.velocity.y += 50.0f * deltaTime; // Gravity
    }
    
    // Fade size over time
    float lifeRatio = particle.life / particle.maxLife;
    particle.size = 3.0f * lifeRatio;
}

// Effect helpers
void BuildingRenderer::createSparkleEffect(const Vector2& position, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 particlePos = {
            position.x + (rand() % 20 - 10),
            position.y + (rand() % 20 - 10)
        };
        Vector2 velocity = getRandomVelocity(40.0f);
        
        globalParticles.emplace_back(particlePos, velocity, GOLD, PARTICLE_LIFETIME, "sparkle");
    }
}

void BuildingRenderer::createStarEffect(const Vector2& position, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 particlePos = {
            position.x + (rand() % 30 - 15),
            position.y + (rand() % 30 - 15)
        };
        Vector2 velocity = getRandomVelocity(60.0f);
        
        globalParticles.emplace_back(particlePos, velocity, WHITE, PARTICLE_LIFETIME, "star");
    }
}

void BuildingRenderer::createGlowEffect(const Vector2& position, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 particlePos = {
            position.x + (rand() % 40 - 20),
            position.y + (rand() % 40 - 20)
        };
        Vector2 velocity = getRandomVelocity(20.0f);
        
        globalParticles.emplace_back(particlePos, velocity, PURPLE, PARTICLE_LIFETIME * 1.5f, "glow");
    }
}

// Drawing utilities
void BuildingRenderer::drawBuildingBase(const Vector2& position, const Vector2& size, Color color) {
    DrawRectangle((int)position.x, (int)position.y, (int)size.x, (int)size.y, color);
    DrawRectangleLines((int)position.x, (int)position.y, (int)size.x, (int)size.y, BLACK);
}

void BuildingRenderer::drawBuildingRoof(const Vector2& position, const Vector2& size, Color color, const std::string& style) {
    if (style == "peaked") {
        // Triangular roof
        Vector2 peak = {position.x + size.x/2, position.y - 15};
        Vector2 left = {position.x - 5, position.y};
        Vector2 right = {position.x + size.x + 5, position.y};
        
        DrawTriangle(peak, left, right, color);
        DrawTriangleLines(peak, left, right, BLACK);
    } else if (style == "flat") {
        // Flat roof with slight overhang
        DrawRectangle((int)position.x - 3, (int)position.y - 8, (int)size.x + 6, 8, color);
        DrawRectangleLines((int)position.x - 3, (int)position.y - 8, (int)size.x + 6, 8, BLACK);
    }
}

void BuildingRenderer::drawBuildingWindows(const Vector2& position, const Vector2& size, Color color, int level) {
    int windowCount = std::min(level, 4); // More windows at higher levels
    float windowSpacing = size.x / (windowCount + 1);
    
    for (int i = 0; i < windowCount; i++) {
        Vector2 windowPos = {
            position.x + windowSpacing * (i + 1) - 8,
            position.y + size.y * 0.3f
        };
        
        // Window frame
        DrawRectangle((int)windowPos.x, (int)windowPos.y, 16, 20, color);
        DrawRectangleLines((int)windowPos.x, (int)windowPos.y, 16, 20, BLACK);
        
        // Window cross
        DrawLine((int)windowPos.x + 8, (int)windowPos.y, (int)windowPos.x + 8, (int)windowPos.y + 20, BLACK);
        DrawLine((int)windowPos.x, (int)windowPos.y + 10, (int)windowPos.x + 16, (int)windowPos.y + 10, BLACK);
    }
}

void BuildingRenderer::drawBuildingDoor(const Vector2& position, const Vector2& size, Color color) {
    Vector2 doorPos = {position.x + size.x/2 - 10, position.y + size.y - 25};
    
    DrawRectangle((int)doorPos.x, (int)doorPos.y, 20, 25, color);
    DrawRectangleLines((int)doorPos.x, (int)doorPos.y, 20, 25, BLACK);
    
    // Door handle
    DrawCircle((int)doorPos.x + 15, (int)doorPos.y + 12, 2, GOLD);
}

void BuildingRenderer::drawBuildingTrim(const Vector2& position, const Vector2& size, Color color) {
    // Decorative trim around the building
    DrawRectangleLines((int)position.x - 2, (int)position.y - 2, (int)size.x + 4, (int)size.y + 4, color);
    DrawRectangleLines((int)position.x - 1, (int)position.y - 1, (int)size.x + 2, (int)size.y + 2, color);
}