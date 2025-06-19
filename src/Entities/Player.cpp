#include "Player.h"
#include "Fish.h"
#include "BonusItem.h"
#include "PowerUp.h"
#include "SpecialFish.h"
#include "Animator.h"
#include "CollisionDetector.h"
#include "GenericFish.h"
#include "Utils/FishSizeUtils.h"
#include <SFML/Window.hpp>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <numeric>

namespace FishGame
{
    // Static member initialization
    const sf::Time Player::m_invulnerabilityDuration = sf::seconds(2.0f);
    const sf::Time Player::m_damageCooldownDuration = sf::seconds(0.5f);
    const sf::Time Player::m_eatAnimationDuration = sf::seconds(0.3f);
    const sf::Time Player::m_turnAnimationDuration = sf::seconds(0.45f);

    Player::Player()
        : Entity()
        , m_score(0)
        , m_currentStage(1)
        , m_growthProgress(0.0f)
        , m_points(0)
        , m_useMouseControl(false)
        , m_targetPosition(0.0f, 0.0f)
        , m_mouseControlActive(true)
        , m_autoOrient(true)
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
        , m_spriteManager(nullptr)
        , m_invulnerabilityTimer(sf::Time::Zero)
        , m_damageCooldown(sf::Time::Zero)
        , m_speedMultiplier(1.0f)
        , m_speedBoostTimer(sf::Time::Zero)
        , m_windowBounds(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT)
        , m_totalFishEaten(0)
        , m_damageTaken(0)
        , m_activeEffects()
        , m_eatAnimationScale(1.0f)
        , m_eatAnimationTimer(sf::Time::Zero)
        , m_damageFlashColor(sf::Color::White)
        , m_damageFlashIntensity(0.0f)
        , m_animator(nullptr)
        , m_currentAnimation()
        , m_facingRight(false)
        , m_turnAnimationTimer(sf::Time::Zero)
        , m_controlsReversed(false)
        , m_poisonColorTimer(sf::Time::Zero)
    {
        m_radius = m_baseRadius;

        // Start at center of screen
        m_position = sf::Vector2f(m_windowBounds.x / 2.0f, m_windowBounds.y / 2.0f);
        m_targetPosition = m_position;
    }

    void Player::initializeSystems(GrowthMeter* growthMeter, FrenzySystem* frenzySystem,
        PowerUpManager* powerUpManager, ScoreSystem* scoreSystem)
    {
        m_growthMeter = growthMeter;
        m_frenzySystem = frenzySystem;
        m_powerUpManager = powerUpManager;
        m_scoreSystem = scoreSystem;

        if (m_growthMeter)
        {
            m_growthMeter->setStage(m_currentStage);
            m_growthMeter->setOnStageComplete([this]() { checkStageAdvancement(); });
        }
    }

    void Player::initializeSprite(SpriteManager& spriteManager)
    {
        m_spriteManager = &spriteManager;

        const sf::Texture& tex = spriteManager.getTexture(getTextureID());
        m_animator = std::make_unique<Animator>(createFishAnimator(tex));
        m_animator->setPosition(m_position);
        setRenderMode(RenderMode::Sprite);
        m_currentAnimation = "idleLeft";
        m_animator->play(m_currentAnimation);
    }

    TextureID Player::getTextureID() const
    {
        return FishSizeUtils::selectBySize(
            getCurrentFishSize(),
            TextureID::PlayerSmall,
            TextureID::PlayerMedium,
            TextureID::PlayerLarge);
    }

    void Player::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update timers
        updateInvulnerability(deltaTime);
        if (m_damageCooldown > sf::Time::Zero)
            m_damageCooldown -= deltaTime;
        if (m_speedBoostTimer > sf::Time::Zero)
            m_speedBoostTimer -= deltaTime;

        // Handle input
        handleInput();

