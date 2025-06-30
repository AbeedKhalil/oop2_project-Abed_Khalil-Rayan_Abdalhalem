#include "CollisionSystem.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <algorithm>

namespace FishGame
{
    CollisionSystem::CollisionSystem(ParticleSystem& particles, ScoreSystem& score,
                                     FrenzySystem& frenzy, PowerUpManager& powerUps,
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
    struct CollisionSystem::FishCollisionHandler
    {
        CollisionSystem* system;
        Player* player;
        void operator()(Entity& entity) const
        {
            if (player->isInvulnerable() || system->m_playerStunned)
                return;

            if (auto* puffer = dynamic_cast<Pufferfish*>(&entity))
            {
                if (puffer->isInflated())
                {
                    if (!player->hasRecentlyTakenDamage())
                    {
                        puffer->pushEntity(*player);
                        system->m_playerStunned = true;
                        system->m_stunTimer = Constants::PUFFERFISH_STUN_DURATION;
                        player->setVelocity(0.0f, 0.0f);
                        system->m_sounds.play(SoundEffectID::PufferBounce);
                        int penalty = Constants::PUFFERFISH_SCORE_PENALTY;
                        system->m_scoreSystem.setCurrentScore(
                            std::max(0, system->m_scoreSystem.getCurrentScore() - penalty));
                        system->createParticle(player->getPosition(), Constants::PUFFERFISH_IMPACT_COLOR);
                    }
                }
                else if (player->canEat(entity))
                {
                    if (player->attemptEat(entity))
                    {
                        system->m_levelCounts[puffer->getTextureID()]++;
                        system->m_sounds.play(SoundEffectID::Bite2);
                        entity.destroy();
                        system->createParticle(entity.getPosition(), Constants::EAT_PARTICLE_COLOR);
                    }
                }
                else if (puffer->canEat(*player) && !player->hasRecentlyTakenDamage())
                {
                    player->takeDamage();
                    system->createParticle(player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
                    system->m_onPlayerDeath();
                }
            }
            else if (auto* angelfish = dynamic_cast<Angelfish*>(&entity))
            {
                if (player->canEat(entity) && player->attemptEat(entity))
                {
                    system->m_levelCounts[angelfish->getTextureID()]++;
                    system->m_sounds.play(SoundEffectID::Bite1);
                    system->createParticle(entity.getPosition(),
                        Constants::ANGELFISH_PARTICLE_COLOR, Constants::ANGELFISH_PARTICLE_COUNT);
                    entity.destroy();
                }
            }
            else if (auto* poison = dynamic_cast<PoisonFish*>(&entity))
            {
                if (player->canEat(entity) && player->attemptEat(entity))
                {
                    system->m_reverseControls();
                    system->m_controlReverseTimer = poison->getPoisonDuration();
                    player->applyPoisonEffect(poison->getPoisonDuration());
                    system->m_sounds.play(SoundEffectID::PlayerPoison);
                    system->createParticle(entity.getPosition(), sf::Color::Magenta, 15);
                    system->createParticle(player->getPosition(), sf::Color::Magenta, 10);
                    system->m_levelCounts[poison->getTextureID()]++;
                    entity.destroy();
                }
            }
            else if (auto* regularFish = dynamic_cast<Fish*>(&entity))
            {
                bool playerCanEat = player->canEat(entity);
                bool fishCanEatPlayer = regularFish->canEat(*player);

                if (playerCanEat && player->attemptEat(entity))
                {
                    system->m_levelCounts[regularFish->getTextureID()]++;
                    SoundEffectID effect = SoundEffectID::Bite1;
                    switch (regularFish->getSize())
                    {
                    case FishSize::Small: effect = SoundEffectID::Bite1; break;
                    case FishSize::Medium: effect = SoundEffectID::Bite2; break;
                    case FishSize::Large: effect = SoundEffectID::Bite3; break;
                    }
                    system->m_sounds.play(effect);
                    entity.destroy();
                    system->createParticle(entity.getPosition(), Constants::EAT_PARTICLE_COLOR);
                }
                else if (fishCanEatPlayer && !player->hasRecentlyTakenDamage())
                {
                    regularFish->playEatAnimation();
                    player->takeDamage();
                    system->createParticle(player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
                    system->m_onPlayerDeath();
                }
            }
        }
    };

    struct CollisionSystem::BonusItemCollisionHandler
    {
        CollisionSystem* system;
        Player* player;
        void operator()(BonusItem& item) const
        {
            item.onCollect();

            if (auto* powerUp = dynamic_cast<PowerUp*>(&item))
            {
                system->handlePowerUpCollision(*player, *powerUp);
            }
            else
            {
                if (item.getBonusType() == BonusType::Starfish)
                {
                    system->m_levelCounts[TextureID::Starfish]++;
                    system->m_scoreSystem.recordFish(TextureID::Starfish);
                    system->m_sounds.play(SoundEffectID::StarPickup);
                }
                int frenzyMultiplier = system->m_frenzySystem.getMultiplier();
                float powerUpMultiplier = system->m_powerUps.getScoreMultiplier();

                system->m_scoreSystem.addScore(ScoreEventType::BonusCollected, item.getPoints(),
                                               item.getPosition(), frenzyMultiplier, powerUpMultiplier);

                system->createParticle(item.getPosition(), Constants::BONUS_PARTICLE_COLOR);
            }
        }
    };

    struct CollisionSystem::HazardCollisionHandler
    {
        CollisionSystem* system;
        Player* player;
        void operator()(Hazard& hazard) const
        {
            if (player->isInvulnerable())
                return;

            switch (hazard.getHazardType())
            {
            case HazardType::Bomb:
                if (auto* bomb = dynamic_cast<Bomb*>(&hazard))
                {
                    bomb->onContact(*player);
                    system->m_sounds.play(SoundEffectID::MineExplode);
                    player->takeDamage();
                    system->m_onPlayerDeath();
                    system->createParticle(player->getPosition(), sf::Color::Red, 20);
                }
                break;

            case HazardType::Jellyfish:
                if (auto* jelly = dynamic_cast<Jellyfish*>(&hazard))
                {
                    jelly->onContact(*player);
                    system->m_playerStunned = true;
                    system->m_stunTimer = jelly->getStunDuration();
                    player->setVelocity(0.0f, 0.0f);
                    system->m_sounds.play(SoundEffectID::PlayerStunned);
                    system->createParticle(player->getPosition(), sf::Color(255,255,0,150), 10);
                }
                break;
            }
        }
    };

    // --- Helper methods ----------------------------------------------------
    void CollisionSystem::handlePowerUpCollision(Player& player, PowerUp& powerUp)
    {
        switch (powerUp.getPowerUpType())
        {
        case PowerUpType::ScoreDoubler:
            m_powerUps.activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            createParticle(powerUp.getPosition(), Constants::SCORE_DOUBLER_COLOR);
            break;
        case PowerUpType::FrenzyStarter:
            m_frenzySystem.forceFrenzy();
            createParticle(powerUp.getPosition(), Constants::FRENZY_STARTER_COLOR);
            break;
        case PowerUpType::SpeedBoost:
            m_powerUps.activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            player.applySpeedBoost(m_powerUps.getSpeedMultiplier(), powerUp.getDuration());
            m_sounds.play(SoundEffectID::SpeedStart);
            createParticle(powerUp.getPosition(), Constants::SPEED_BOOST_COLOR);
            break;
        case PowerUpType::Freeze:
            m_powerUps.activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            m_applyFreeze();
            createParticle(powerUp.getPosition(), sf::Color::Cyan, 20);
            break;
        case PowerUpType::ExtraLife:
            m_playerLives++;
            m_sounds.play(SoundEffectID::LifePowerup);
            createParticle(powerUp.getPosition(), sf::Color::Green, 15);
            break;
        case PowerUpType::AddTime:
            break;
        }
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
                FishCollisionHandler{this,&player}(e);
            }
        });

        EntityUtils::forEachAlive(bonusItems, [this,&player](Entity& e){
            if (EntityUtils::areColliding(player, *static_cast<BonusItem*>(&e))) {
                BonusItemCollisionHandler{this,&player}(*static_cast<BonusItem*>(&e));
            }
        });

        EntityUtils::forEachAlive(hazards, [this,&player](Entity& h){
            if (EntityUtils::areColliding(player, *static_cast<Hazard*>(&h))) {
                HazardCollisionHandler{this,&player}(*static_cast<Hazard*>(&h));
            }
        });

        if (currentLevel >= 2 && oysters)
        {
            oysters->checkCollisions(player, [this,&player](PermanentOyster* o){ handleOysterCollision(player,o); });
        }

        StateUtils::processCollisionsBetween(entities, entities,
            [this](Entity& a, Entity& b){
                auto* f1 = dynamic_cast<Fish*>(&a);
                auto* f2 = dynamic_cast<Fish*>(&b);
                if (!f1 || !f2) return;
                if (f1->canEat(*f2))
                {
                    if (auto* poison = dynamic_cast<PoisonFish*>(f2))
                    {
                        f1->setPoisoned(poison->getPoisonDuration());
                        createParticle(f1->getPosition(), sf::Color::Magenta, 10);
                    }
                    f1->playEatAnimation();
                    f2->destroy();
                    createParticle(f2->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
                else if (f2->canEat(*f1))
                {
                    if (auto* poison = dynamic_cast<PoisonFish*>(f1))
                    {
                        f2->setPoisoned(poison->getPoisonDuration());
                        createParticle(f2->getPosition(), sf::Color::Magenta, 10);
                    }
                    f2->playEatAnimation();
                    f1->destroy();
                    createParticle(f1->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
            });

        ::FishGame::FishCollisionHandler::processFishHazardCollisions(entities, hazards, &m_sounds);
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
                if (auto* fish = dynamic_cast<Fish*>(&e))
                {
                    oysters->checkCollisions(*fish, [this,fish](PermanentOyster* o){
                        if (o->canDamagePlayer())
                        {
                            fish->destroy();
                            createParticle(fish->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                            createParticle(o->getPosition(), Constants::OYSTER_IMPACT_COLOR);
                        }
                    });
                }
            });
        }
    }
}
