#pragma once

#include "BonusItem.h"

namespace FishGame
{
    // Power-up types
    enum class PowerUpType
    {
        ScoreDoubler,
        FrenzyStarter,
        SpeedBoost,
        Invincibility
    };

    // Base class for all power-ups
    class PowerUp : public BonusItem
    {
    public:
        PowerUp(PowerUpType type, sf::Time duration);
        virtual ~PowerUp() = default;

        // Power-up interface
        PowerUpType getPowerUpType() const { return m_powerUpType; }
        sf::Time getDuration() const { return m_duration; }

        // Visual indicator
        virtual sf::Color getAuraColor() const = 0;

    protected:
        PowerUpType m_powerUpType;
        sf::Time m_duration;

        // Shared visual components
        sf::CircleShape m_iconBackground;
        sf::CircleShape m_aura;
        float m_pulseAnimation;
    };

    // Score Doubler - doubles all points for duration
    class ScoreDoublerPowerUp : public PowerUp
    {
    public:
        ScoreDoublerPowerUp();
        ~ScoreDoublerPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;
        sf::Color getAuraColor() const override { return sf::Color::Yellow; }

        void setFont(const sf::Font& font) { m_icon.setFont(font); }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_icon; // "2X" text
        static constexpr float m_doubleDuration = 10.0f;
    };

    // Frenzy Starter - instantly activates Frenzy Mode
    class FrenzyStarterPowerUp : public PowerUp
    {
    public:
        FrenzyStarterPowerUp();
        ~FrenzyStarterPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;
        sf::Color getAuraColor() const override { return sf::Color::Magenta; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        std::vector<sf::CircleShape> m_lightningBolts;
        float m_sparkAnimation;
    };

    // Template factory for creating power-ups
    template<typename T>
    class PowerUpFactory
    {
        static_assert(std::is_base_of_v<PowerUp, T>, "T must be derived from PowerUp");

    public:
        static std::unique_ptr<PowerUp> create()
        {
            return std::make_unique<T>();
        }
    };

    // Power-up manager to handle active effects
    class PowerUpManager
    {
    public:
        PowerUpManager();
        ~PowerUpManager() = default;

        // Delete copy operations
        PowerUpManager(const PowerUpManager&) = delete;
        PowerUpManager& operator=(const PowerUpManager&) = delete;

        // Allow move operations
        PowerUpManager(PowerUpManager&&) = default;
        PowerUpManager& operator=(PowerUpManager&&) = default;

        // Power-up management
        void activatePowerUp(PowerUpType type, sf::Time duration);
        void update(sf::Time deltaTime);
        void reset();

        // Query active effects
        bool isActive(PowerUpType type) const;
        sf::Time getRemainingTime(PowerUpType type) const;
        float getScoreMultiplier() const;

        // Get all active power-ups
        std::vector<PowerUpType> getActivePowerUps() const;

    private:
        struct ActivePowerUp
        {
            PowerUpType type;
            sf::Time remainingTime;
        };

        std::vector<ActivePowerUp> m_activePowerUps;

        // Use template to find power-up in vector
        template<typename Predicate>
        auto findPowerUp(Predicate pred)
        {
            return std::find_if(m_activePowerUps.begin(), m_activePowerUps.end(), pred);
        }
    };
}