#pragma once

#include "BonusItem.h"
#include "SpriteComponent.h"
#include "Utils/SpriteDrawable.h"

namespace FishGame
{
    // Extended power-up types
    enum class PowerUpType
    {
        ScoreDoubler,
        FrenzyStarter,
        SpeedBoost,
        Freeze,
        ExtraLife
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

        float m_pulseAnimation;

        // Common update logic shared by many power-ups
        void commonUpdate(sf::Time deltaTime, float pulseSpeed,
            float freqMul = 1.f, float ampMul = 1.f);
    };

    // Score Doubler - doubles all points for duration
    class ScoreDoublerPowerUp : public PowerUp, public SpriteDrawable<ScoreDoublerPowerUp>
    {
    public:
        ScoreDoublerPowerUp();
        ~ScoreDoublerPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        sf::Color getAuraColor() const override { return sf::Color::Yellow; }

        void setFont(const sf::Font& font) {}

    };

    // Frenzy Starter - instantly activates Frenzy Mode
    class FrenzyStarterPowerUp : public PowerUp, public SpriteDrawable<FrenzyStarterPowerUp>
    {
    public:
        FrenzyStarterPowerUp();
        ~FrenzyStarterPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        sf::Color getAuraColor() const override { return sf::Color::Magenta; }

    private:
        float m_sparkAnimation;
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

        // Specific power-up queries
        bool isFreezeActive() const { return isActive(PowerUpType::Freeze); }
        bool hasSpeedBoost() const { return isActive(PowerUpType::SpeedBoost); }

        // Get effect values
        float getSpeedMultiplier() const;

    private:
        struct ActivePowerUp
        {
            PowerUpType type = PowerUpType::ScoreDoubler;
            sf::Time remainingTime = sf::Time::Zero;
        };

        std::vector<ActivePowerUp> m_activePowerUps;

        // Use template to find power-up in vector
        template<typename Predicate, typename Container>
        static auto findPowerUpImpl(Container& container, Predicate pred)
        {
            return std::find_if(container.begin(), container.end(), pred);
        }

        template<typename Predicate>
        auto findPowerUp(Predicate pred)
        {
            return findPowerUpImpl(m_activePowerUps, pred);
        }

        template<typename Predicate>
        auto findPowerUp(Predicate pred) const
        {
            return findPowerUpImpl(m_activePowerUps, pred);
        }
    };
}