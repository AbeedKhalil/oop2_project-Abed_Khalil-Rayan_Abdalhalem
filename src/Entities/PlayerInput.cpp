#include "PlayerInput.h"
#include "Player.h"

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

    if (m_player.areControlsReversed())
    {
        inputDirection = -inputDirection;
    }

    if (keyboardUsed)
    {
        float length = std::sqrt(inputDirection.x * inputDirection.x + inputDirection.y * inputDirection.y);
        if (length > 0.f)
        {
            inputDirection /= length;
            float speed = Player::baseSpeed() *
                (m_player.getSpeedBoostTimer() > sf::Time::Zero ? m_player.getSpeedMultiplier() : 1.f);
            m_player.setVelocity(inputDirection * speed);
        }
    }
    else
    {
        m_player.setVelocity(m_player.getVelocity() * 0.9f);
    }
}

} // namespace FishGame
