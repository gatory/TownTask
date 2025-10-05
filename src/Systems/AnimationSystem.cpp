#include "../include/Systems/AnimationSystem.h"

void AnimationSystem::Update(World& world, float deltaTime) {
    for (Entity e : world.GetEntities()) {
        Animation* anim = world.GetAnimation(e);
        if (!anim) continue;
        
        anim->timer -= deltaTime;
        if (anim->timer <= 0) {
            anim->timer = anim->frameTime;
            anim->currentFrame = (anim->currentFrame + 1) % anim->frameCount;
        }
    }
}
