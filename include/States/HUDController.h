#pragma once

#include "HUDSystem.h"
#include <memory>

namespace FishGame {

class HUDController {
public:
    HUDController(const sf::Font& font, const sf::Vector2u& windowSize);

    void update(sf::Time dt,
                int score,
                int lives,
                int level,
                int chainBonus,
                const std::vector<PowerUpInfo>& active,
                bool frozen, sf::Time freeze,
                bool reversed, sf::Time reverse,
                bool stunned, sf::Time stun);

    void showMessage(const std::string& msg) { m_hud->showMessage(msg); }
    HUDSystem& getSystem() { return *m_hud; }
    float getFPS() const { return m_currentFPS; }

private:
    std::unique_ptr<HUDSystem> m_hud;
    sf::Time m_fpsUpdate{sf::Time::Zero};
    int m_frameCount{0};
    float m_currentFPS{0.f};
};

} // namespace FishGame
