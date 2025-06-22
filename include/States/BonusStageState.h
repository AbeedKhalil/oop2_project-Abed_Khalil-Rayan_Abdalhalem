#pragma once

#include "State.h"
#include "BonusItem.h"
#include "EnvironmentSystem.h"
#include "Player.h"
#include <memory>
#include <vector>
#include <random>
#include <functional>
#include <chrono>

namespace FishGame
{
    // Template trait specialization
    class BonusStageState;
    template<> struct is_state<BonusStageState> : std::true_type {};

    // Bonus stage types
    enum class BonusStageType
    {
        TreasureHunt,    // Collect as many pearls as possible
        FeedingFrenzy,   // Eat small fish in limited time
        SurvivalChallenge // Survive waves of predators
    };

    class BonusStageState : public State
    {
    public:
        explicit BonusStageState(Game& game, BonusStageType type, int playerLevel);
        ~BonusStageState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        struct BonusObjective
        {
            std::string description{};
            int targetCount = 0;
            int currentCount = 0;
            int pointsPerItem = 0;
        };

        // Stage-specific update methods
        void updateTreasureHunt(sf::Time deltaTime);
        void updateFeedingFrenzy(sf::Time deltaTime);
        void updateSurvivalChallenge(sf::Time deltaTime);

        // Spawning methods
        void spawnTreasureItems();
        void spawnBonusFish();
        void spawnPredatorWave();
        void spawnTimePowerUp();

        // Completion handling
        void checkCompletion();
        void completeStage();
        int calculateBonus() const;

        // Template helper for spawning entities
        template<typename EntityType, typename... Args>
        void spawnEntity(std::vector<std::unique_ptr<Entity>>& container,
            const sf::Vector2f& position, Args&&... args)
        {
            auto entity = std::make_unique<EntityType>(std::forward<Args>(args)...);
            entity->setPosition(position);
            container.push_back(std::move(entity));
        }

    private:
        BonusStageType m_stageType;
        int m_playerLevel;

        // Core objects
        std::unique_ptr<Player> m_player;
        std::vector<std::unique_ptr<Entity>> m_entities;
        std::vector<std::unique_ptr<BonusItem>> m_bonusItems;
        std::unique_ptr<EnvironmentSystem> m_environment;

        // Stage state
        sf::Time m_timeLimit = sf::Time::Zero;
        sf::Time m_timeElapsed = sf::Time::Zero;
        BonusObjective m_objective{};
        bool m_stageComplete = false;
        int m_bonusScore = 0;

        // UI elements
        sf::Text m_objectiveText;
        sf::Text m_timerText;
        sf::Text m_scoreText;
        sf::RectangleShape m_timerBar;
        sf::RectangleShape m_timerBackground;

        // Spawning
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_xDist;
        std::uniform_real_distribution<float> m_yDist;

        sf::Time m_timePowerUpTimer{ sf::Time::Zero };

        // Short grace period after collecting a pearl to avoid instant failure
        sf::Time m_oysterSafetyTimer{ sf::Time::Zero };

        // Stage configuration
        static constexpr int m_requiredPearlCount = 10;
        static constexpr float m_treasureHuntDuration = 30.0f;
        static constexpr float m_feedingFrenzyDuration = 45.0f;
        static constexpr float m_survivalDuration = 60.0f;
    };
}