#include "Player.h"
#include "Fish.h"
#include "BonusItem.h"
#include "PowerUp.h"
#include "SpecialFish.h"
#include "CollisionDetector.h"
#include "GenericFish.h"
#include <SFML/Window.hpp>
#include <cmath>
#include <algorithm>
#include <array>

namespace FishGame
{
    // Static member initialization
    const sf::Time Player::m_invulnerabilityDuration = sf::seconds(2.0f);
    const sf::Time Player::m_damageCooldownDuration = sf::seconds(0.5f);

    Player::Player()
        : Entity()
        , m_shape(m_baseRadius)
        , m_score(0)
        , m_currentStage(1)
        , m_growthProgress(0.0f)
        , m_points(0)
        , m_useMouseControl(false)
        , m_targetPosition(0.0f, 0.0f)
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
        , m_spriteManager(nullptr)
        , m_invulnerabilityTimer(sf::Time::Zero)
        , m_damageCooldown(sf::Time::Zero)
        , m_speedMultiplier(1.0f)
        , m_speedBoostTimer(sf::Time::Zero)
        , m_invincibilityTimer(sf::Time::Zero)
        , m_windowBounds(1920, 1080)
        , m_totalFishEaten(0)
        , m_damageTaken(0)
        , m_activeEffects()
        , m_eatAnimationScale(1.0f)
        , m_damageFlashColor(sf::Color::White)
        , m_damageFlashIntensity(0.0f)
    {
        m_radius = m_baseRadius;
        m_shape.setFillColor(sf::Color::Yellow);
        m_shape.setOutlineColor(sf::Color(255, 200, 0));
        m_shape.setOutlineThickness(2.0f);
        m_shape.setOrigin(m_radius, m_radius);

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

        auto sprite = spriteManager.createSpriteComponent(static_cast<Entity*>(this), getTextureID());
        if (sprite)
        {
            auto playerConfig = spriteManager.getSpriteConfig<Player>(getTextureID(), getCurrentFishSize());

            SpriteConfig<Entity> entityConfig;
            entityConfig.textureName = std::move(playerConfig.textureName);
            entityConfig.baseSize = playerConfig.baseSize;
            entityConfig.origin = playerConfig.origin;
            entityConfig.scaleMultiplier = playerConfig.scaleMultiplier;
            entityConfig.maintainAspectRatio = playerConfig.maintainAspectRatio;
            entityConfig.textureRect = playerConfig.textureRect;
            entityConfig.rotationOffset = playerConfig.rotationOffset;

            sprite->configure(entityConfig);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);
        }
    }

    TextureID Player::getTextureID() const
    {
        switch (getCurrentFishSize())
        {
        case FishSize::Small:
            return TextureID::PlayerSmall;
        case FishSize::Medium:
            return TextureID::PlayerMedium;
        case FishSize::Large:
            return TextureID::PlayerLarge;
        default:
            return TextureID::PlayerSmall;
        }
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
        if (m_invincibilityTimer > sf::Time::Zero)
            m_invincibilityTimer -= deltaTime;

        // Handle input
        handleInput();

        // Calculate effective speed
        float effectiveSpeed = m_baseSpeed;
        if (m_speedBoostTimer > sf::Time::Zero)
            effectiveSpeed *= m_speedMultiplier;

        // Update movement
        if (m_useMouseControl)
        {
            // Smooth movement towards mouse position
            sf::Vector2f direction = m_targetPosition - m_position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 5.0f)
            {
                direction /= distance; // Normalize
                sf::Vector2f targetVelocity = direction * effectiveSpeed;

                // Smooth acceleration
                m_velocity.x += (targetVelocity.x - m_velocity.x) * m_acceleration * deltaTime.asSeconds();
                m_velocity.y += (targetVelocity.y - m_velocity.y) * m_acceleration * deltaTime.asSeconds();
            }
            else
            {
                // Decelerate when near target and stop completely when slow enough
                m_velocity *= (1.0f - m_deceleration * deltaTime.asSeconds());

                // Snap to target to avoid jitter once velocity is very small
                if (std::abs(m_velocity.x) < 1.0f && std::abs(m_velocity.y) < 1.0f)
                {
                    m_velocity = sf::Vector2f(0.0f, 0.0f);
                    m_position = m_targetPosition;
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

        // Check for stage advancement
        checkStageAdvancement();

        // Update visual effects
        updateVisualEffects(deltaTime);

        // Update visual representation
        m_shape.setPosition(m_position);
        m_shape.setScale(m_eatAnimationScale, m_eatAnimationScale);

        if (m_renderMode == RenderMode::Sprite && m_sprite)
        {
            m_sprite->update(deltaTime);

            // Preserve horizontal orientation when applying scale animation
            float sign = (m_sprite->getSprite().getScale().x < 0.0f) ? -1.0f : 1.0f;
            float stageScale = 1.0f;
            if (m_spriteManager)
            {
                const auto& cfg = m_spriteManager->getScaleConfig();
                switch (getCurrentFishSize())
                {
                case FishSize::Small:
                    stageScale = cfg.small;
                    break;
                case FishSize::Medium:
                    stageScale = (cfg.medium) + 0.18f;
                    break;
                case FishSize::Large:
                    stageScale = (cfg.large) + 0.4f;
                    break;
                default:
                    stageScale = 1.0f;
                    break;
                }
            }

            m_sprite->setScale(sf::Vector2f(sign * stageScale * m_eatAnimationScale,
                stageScale * m_eatAnimationScale));
        }
    }

    void Player::handleInput()
    {
        // Check for mouse control toggle
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            m_useMouseControl = true;
        }

        // Keyboard controls
        sf::Vector2f inputDirection(0.0f, 0.0f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            inputDirection.y -= 1.0f;
            m_useMouseControl = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            inputDirection.y += 1.0f;
            m_useMouseControl = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            inputDirection.x -= 1.0f;
            m_useMouseControl = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            inputDirection.x += 1.0f;
            m_useMouseControl = false;
        }

        // Apply keyboard input if not using mouse
        if (!m_useMouseControl && (inputDirection.x != 0.0f || inputDirection.y != 0.0f))
        {
            // Normalize diagonal movement
            float length = std::sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
            if (length > 0.0f)
            {
                inputDirection /= length;
                float speed = m_baseSpeed * (m_speedBoostTimer > sf::Time::Zero ? m_speedMultiplier : 1.0f);
                m_velocity = inputDirection * speed;
            }
        }
        else if (!m_useMouseControl)
        {
            // Apply deceleration when no input
            m_velocity *= 0.9f;
        }
    }

    void Player::followMouse(const sf::Vector2f& mousePosition)
    {
        m_targetPosition = mousePosition;
    }

    sf::FloatRect Player::getBounds() const
    {
        return sf::FloatRect(m_position.x - m_radius, m_position.y - m_radius,
            m_radius * 2.0f, m_radius * 2.0f);
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

    void Player::advanceStage()
    {
        checkStageAdvancement();
    }

    void Player::resetSize()
    {
        m_score = 0;
        m_currentStage = 1;
        m_growthProgress = 0.0f;
        m_radius = m_baseRadius;
        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);

        if (m_growthMeter)
        {
            m_growthMeter->reset();
            m_growthMeter->setStage(1);
        }
    }

    void Player::fullReset()
    {
        resetSize();
        m_points = 0;
        m_totalFishEaten = 0;
        m_damageTaken = 0;
    }

    bool Player::canEat(const Entity& other) const
    {
        if (m_invulnerabilityTimer > sf::Time::Zero && m_invincibilityTimer <= sf::Time::Zero)
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
        bool facingRight = true;
        if (m_sprite)
        {
            facingRight = m_sprite->getSprite().getScale().x < 0.0f;
        }
        else if (m_velocity.x != 0.0f)
        {
            facingRight = m_velocity.x > 0.0f;
        }
        mouthOffset.x = facingRight ? mouthOffset.x : -mouthOffset.x;

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
        if (m_invulnerabilityTimer > sf::Time::Zero || m_invincibilityTimer > sf::Time::Zero)
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
    }

    void Player::respawn()
    {
        m_isAlive = true;
        m_position = sf::Vector2f(m_windowBounds.x / 2.0f, m_windowBounds.y / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_targetPosition = m_position;
        m_invulnerabilityTimer = m_invulnerabilityDuration;
    }

    void Player::applySpeedBoost(float multiplier, sf::Time duration)
    {
        m_speedMultiplier = multiplier;
        m_speedBoostTimer = duration;
    }

    void Player::applyInvincibility(sf::Time duration)
    {
        m_invincibilityTimer = duration;
    }

    void Player::triggerEatEffect()
    {
        m_eatAnimationScale = 1.3f;

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
        for (const auto& effect : m_activeEffects)
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
        }

        if (m_renderMode == RenderMode::Sprite && m_sprite)
        {
            target.draw(*m_sprite, states);
        }
        else
        {
            target.draw(m_shape, states);
        }
    }

    void Player::updateStage()
    {
        m_radius = m_baseRadius * std::pow(m_growthFactor, m_currentStage - 1);
        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);

        const std::array<sf::Color, 3> stageColors{
            sf::Color::Yellow,
            sf::Color(255, 150, 0),
            sf::Color(255, 50, 50)
        };

        if (m_currentStage >= 1 && m_currentStage <= Constants::MAX_STAGES)
        {
            m_shape.setFillColor(stageColors[m_currentStage - 1]);
        }

        if (m_growthMeter)
        {
            m_growthMeter->setStage(m_currentStage);
        }

        if (m_sprite && m_renderMode == RenderMode::Sprite && m_spriteManager)
        {
            auto playerConfig = m_spriteManager->getSpriteConfig<Player>(getTextureID(), getCurrentFishSize());

            SpriteConfig<Entity> entityConfig;
            entityConfig.textureName = std::move(playerConfig.textureName);
            entityConfig.baseSize = playerConfig.baseSize;
            entityConfig.origin = playerConfig.origin;
            entityConfig.scaleMultiplier = playerConfig.scaleMultiplier;
            entityConfig.maintainAspectRatio = playerConfig.maintainAspectRatio;
            entityConfig.textureRect = playerConfig.textureRect;
            entityConfig.rotationOffset = playerConfig.rotationOffset;

            m_sprite->configure(entityConfig);
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

        std::for_each(m_activeEffects.begin(), m_activeEffects.end(),
            [deltaTime](VisualEffect& effect) {
                effect.duration -= deltaTime;
            });

        m_activeEffects.erase(
            std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
                [](const VisualEffect& effect) { return effect.duration <= sf::Time::Zero; }),
            m_activeEffects.end()
        );

        sf::Color currentColor = m_shape.getFillColor();

        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            float alpha = std::sin(m_invulnerabilityTimer.asSeconds() * 10.0f) * 0.5f + 0.5f;
            currentColor.a = static_cast<sf::Uint8>(255 * alpha);
        }
        else if (m_invincibilityTimer > sf::Time::Zero)
        {
            currentColor = sf::Color(255, 215, 0);
            float glow = std::sin(m_invincibilityTimer.asSeconds() * 5.0f) * 0.3f + 0.7f;
            currentColor.r = static_cast<sf::Uint8>(currentColor.r * glow);
        }
        else if (m_damageFlashIntensity > 0.0f)
        {
            currentColor.r = static_cast<sf::Uint8>(currentColor.r + (m_damageFlashColor.r - currentColor.r) * m_damageFlashIntensity);
            currentColor.g = static_cast<sf::Uint8>(currentColor.g + (m_damageFlashColor.g - currentColor.g) * m_damageFlashIntensity);
            currentColor.b = static_cast<sf::Uint8>(currentColor.b + (m_damageFlashColor.b - currentColor.b) * m_damageFlashIntensity);
        }
        else
        {
            currentColor.a = 255;
        }

        m_shape.setFillColor(currentColor);
    }

    void Player::handlePredatorBehavior(const Entity& predator)
    {
        if (m_invulnerabilityTimer > sf::Time::Zero || m_invincibilityTimer > sf::Time::Zero)
            return;

        sf::Vector2f pushDirection = m_position - predator.getPosition();
        float distance = std::sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);

        if (distance > 0)
        {
            pushDirection /= distance;
            m_velocity = pushDirection * 300.0f;
        }

        takeDamage();
    }
}