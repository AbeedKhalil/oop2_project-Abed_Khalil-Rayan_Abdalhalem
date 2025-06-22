#pragma once

#include "Levels/Level.h"
#include <unordered_map>

namespace FishGame
{
    template<typename EnemyT, typename PowerUpT>
    class LevelManager
    {
    public:
        using LevelType = Level<EnemyT, PowerUpT>;

        LevelManager() = default;
        ~LevelManager() = default;

        // Add a new level to the manager
        void addLevel(int number, LevelType level)
        {
            m_levels.emplace(number, std::move(level));
        }

        // Retrieve a level by number, or nullptr if not found
        LevelType* getLevel(int number)
        {
            auto it = m_levels.find(number);
            if (it != m_levels.end())
                return &it->second;
            return nullptr;
        }

        // Set the current level number
        void setCurrentLevel(int number)
        {
            m_currentLevel = number;
        }

        // Get the current level or nullptr if none
        LevelType* getCurrentLevel()
        {
            return getLevel(m_currentLevel);
        }

    private:
        int m_currentLevel{0};
        std::unordered_map<int, LevelType> m_levels;
    };
}

