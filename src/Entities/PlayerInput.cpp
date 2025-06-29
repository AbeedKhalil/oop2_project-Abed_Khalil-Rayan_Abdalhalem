#include "PlayerInput.h"
#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

namespace FishGame {

PlayerInput::PlayerInput(Player& player) : m_player(player) {}

void PlayerInput::handleInput()
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
            float speed = Player::m_baseSpeed * (m_player.m_speedBoostTimer > sf::Time::Zero ? m_player.m_speedMultiplier : 1.f);
            m_player.m_velocity = inputDirection * speed;
        }
    }
    else
    {
        m_player.m_velocity *= 0.9f;
    }
}

} // namespace FishGame
