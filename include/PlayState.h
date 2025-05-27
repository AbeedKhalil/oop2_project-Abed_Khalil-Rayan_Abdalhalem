// PlayState.h
#pragma once

#include "State.h"
#include "Player.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace FishGame
{
    class PlayState : public State
    {
    public:
        explicit PlayState(Game& game);
        ~PlayState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;

    private:
        void initializeEntities();
        void updateHUD();
        void checkCollisions();
        void spawnTestFish();

    private:
        std::unique_ptr<Player> m_player;
        std::vector<std::unique_ptr<Entity>> m_entities;

        // HUD elements
        sf::Text m_scoreText;
        sf::Text m_livesText;
        sf::Text m_levelText;
        sf::Text m_fpsText;

        // Game state
        int m_currentLevel;
        int m_playerLives;
        int m_playerScore;

        // Performance tracking
        sf::Time m_fpsUpdateTime;
        int m_frameCount;
        float m_currentFPS;

        // Test spawn timer for Stage 1
        sf::Time m_spawnTimer;
        static const sf::Time m_spawnInterval;
    };
}