#pragma once

#include "Entity.h"
#include <SFML/System.hpp>
#include <memory>
#include <vector>
#include <type_traits>

namespace FishGame
{
    struct LevelConfig
    {
        std::vector<sf::Vector2f> enemyPositions;
        std::vector<sf::Vector2f> powerUpPositions;
    };

    template<typename EnemyT, typename PowerUpT>
    class Level
    {
        static_assert(std::is_base_of_v<Entity, EnemyT>, "EnemyT must derive from Entity");
        static_assert(std::is_base_of_v<Entity, PowerUpT>, "PowerUpT must derive from Entity");

    public:
        Level() = default;
        ~Level() = default;

        void load(const LevelConfig& config);
        bool isComplete() const;

        std::vector<std::unique_ptr<Entity>>& getEnemies() { return m_enemies; }
        std::vector<std::unique_ptr<Entity>>& getPowerUps() { return m_powerUps; }

    private:
        std::vector<std::unique_ptr<Entity>> m_enemies;
        std::vector<std::unique_ptr<Entity>> m_powerUps;
    };

    template<typename EnemyT, typename PowerUpT>
    void Level<EnemyT, PowerUpT>::load(const LevelConfig& config)
    {
        m_enemies.clear();
        m_powerUps.clear();

        for (const auto& pos : config.enemyPositions)
        {
            auto enemy = std::make_unique<EnemyT>();
            enemy->setPosition(pos);
            m_enemies.push_back(std::move(enemy));
        }

        for (const auto& pos : config.powerUpPositions)
        {
            auto power = std::make_unique<PowerUpT>();
            power->setPosition(pos);
            m_powerUps.push_back(std::move(power));
        }
    }

    template<typename EnemyT, typename PowerUpT>
    bool Level<EnemyT, PowerUpT>::isComplete() const
    {
        auto alive = [](const auto& e) { return e && e->isAlive(); };
        bool enemiesGone = std::none_of(m_enemies.begin(), m_enemies.end(), alive);
        bool powerUpsGone = std::none_of(m_powerUps.begin(), m_powerUps.end(), alive);
        return enemiesGone && powerUpsGone;
    }
}

