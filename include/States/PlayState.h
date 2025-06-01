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
#include "ExtendedPowerUps.h"
#include "Hazard.h"
#include "EnvironmentSystem.h"
#include "BonusStageState.h"
#include "GameConstants.h"
#include <memory>
#include <vector>
#include <random>
#include <unordered_map>
#include <functional>
#include <optional>
#include <algorithm>
#include <numeric>

namespace FishGame
{
    // Template trait specialization
    class PlayState;
    template<> struct is_state<PlayState> : std::true_type {};

    class PlayState : public State
    {
    public:
        explicit PlayState(Game& game);
        ~PlayState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        // Template for update functions
        template<typename System>
        struct UpdateTask
        {
            std::function<void(System&, sf::Time)> update;
            System* system;
            bool enabled;
        };

        // Template for collision handlers
        template<typename EntityType>
        using CollisionHandler = std::function<void(EntityType&)>;

        // Template for visual effects
        template<typename EffectType>
        struct VisualEffect
        {
            EffectType effect;
            sf::Time lifetime;
            std::function<void(EffectType&, sf::Time)> updateFunc;
            std::function<void(const EffectType&, sf::RenderTarget&)> renderFunc;
        };

        // Particle effect structure
        struct ParticleEffect
        {
            sf::CircleShape shape;
            sf::Vector2f velocity;
            sf::Time lifetime;
            float alpha;
        };

        // Level statistics
        struct LevelStats
        {
            sf::Time completionTime;
            bool reachedMaxSize;
            bool tookNoDamage;
            int timeBonus;
            int growthBonus;
            int untouchableBonus;
            int totalBonus;

            int calculateTotalBonus() const
            {
                return timeBonus + growthBonus + untouchableBonus;
            }
        };

        // Game state tracking
        struct GameStateData
        {
            int currentLevel = 1;
            int playerLives = Constants::INITIAL_LIVES;
            int totalScore = 0;
            sf::Time levelTime = sf::Time::Zero;
            bool levelComplete = false;
            bool gameWon = false;
            bool enemiesFleeing = false;
            sf::Time winTimer = sf::Time::Zero;
        };

        // HUD elements collection
        struct HUDElements
        {
            sf::Text scoreText;
            sf::Text livesText;
            sf::Text levelText;
            sf::Text fpsText;
            sf::Text messageText;
            sf::Text chainText;
            sf::Text powerUpText;
            sf::Text environmentText;
            sf::Text effectsText;
        };

    private:
        // Core update methods
        void updateGameplay(sf::Time deltaTime);
        void updateSystems(sf::Time deltaTime);
        void updateEntities(sf::Time deltaTime);
        void updateHUD();
        void updatePerformanceMetrics(sf::Time deltaTime);
        void updateEnvironmentalEffects(sf::Time deltaTime);
        void updateHazards(sf::Time deltaTime);
        void updatePlayerEffects(sf::Time deltaTime);

        // Template update methods
        template<typename Container>
        void updateContainer(Container& container, sf::Time deltaTime)
        {
            std::for_each(container.begin(), container.end(),
                [deltaTime](auto& item) {
                    if (item && item->isAlive())
                    {
                        item->update(deltaTime);
                    }
                });
        }

        template<typename Container, typename Predicate>
        void removeDeadEntities(Container& container, Predicate pred)
        {
            container.erase(
                std::remove_if(container.begin(), container.end(), pred),
                container.end()
            );
        }

        // Collision handling
        void checkCollisions();
        void checkHazardCollisions();

        template<typename Entity1, typename Entity2, typename Handler>
        void checkCollisionPair(Entity1& entity1, Entity2& entity2, Handler handler)
        {
            if (CollisionDetector::checkCircleCollision(entity1, entity2))
            {
                handler(entity1, entity2);
            }
        }

        template<typename Container, typename Entity, typename Handler>
        void checkCollisionsWithContainer(Entity& entity, Container& container, Handler handler)
        {
            std::for_each(container.begin(), container.end(),
                [&entity, &handler](auto& item) {
                    if (item && item->isAlive() &&
                        CollisionDetector::checkCircleCollision(entity, *item))
                    {
                        handler(*item);
                    }
                });
        }

