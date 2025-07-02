#pragma once

#include <SFML/Graphics.hpp>
#include "PowerUp.h"
#include "OysterManager.h"
#include "GameConstants.h"

namespace FishGame {
    class PlayState;

    // Separate class responsible for PlayState game logic.
    class PlayLogic
    {
    public:
        explicit PlayLogic(PlayState& state);

        void handleEvent(const sf::Event& event);
        bool update(sf::Time deltaTime);

    private:
        // Gameplay update helpers moved from PlayState
        void updateGameplay(sf::Time deltaTime);
        void updateRespawn(sf::Time deltaTime);
        void updateEnvironment(sf::Time deltaTime);
        void updateGameState(sf::Time deltaTime);
        void updateEntities(sf::Time deltaTime);
        void updateSpawning(sf::Time deltaTime);
        void updateSystems(sf::Time deltaTime);
        void updateHUD();
        void updatePerformanceMetrics(sf::Time deltaTime);
        void updateCamera();
        void updateEffectTimers(sf::Time deltaTime);
        void applyEnvironmentalForces(sf::Time deltaTime);

        void handlePowerUpCollision(PowerUp& powerUp);
        void handleOysterCollision(PermanentOyster* oyster);

        void handlePlayerDeath();
        void advanceLevel();
        void gameOver();
        void checkWinCondition();
        void triggerWinSequence();
        void checkBonusStage();
        void updateLevelDifficulty();
        void resetLevel();
        bool areAllEnemiesGone() const;
        void makeAllEnemiesFlee();
        void applyFreeze();
        void reverseControls();
        void createParticleEffect(const sf::Vector2f& position, const sf::Color& color, int count = Constants::DEFAULT_PARTICLE_COUNT);
        void showMessage(const std::string& message);

        PlayState& m_state;
    };
}

