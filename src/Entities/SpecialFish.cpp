#include "SpecialFish.h"
#include "CollisionDetector.h"
#include "Player.h"
#include <random>

namespace FishGame
{
    // AdvancedFish implementation
    AdvancedFish::AdvancedFish(FishSize size, float speed, int currentLevel, MovementPattern pattern)
        : Fish(size, speed, currentLevel)
        , m_movementPattern(pattern)
        , m_patternTimer(0.0f)
        , m_baseY(0.0f)
        , m_amplitude(30.0f)
        , m_frequency(2.0f)
    {
    }

    void AdvancedFish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        m_patternTimer += deltaTime.asSeconds();

        // Update movement based on pattern
        updateMovementPattern(deltaTime);

        // Update position
        updateMovement(deltaTime);

        // Check boundaries
        if (m_velocity.x > 0 && m_position.x > m_windowBounds.x + m_radius)
        {
            destroy();
        }
        else if (m_velocity.x < 0 && m_position.x < -m_radius)
        {
            destroy();
        }

        // Update visual
        m_shape.setPosition(m_position);
    }

    void AdvancedFish::updateMovementPattern(sf::Time deltaTime)
    {
        switch (m_movementPattern)
        {
        case MovementPattern::Sinusoidal:
        {
            // Store base Y on first frame
            if (m_baseY == 0.0f)
                m_baseY = m_position.y;

            // Apply sinusoidal movement
            float yOffset = m_amplitude * std::sin(m_patternTimer * m_frequency);
            m_position.y = m_baseY + yOffset;
            break;
        }

        case MovementPattern::ZigZag:
        {
            // Change Y direction periodically
            float zigzagPeriod = 1.0f;
            if (std::fmod(m_patternTimer, zigzagPeriod) < zigzagPeriod / 2)
            {
                m_velocity.y = m_speed * 0.5f;
            }
            else
            {
                m_velocity.y = -m_speed * 0.5f;
            }
            break;
        }

        case MovementPattern::Circular:
        {
            float radius = 50.0f;
            float angle = m_patternTimer * 2.0f;
            sf::Vector2f circularOffset(
                radius * std::cos(angle),
                radius * std::sin(angle)
            );

            // Apply circular motion around current trajectory
            m_velocity.x = m_speed * std::cos(angle);
            m_velocity.y = m_speed * std::sin(angle);
            break;
        }

        case MovementPattern::Linear:
        default:
            // No special movement
            break;
        }
    }

    // Barracuda implementation
    Barracuda::Barracuda(int currentLevel)
        : AdvancedFish(FishSize::Large, 180.0f, currentLevel, MovementPattern::Aggressive)
        , m_currentTarget(nullptr)
        , m_huntTimer(sf::Time::Zero)
        , m_dashSpeed(450.0f)
        , m_isDashing(false)
    {
        // Barracuda appearance
        m_baseColor = sf::Color(100, 100, 150);  // Dark blue-gray
        m_outlineColor = sf::Color(50, 50, 100);
        m_outlineThickness = 3.0f;
        m_pointValue = getPointValue(m_size, m_currentLevel) * 2;  // Double points

        updateVisual();
    }

    void Barracuda::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        m_huntTimer += deltaTime;

        // Find closest prey
        const Entity* closestPrey = nullptr;
        float closestDistance = m_huntRange;

        // Check player first - Barracuda can hunt player if player is smaller
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr && canEat(*player))
            {
                float distance = CollisionDetector::getDistance(m_position, player->getPosition());
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closestPrey = player;
                }
            }
        }

        // Check other fish
        std::for_each(entities.begin(), entities.end(),
            [this, &closestPrey, &closestDistance](const std::unique_ptr<Entity>& entity)
            {
                if (entity.get() == this || !entity->isAlive())
                    return;

                if (canEat(*entity))
                {
                    float distance = CollisionDetector::getDistance(m_position, entity->getPosition());
                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        closestPrey = entity.get();
                    }
                }
            });

        // Update hunting behavior
        if (closestPrey)
        {
            m_currentTarget = closestPrey;
            updateHuntingBehavior(closestPrey, deltaTime);
        }
        else
        {
            m_isDashing = false;
            m_currentTarget = nullptr;
        }
    }

    void Barracuda::updateHuntingBehavior(const Entity* target, sf::Time deltaTime)
    {
        sf::Vector2f direction = target->getPosition() - m_position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0)
        {
            direction /= distance;  // Normalize

            // Start dash if close enough
            if (distance < 150.0f && !m_isDashing)
            {
                m_isDashing = true;
                m_huntTimer = sf::Time::Zero;
            }

            // Apply speed based on state
            float currentSpeed = m_isDashing ? m_dashSpeed : m_speed;

            // End dash after duration
            if (m_isDashing && m_huntTimer.asSeconds() > m_dashDuration)
            {
                m_isDashing = false;
            }

            m_velocity = direction * currentSpeed;
        }
    }

    // Pufferfish implementation
    Pufferfish::Pufferfish(int currentLevel)
        : AdvancedFish(FishSize::Medium, 100.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_inflationLevel(0.0f)
        , m_normalRadius(25.0f)
        , m_inflationTimer(sf::Time::Zero)
        , m_threatCheckTimer(sf::Time::Zero)
    {
        m_radius = m_normalRadius;

        // Pufferfish appearance
        m_baseColor = sf::Color(255, 220, 100);  // Yellow
        m_outlineColor = sf::Color(200, 170, 50);

        // Create spikes
        m_spikes.reserve(m_spikeCount);
        for (int i = 0; i < m_spikeCount; ++i)
        {
            sf::CircleShape spike(3.0f, 3);  // Triangle
            spike.setFillColor(sf::Color(150, 100, 50));
            spike.setOrigin(3.0f, 3.0f);
            m_spikes.push_back(spike);
        }

        updateVisual();
    }

    void Pufferfish::update(sf::Time deltaTime)
    {
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        updateInflation(deltaTime);

        // Update spike positions
        for (size_t i = 0; i < m_spikes.size(); ++i)
        {
            float angle = (360.0f / m_spikeCount) * i * 3.14159f / 180.0f;
            float spikeRadius = m_radius + (m_inflationLevel * 10.0f);

            sf::Vector2f spikePos(
                m_position.x + std::cos(angle) * spikeRadius,
                m_position.y + std::sin(angle) * spikeRadius
            );

            m_spikes[i].setPosition(spikePos);
            m_spikes[i].setRotation(angle * 180.0f / 3.14159f);
        }
    }

    bool Pufferfish::canEat(const Entity& other) const
    {
        // Pufferfish can't eat when inflated
        if (isInflated())
            return false;

        return Fish::canEat(other);
    }

    void Pufferfish::startInflation()
    {
        m_inflationTimer = sf::Time::Zero;
    }

    void Pufferfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        Fish::draw(target, states);

        // Draw spikes when inflating
        if (m_inflationLevel > 0.2f)
        {
            std::for_each(m_spikes.begin(), m_spikes.end(),
                [&target, &states](const sf::CircleShape& spike)
                {
                    target.draw(spike, states);
                });
        }
    }

    void Pufferfish::updateVisual()
    {
        Fish::updateVisual();

        // Update color based on inflation
        sf::Color currentColor = m_baseColor;
        currentColor.r = static_cast<sf::Uint8>(currentColor.r + (255 - currentColor.r) * m_inflationLevel * 0.3f);
        m_shape.setFillColor(currentColor);
    }

    void Pufferfish::updateInflation(sf::Time deltaTime)
    {
        m_threatCheckTimer += deltaTime;

        // Check for threats periodically
        if (m_threatCheckTimer.asSeconds() > 0.5f)
        {
            m_threatCheckTimer = sf::Time::Zero;
            // Note: Threat checking would need access to entities list
            // This would be done in the AI update method
        }

        // Update inflation state
        if (m_inflationTimer < sf::seconds(2.0f))
        {
            // Inflate
            m_inflationLevel = std::min(1.0f, m_inflationLevel + m_inflationSpeed * deltaTime.asSeconds());
        }
        else
        {
            // Deflate
            m_inflationLevel = std::max(0.0f, m_inflationLevel - m_deflationSpeed * deltaTime.asSeconds());
        }

        // Update radius
        m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);

        // Slow down when inflated
        float speedMultiplier = 1.0f - (m_inflationLevel * 0.7f);
        m_speed = 100.0f * speedMultiplier;
    }

    // Angelfish implementation
    Angelfish::Angelfish(int currentLevel)
        : AdvancedFish(FishSize::Small, 200.0f, currentLevel, MovementPattern::ZigZag)
        , m_bonusPoints(m_baseBonus* currentLevel)
        , m_colorShift(0.0f)
        , m_directionChangeTimer(sf::Time::Zero)
    {
        // Angelfish appearance - rainbow effect
        m_baseColor = sf::Color::Cyan;
        m_outlineColor = sf::Color::Blue;
        m_outlineThickness = 2.0f;

        // Create decorative fins
        m_fins.reserve(3);
        for (int i = 0; i < 3; ++i)
        {
            sf::CircleShape fin(10.0f, 3);
            fin.setFillColor(sf::Color(255, 200, 100, 150));
            fin.setOrigin(10.0f, 10.0f);
            m_fins.push_back(fin);
        }

        updateVisual();
    }

    void Angelfish::update(sf::Time deltaTime)
    {
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        updateErraticMovement(deltaTime);

        // Update color shift for rainbow effect
        m_colorShift += deltaTime.asSeconds() * 2.0f;

        // Update fin positions
        for (size_t i = 0; i < m_fins.size(); ++i)
        {
            float finAngle = (m_colorShift + i * 120.0f) * 3.14159f / 180.0f;
            float finRadius = 20.0f;

            sf::Vector2f finPos(
                m_position.x + std::cos(finAngle) * finRadius,
                m_position.y + std::sin(finAngle) * finRadius
            );

            m_fins[i].setPosition(finPos);
            m_fins[i].setRotation(finAngle * 180.0f / 3.14159f);
        }

        updateVisual();
    }

    void Angelfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw fins first
        std::for_each(m_fins.begin(), m_fins.end(),
            [&target, &states](const sf::CircleShape& fin)
            {
                target.draw(fin, states);
            });

        Fish::draw(target, states);
    }

    void Angelfish::updateVisual()
    {
        Fish::updateVisual();

        // Rainbow color effect
        float hue = std::fmod(m_colorShift * 60.0f, 360.0f);

        // Simple HSV to RGB conversion
        float c = 1.0f;
        float x = c * (1 - std::abs(std::fmod(hue / 60.0f, 2) - 1));
        float m = 0.5f;

        float r, g, b;
        if (hue < 60) { r = c; g = x; b = 0; }
        else if (hue < 120) { r = x; g = c; b = 0; }
        else if (hue < 180) { r = 0; g = c; b = x; }
        else if (hue < 240) { r = 0; g = x; b = c; }
        else if (hue < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        sf::Color rainbowColor(
            static_cast<sf::Uint8>((r + m) * 255),
            static_cast<sf::Uint8>((g + m) * 255),
            static_cast<sf::Uint8>((b + m) * 255)
        );

        m_shape.setFillColor(rainbowColor);
    }

    void Angelfish::updateErraticMovement(sf::Time deltaTime)
    {
        m_directionChangeTimer += deltaTime;

        if (m_directionChangeTimer.asSeconds() > m_directionChangeInterval)
        {
            m_directionChangeTimer = sf::Time::Zero;

            // Random direction change
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> angleDist(-45.0f, 45.0f);

            float angleChange = angleDist(rng) * 3.14159f / 180.0f;

            // Rotate velocity
            float cos_a = std::cos(angleChange);
            float sin_a = std::sin(angleChange);

            sf::Vector2f newVelocity(
                m_velocity.x * cos_a - m_velocity.y * sin_a,
                m_velocity.x * sin_a + m_velocity.y * cos_a
            );

            m_velocity = newVelocity;
        }
    }
}