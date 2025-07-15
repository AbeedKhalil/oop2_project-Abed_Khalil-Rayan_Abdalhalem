#pragma once

#include <SFML/System/Time.hpp>

namespace FishGame
{
    class Player;
    class Entity;

    class PlayerStatus
    {
    public:
        explicit PlayerStatus(Player& player);

        void update(sf::Time deltaTime);

        bool canEat(const Entity& other) const;
        bool attemptEat(Entity& other);
        bool canTailBite(const Entity& other) const;
        bool attemptTailBite(Entity& other);

        void takeDamage();
        void die();
        void respawn();

        bool isInvulnerable() const { return m_invulnerabilityTimer > sf::Time::Zero; }
        bool hasRecentlyTakenDamage() const { return m_damageCooldown > sf::Time::Zero; }
        sf::Time getInvulnerabilityTimer() const { return m_invulnerabilityTimer; }

    private:
        Player& m_player;
        sf::Time m_invulnerabilityTimer;
        sf::Time m_damageCooldown;
        static const sf::Time m_invulnerabilityDuration;
        static const sf::Time m_damageCooldownDuration;
    };
}
