#include "PlayerInput.h"
#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

namespace FishGame {

PlayerInput::PlayerInput(Player& player) : m_player(player) {}

void PlayerInput::handleInput(sf::Time deltaTime)
{
    sf::Vector2f inputDirection(0.f, 0.f);
    bool keyboardUsed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        inputDirection.y -= 1.f;
        keyboardUsed = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        inputDirection.y += 1.f;
        keyboardUsed = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        inputDirection.x -= 1.f;
        keyboardUsed = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        inputDirection.x += 1.f;
        keyboardUsed = true;
    }

    if (m_player.m_controlsReversed)
    {
        inputDirection = -inputDirection;
    }

    if (keyboardUsed)
    {
        float length = std::sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
        if (length > 0.f)
        {
            inputDirection /= length;
            float maxSpeed = Player::m_baseSpeed * (m_player.m_speedBoostTimer > sf::Time::Zero ? m_player.m_speedMultiplier : 1.f);
            sf::Vector2f desiredVelocity = inputDirection * maxSpeed;
            sf::Vector2f diff = desiredVelocity - m_player.m_velocity;
            float diffLength = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            float accelStep = Player::m_acceleration * deltaTime.asSeconds();
            if (diffLength > accelStep)
                diff = diff / diffLength * accelStep;
            m_player.m_velocity += diff;
        }
    }
    else
    {
        float speed = std::sqrt(m_player.m_velocity.x * m_player.m_velocity.x + m_player.m_velocity.y * m_player.m_velocity.y);
        if (speed > 0.f)
        {
            float decelStep = Player::m_deceleration * deltaTime.asSeconds();
            if (speed > decelStep)
                m_player.m_velocity -= (m_player.m_velocity / speed) * decelStep;
            else
                m_player.m_velocity = sf::Vector2f(0.f, 0.f);
        }
    }
}

} // namespace FishGame
