#include "HUDController.h"
#include "GameConstants.h"

namespace FishGame {

HUDController::HUDController(const sf::Font& font, const sf::Vector2u& size)
    : m_hud(std::make_unique<HUDSystem>(font, size)) {}

void HUDController::update(sf::Time dt,
                           int score,
                           int lives,
                           int level,
                           int chainBonus,
                           const std::vector<PowerUpInfo>& active,
                           bool frozen, sf::Time freeze,
                           bool reversed, sf::Time reverse,
                           bool stunned, sf::Time stun)
{
    m_frameCount++;
    m_fpsUpdate += dt;
    if (m_fpsUpdate >= Constants::FPS_UPDATE_INTERVAL) {
        m_currentFPS = static_cast<float>(m_frameCount) / m_fpsUpdate.asSeconds();
        m_frameCount = 0;
        m_fpsUpdate = sf::Time::Zero;
    }

    m_hud->update(score, lives, level, chainBonus, active,
                  frozen, freeze, reversed, reverse, stunned, stun, m_currentFPS);
}

} // namespace FishGame
