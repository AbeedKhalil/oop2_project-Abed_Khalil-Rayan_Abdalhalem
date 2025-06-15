//#include "FishAnimator.h"
//#include <algorithm>
//#include <stdexcept>
//
//namespace FishGame
//{
//    // Static animation definitions
//    const std::unordered_map<std::string, FishAnimator::AnimationData> FishAnimator::m_animationDefinitions = {
//        // Left-facing animations (original sprites)
//        {"eatLeft", {0, 0, 6, std::chrono::milliseconds(100), AnimationClip::PlayMode::Loop}},
//        {"idleLeft", {1, 0, 6, std::chrono::milliseconds(120), AnimationClip::PlayMode::Loop}},
//        {"swimLeft", {2, 0, 15, std::chrono::milliseconds(80), AnimationClip::PlayMode::Loop}},
//        {"turnLeft?Right", {3, 0, 5, std::chrono::milliseconds(90), AnimationClip::PlayMode::Once}},
//        {"turnRight?Left", {3, 4, 5, std::chrono::milliseconds(90), AnimationClip::PlayMode::Once}}, // Reversed
//    };
//
//    FishAnimator::FishAnimator()
//        : m_texture(nullptr)
//        , m_currentAnimation(nullptr)
//        , m_currentAnimationName("")
//        , m_isPlaying(false)
//        , m_isPaused(false)
//        , m_currentFrameIndex(0)
//        , m_elapsedTime(0)
//        , m_frameElapsedTime(0)
//        , m_onAnimationFinished(nullptr)
//    {
//    }
//
//    bool FishAnimator::initialize(const sf::Texture& spriteSheet)
//    {
//        m_texture = &spriteSheet;
//        m_sprite.setTexture(spriteSheet);
//
//        // Build all animations from the sprite sheet
//        buildAnimations();
//
//        // Start with idle animation
//        play("idleLeft");
//
//        return true;
//    }
//
//    void FishAnimator::buildAnimations()
//    {
//        // Build left-facing animations from definitions
//        for (const auto& [name, data] : m_animationDefinitions)
//        {
//            auto clip = std::make_unique<AnimationClip>(name, data.playMode);
//
//            if (name == "turnRight?Left")
//            {
//                // Special case: reverse frames for right-to-left turn
//                for (int i = data.frameCount - 1; i >= 0; --i)
//                {
//                    int col = i;
//                    int x = col * m_cellWidth + m_gridLineWidth;
//                    int y = data.row * m_cellHeight + m_gridLineWidth;
//
//                    sf::IntRect frameRect(x, y, m_spriteWidth, m_spriteHeight);
//                    clip->addFrame(frameRect, data.frameDuration);
//                }
//            }
//            else
//            {
//                // Normal left-to-right frame order
//                for (int i = 0; i < data.frameCount; ++i)
//                {
//                    int col = data.startFrame + i;
//                    int x = col * m_cellWidth + m_gridLineWidth;
//                    int y = data.row * m_cellHeight + m_gridLineWidth;
//
//                    sf::IntRect frameRect(x, y, m_spriteWidth, m_spriteHeight);
//                    clip->addFrame(frameRect, data.frameDuration);
//                }
//            }
//
//            m_animations[name] = std::move(clip);
//        }
//
//        // Create right-facing animations (horizontally flipped)
//        m_animations["eatRight"] = createFlippedAnimation(*m_animations["eatLeft"], "eatRight");
//        m_animations["idleRight"] = createFlippedAnimation(*m_animations["idleLeft"], "idleRight");
//        m_animations["swimRight"] = createFlippedAnimation(*m_animations["swimLeft"], "swimRight");
//    }
//
//    std::unique_ptr<AnimationClip> FishAnimator::createFlippedAnimation(const AnimationClip& source, const std::string& newName)
//    {
//        auto flipped = std::make_unique<AnimationClip>(newName, source.getPlayMode());
//
//        for (size_t i = 0; i < source.getFrameCount(); ++i)
//        {
//            sf::IntRect rect = source.getFrame(i);
//            // For flipped animations, we keep the same rect but will flip the sprite
//            flipped->addFrame(rect, source.getFrameDuration(i));
//        }
//
//        return flipped;
//    }
//
//    void FishAnimator::play(const std::string& animationName)
//    {
//        auto it = m_animations.find(animationName);
//        if (it == m_animations.end())
//        {
//            throw std::runtime_error("Animation not found: " + animationName);
//        }
//
//        // Reset if switching animations
//        if (m_currentAnimationName != animationName)
//        {
//            m_currentAnimation = it->second.get();
//            m_currentAnimationName = animationName;
//            m_currentFrameIndex = 0;
//            m_frameElapsedTime = std::chrono::milliseconds(0);
//            m_elapsedTime = std::chrono::milliseconds(0);
//
//            // Apply horizontal flip for right-facing animations
//            bool shouldFlip = animationName.find("Right") != std::string::npos;
//            float absScaleX = std::abs(m_sprite.getScale().x);
//            float scaleY = m_sprite.getScale().y;
//
//            if (shouldFlip)
//            {
//                m_sprite.setScale(-absScaleX, scaleY);
//                // Adjust origin for flipped sprite
//                m_sprite.setOrigin(m_spriteWidth, 0);
//            }
//            else
//            {
//                m_sprite.setScale(absScaleX, scaleY);
//                m_sprite.setOrigin(0, 0);
//            }
//
//            updateFrame();
//        }
//
//        m_isPlaying = true;
//        m_isPaused = false;
//    }
//
//    void FishAnimator::stop()
//    {
//        m_isPlaying = false;
//        m_isPaused = false;
//        m_currentFrameIndex = 0;
//        m_frameElapsedTime = std::chrono::milliseconds(0);
//        m_elapsedTime = std::chrono::milliseconds(0);
//        updateFrame();
//    }
//
//    void FishAnimator::pause()
//    {
//        m_isPaused = true;
//    }
//
//    void FishAnimator::resume()
//    {
//        m_isPaused = false;
//    }
//
//    void FishAnimator::update(sf::Time deltaTime)
//    {
//        if (!m_isPlaying || m_isPaused || !m_currentAnimation)
//            return;
//
//        auto deltaMs = std::chrono::milliseconds(deltaTime.asMilliseconds());
//        m_frameElapsedTime += deltaMs;
//        m_elapsedTime += deltaMs;
//
//        // Check if we need to advance to next frame
//        if (m_frameElapsedTime >= m_currentAnimation->getFrameDuration(m_currentFrameIndex))
//        {
//            m_frameElapsedTime = std::chrono::milliseconds(0);
//            m_currentFrameIndex++;
//
//            // Handle animation completion
//            if (m_currentFrameIndex >= m_currentAnimation->getFrameCount())
//            {
//                if (m_currentAnimation->getPlayMode() == AnimationClip::PlayMode::Once)
//                {
//                    m_isPlaying = false;
//                    m_currentFrameIndex = m_currentAnimation->getFrameCount() - 1; // Stay on last frame
//
//                    if (m_onAnimationFinished)
//                    {
//                        m_onAnimationFinished();
//                    }
//                }
//                else // Loop
//                {
//                    m_currentFrameIndex = 0;
//                    m_elapsedTime = std::chrono::milliseconds(0);
//                }
//            }
//
//            updateFrame();
//        }
//    }
//
//    void FishAnimator::updateFrame()
//    {
//        if (!m_currentAnimation || m_currentAnimation->getFrameCount() == 0)
//            return;
//
//        m_sprite.setTextureRect(m_currentAnimation->getFrame(m_currentFrameIndex));
//    }
//
//    void FishAnimator::setPosition(float x, float y)
//    {
//        m_sprite.setPosition(x, y);
//    }
//
//    void FishAnimator::setPosition(const sf::Vector2f& position)
//    {
//        m_sprite.setPosition(position);
//    }
//
//    void FishAnimator::setScale(float scaleX, float scaleY)
//    {
//        // Preserve flip state when setting scale
//        bool isFlipped = m_sprite.getScale().x < 0;
//        float finalScaleX = isFlipped ? -std::abs(scaleX) : std::abs(scaleX);
//        m_sprite.setScale(finalScaleX, scaleY);
//    }
//
//    void FishAnimator::setScale(const sf::Vector2f& scale)
//    {
//        setScale(scale.x, scale.y);
//    }
//
//    void FishAnimator::setOrigin(float x, float y)
//    {
//        // Note: Origin is handled automatically for flipped sprites in play()
//        // This method is for custom origin adjustments
//        m_sprite.setOrigin(x, y);
//    }
//
//    void FishAnimator::setColor(const sf::Color& color)
//    {
//        m_sprite.setColor(color);
//    }
//
//    void FishAnimator::draw(sf::RenderTarget& target, sf::RenderStates states) const
//    {
//        target.draw(m_sprite, states);
//    }
//}