        // Specialized collision handlers
        void handleFishCollision(Entity& fish);
        void handleBonusItemCollision(BonusItem& item);
        void handlePowerUpCollision(PowerUp& powerUp);
        void handleOysterCollision(PermanentOyster* oyster);
        void handleHazardCollision(Hazard& hazard);

        // Game flow
        void handlePlayerDeath();
        void advanceLevel();
        void gameOver();
        void checkWinCondition();
        void triggerWinSequence();
        void checkBonusStage();

        // Spawning methods
        void spawnHazards(sf::Time deltaTime);
        void spawnExtendedPowerUps();

        // Template utility methods
        template<typename MessageType>
        void showMessage(const MessageType& message)
        {
            std::ostringstream stream;
            stream << message;
            m_hud.messageText.setString(stream.str());
            centerText(m_hud.messageText);
        }

        void centerText(sf::Text& text);

        // Visual effects
        void createParticleEffect(const sf::Vector2f& position, const sf::Color& color, int count = Constants::DEFAULT_PARTICLE_COUNT);

        template<typename EffectParams>
        void createCustomEffect(const sf::Vector2f& position, const EffectParams& params);

        // Level management
        void updateLevelDifficulty();
        void resetLevel();
        void initializeSystems();

        // Helper methods
        bool areAllEnemiesGone() const;
        void makeAllEnemiesFlee();
        void applyFreeze();
        void reverseControls();

        // Template spawn management
        template<typename EntityType>
        void processSpawnedEntities(std::vector<std::unique_ptr<EntityType>>& source)
        {
            std::move(source.begin(), source.end(), std::back_inserter(m_entities));
            source.clear();
        }

    private:
        // Core game objects
        std::unique_ptr<Player> m_player;
        std::unique_ptr<EnhancedFishSpawner> m_fishSpawner;
        std::unique_ptr<SchoolingSystem> m_schoolingSystem;
        std::vector<std::unique_ptr<Entity>> m_entities;
        std::vector<std::unique_ptr<BonusItem>> m_bonusItems;
        std::vector<std::unique_ptr<Hazard>> m_hazards;

        // Environmental system
        std::unique_ptr<EnvironmentSystem> m_environmentSystem;

        // Game systems (using unordered_map for extensibility)
        std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>> m_systems;

        // Direct system pointers for performance
        GrowthMeter* m_growthMeter;
        FrenzySystem* m_frenzySystem;
        PowerUpManager* m_powerUpManager;
        ScoreSystem* m_scoreSystem;
        BonusItemManager* m_bonusItemManager;
        FixedOysterManager* m_oysterManager;

        // State tracking
        GameStateData m_gameState;
        LevelStats m_levelStats;
        HUDElements m_hud;

        // Extended state tracking for Stage 4
        bool m_isPlayerFrozen;
        bool m_hasControlsReversed;
        bool m_isPlayerStunned;
        sf::Time m_controlReverseTimer;
        sf::Time m_freezeTimer;
        sf::Time m_stunTimer;
        sf::Time m_hazardSpawnTimer;
        sf::Time m_extendedPowerUpSpawnTimer;

        // Bonus stage tracking
        int m_levelsUntilBonus;
        bool m_bonusStageTriggered;
        bool m_returningFromBonusStage;
        int m_savedLevel;

        // Performance tracking
        struct PerformanceMetrics
        {
            sf::Time fpsUpdateTime = sf::Time::Zero;
            int frameCount = 0;
            float currentFPS = 0.0f;
        } m_metrics;

        // Visual effects
        std::vector<ParticleEffect> m_particles;

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_angleDist;
        std::uniform_real_distribution<float> m_speedDist;
        std::uniform_int_distribution<int> m_hazardTypeDist;
        std::uniform_int_distribution<int> m_extendedPowerUpDist;

        // Spawn rates for Stage 4
        static constexpr float m_hazardSpawnInterval = 8.0f;
        static constexpr float m_extendedPowerUpInterval = 15.0f;
    };
}