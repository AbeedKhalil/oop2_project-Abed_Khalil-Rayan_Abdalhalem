#include "EnvironmentController.h"
#include "Fish.h"
#include "GameConstants.h"
#include "Entity.h"

namespace FishGame {

EnvironmentController::EnvironmentController(EnvironmentSystem& env,
                                             Player& player,
                                             std::vector<std::unique_ptr<Entity>>& entities,
                                             IAudioPlayer& sounds)
    : m_environment(env), m_player(player), m_entities(entities), m_soundPlayer(sounds) {}

void EnvironmentController::update(sf::Time dt)
{
    m_environment.update(dt);
    updateEffectTimers(dt);
    applyEnvironmentalForces(dt);
}

void EnvironmentController::updateEffectTimers(sf::Time dt)
{
    if (m_isPlayerFrozen) {
        m_freezeTimer -= dt;
        if (m_freezeTimer <= sf::Time::Zero) {
            m_isPlayerFrozen = false;
            EntityUtils::forEachAlive(m_entities, [](Entity& e) {
                if (auto* f = dynamic_cast<Fish*>(&e))
                    f->setFrozen(false);
            });
        }
    }

    if (m_hasControlsReversed) {
        m_controlReverseTimer -= dt;
        if (m_controlReverseTimer <= sf::Time::Zero) {
            m_hasControlsReversed = false;
            m_player.setControlsReversed(false);
        }
    }

    if (m_isPlayerStunned) {
        m_stunTimer -= dt;
        if (m_stunTimer <= sf::Time::Zero)
            m_isPlayerStunned = false;
    }
}

void EnvironmentController::applyFreeze()
{
    m_isPlayerFrozen = true;
    m_freezeTimer = sf::seconds(5.f);
    m_soundPlayer.playSound(SoundEffectID::FreezePowerup);
    EntityUtils::forEachAlive(m_entities, [](Entity& e) {
        if (auto* f = dynamic_cast<Fish*>(&e))
            f->setFrozen(true);
    });
}

void EnvironmentController::reverseControls()
{
    m_hasControlsReversed = true;
    m_player.setControlsReversed(true);
}

void EnvironmentController::applyEnvironmentalForces(sf::Time dt)
{
    if (!m_isPlayerStunned) {
        sf::Vector2f force = m_environment.getOceanCurrentForce(m_player.getPosition());
        m_player.setVelocity(m_player.getVelocity() + force * dt.asSeconds() * 0.3f);
    }

    EntityUtils::forEachAlive(m_entities, [this, dt](Entity& e) {
        sf::Vector2f force = m_environment.getOceanCurrentForce(e.getPosition());
        e.setVelocity(e.getVelocity() + force * dt.asSeconds() * 0.1f);
    });
}

void EnvironmentController::reset()
{
    m_isPlayerFrozen = false;
    m_hasControlsReversed = false;
    m_isPlayerStunned = false;
    m_controlReverseTimer = sf::Time::Zero;
    m_freezeTimer = sf::Time::Zero;
    m_stunTimer = sf::Time::Zero;
}

} // namespace FishGame
