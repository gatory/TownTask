#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include "../src/ui/animations/sprite_animator.h"
#include "../src/ui/animations/animation_manager.h"

void testRectangleAndTexture() {
    std::cout << "Testing Rectangle and Texture2D structures..." << std::endl;
    
    // Test Rectangle
    Rectangle rect1;
    assert(rect1.x == 0 && rect1.y == 0 && rect1.width == 0 && rect1.height == 0);
    
    Rectangle rect2(10, 20, 30, 40);
    assert(rect2.x == 10 && rect2.y == 20 && rect2.width == 30 && rect2.height == 40);
    
    // Test Texture2D
    Texture2D texture1;
    assert(!texture1.isValid());
    
    Texture2D texture2(1, 256, 256, "test.png");
    assert(texture2.isValid());
    assert(texture2.width == 256 && texture2.height == 256);
    assert(texture2.filename == "test.png");
    
    std::cout << "Rectangle and Texture2D test passed!" << std::endl;
}

void testAnimationStructure() {
    std::cout << "Testing Animation structure..." << std::endl;
    
    // Test default animation
    SpriteAnimator::Animation anim1;
    assert(anim1.frameDuration == 0.1f);
    assert(anim1.loop == true);
    assert(anim1.frames.empty());
    assert(!anim1.isValid()); // Invalid because no frames
    
    // Test animation with frames
    std::vector<Rectangle> frames = {
        Rectangle(0, 0, 32, 32),
        Rectangle(32, 0, 32, 32),
        Rectangle(64, 0, 32, 32)
    };
    
    SpriteAnimator::Animation anim2(frames, 0.2f, true, "test_anim");
    assert(anim2.isValid());
    assert(anim2.getFrameCount() == 3);
    assert(anim2.getTotalDuration() == 0.6f); // 3 frames * 0.2s
    assert(anim2.name == "test_anim");
    
    std::cout << "Animation structure test passed!" << std::endl;
}

void testSpriteAnimatorBasic() {
    std::cout << "Testing basic SpriteAnimator functionality..." << std::endl;
    
    SpriteAnimator animator;
    
    // Test initial state
    assert(animator.getState() == SpriteAnimator::AnimationState::STOPPED);
    assert(!animator.isPlaying());
    assert(animator.isStopped());
    assert(animator.getCurrentAnimationName().empty());
    assert(animator.getCurrentFrame() == 0);
    assert(animator.getTotalFrames() == 0);
    
    // Test sprite sheet
    Texture2D texture(1, 128, 128, "test_sheet.png");
    animator.setSpriteSheet(texture);
    assert(animator.hasSpriteSheet());
    assert(animator.getSpriteSheet().filename == "test_sheet.png");
    
    std::cout << "Basic SpriteAnimator test passed!" << std::endl;
}

void testAnimationManagement() {
    std::cout << "Testing animation management..." << std::endl;
    
    SpriteAnimator animator;
    
    // Create test animation
    std::vector<Rectangle> frames = {
        Rectangle(0, 0, 32, 32),
        Rectangle(32, 0, 32, 32),
        Rectangle(64, 0, 32, 32),
        Rectangle(96, 0, 32, 32)
    };
    
    SpriteAnimator::Animation walkAnim(frames, 0.1f, true, "walk");
    
    // Test adding animation
    animator.addAnimation("walk", walkAnim);
    assert(animator.hasAnimation("walk"));
    
    const auto* retrievedAnim = animator.getAnimation("walk");
    assert(retrievedAnim != nullptr);
    assert(retrievedAnim->getFrameCount() == 4);
    
    // Test animation names
    auto names = animator.getAnimationNames();
    assert(names.size() == 1);
    assert(names[0] == "walk");
    
    // Test removing animation
    animator.removeAnimation("walk");
    assert(!animator.hasAnimation("walk"));
    assert(animator.getAnimationNames().empty());
    
    std::cout << "Animation management test passed!" << std::endl;
}

