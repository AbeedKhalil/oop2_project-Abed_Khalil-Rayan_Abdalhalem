#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "GameConstants.h"
#include <random>
#include <vector>
#include <functional>

namespace FishGame
{
    // Spawner configuration
    template<typename T>
    struct SpawnerConfig
    {
        float spawnRate = 1.0f;
        sf::Vector2f minBounds = { 0.0f, 0.0f };
        sf::Vector2f maxBounds = { static_cast<float>(Constants::WINDOW_WIDTH), static_cast<float>(Constants::WINDOW_HEIGHT) };
        std::function<void(T&)> customizer = nullptr;
    };

    // Generic spawner template
    template<typename T>
    class GenericSpawner
    {
    public:
        using ConfigType = SpawnerConfig<T>;
        using FactoryFunc = std::function<std::unique_ptr<T>()>;

        explicit GenericSpawner(const ConfigType& config = {})
            : m_config(config)
            , m_factory(nullptr)
            , m_spawnTimer(sf::Time::Zero)
            , m_spawnBuffer()
            , m_randomEngine(std::random_device{}())
            , m_xDist(config.minBounds.x, config.maxBounds.x)
            , m_yDist(config.minBounds.y, config.maxBounds.y)
        {
        }

        void setFactory(FactoryFunc factory) { m_factory = std::move(factory); }

        void setConfig(const ConfigType& config)
        {
            m_config = config;
            m_xDist = std::uniform_real_distribution<float>(config.minBounds.x, config.maxBounds.x);
            m_yDist = std::uniform_real_distribution<float>(config.minBounds.y, config.maxBounds.y);
        }

        void update(sf::Time deltaTime)
        {
            if (m_config.spawnRate <= 0.0f)
                return;

            m_spawnTimer += deltaTime;
            sf::Time spawnInterval = sf::seconds(1.0f / m_config.spawnRate);

            while (m_spawnTimer >= spawnInterval)
            {
                m_spawnTimer -= spawnInterval;
                if (auto spawned = spawn())
                {
                    m_spawnBuffer.push_back(std::move(spawned));
                }
            }
        }

        std::vector<std::unique_ptr<T>> collectSpawned()
        {
            std::vector<std::unique_ptr<T>> result;
            result.swap(m_spawnBuffer);
            return result;
        }


    private:
        std::unique_ptr<T> spawn()
        {
            if (!m_factory)
                return nullptr;

            auto entity = m_factory();
            if (entity)
            {
                entity->setPosition(m_xDist(m_randomEngine), m_yDist(m_randomEngine));
                if (m_config.customizer)
                {
                    m_config.customizer(*entity);
                }
            }
            return entity;
        }

    private:
        ConfigType m_config;
        FactoryFunc m_factory;
        sf::Time m_spawnTimer;
        std::vector<std::unique_ptr<T>> m_spawnBuffer;

        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_xDist;
        std::uniform_real_distribution<float> m_yDist;
    };
}
