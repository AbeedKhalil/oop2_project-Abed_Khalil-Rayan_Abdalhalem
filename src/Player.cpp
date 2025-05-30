#include "Player.h"
#include "Fish.h"
#include "BonusItem.h"
#include "PowerUp.h"
#include <SFML/Window.hpp>
#include <cmath>
#include <algorithm>

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
        , m_useMouseControl(false)
        , m_targetPosition(0.0f, 0.0f)
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
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
            m_growthMeter->setOnStageComplete([this]() { advanceStage(); });
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
                // Decelerate when near target
                m_velocity *= (1.0f - m_deceleration * deltaTime.asSeconds());
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

        // Update visual effects
        updateVisualEffects(deltaTime);

        // Update visual representation
        m_shape.setPosition(m_position);
        m_shape.setScale(m_eatAnimationScale, m_eatAnimationScale);
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

    void Player::grow(int points)
    {
        // Convert score points to growth points
        float growthPoints = 0.0f;

        // Determine growth points based on fish size/points
        if (points <= 3)  // Small fish
            growthPoints = m_tinyFishGrowth;
        else if (points <= 6)  // Medium fish
            growthPoints = m_smallFishGrowth;
        else if (points <= 9)  // Large fish
            growthPoints = m_mediumFishGrowth;
        else  // Bonus items (oysters give 15 or 30 growth points)
            growthPoints = static_cast<float>(points);

        m_growthProgress += growthPoints;

        if (m_growthMeter)
        {
            m_growthMeter->addProgress(growthPoints);
        }

        // Trigger eat effect
        triggerEatEffect();
    }

    void Player::advanceStage()
    {
        if (m_currentStage < 4)  // Changed from 3 to 4
        {
            m_currentStage++;
            updateStage();

            // Visual effect for stage advancement
            m_activeEffects.push_back({
                1.5f,  // scale
                0.0f,  // rotation
                sf::Color::Cyan,
                sf::seconds(0.5f)
                });
        }
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

    bool Player::canEat(const Entity& other) const
    {
        // Can't eat while invulnerable
        if (m_invulnerabilityTimer > sf::Time::Zero && m_invincibilityTimer <= sf::Time::Zero)
            return false;

        // Check entity type
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

            // Can eat same size or smaller fish
            return static_cast<int>(playerSize) >= static_cast<int>(fishSize);
        }

        return false;
    }

    bool Player::attemptEat(Entity& other)
    {
        if (canEat(other))
        {
            const Fish* fish = dynamic_cast<const Fish*>(&other);
            if (fish)
            {
                grow(fish->getPointValue());
                m_totalFishEaten++;

                // Register hit for chain bonus
                if (m_scoreSystem)
                {
                    m_scoreSystem->registerHit();

                    // Add score with all multipliers
                    int frenzyMultiplier = m_frenzySystem ? m_frenzySystem->getMultiplier() : 1;
                    float powerUpMultiplier = m_powerUpManager ? m_powerUpManager->getScoreMultiplier() : 1.0f;

                    m_scoreSystem->addScore(ScoreEventType::FishEaten, fish->getPointValue(),
                        other.getPosition(), frenzyMultiplier, powerUpMultiplier);
                }

                // Register fish eaten for frenzy
                if (m_frenzySystem)
                {
                    m_frenzySystem->registerFishEaten();
                }

                return true;
            }
        }
        else
        {
            // Check if it's a predator that can damage us
            const Fish* fish = dynamic_cast<const Fish*>(&other);
            if (fish && fish->canEat(*this))
            {
                handlePredatorBehavior(other);
            }
        }

        return false;
    }

    bool Player::canTailBite(const Entity& other) const
    {
        // Can only tail-bite much larger fish
        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (!fish)
            return false;

        // Must be at least 2 sizes larger
        int sizeDifference = static_cast<int>(fish->getSize()) - static_cast<int>(getCurrentFishSize());
        return sizeDifference >= 2;
    }

    bool Player::attemptTailBite(Entity& other)
    {
        if (canTailBite(other) && !hasRecentlyTakenDamage())
        {
            // Check if we're near the tail (back portion of the fish)
            sf::Vector2f fishPos = other.getPosition();
            sf::Vector2f fishVelocity = other.getVelocity();

            // Calculate tail position (opposite of movement direction)
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
                    // Successful tail bite
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
        case 4:  // Stage 3 and 4 are both "Large" size
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

        // Register miss for chain bonus
        if (m_scoreSystem)
        {
            m_scoreSystem->registerMiss();
        }

        triggerDamageEffect();
    }

    void Player::die()
    {
        // Lose some growth progress but stay in current stage
        m_growthProgress = std::max(0.0f, m_growthProgress - 20.0f);

        if (m_growthMeter)
        {
            m_growthMeter->reset();
            m_growthMeter->addProgress(m_growthProgress);
        }

        // Reset position and start invulnerability
        m_position = sf::Vector2f(m_windowBounds.x / 2.0f, m_windowBounds.y / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_invulnerabilityTimer = m_invulnerabilityDuration;
    }

    void Player::respawn()
    {
        m_isAlive = true;
        die(); // Use die() to reset position and start invulnerability
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

        // Add visual effect
        m_activeEffects.push_back({
            1.2f,  // scale
            0.0f,  // rotation
            sf::Color::Green,
            sf::seconds(0.2f)
            });
    }

    void Player::triggerDamageEffect()
    {
        m_damageFlashIntensity = 1.0f;
        m_damageFlashColor = sf::Color::Red;

        // Add visual effect
        m_activeEffects.push_back({
            0.8f,  // scale
            15.0f, // rotation
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
        // Apply visual effects
        for (const auto& effect : m_activeEffects)
        {
            if (effect.duration > sf::Time::Zero)
            {
                // Create a transform for this effect
                sf::Transform effectTransform;
                effectTransform.translate(m_position);
                effectTransform.scale(effect.scale, effect.scale);
                effectTransform.rotate(effect.rotation);
                effectTransform.translate(-m_position);

                states.transform *= effectTransform;
            }
        }

        target.draw(m_shape, states);
    }

    void Player::updateStage()
    {
        // Update radius based on stage
        m_radius = m_baseRadius * std::pow(m_growthFactor, m_currentStage - 1);
        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);

        // Update color based on stage
        sf::Color stageColors[] = {
            sf::Color::Yellow,          // Stage 1
            sf::Color(255, 150, 0),     // Stage 2 - Orange
            sf::Color(255, 50, 50),     // Stage 3 - Red
            sf::Color(148, 0, 211)      // Stage 4 - Purple (Maximum size)
        };

        if (m_currentStage >= 1 && m_currentStage <= 4)
        {
            m_shape.setFillColor(stageColors[m_currentStage - 1]);
        }

        if (m_growthMeter)
        {
            m_growthMeter->setStage(m_currentStage);
        }
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
        // Update eat animation
        if (m_eatAnimationScale > 1.0f)
        {
            m_eatAnimationScale -= m_eatAnimationSpeed * deltaTime.asSeconds();
            m_eatAnimationScale = std::max(1.0f, m_eatAnimationScale);
        }

        // Update damage flash
        if (m_damageFlashIntensity > 0.0f)
        {
            m_damageFlashIntensity -= 3.0f * deltaTime.asSeconds();
            m_damageFlashIntensity = std::max(0.0f, m_damageFlashIntensity);
        }

        // Update active effects
        std::for_each(m_activeEffects.begin(), m_activeEffects.end(),
            [deltaTime](VisualEffect& effect) {
                effect.duration -= deltaTime;
            });

        // Remove expired effects
        m_activeEffects.erase(
            std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
                [](const VisualEffect& effect) { return effect.duration <= sf::Time::Zero; }),
            m_activeEffects.end()
        );

        // Apply visual states
        sf::Color currentColor = m_shape.getFillColor();

        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            // Flashing effect
            float alpha = std::sin(m_invulnerabilityTimer.asSeconds() * 10.0f) * 0.5f + 0.5f;
            currentColor.a = static_cast<sf::Uint8>(255 * alpha);
        }
        else if (m_invincibilityTimer > sf::Time::Zero)
        {
            // Golden glow for invincibility
            currentColor = sf::Color(255, 215, 0); // Gold
            float glow = std::sin(m_invincibilityTimer.asSeconds() * 5.0f) * 0.3f + 0.7f;
            currentColor.r = static_cast<sf::Uint8>(currentColor.r * glow);
        }
        else if (m_damageFlashIntensity > 0.0f)
        {
            // Mix damage flash color
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
        // Only handle if not invulnerable
        if (m_invulnerabilityTimer > sf::Time::Zero || m_invincibilityTimer > sf::Time::Zero)
            return;

        // Bump effect - push player away from predator
        sf::Vector2f pushDirection = m_position - predator.getPosition();
        float distance = std::sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);

        if (distance > 0)
        {
            pushDirection /= distance;
            m_velocity = pushDirection * 300.0f; // Push force
        }

        // Take damage
        takeDamage();
    }
}