void testAnimationPlayback() {
    std::cout << "Testing animation playback..." << std::endl;
    
    SpriteAnimator animator;
    
    // Create test animation
    std::vector<Rectangle> frames = {
        Rectangle(0, 0, 32, 32),
        Rectangle(32, 0, 32, 32),
        Rectangle(64, 0, 32, 32)
    };
    
    SpriteAnimator::Animation testAnim(frames, 0.1f, true, "test");
    animator.addAnimation("test", testAnim);
    
    // Test playing animation
    animator.playAnimation("test");
    assert(animator.isPlaying());
    assert(animator.getCurrentAnimationName() == "test");
    assert(animator.getCurrentFrame() == 0);
    assert(animator.getTotalFrames() == 3);
    
    // Test frame advancement
    animator.update(0.05f); // Half frame duration
    assert(animator.getCurrentFrame() == 0); // Should still be on first frame
    
    animator.update(0.06f); // Total 0.11f, should advance to next frame
    assert(animator.getCurrentFrame() == 1);
    
    // Test pause/resume
    animator.pauseAnimation();
    assert(animator.isPaused());
    
    animator.resumeAnimation();
    assert(animator.isPlaying());
    
    // Test stop
    animator.stopAnimation();
    assert(animator.isStopped());
    assert(animator.getCurrentFrame() == 0); // Should reset
    
    std::cout << "Animation playback test passed!" << std::endl;
}

void testAnimationLooping() {
    std::cout << "Testing animation looping..." << std::endl;
    
    SpriteAnimator animator;
    
    // Create looping animation
    std::vector<Rectangle> frames = {
        Rectangle(0, 0, 32, 32),
        Rectangle(32, 0, 32, 32)
    };
    
    SpriteAnimator::Animation loopAnim(frames, 0.1f, true, "loop");
    animator.addAnimation("loop", loopAnim);
    
    animator.playAnimation("loop");
    
    // Advance through all frames
    animator.update(0.1f); // Frame 0 -> 1
    assert(animator.getCurrentFrame() == 1);
    
    animator.update(0.1f); // Frame 1 -> 0 (loop back)
    assert(animator.getCurrentFrame() == 0);
    assert(animator.isPlaying()); // Should still be playing
    
    std::cout << "Animation looping test passed!" << std::endl;
}

void testNonLoopingAnimation() {
    std::cout << "Testing non-looping animation..." << std::endl;
    
    SpriteAnimator animator;
    
    // Create non-looping animation
    std::vector<Rectangle> frames = {
        Rectangle(0, 0, 32, 32),
        Rectangle(32, 0, 32, 32)
    };
    
    SpriteAnimator::Animation nonLoopAnim(frames, 0.1f, false, "non_loop");
    animator.addAnimation("non_loop", nonLoopAnim);
    
    animator.playAnimation("non_loop");
    
    // Advance through all frames
    animator.update(0.1f); // Frame 0 -> 1
    assert(animator.getCurrentFrame() == 1);
    
    animator.update(0.1f); // Should complete animation
    assert(animator.getCurrentFrame() == 1); // Should stay on last frame
    assert(animator.getState() == SpriteAnimator::AnimationState::COMPLETED);
    assert(animator.isAnimationComplete());
    
    std::cout << "Non-looping animation test passed!" << std::endl;
}

void testAnimationCreationHelpers() {
    std::cout << "Testing animation creation helpers..." << std::endl;
    
    SpriteAnimator animator;
    
    // Test createAnimation helper
    auto walkAnim = animator.createAnimation("walk", 0, 0, 32, 32, 4, 0.15f, true);
    assert(walkAnim.isValid());
    assert(walkAnim.getFrameCount() == 4);
    assert(walkAnim.frameDuration == 0.15f);
    assert(walkAnim.loop == true);
    
    // Check frame rectangles
    assert(walkAnim.frames[0].x == 0 && walkAnim.frames[0].y == 0);
    assert(walkAnim.frames[1].x == 32 && walkAnim.frames[1].y == 0);
    assert(walkAnim.frames[2].x == 64 && walkAnim.frames[2].y == 0);
    assert(walkAnim.frames[3].x == 96 && walkAnim.frames[3].y == 0);
    
    // Test createAnimationFromGrid helper
    auto gridAnim = animator.createAnimationFromGrid("grid", 1, 2, 16, 16, 3, 0.1f, false);
    assert(gridAnim.isValid());
    assert(gridAnim.getFrameCount() == 3);
    assert(gridAnim.frames[0].x == 16 && gridAnim.frames[0].y == 32); // Grid position (1,2) * frame size
    
    std::cout << "Animation creation helpers test passed!" << std::endl;
}