        // Mouse control movement
        if (m_mouseControlActive)
        {
            // Calculate direction to mouse
            sf::Vector2f direction = m_targetPosition - m_position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > m_mouseDeadzone)
            {
                // Normalize direction
                direction /= distance;

                // Calculate speed based on power-ups
                float effectiveSpeed = m_baseSpeed;
                if (m_speedBoostTimer > sf::Time::Zero)
                    effectiveSpeed *= m_speedMultiplier;

                // Target velocity
                sf::Vector2f targetVelocity = direction * effectiveSpeed;

                // Smooth velocity transition
                m_velocity.x = m_velocity.x + (targetVelocity.x - m_velocity.x) * m_mouseSmoothingFactor;
                m_velocity.y = m_velocity.y + (targetVelocity.y - m_velocity.y) * m_mouseSmoothingFactor;
            }
            else
            {
                // Close to target - decelerate smoothly
                m_velocity *= (1.0f - m_deceleration * deltaTime.asSeconds());

                // Stop completely when very slow
                if (std::abs(m_velocity.x) < 1.0f && std::abs(m_velocity.y) < 1.0f)
                {
                    m_velocity = sf::Vector2f(0.0f, 0.0f);
                }
            }
        }

        // Limit maximum speed
        float maxSpeed = m_maxSpeed * (m_speedBoostTimer > sf::Time::Zero ? m_speedMultiplier : 1.0f);
        float currentSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (currentSpeed > maxSpeed)
        {
            m_velocity = (m_velocity / currentSpeed) * maxSpeed;
        }

        // Update position
        updateMovement(deltaTime);

        // Keep player within window bounds
        constrainToWindow();

        // Auto-orient sprite based on velocity
        if (m_autoOrient && m_animator && m_renderMode == RenderMode::Sprite && m_eatAnimationTimer <= sf::Time::Zero)
        {
            if (std::abs(m_velocity.x) > m_orientationThreshold)
            {
                bool newFacingRight = m_velocity.x > 0;
                if (newFacingRight != m_facingRight)
                {
                    m_facingRight = newFacingRight;
                    m_turnAnimationTimer = m_turnAnimationDuration;
                    std::string turnAnim = m_facingRight ? "turnLeftToRight" : "turnRightToLeft";
                    m_animator->play(turnAnim);
                    m_currentAnimation = turnAnim;
                }
            }
        }

        // Rest of the update logic remains the same...
        checkStageAdvancement();
        updateVisualEffects(deltaTime);

        if (m_renderMode == RenderMode::Sprite && m_animator)
        {
            m_animator->update(deltaTime);

            float stageScale = 1.0f;
            if (m_spriteManager)
            {
                const auto& cfg = m_spriteManager->getScaleConfig();
                stageScale = FishSizeUtils::valueFromConfig(cfg, getCurrentFishSize());
                if (getCurrentFishSize() == FishSize::Medium)
                    stageScale += 0.18f;
                else if (getCurrentFishSize() == FishSize::Large)
                    stageScale += 0.4f;
            }

            m_animator->setScale(sf::Vector2f(stageScale * m_eatAnimationScale,
                stageScale * m_eatAnimationScale));

            std::string desired;
            float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
            if (speed > 10.f)
                desired = m_facingRight ? "swimRight" : "swimLeft";
            else
                desired = m_facingRight ? "idleRight" : "idleLeft";

            if (m_turnAnimationTimer <= sf::Time::Zero && m_eatAnimationTimer <= sf::Time::Zero && desired != m_currentAnimation)
            {
                m_animator->play(desired);
                m_currentAnimation = desired;
            }

            m_animator->setPosition(m_position);
        }
    }

    void Player::handleInput()
    {
        static const std::unordered_map<sf::Keyboard::Key, sf::Vector2f> keyMap = {
            {sf::Keyboard::W, {0.f, -1.f}}, {sf::Keyboard::Up, {0.f, -1.f}},
            {sf::Keyboard::S, {0.f, 1.f}},  {sf::Keyboard::Down, {0.f, 1.f}},
            {sf::Keyboard::A, {-1.f, 0.f}}, {sf::Keyboard::Left, {-1.f, 0.f}},
            {sf::Keyboard::D, {1.f, 0.f}},  {sf::Keyboard::Right, {1.f, 0.f}}
        };

        sf::Vector2f inputDirection{ 0.f, 0.f };
        if (!m_pressedKeys.empty())
        {
            for (const auto& key : m_pressedKeys)
            {
                auto it = keyMap.find(key);
                if (it != keyMap.end())
                {
                    inputDirection += it->second;
                }
            }
        }
        else
        {
            // Fallback to real-time polling if no events were captured
            for (const auto& [key, dir] : keyMap)
            {
                if (sf::Keyboard::isKeyPressed(key))
                {
                    inputDirection += dir;
                }
            }
        }

        if (m_controlsReversed)
        {
            inputDirection = -inputDirection;
        }

        bool keyboardUsed = (inputDirection.x != 0.f || inputDirection.y != 0.f);

        if (keyboardUsed)
        {
            m_mouseControlActive = false;
            float length = std::sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
            if (length > 0.f)
            {
                inputDirection /= length;
                float speed = m_baseSpeed * (m_speedBoostTimer > sf::Time::Zero ? m_speedMultiplier : 1.0f);
                m_velocity = inputDirection * speed;
            }
        }
        else if (!m_mouseControlActive)
        {
            m_velocity *= 0.9f;
        }
    }

    void Player::onKeyPressed(sf::Keyboard::Key key)
    {
        m_pressedKeys.insert(key);
    }

    void Player::onKeyReleased(sf::Keyboard::Key key)
    {
        m_pressedKeys.erase(key);
    }

    void Player::followMouse(const sf::Vector2f& mousePosition)
    {
        m_targetPosition = mousePosition;
    }

