#pragma once

#include "EnvironmentSystem.h"
#include "Player.h"
#include "Entity.h"
#include "SoundPlayer.h"
#include <vector>
#include <memory>

namespace FishGame {

class EnvironmentController {
public:
    EnvironmentController(EnvironmentSystem& env, Player& player,
                          std::vector<std::unique_ptr<Entity>>& entities,
                          SoundPlayer& sounds);

    void update(sf::Time dt);

    void reset();

    void applyFreeze();
    void reverseControls();

    bool isPlayerFrozen() const { return m_isPlayerFrozen; }
    bool hasControlsReversed() const { return m_hasControlsReversed; }
    bool isPlayerStunned() const { return m_isPlayerStunned; }

    sf::Time getFreezeTimer() const { return m_freezeTimer; }
    sf::Time getControlReverseTimer() const { return m_controlReverseTimer; }
    sf::Time getStunTimer() const { return m_stunTimer; }

    bool& stunnedRef() { return m_isPlayerStunned; }
    sf::Time& stunTimerRef() { return m_stunTimer; }
    sf::Time& controlReverseTimerRef() { return m_controlReverseTimer; }

private:
    void updateEffectTimers(sf::Time dt);
    void applyEnvironmentalForces(sf::Time dt);

    EnvironmentSystem& m_environment;
    Player& m_player;
    std::vector<std::unique_ptr<Entity>>& m_entities;
    SoundPlayer& m_soundPlayer;

    bool m_isPlayerFrozen{false};
    bool m_hasControlsReversed{false};
    bool m_isPlayerStunned{false};
    sf::Time m_controlReverseTimer{sf::Time::Zero};
    sf::Time m_freezeTimer{sf::Time::Zero};
    sf::Time m_stunTimer{sf::Time::Zero};
};

}