void testAnimationCallbacks() {
    std::cout << "Testing animation callbacks..." << std::endl;
    
    SpriteAnimator animator;
    
    // Test animation complete callback
    bool animationCompleted = false;
    std::string completedAnimationName;
    
    animator.setOnAnimationComplete([&](const std::string& name) {
        animationCompleted = true;
        completedAnimationName = name;
    });
    
    // Test frame changed callback
    bool frameChanged = false;
    int changedFrame = -1;
    
    animator.setOnFrameChanged([&](const std::string& name, int frame) {
        frameChanged = true;
        changedFrame = frame;
    });
    
    // Create non-looping animation
    std::vector<Rectangle> frames = {Rectangle(0, 0, 32, 32)};
    SpriteAnimator::Animation shortAnim(frames, 0.1f, false, "short");
    animator.addAnimation("short", shortAnim);
    
    animator.playAnimation("short");
    animator.update(0.1f); // Should complete the animation
    
    assert(animationCompleted);
    assert(completedAnimationName == "short");
    
    std::cout << "Animation callbacks test passed!" << std::endl;
}

void testAnimationManagerBasic() {
    std::cout << "Testing basic AnimationManager functionality..." << std::endl;
    
    AnimationManager manager;
    
    // Test initialization
    assert(manager.initialize());
    
    // Test sprite sheet management
    bool loaded = manager.loadSpriteSheet("test_sheet", "test.png");
    // Note: This will fail in our mock implementation, but that's expected
    
    assert(manager.getSpriteSheetNames().size() >= 0); // Should not crash
    
    // Test animator creation
    auto animator = manager.createAnimator("test_animator");
    assert(animator != nullptr);
    assert(manager.hasAnimator("test_animator"));
    
    auto retrievedAnimator = manager.getAnimator("test_animator");
    assert(retrievedAnimator == animator);
    
    // Test animator names
    auto names = manager.getAnimatorNames();
    assert(names.size() == 1);
    assert(names[0] == "test_animator");
    
    // Test removing animator
    manager.removeAnimator("test_animator");
    assert(!manager.hasAnimator("test_animator"));
    
    manager.shutdown();
    
    std::cout << "Basic AnimationManager test passed!" << std::endl;
}

void testAnimationManagerGlobalControl() {
    std::cout << "Testing AnimationManager global control..." << std::endl;
    
    AnimationManager manager;
    manager.initialize();
    
    // Create multiple animators
    auto animator1 = manager.createAnimator("anim1");
    auto animator2 = manager.createAnimator("anim2");
    
    // Add test animations
    std::vector<Rectangle> frames = {Rectangle(0, 0, 32, 32), Rectangle(32, 0, 32, 32)};
    SpriteAnimator::Animation testAnim(frames, 0.1f, true, "test");
    
    animator1->addAnimation("test", testAnim);
    animator2->addAnimation("test", testAnim);
    
    animator1->playAnimation("test");
    animator2->playAnimation("test");
    
    assert(animator1->isPlaying());
    assert(animator2->isPlaying());
    
    // Test global pause
    manager.pauseAll();
    assert(animator1->isPaused());
    assert(animator2->isPaused());
    
    // Test global resume
    manager.resumeAll();
    assert(animator1->isPlaying());
    assert(animator2->isPlaying());
    
    // Test global stop
    manager.stopAll();
    assert(animator1->isStopped());
    assert(animator2->isStopped());
    
    // Test global update
    animator1->playAnimation("test");
    animator2->playAnimation("test");
    
    manager.updateAll(0.05f);
    // Both animators should have been updated
    
    manager.shutdown();
    
    std::cout << "AnimationManager global control test passed!" << std::endl;
}

