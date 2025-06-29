#include "PlayerVisual.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

namespace FishGame {

PlayerVisual::PlayerVisual(Player& player) : m_player(player) {}

void PlayerVisual::triggerEatEffect()
{
    m_player.m_eatAnimationScale = 1.3f;
    m_player.m_eatAnimationTimer = Player::m_eatAnimationDuration;

    if (m_player.m_animator)
    {
        std::string eatAnim = m_player.m_facingRight ? "eatRight" : "eatLeft";
        m_player.m_animator->play(eatAnim);
        m_player.m_currentAnimation = eatAnim;
    }

    m_player.m_activeEffects.push_back({1.2f, 0.f, sf::Color::Green, sf::seconds(0.2f)});
}

void PlayerVisual::triggerDamageEffect()
{
    m_player.m_damageFlashIntensity = 1.f;
    m_player.m_damageFlashColor = sf::Color::Red;

    m_player.m_activeEffects.push_back({0.8f, 15.f, sf::Color::Red, sf::seconds(0.3f)});
}

void PlayerVisual::update(sf::Time deltaTime)
{
    if (m_player.m_eatAnimationTimer > sf::Time::Zero)
    {
        m_player.m_eatAnimationTimer -= deltaTime;
        if (m_player.m_eatAnimationTimer < sf::Time::Zero)
            m_player.m_eatAnimationTimer = sf::Time::Zero;
    }

    if (m_player.m_turnAnimationTimer > sf::Time::Zero)
    {
        m_player.m_turnAnimationTimer -= deltaTime;
        if (m_player.m_turnAnimationTimer < sf::Time::Zero)
            m_player.m_turnAnimationTimer = sf::Time::Zero;
    }

    if (m_player.m_eatAnimationScale > 1.f)
    {
        m_player.m_eatAnimationScale -= Player::m_eatAnimationSpeed * deltaTime.asSeconds();
        m_player.m_eatAnimationScale = std::max(1.f, m_player.m_eatAnimationScale);
    }

    if (m_player.m_damageFlashIntensity > 0.f)
    {
        m_player.m_damageFlashIntensity -= 3.f * deltaTime.asSeconds();
        m_player.m_damageFlashIntensity = std::max(0.f, m_player.m_damageFlashIntensity);
    }

    if (m_player.m_poisonColorTimer > sf::Time::Zero)
    {
        m_player.m_poisonColorTimer -= deltaTime;
        if (m_player.m_poisonColorTimer <= sf::Time::Zero)
        {
            m_player.m_poisonColorTimer = sf::Time::Zero;
            m_player.m_controlsReversed = false;
        }
    }

    std::for_each(m_player.m_activeEffects.begin(), m_player.m_activeEffects.end(),
        [deltaTime](Player::VisualEffect& effect) { effect.duration -= deltaTime; });

    m_player.m_activeEffects.erase(
        std::remove_if(m_player.m_activeEffects.begin(), m_player.m_activeEffects.end(),
            [](const Player::VisualEffect& effect) { return effect.duration <= sf::Time::Zero; }),
        m_player.m_activeEffects.end());

    sf::Color currentColor = sf::Color::White;

    if (m_player.m_invulnerabilityTimer > sf::Time::Zero)
    {
        float alpha = std::sin(m_player.m_invulnerabilityTimer.asSeconds() * 10.f) * 0.5f + 0.5f;
        currentColor.a = static_cast<sf::Uint8>(255 * alpha);
    }
    else if (m_player.m_damageFlashIntensity > 0.f)
    {
        currentColor.r = static_cast<sf::Uint8>(currentColor.r + (m_player.m_damageFlashColor.r - currentColor.r) * m_player.m_damageFlashIntensity);
        currentColor.g = static_cast<sf::Uint8>(currentColor.g + (m_player.m_damageFlashColor.g - currentColor.g) * m_player.m_damageFlashIntensity);
        currentColor.b = static_cast<sf::Uint8>(currentColor.b + (m_player.m_damageFlashColor.b - currentColor.b) * m_player.m_damageFlashIntensity);
    }
    else if (m_player.m_poisonColorTimer > sf::Time::Zero)
    {
        currentColor = sf::Color(50, 255, 50);
    }
    else
    {
        currentColor.a = 255;
    }

    if (m_player.m_animator)
        m_player.m_animator->setColor(currentColor);
}

void PlayerVisual::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_player.m_isAlive)
        return;

    for (const auto& effect : m_player.m_activeEffects)
    {
        if (effect.duration > sf::Time::Zero)
        {
            sf::Transform effectTransform;
            effectTransform.translate(m_player.m_position);
            effectTransform.scale(effect.scale, effect.scale);
            effectTransform.rotate(effect.rotation);
            effectTransform.translate(-m_player.m_position);

            states.transform *= effectTransform;
        }
    }

    if (m_player.m_animator)
    {
        target.draw(*m_player.m_animator, states);
    }
}

} // namespace FishGame
