#include "CollisionSystem.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include "Pufferfish.h"
#include "Angelfish.h"
#include "PoisonFish.h"
#include <algorithm>

namespace FishGame
{
    CollisionSystem::CollisionSystem(ParticleSystem& particles, IScoreSystem& score,
                                     FrenzySystem& frenzy, IPowerUpManager& powerUps,
                                     std::unordered_map<TextureID,int>& levelCounts,
                                     SoundPlayer& sounds,
                                     bool& playerStunned, sf::Time& stunTimer,
                                     sf::Time& controlReverseTimer, int& playerLives,
                                     std::function<void()> onPlayerDeath,
                                     std::function<void()> applyFreeze,
                                     std::function<void()> reverseControls)
        : m_particles(particles)
        , m_scoreSystem(score)
        , m_frenzySystem(frenzy)
        , m_powerUps(powerUps)
        , m_levelCounts(levelCounts)
        , m_sounds(sounds)
        , m_playerStunned(playerStunned)
        , m_stunTimer(stunTimer)
        , m_controlReverseTimer(controlReverseTimer)
        , m_playerLives(playerLives)
        , m_onPlayerDeath(std::move(onPlayerDeath))
        , m_applyFreeze(std::move(applyFreeze))
        , m_reverseControls(std::move(reverseControls))
    {
    }

    void CollisionSystem::createParticle(const sf::Vector2f& pos, const sf::Color& color, int count)
    {
        m_particles.createEffect(pos, color, count);
    }

    // --- Collision Handlers ------------------------------------------------

    // --- Helper methods ----------------------------------------------------
    void CollisionSystem::handlePowerUpCollision(Player& player, PowerUp& powerUp)
    {
        powerUp.applyEffect(player, *this);
    }

    void CollisionSystem::handleOysterCollision(Player& player, PermanentOyster* oyster)
    {
        if (oyster->canDamagePlayer() && !player.isInvulnerable())
        {
            player.takeDamage();
            m_onPlayerDeath();
            createParticle(player.getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
        }
        else if (oyster->canBeEaten())
        {
            oyster->onCollect();
            m_sounds.play(SoundEffectID::OysterPearl);

            int points = oyster->hasBlackPearl() ? Constants::BLACK_OYSTER_POINTS
                                                 : Constants::WHITE_OYSTER_POINTS;

            player.addPoints(points);
            player.grow(oyster->getGrowthPoints());

            int frenzyMultiplier = m_frenzySystem.getMultiplier();
            float powerUpMultiplier = m_powerUps.getScoreMultiplier();

            m_scoreSystem.addScore(ScoreEventType::BonusCollected, oyster->getPoints(),
                                   oyster->getPosition(), frenzyMultiplier, powerUpMultiplier);

            createParticle(oyster->getPosition(),
                           oyster->hasBlackPearl() ? Constants::BLACK_PEARL_COLOR : Constants::WHITE_PEARL_COLOR);
        }
    }

    // --- Process -----------------------------------------------------------
    void CollisionSystem::process(Player& player,
                                  std::vector<std::unique_ptr<Entity>>& entities,
                                  std::vector<std::unique_ptr<BonusItem>>& bonusItems,
                                  std::vector<std::unique_ptr<Hazard>>& hazards,
                                  FixedOysterManager* oysters,
                                  int currentLevel)
    {
        EntityUtils::forEachAlive(entities, [this,&player](Entity& e){
            if (EntityUtils::areColliding(player, e)) {
                e.onCollide(player, *this);
            }
        });

        EntityUtils::forEachAlive(bonusItems, [this,&player](auto& e){
            if (EntityUtils::areColliding(player, e)) {
                e.onCollide(player, *this);
            }
        });

        EntityUtils::forEachAlive(hazards, [this,&player](auto& h){
            if (EntityUtils::areColliding(player, h)) {
                h.onCollide(player, *this);
            }
        });

        if (currentLevel >= 2 && oysters)
        {
            oysters->checkCollisions(player, [this,&player](PermanentOyster* o){ handleOysterCollision(player,o); });
        }

        StateUtils::processCollisionsBetween(entities, entities,
            [this](Entity& a, Entity& b){ a.onCollideWith(b, *this); });

        StateUtils::processCollisionsBetween(entities, hazards,
            [this](Entity& e, Hazard& h){ e.onCollideWith(h, *this); });
        processBombExplosions(entities, hazards);

        StateUtils::applyToEntities(entities, [this,&player](Entity& e){
            if (player.attemptTailBite(e))
            {
                createParticle(player.getPosition(), Constants::TAILBITE_PARTICLE_COLOR);
            }
        });

        if (currentLevel >= 2 && oysters)
        {
            StateUtils::applyToEntities(entities, [this,oysters](Entity& e){
                oysters->checkCollisions(e, [this,&e](PermanentOyster* o){
                    e.onCollideWith(*o, *this);
                });
            });
        }
    }
}
