#pragma once

#include <memory>
#include <random>
#include <SFML/System.hpp>

#include "Hazard.h"
#include "PowerUp.h"
#include "ExtendedPowerUps.h"

namespace FishGame
{
    class SpriteManager;

    class SpawnSystem
    {
    public:
        SpawnSystem(SpriteManager& sprites, std::mt19937& rng, int& currentLevel, const sf::Font& font);

        std::unique_ptr<Hazard> spawnRandomHazard();
        std::unique_ptr<PowerUp> spawnRandomPowerUp();

    private:
        sf::Vector2f generateRandomPosition();

        SpriteManager& m_spriteManager;
        std::mt19937& m_randomEngine;
        int& m_currentLevel;
        const sf::Font& m_font;

        std::uniform_real_distribution<float> m_xDist;
        std::uniform_real_distribution<float> m_yDist;
        std::uniform_int_distribution<int> m_hazardTypeDist;
        std::uniform_int_distribution<int> m_powerUpTypeDist;
    };
}

