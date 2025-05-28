// PlayState.h
#pragma once

#include "State.h"
#include "Player.h"
#include "FishSpawner.h"
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
        void updateHUD();
        void checkCollisions();
        void handlePlayerDeath();
        void advanceLevel();
        void gameOver();

    private:
        std::unique_ptr<Player> m_player;
        std::unique_ptr<FishSpawner> m_fishSpawner;
        std::vector<std::unique_ptr<Entity>> m_entities;

        // HUD elements
        sf::Text m_scoreText;
        sf::Text m_livesText;
        sf::Text m_levelText;
        sf::Text m_fpsText;
        sf::Text m_messageText; // For level complete/game over messages

        // Game state
        int m_currentLevel;
        int m_playerLives;
        int m_totalScore;

        // Level transition
        bool m_levelComplete;
        sf::Time m_levelTransitionTimer;
        static const sf::Time m_levelTransitionDuration;

        // Performance tracking
        sf::Time m_fpsUpdateTime;
        int m_frameCount;
        float m_currentFPS;
    };
}