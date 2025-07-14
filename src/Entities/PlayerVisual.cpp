#include "PlayerVisual.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

namespace FishGame {

PlayerVisual::PlayerVisual(Player& player) : m_player(player) {}

void PlayerVisual::triggerEatEffect()
{
    m_player.setEatAnimationScale(1.3f);
    m_player.setEatAnimationTimer(Player::eatAnimationDuration());

    if (m_player.getAnimator())
    {
        std::string eatAnim = m_player.isFacingRight() ? "eatRight" : "eatLeft";
        m_player.getAnimator()->play(eatAnim);
        m_player.setCurrentAnimation(eatAnim);
    }

    m_player.getActiveEffects().push_back({1.2f, 0.f, sf::Color::Green, sf::seconds(0.2f)});
}

void PlayerVisual::triggerDamageEffect()
{
    m_player.setDamageFlashIntensity(1.f);
    m_player.setDamageFlashColor(sf::Color::Red);

    m_player.getActiveEffects().push_back({0.8f, 15.f, sf::Color::Red, sf::seconds(0.3f)});
}

void PlayerVisual::update(sf::Time deltaTime)
{
    if (m_player.getEatAnimationTimer() > sf::Time::Zero)
    {
        m_player.setEatAnimationTimer(m_player.getEatAnimationTimer() - deltaTime);
        if (m_player.getEatAnimationTimer() < sf::Time::Zero)
            m_player.setEatAnimationTimer(sf::Time::Zero);
    }

    if (m_player.getTurnAnimationTimer() > sf::Time::Zero)
    {
        m_player.setTurnAnimationTimer(m_player.getTurnAnimationTimer() - deltaTime);
        if (m_player.getTurnAnimationTimer() < sf::Time::Zero)
            m_player.setTurnAnimationTimer(sf::Time::Zero);
    }

    if (m_player.getEatAnimationScale() > 1.f)
    {
        m_player.setEatAnimationScale(m_player.getEatAnimationScale() - Player::eatAnimationSpeed() * deltaTime.asSeconds());
        m_player.setEatAnimationScale(std::max(1.f, m_player.getEatAnimationScale()));
    }

    if (m_player.getDamageFlashIntensity() > 0.f)
    {
        m_player.setDamageFlashIntensity(m_player.getDamageFlashIntensity() - 3.f * deltaTime.asSeconds());
        m_player.setDamageFlashIntensity(std::max(0.f, m_player.getDamageFlashIntensity()));
    }

    if (m_player.getPoisonColorTimer() > sf::Time::Zero)
    {
        m_player.setPoisonColorTimer(m_player.getPoisonColorTimer() - deltaTime);
        if (m_player.getPoisonColorTimer() <= sf::Time::Zero)
        {
            m_player.setPoisonColorTimer(sf::Time::Zero);
            m_player.setControlsReversed(false);
        }
    }

    std::for_each(m_player.getActiveEffects().begin(), m_player.getActiveEffects().end(),
        [deltaTime](Player::VisualEffect& effect) { effect.duration -= deltaTime; });

    m_player.getActiveEffects().erase(
        std::remove_if(m_player.getActiveEffects().begin(), m_player.getActiveEffects().end(),
            [](const Player::VisualEffect& effect) { return effect.duration <= sf::Time::Zero; }),
        m_player.getActiveEffects().end());

    sf::Color currentColor = sf::Color::White;

    if (m_player.getInvulnerabilityTimer() > sf::Time::Zero)
    {
        float alpha = std::sin(m_player.getInvulnerabilityTimer().asSeconds() * 10.f) * 0.5f + 0.5f;
        currentColor.a = static_cast<sf::Uint8>(255 * alpha);
    }
    else if (m_player.getDamageFlashIntensity() > 0.f)
    {
        currentColor.r = static_cast<sf::Uint8>(currentColor.r + (m_player.getDamageFlashColor().r - currentColor.r) * m_player.getDamageFlashIntensity());
        currentColor.g = static_cast<sf::Uint8>(currentColor.g + (m_player.getDamageFlashColor().g - currentColor.g) * m_player.getDamageFlashIntensity());
        currentColor.b = static_cast<sf::Uint8>(currentColor.b + (m_player.getDamageFlashColor().b - currentColor.b) * m_player.getDamageFlashIntensity());
    }
    else if (m_player.getPoisonColorTimer() > sf::Time::Zero)
    {
        currentColor = sf::Color(50, 255, 50);
    }
    else
    {
        currentColor.a = 255;
    }

    if (m_player.getAnimator())
        m_player.getAnimator()->setColor(currentColor);
}

void PlayerVisual::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_player.isAlive())
        return;

    for (const auto& effect : m_player.getActiveEffects())
    {
        if (effect.duration > sf::Time::Zero)
        {
            sf::Transform effectTransform;
            effectTransform.translate(m_player.getPosition());
            effectTransform.scale(effect.scale, effect.scale);
            effectTransform.rotate(effect.rotation);
            effectTransform.translate(-m_player.getPosition());

            states.transform *= effectTransform;
        }
    }

    if (m_player.getAnimator())
    {
        target.draw(*m_player.getAnimator(), states);
    }
}

} // namespace FishGame
