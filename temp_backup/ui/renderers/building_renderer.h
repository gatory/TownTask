#pragma once

#include "../../core/systems/building_upgrade_system.h"
#include "../../core/models/town_state.h"
#include <raylib.h>
#include <vector>
#include <string>
#include <unordered_map>

class BuildingRenderer {
public:
    // Particle effect for upgrades
    struct UpgradeParticle {
        Vector2 position;
        Vector2 velocity;
        Color color;
        float life;
        float maxLife;
        float size;
        std::string type; // "sparkle", "star", "glow"
        
        UpgradeParticle(const Vector2& pos, const Vector2& vel, Color col, float lifetime, const std::string& t)
            : position(pos), velocity(vel), color(col), life(lifetime), maxLife(lifetime), size(3.0f), type(t) {}
    };
    
    // Building visual state
    struct BuildingVisuals {
        Color primaryColor;
        Color secondaryColor;
        Color accentColor;
        std::vector<std::string> visualElements;
        std::vector<UpgradeParticle> particles;
        float upgradeGlowTimer;
        bool showUpgradeGlow;
        
        BuildingVisuals() : upgradeGlowTimer(0.0f), showUpgradeGlow(false) {}
    };

public:
    BuildingRenderer(BuildingUpgradeSystem& upgradeSystem);
    
    // Main rendering
    void renderBuilding(const Building& building, const Vector2& screenPosition);
    void renderBuildingWithUpgrades(const Building& building, const Vector2& screenPosition);
    void renderUpgradeEffects(const Building& building, const Vector2& screenPosition);
    
    // Update system
    void update(float deltaTime);
    void updateBuildingVisuals(const std::string& buildingId);
    void triggerUpgradeAnimation(const std::string& buildingId);
    void triggerDecorationAnimation(const std::string& buildingId, const Vector2& position);
    
    // Visual customization
    void setBuildingScale(float scale);
    float getBuildingScale() const;
    void setShowUpgradeEffects(bool show);
    bool isShowingUpgradeEffects() const;
    void setShowDecorations(bool show);
    bool isShowingDecorations() const;
    
    // Particle system
    void addUpgradeParticles(const Vector2& position, int count = 10);
    void addDecorationParticles(const Vector2& position, int count = 5);
    void updateParticles(float deltaTime);
    void renderParticles();
    
    // Building-specific rendering
    void renderCoffeeShop(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    void renderLibrary(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    void renderGym(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    void renderBulletinBoard(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    void renderHome(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    
    // Decoration rendering
    void renderDecorations(const Building& building, const Vector2& position);
    void renderDecoration(const std::string& decorationId, const Vector2& position);
    
    // Visual effects
    void renderUpgradeGlow(const Vector2& position, const Vector2& size, float intensity);
    void renderLevelIndicator(const Building& building, const Vector2& position);
    void renderBuildingName(const Building& building, const Vector2& position);

private:
    BuildingUpgradeSystem& upgradeSystem;
    
    // Visual state
    std::unordered_map<std::string, BuildingVisuals> buildingVisuals;
    std::vector<UpgradeParticle> globalParticles;
    
    // Settings
    float buildingScale;
    bool showUpgradeEffects;
    bool showDecorations;
    
    // Helper methods
    void initializeBuildingVisuals(const std::string& buildingId);
    void updateBuildingColors(const std::string& buildingId);
    void updateVisualElements(const std::string& buildingId);
    
    // Rendering helpers
    void renderBasicBuilding(const Building& building, const Vector2& position, const BuildingVisuals& visuals);
    void renderBuildingLevel(const Building& building, const Vector2& position, int level);
    void renderVisualElement(const std::string& element, const Vector2& position, const Vector2& size, const BuildingVisuals& visuals);
    
    // Particle helpers
    Vector2 getRandomVelocity(float speed) const;
    Color getRandomUpgradeColor() const;
    Color getRandomDecorationColor() const;
    void updateParticle(UpgradeParticle& particle, float deltaTime);
    
    // Effect helpers
    void createSparkleEffect(const Vector2& position, int count);
    void createStarEffect(const Vector2& position, int count);
    void createGlowEffect(const Vector2& position, int count);
    
    // Drawing utilities
    void drawBuildingBase(const Vector2& position, const Vector2& size, Color color);
    void drawBuildingRoof(const Vector2& position, const Vector2& size, Color color, const std::string& style);
    void drawBuildingWindows(const Vector2& position, const Vector2& size, Color color, int level);
    void drawBuildingDoor(const Vector2& position, const Vector2& size, Color color);
    void drawBuildingTrim(const Vector2& position, const Vector2& size, Color color);
    
    // Constants
    static constexpr float UPGRADE_GLOW_DURATION = 3.0f;
    static constexpr float PARTICLE_LIFETIME = 2.0f;
    static constexpr float DECORATION_PARTICLE_LIFETIME = 1.5f;
    static constexpr int MAX_PARTICLES_PER_BUILDING = 50;
};