//#pragma once
//
//#include <SFML/Graphics.hpp>
//#include <memory>
//#include <unordered_map>
//#include <string>
//#include <chrono>
//#include "AnimationClip.h"
//
//namespace FishGame
//{
//    // Manages sprite sheet animations for the angelfish
//    class FishAnimator : public sf::Drawable
//    {
//    public:
//        FishAnimator();
//        ~FishAnimator() = default;
//
//        // Initialize with sprite sheet texture
//        bool initialize(const sf::Texture& spriteSheet);
//
//        // Animation control
//        void play(const std::string& animationName);
//        void stop();
//        void pause();
//        void resume();
//        bool isPlaying() const { return m_isPlaying; }
//        const std::string& getCurrentAnimation() const { return m_currentAnimationName; }
//
//        // Update animation state
//        void update(sf::Time deltaTime);
//
//        // Position and transform
//        void setPosition(float x, float y);
//        void setPosition(const sf::Vector2f& position);
//        const sf::Vector2f& getPosition() const { return m_sprite.getPosition(); }
//
//        void setScale(float scaleX, float scaleY);
//        void setScale(const sf::Vector2f& scale);
//
//        void setOrigin(float x, float y);
//        void setColor(const sf::Color& color);
//
//        // Get sprite for additional manipulation
//        sf::Sprite& getSprite() { return m_sprite; }
//        const sf::Sprite& getSprite() const { return m_sprite; }
//
//        // Animation finished callback (for play-once animations)
//        using AnimationCallback = std::function<void()>;
//        void setAnimationFinishedCallback(AnimationCallback callback) { m_onAnimationFinished = callback; }
//
//    protected:
//        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
//
//    private:
//        // Build all animation clips from the sprite sheet
//        void buildAnimations();
//
//        // Create flipped version of an animation
//        std::unique_ptr<AnimationClip> createFlippedAnimation(const AnimationClip& source, const std::string& newName);
//
//        // Update current frame based on elapsed time
//        void updateFrame();
//
//    private:
//        sf::Sprite m_sprite;
//        const sf::Texture* m_texture;
//
//        // Animation management
//        std::unordered_map<std::string, std::unique_ptr<AnimationClip>> m_animations;
//        AnimationClip* m_currentAnimation;
//        std::string m_currentAnimationName;
//
//        // Playback state
//        bool m_isPlaying;
//        bool m_isPaused;
//        size_t m_currentFrameIndex;
//        std::chrono::milliseconds m_elapsedTime;
//        std::chrono::milliseconds m_frameElapsedTime;
//
//        // Callback
//        AnimationCallback m_onAnimationFinished;
//
//        // Sprite sheet dimensions
//        static constexpr int m_cellWidth = 133;
//        static constexpr int m_cellHeight = 85;
//        static constexpr int m_spriteWidth = 128;
//        static constexpr int m_spriteHeight = 78;
//        static constexpr int m_gridLineWidth = 1;
//
//        // Animation frame counts and timings
//        struct AnimationData
//        {
//            int row;
//            int startFrame;
//            int frameCount;
//            std::chrono::milliseconds frameDuration;
//            AnimationClip::PlayMode playMode;
//        };
//
//        static const std::unordered_map<std::string, AnimationData> m_animationDefinitions;
//    };
//}