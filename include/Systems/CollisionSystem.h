#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "Player.h"
#include "BonusItem.h"
#include "Hazard.h"
#include "ParticleSystem.h"
#include "ScoreSystem.h"
#include "FrenzySystem.h"
#include "PowerUp.h"
#include "OysterManager.h"
#include "FishCollisionHandler.h"

namespace FishGame
{
    class CollisionSystem
    {
    public:
        CollisionSystem(ParticleSystem& particles, ScoreSystem& score,
                        FrenzySystem& frenzy, PowerUpManager& powerUps,
                        std::unordered_map<TextureID,int>& levelCounts,
                        SoundPlayer& sounds,
                        bool& playerStunned, sf::Time& stunTimer,
                        sf::Time& controlReverseTimer, int& playerLives,
                        std::function<void()> onPlayerDeath,
                        std::function<void()> applyFreeze,
                        std::function<void()> reverseControls);

        void process(Player& player,
                     std::vector<std::unique_ptr<Entity>>& entities,
                     std::vector<std::unique_ptr<BonusItem>>& bonusItems,
                     std::vector<std::unique_ptr<Hazard>>& hazards,
                     FixedOysterManager* oysters, int currentLevel);

    private:
        void createParticle(const sf::Vector2f& pos, const sf::Color& color, int count = Constants::DEFAULT_PARTICLE_COUNT);
        void handlePowerUpCollision(Player& player, PowerUp& powerUp);
        void handleOysterCollision(Player& player, PermanentOyster* oyster);

        struct FishCollisionHandler;
        struct BonusItemCollisionHandler;
        struct HazardCollisionHandler;

        ParticleSystem& m_particles;
        ScoreSystem& m_scoreSystem;
        FrenzySystem& m_frenzySystem;
        PowerUpManager& m_powerUps;
        std::unordered_map<TextureID,int>& m_levelCounts;
        SoundPlayer& m_sounds;
        bool& m_playerStunned;
        sf::Time& m_stunTimer;
        sf::Time& m_controlReverseTimer;
        int& m_playerLives;
        std::function<void()> m_onPlayerDeath;
        std::function<void()> m_applyFreeze;
        std::function<void()> m_reverseControls;
    };
}