sf::FloatRect Player::getBounds() const
{
    return EntityUtils::makeBounds(m_position, m_radius);
}

    void Player::grow(int scoreValue)
    {
        // Growth points for visual growth
        float growthPoints = 0.0f;

        if (scoreValue <= 3)
            growthPoints = m_tinyFishGrowth;
        else if (scoreValue <= 6)
            growthPoints = m_smallFishGrowth;
        else if (scoreValue <= 9)
            growthPoints = m_mediumFishGrowth;
        else
            growthPoints = static_cast<float>(scoreValue);

        m_growthProgress += growthPoints;

        if (m_growthMeter)
        {
            m_growthMeter->setPoints(m_points);
        }

        triggerEatEffect();
    }

    void Player::addPoints(int points)
    {
        m_points += points;

        if (m_growthMeter)
        {
            m_growthMeter->setPoints(m_points);
        }
    }

    void Player::checkStageAdvancement()
    {
        if (m_currentStage == 1 && m_points >= Constants::POINTS_FOR_STAGE_2)
        {
            m_currentStage = 2;
            updateStage();
        }
        else if (m_currentStage == 2 && m_points >= Constants::POINTS_FOR_STAGE_3)
        {
            m_currentStage = 3;
            updateStage();
        }
    }

    void Player::resetSize()
    {
        m_score = 0;
        m_currentStage = 1;
        m_growthProgress = 0.0f;
        m_radius = m_baseRadius;

        if (m_growthMeter)
        {
            m_growthMeter->reset();
            m_growthMeter->setStage(1);
        }

        updateStage();
    }

    void Player::fullReset()
    {
        resetSize();
        m_points = 0;
        m_totalFishEaten = 0;
        m_damageTaken = 0;
        m_controlsReversed = false;
        m_poisonColorTimer = sf::Time::Zero;
    }

    void Player::enableMouseControl(bool enable)
    {
        m_mouseControlActive = enable;
    }

    void Player::setMousePosition(const sf::Vector2f& screenPos)
    {
        if (!m_mouseControlActive)
            return;

        m_targetPosition = screenPos;
    }

    bool Player::canEat(const Entity& other) const
    {
        if (m_invulnerabilityTimer > sf::Time::Zero)
            return false;

        EntityType otherType = other.getType();
        if (otherType == EntityType::SmallFish ||
            otherType == EntityType::MediumFish ||
            otherType == EntityType::LargeFish)
        {
            const Fish* fish = dynamic_cast<const Fish*>(&other);
            if (!fish)
                return false;

            FishSize playerSize = getCurrentFishSize();
            FishSize fishSize = fish->getSize();

            return static_cast<int>(playerSize) >= static_cast<int>(fishSize);
        }

        return false;
    }

    bool Player::attemptEat(Entity& other)
    {
        if (!canEat(other))
            return false;

        // Only eat if the target intersects with the player's mouth area
        sf::Vector2f mouthOffset(m_radius, 0.0f);
        mouthOffset.x = m_facingRight ? mouthOffset.x : -mouthOffset.x;

        sf::Vector2f mouthPos = m_position + mouthOffset * 0.8f;
        float mouthRadius = m_radius * 0.5f;

        float distance = CollisionDetector::getDistance(mouthPos, other.getPosition());
        if (distance > mouthRadius + other.getRadius())
            return false;

        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (fish)
        {
            int pointsToAdd = 0;

            // Determine points based on fish type
            if (dynamic_cast<const SmallFish*>(fish))
            {
                pointsToAdd = Constants::SMALL_FISH_POINTS;
            }
            else if (dynamic_cast<const MediumFish*>(fish))
            {
                pointsToAdd = Constants::MEDIUM_FISH_POINTS;
            }
            else if (dynamic_cast<const LargeFish*>(fish))
            {
                pointsToAdd = Constants::LARGE_FISH_POINTS;
            }
            else if (dynamic_cast<const Barracuda*>(fish))
            {
                pointsToAdd = Constants::BARRACUDA_POINTS;
            }
            else if (dynamic_cast<const Pufferfish*>(fish))
            {
                pointsToAdd = Constants::PUFFERFISH_POINTS;
            }
            else if (dynamic_cast<const Angelfish*>(fish))
            {
                pointsToAdd = Constants::ANGELFISH_POINTS;
            }

            // Add points
            addPoints(pointsToAdd);
            m_totalFishEaten++;

            // Visual growth
            grow(fish->getPointValue());

            // Register hit for chain bonus
            if (m_scoreSystem)
            {
                m_scoreSystem->registerHit();

                int frenzyMultiplier = m_frenzySystem ? m_frenzySystem->getMultiplier() : 1;
                float powerUpMultiplier = m_powerUpManager ? m_powerUpManager->getScoreMultiplier() : 1.0f;

                m_scoreSystem->addScore(ScoreEventType::FishEaten, fish->getPointValue(),
                    other.getPosition(), frenzyMultiplier, powerUpMultiplier);
            }

            if (m_frenzySystem)
            {
                m_frenzySystem->registerFishEaten();
            }

            if (m_animator)
            {
                std::string eatAnim = m_facingRight ? "eatRight" : "eatLeft";
                m_animator->play(eatAnim);
                m_currentAnimation = eatAnim;
            }

            return true;
        }

        return false;
    }

    bool Player::canTailBite(const Entity& other) const
    {
        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (!fish)
            return false;

        int sizeDifference = static_cast<int>(fish->getSize()) - static_cast<int>(getCurrentFishSize());
        return sizeDifference >= 2;
    }

    bool Player::attemptTailBite(Entity& other)
    {
        if (canTailBite(other) && !hasRecentlyTakenDamage())
        {
            sf::Vector2f fishPos = other.getPosition();
            sf::Vector2f fishVelocity = other.getVelocity();

            sf::Vector2f tailOffset = -fishVelocity;
            float length = std::sqrt(tailOffset.x * tailOffset.x + tailOffset.y * tailOffset.y);
            if (length > 0)
            {
                tailOffset = (tailOffset / length) * other.getRadius() * 0.8f;
                sf::Vector2f tailPos = fishPos + tailOffset;

                float distance = std::sqrt(std::pow(m_position.x - tailPos.x, 2) +
                    std::pow(m_position.y - tailPos.y, 2));

                if (distance < m_radius + 10.0f)
                {
                    if (m_scoreSystem)
                    {
                        int frenzyMultiplier = m_frenzySystem ? m_frenzySystem->getMultiplier() : 1;
                        float powerUpMultiplier = m_powerUpManager ? m_powerUpManager->getScoreMultiplier() : 1.0f;

                        m_scoreSystem->registerTailBite(m_position, frenzyMultiplier, powerUpMultiplier);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    FishSize Player::getCurrentFishSize() const
    {
        switch (m_currentStage)
        {
        case 1:
            return FishSize::Small;
        case 2:
            return FishSize::Medium;
        case 3:
            return FishSize::Large;
        default:
            return FishSize::Small;
        }
    }

    void Player::takeDamage()
    {
        if (m_invulnerabilityTimer > sf::Time::Zero)
            return;

        m_damageTaken++;
        m_damageCooldown = m_damageCooldownDuration;

        if (m_scoreSystem)
        {
            m_scoreSystem->registerMiss();
        }

        triggerDamageEffect();
    }

    void Player::die()
    {
        m_position = sf::Vector2f(m_windowBounds.x / 2.0f, m_windowBounds.y / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_targetPosition = m_position;

        m_invulnerabilityTimer = m_invulnerabilityDuration;

        m_growthProgress = std::max(0.0f, m_growthProgress - 20.0f);

        if (m_growthMeter)
        {
            m_growthMeter->setPoints(m_points);
        }

        m_eatAnimationScale = 1.0f;
        m_damageFlashIntensity = 0.0f;
        m_controlsReversed = false;
        m_poisonColorTimer = sf::Time::Zero;
    }

    void Player::respawn()
    {
        m_isAlive = true;
        m_position = sf::Vector2f(m_windowBounds.x / 2.0f, m_windowBounds.y / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_targetPosition = m_position;
        m_invulnerabilityTimer = m_invulnerabilityDuration;
        m_controlsReversed = false;
        m_poisonColorTimer = sf::Time::Zero;
    }

    void Player::applySpeedBoost(float multiplier, sf::Time duration)
    {
        m_speedMultiplier = multiplier;
        m_speedBoostTimer = duration;
    }

    void Player::applyPoisonEffect(sf::Time duration)
    {
        m_poisonColorTimer = duration;
        m_controlsReversed = true;
    }

    void Player::triggerEatEffect()
    {
        m_eatAnimationScale = 1.3f;
        m_eatAnimationTimer = m_eatAnimationDuration;

        if (m_animator)
        {
            std::string eatAnim = m_facingRight ? "eatRight" : "eatLeft";
            m_animator->play(eatAnim);
            m_currentAnimation = eatAnim;
        }

        m_activeEffects.push_back({
            1.2f,
            0.0f,
            sf::Color::Green,
            sf::seconds(0.2f)
            });
    }

    void Player::triggerDamageEffect()
    {
        m_damageFlashIntensity = 1.0f;
        m_damageFlashColor = sf::Color::Red;

        m_activeEffects.push_back({
            0.8f,
            15.0f,
            sf::Color::Red,
            sf::seconds(0.3f)
            });
    }

    void Player::setWindowBounds(const sf::Vector2u& windowSize)
    {
        m_windowBounds = windowSize;
    }

    void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        std::for_each(m_activeEffects.begin(), m_activeEffects.end(),
            [&](const VisualEffect& effect)
            {
                if (effect.duration > sf::Time::Zero)
                {
                    sf::Transform effectTransform;
                    effectTransform.translate(m_position);
                    effectTransform.scale(effect.scale, effect.scale);
                    effectTransform.rotate(effect.rotation);
                    effectTransform.translate(-m_position);

                    states.transform *= effectTransform;
                }
            });

        if (m_animator)
        {
            target.draw(*m_animator, states);
        }
    }

    void Player::updateStage()
    {
        m_radius = m_baseRadius * std::pow(m_growthFactor, m_currentStage - 1);

        if (m_growthMeter)
        {
            m_growthMeter->setStage(m_currentStage);
        }

        if (m_animator && m_renderMode == RenderMode::Sprite && m_spriteManager)
        {
            float stageScale = 1.0f;
            const auto& cfg = m_spriteManager->getScaleConfig();
            stageScale = FishSizeUtils::valueFromConfig(cfg, getCurrentFishSize());
            if (getCurrentFishSize() == FishSize::Medium)
                stageScale += 0.18f;
            else if (getCurrentFishSize() == FishSize::Large)
                stageScale += 0.4f;
            
            m_animator->setScale(sf::Vector2f(stageScale, stageScale));
        }

        m_activeEffects.push_back({
            1.5f,
            0.0f,
            sf::Color::Cyan,
            sf::seconds(0.5f)
            });
    }

    void Player::constrainToWindow()
    {
        m_position.x = std::clamp(m_position.x, m_radius,
            static_cast<float>(m_windowBounds.x) - m_radius);
        m_position.y = std::clamp(m_position.y, m_radius,
            static_cast<float>(m_windowBounds.y) - m_radius);
    }

    void Player::updateInvulnerability(sf::Time deltaTime)
    {
        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            m_invulnerabilityTimer -= deltaTime;
            if (m_invulnerabilityTimer < sf::Time::Zero)
            {
                m_invulnerabilityTimer = sf::Time::Zero;
            }
        }
    }

    void Player::updateVisualEffects(sf::Time deltaTime)
    {
        if (m_eatAnimationTimer > sf::Time::Zero)
        {
            m_eatAnimationTimer -= deltaTime;
            if (m_eatAnimationTimer < sf::Time::Zero)
                m_eatAnimationTimer = sf::Time::Zero;
        }

        if (m_turnAnimationTimer > sf::Time::Zero)
        {
            m_turnAnimationTimer -= deltaTime;
            if (m_turnAnimationTimer < sf::Time::Zero)
                m_turnAnimationTimer = sf::Time::Zero;
        }

        if (m_eatAnimationScale > 1.0f)
        {
            m_eatAnimationScale -= m_eatAnimationSpeed * deltaTime.asSeconds();
            m_eatAnimationScale = std::max(1.0f, m_eatAnimationScale);
        }

        if (m_damageFlashIntensity > 0.0f)
        {
            m_damageFlashIntensity -= 3.0f * deltaTime.asSeconds();
            m_damageFlashIntensity = std::max(0.0f, m_damageFlashIntensity);
        }

        if (m_poisonColorTimer > sf::Time::Zero)
        {
            m_poisonColorTimer -= deltaTime;
            if (m_poisonColorTimer <= sf::Time::Zero)
            {
                m_poisonColorTimer = sf::Time::Zero;
                m_controlsReversed = false;
            }
        }

        std::for_each(m_activeEffects.begin(), m_activeEffects.end(),
            [deltaTime](VisualEffect& effect) {
                effect.duration -= deltaTime;
            });

        m_activeEffects.erase(
            std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
                [](const VisualEffect& effect) { return effect.duration <= sf::Time::Zero; }),
            m_activeEffects.end()
        );

        sf::Color currentColor = sf::Color::White;

        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            float alpha = std::sin(m_invulnerabilityTimer.asSeconds() * 10.0f) * 0.5f + 0.5f;
            currentColor.a = static_cast<sf::Uint8>(255 * alpha);
        }
        else if (m_damageFlashIntensity > 0.0f)
        {
            currentColor.r = static_cast<sf::Uint8>(currentColor.r + (m_damageFlashColor.r - currentColor.r) * m_damageFlashIntensity);
            currentColor.g = static_cast<sf::Uint8>(currentColor.g + (m_damageFlashColor.g - currentColor.g) * m_damageFlashIntensity);
            currentColor.b = static_cast<sf::Uint8>(currentColor.b + (m_damageFlashColor.b - currentColor.b) * m_damageFlashIntensity);
        }
        else if (m_poisonColorTimer > sf::Time::Zero)
        {
            currentColor = sf::Color(50, 255, 50);
        }
        else
        {
            currentColor.a = 255;
        }

        if (m_animator)
            m_animator->setColor(currentColor);
    }
}