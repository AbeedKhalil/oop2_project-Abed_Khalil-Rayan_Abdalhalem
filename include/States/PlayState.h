#pragma once

#include "State.h"
#include "Player.h"
#include "EnhancedFishSpawner.h"
#include "SchoolingSystem.h"
#include "SpecialFish.h"
#include "GrowthMeter.h"
#include "FrenzySystem.h"
#include "BonusItemManager.h"
#include "OysterManager.h"
#include "ScoreSystem.h"
#include "PowerUp.h"
#include <memory>
#include <vector>
#include <random>

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
        // Core update methods
        void updateHUD();
        void updateGameplay(sf::Time deltaTime);
        void updateBonusItems(sf::Time deltaTime);
        void updatePowerUps(sf::Time deltaTime);
        void updateSchoolingBehavior(sf::Time deltaTime);
        void handleSpecialFishBehaviors(sf::Time deltaTime);

        // Collision handling
        void checkCollisions();
        void handleFishCollision(Entity& fish);
        void handleBonusItemCollision(BonusItem& item);
        void handlePowerUpCollision(PowerUp& powerUp);
        void checkTailBiteOpportunities();
        void checkPufferfishThreat(Pufferfish* pufferfish);

        // Game flow
        void handlePlayerDeath();
        void advanceLevel();
        void gameOver();
        void completeLevel();
        void showEndOfLevelStats();

        // Helper methods
        void updateLevelDifficulty();
        void createParticleEffect(sf::Vector2f position, sf::Color color);

    private:
        // Core game objects
        std::unique_ptr<Player> m_player;
        std::unique_ptr<EnhancedFishSpawner> m_fishSpawner;
        std::unique_ptr<SchoolingSystem> m_schoolingSystem;
        std::vector<std::unique_ptr<Entity>> m_entities;
        std::vector<std::unique_ptr<BonusItem>> m_bonusItems;

        // Game systems
        std::unique_ptr<GrowthMeter> m_growthMeter;
        std::unique_ptr<FrenzySystem> m_frenzySystem;
        std::unique_ptr<PowerUpManager> m_powerUpManager;
        std::unique_ptr<ScoreSystem> m_scoreSystem;
        std::unique_ptr<BonusItemManager> m_bonusItemManager;
        std::unique_ptr<FixedOysterManager> m_oysterManager;

        // HUD elements
        sf::Text m_scoreText;
        sf::Text m_livesText;
        sf::Text m_levelText;
        sf::Text m_fpsText;
        sf::Text m_messageText;
        sf::Text m_chainText;
        sf::Text m_powerUpText;

        // Game state
        int m_currentLevel;
        int m_playerLives;
        int m_totalScore;
        sf::Time m_levelStartTime;
        sf::Time m_levelTime;

        // Level transition
        bool m_levelComplete;
        sf::Time m_levelTransitionTimer;
        static const sf::Time m_levelTransitionDuration;

        // End-of-level stats
        struct LevelStats
        {
            sf::Time completionTime;
            bool reachedMaxSize;
            bool tookNoDamage;
            int timeBonus;
            int growthBonus;
            int untouchableBonus;
            int totalBonus;
        };
        LevelStats m_levelStats;

        // Performance tracking
        sf::Time m_fpsUpdateTime;
        int m_frameCount;
        float m_currentFPS;

        // Visual effects
        struct ParticleEffect
        {
            sf::CircleShape shape;
            sf::Vector2f velocity;
            sf::Time lifetime;
            float alpha;
        };
        std::vector<ParticleEffect> m_particles;

        // Random number generation
        std::mt19937 m_randomEngine;

        // Constants
        static const sf::Time m_targetLevelTime;
    };
}