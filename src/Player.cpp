// Player.cpp
#include "Player.h"
#include "Fish.h"
#include <SFML/Window.hpp>
#include <cmath>
#include <algorithm>

namespace FishGame
{
    // Static member initialization
    const sf::Time Player::m_invulnerabilityDuration = sf::seconds(2.0f);

    Player::Player()
        : Entity()
        , m_shape(m_baseRadius)
        , m_score(0)
        , m_currentStage(1)
        , m_useMouseControl(false)
        , m_targetPosition(0.0f, 0.0f)
        , m_invulnerabilityTimer(sf::Time::Zero)
        , m_windowBounds(1920, 1080)
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

    void Player::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update invulnerability
        updateInvulnerability(deltaTime);

        // Handle input
        handleInput();

        // Update movement
        if (m_useMouseControl)
        {
            // Smooth movement towards mouse position
            sf::Vector2f direction = m_targetPosition - m_position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 5.0f)
            {
                direction /= distance; // Normalize
                sf::Vector2f targetVelocity = direction * m_baseSpeed;

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
        float currentSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (currentSpeed > m_maxSpeed)
        {
            m_velocity = (m_velocity / currentSpeed) * m_maxSpeed;
        }

        // Update position
        updateMovement(deltaTime);

        // Keep player within window bounds
        constrainToWindow();

        // Update visual representation
        m_shape.setPosition(m_position);

        // Update visual effects for invulnerability
        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            // Flashing effect
            float alpha = std::sin(m_invulnerabilityTimer.asSeconds() * 10.0f) * 0.5f + 0.5f;
            sf::Color color = m_shape.getFillColor();
            color.a = static_cast<sf::Uint8>(255 * alpha);
            m_shape.setFillColor(color);
        }
        else
        {
            m_shape.setFillColor(sf::Color::Yellow);
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
                m_velocity = inputDirection * m_baseSpeed;
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
        m_score = std::min(m_score + points, m_maxScore);
        updateStage();
    }

    void Player::resetSize()
    {
        m_score = 0;
        m_currentStage = 1;
        m_radius = m_baseRadius;
        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);
    }

    bool Player::canEat(const Entity& other) const
    {
        // Can't eat while invulnerable
        if (m_invulnerabilityTimer > sf::Time::Zero)
            return false;

        // Check entity type
        EntityType otherType = other.getType();
        if (otherType != EntityType::SmallFish &&
            otherType != EntityType::MediumFish &&
            otherType != EntityType::LargeFish)
        {
            return false;
        }

        // Cast to Fish to get size
        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (!fish)
            return false;

        // Stage 1 Player (Small) - Can eat small fish
        // Stage 2 Player (Medium) - Can eat small and medium fish  
        // Stage 3 Player (Large) - Can eat all fish types

        // Compare sizes based on current stage
        FishSize playerSize = getCurrentFishSize();
        FishSize fishSize = fish->getSize();

        // Can eat same size or smaller fish
        return static_cast<int>(playerSize) >= static_cast<int>(fishSize);
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

    void Player::die()
    {
        // Reset to starting score of current stage (not always stage 1)
        int stageStartScore = getStageStartingScore(m_currentStage);
        m_score = stageStartScore;
        updateStage(); // This will set the correct stage based on score

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

    void Player::setWindowBounds(const sf::Vector2u& windowSize)
    {
        m_windowBounds = windowSize;
    }

    void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_shape, states);
    }

    void Player::updateStage()
    {
        int previousStage = m_currentStage;

        if (m_score >= m_stage3Threshold)
        {
            m_currentStage = 3;
        }
        else if (m_score >= m_stage2Threshold)
        {
            m_currentStage = 2;
        }
        else
        {
            m_currentStage = 1;
        }

        // Update size if stage changed
        if (m_currentStage != previousStage)
        {
            m_radius = m_baseRadius * std::pow(m_growthFactor, m_currentStage - 1);
            m_shape.setRadius(m_radius);
            m_shape.setOrigin(m_radius, m_radius);
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

    int Player::getStageStartingScore(int stage)
    {
        switch (stage)
        {
        case 1:
            return m_stage1Threshold; // 0
        case 2:
            return m_stage2Threshold; // 33
        case 3:
            return m_stage3Threshold; // 66
        default:
            return m_stage1Threshold;
        }
    }
}