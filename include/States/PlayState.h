#pragma once

#include "Player.h"
#include "EnhancedFishSpawner.h"
#include "SchoolingSystem.h"
#include "FishCollisionHandler.h"
#include "SpecialFish.h"
#include "GrowthMeter.h"
#include "FrenzySystem.h"
#include "BonusItemManager.h"
#include "OysterManager.h"
#include "ScoreSystem.h"
#include "HUDSystem.h"
#include "ParticleSystem.h"
#include "InputHandler.h"
#include "PowerUp.h"
#include "ExtendedPowerUps.h"
#include "Hazard.h"
#include "EnvironmentSystem.h"
#include "BonusStageState.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <vector>
#include <random>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <numeric>
#include <type_traits>

namespace FishGame
{
    enum class TextureID;
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
        void onDeactivate() override;

    private:
        // ==================== Type Definitions ====================

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

        // Performance metrics
        struct PerformanceMetrics
        {
            sf::Time fpsUpdateTime = sf::Time::Zero;
            int frameCount = 0;
            float currentFPS = 0.0f;
        };

        // ==================== Collision Handler Functors ====================

        struct FishCollisionHandler
        {
            PlayState* state;

            void operator()(Entity& fish) const;
        };

        struct BonusItemCollisionHandler
        {
            PlayState* state;

            void operator()(BonusItem& item) const;
        };

        struct HazardCollisionHandler
        {
            PlayState* state;

            void operator()(Hazard& hazard) const;
        };

        // ==================== Helper Functions ====================

        // Spawning helpers
        void spawnRandomHazard();
        void spawnRandomPowerUp();
        sf::Vector2f generateRandomPosition();

        // Effect helpers
        void createParticleEffect(const sf::Vector2f& position, const sf::Color& color,
            int count = Constants::DEFAULT_PARTICLE_COUNT);
        void applyEnvironmentalForces(sf::Time deltaTime);

        // State helpers
        bool shouldSpawnSpecialEntity(sf::Time& timer, float interval);
        void updateEffectTimers(sf::Time deltaTime);

        // ==================== Core Methods ====================

        // Initialization
        void initializeSystems();
        template<typename SystemType>
        SystemType* createAndStoreSystem(const std::string& name, const sf::Font& font);

        // Update methods
        void updateGameplay(sf::Time deltaTime);
        void updateSystems(sf::Time deltaTime);
        void updateAllEntities(sf::Time deltaTime);
        void updateHUD();
        void updatePerformanceMetrics(sf::Time deltaTime);
        void updateCamera();

        // Collision handling
        void checkCollisions();
        void handlePowerUpCollision(PowerUp& powerUp);
        void handleOysterCollision(PermanentOyster* oyster);

        // Game flow
        void handlePlayerDeath();
        void advanceLevel();
        void gameOver();
        void checkWinCondition();
        void triggerWinSequence();
        void checkBonusStage();

        // Level management
        void updateLevelDifficulty();
        void resetLevel();

        // Helper methods
        bool areAllEnemiesGone() const;
        void makeAllEnemiesFlee();
        void applyFreeze();
        void reverseControls();
        void showMessage(const std::string& message);
        void updateBackground(int level);

    private:
        // ==================== Core Game Objects ====================
        std::unique_ptr<Player> m_player;
        std::unique_ptr<EnhancedFishSpawner> m_fishSpawner;
        std::unique_ptr<SchoolingSystem> m_schoolingSystem;
        std::vector<std::unique_ptr<Entity>> m_entities;
        std::vector<std::unique_ptr<BonusItem>> m_bonusItems;
        std::vector<std::unique_ptr<Hazard>> m_hazards;

        // Environmental system
        std::unique_ptr<EnvironmentSystem> m_environmentSystem;

        // Game systems
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
        std::unique_ptr<HUDSystem> m_hudSystem;
        std::unordered_map<TextureID, int> m_levelCounts;

        // Effect states
        bool m_isPlayerFrozen;
        bool m_hasControlsReversed;
        bool m_isPlayerStunned;
        sf::Time m_controlReverseTimer;
        sf::Time m_freezeTimer;
        sf::Time m_stunTimer;
        sf::Time m_hazardSpawnTimer;
        sf::Time m_extendedPowerUpSpawnTimer;
        InputHandler m_inputHandler;

        // Bonus stage tracking
        bool m_bonusStageTriggered;
        bool m_returningFromBonusStage;
        int m_savedLevel;

        // Performance tracking
        PerformanceMetrics m_metrics;

        std::unique_ptr<ParticleSystem> m_particleSystem;

        // Camera and background
        sf::Sprite m_backgroundSprite;
        sf::View m_view;
        sf::Vector2f m_worldSize;

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_angleDist;
        std::uniform_real_distribution<float> m_speedDist;
        std::uniform_real_distribution<float> m_positionDist;
        std::uniform_int_distribution<int> m_hazardTypeDist;
        std::uniform_int_distribution<int> m_powerUpTypeDist;

        bool m_initialized;

        // Track resuming of background music after death
        bool m_musicResumePending{false};
        sf::Time m_musicResumeTimer{sf::Time::Zero};

        bool m_respawnPending{false};
        sf::Time m_respawnTimer{sf::Time::Zero};

        // Camera freeze when player dies
        bool m_cameraFrozen{false};
        sf::Vector2f m_cameraFreezePos;

        // Constants
        static constexpr float m_hazardSpawnInterval = 8.0f;
        static constexpr float m_extendedPowerUpInterval = 15.0f;
        static constexpr float m_cameraSmoothing = 0.1f;
    };
}