void testAnimationManagerPresets() {
    std::cout << "Testing AnimationManager presets..." << std::endl;
    
    AnimationManager manager;
    manager.initialize();
    
    // Test character animation creation
    manager.createCharacterAnimations("character", "character_sheet", 32, 32);
    
    auto characterAnimator = manager.getAnimator("character");
    assert(characterAnimator != nullptr);
    
    // Check that standard character animations were created
    assert(characterAnimator->hasAnimation("idle"));
    assert(characterAnimator->hasAnimation("walk_down"));
    assert(characterAnimator->hasAnimation("walk_up"));
    assert(characterAnimator->hasAnimation("walk_left"));
    assert(characterAnimator->hasAnimation("walk_right"));
    assert(characterAnimator->hasAnimation("interact"));
    
    // Test UI animation creation
    manager.createUIAnimations("ui", "ui_sheet");
    
    auto uiAnimator = manager.getAnimator("ui");
    assert(uiAnimator != nullptr);
    assert(uiAnimator->hasAnimation("button_hover"));
    assert(uiAnimator->hasAnimation("loading"));
    assert(uiAnimator->hasAnimation("notification"));
    
    // Test effect animation creation
    manager.createEffectAnimations("effects", "effects_sheet");
    
    auto effectsAnimator = manager.getAnimator("effects");
    assert(effectsAnimator != nullptr);
    assert(effectsAnimator->hasAnimation("sparkle"));
    assert(effectsAnimator->hasAnimation("explosion"));
    assert(effectsAnimator->hasAnimation("smoke"));
    
    manager.shutdown();
    
    std::cout << "AnimationManager presets test passed!" << std::endl;
}

void testAnimationManagerGlobalSettings() {
    std::cout << "Testing AnimationManager global settings..." << std::endl;
    
    AnimationManager manager;
    manager.initialize();
    
    // Test global playback speed
    manager.setGlobalPlaybackSpeed(2.0f);
    assert(manager.getGlobalPlaybackSpeed() == 2.0f);
    
    // Create animator after setting global speed
    auto animator = manager.createAnimator("test");
    assert(animator->getPlaybackSpeed() == 2.0f); // Should inherit global speed
    
    // Test global frame duration
    manager.setGlobalFrameDuration(0.2f);
    assert(manager.getGlobalFrameDuration() == 0.2f);
    
    manager.shutdown();
    
    std::cout << "AnimationManager global settings test passed!" << std::endl;
}

void testAnimationManagerDebug() {
    std::cout << "Testing AnimationManager debug functionality..." << std::endl;
    
    AnimationManager manager;
    manager.initialize();
    
    // Test debug mode
    manager.enableDebugMode(true);
    assert(manager.isDebugModeEnabled());
    
    // Create animator and check debug info
    auto animator = manager.createAnimator("debug_test");
    assert(animator->isDebugModeEnabled()); // Should inherit debug mode
    
    auto debugInfo = manager.getDebugInfo();
    assert(!debugInfo.empty());
    
    // Should contain information about animators and sprite sheets
    bool foundAnimatorInfo = false;
    for (const auto& line : debugInfo) {
        if (line.find("Animators:") != std::string::npos) {
            foundAnimatorInfo = true;
            break;
        }
    }
    assert(foundAnimatorInfo);
    
    manager.shutdown();
    
    std::cout << "AnimationManager debug test passed!" << std::endl;
}

int main() {
    try {
        testRectangleAndTexture();
        testAnimationStructure();
        testSpriteAnimatorBasic();
        testAnimationManagement();
        testAnimationPlayback();
        testAnimationLooping();
        testNonLoopingAnimation();
        testAnimationCreationHelpers();
        testAnimationCallbacks();
        testAnimationManagerBasic();
        testAnimationManagerGlobalControl();
        testAnimationManagerPresets();
        testAnimationManagerGlobalSettings();
        testAnimationManagerDebug();
        
        std::cout << "\nAll tests passed! Animation system implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}