#pragma once

#include "BonusItem.h"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <type_traits>

namespace FishGame
{
    // Extended power-up types
    enum class PowerUpType
    {
        ScoreDoubler,
        FrenzyStarter,
        SpeedBoost,
        Invincibility,
        Freeze,
        ExtraLife,
        Shield
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

        // Template method for creating with custom parameters
        template<typename... Args>
        static std::unique_ptr<PowerUp> createWithParams(Args&&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
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

        // Specific power-up queries
        bool hasShield() const { return isActive(PowerUpType::Shield); }
        bool isFreezeActive() const { return isActive(PowerUpType::Freeze); }
        bool hasSpeedBoost() const { return isActive(PowerUpType::SpeedBoost); }
        bool hasInvincibility() const { return isActive(PowerUpType::Invincibility); }

        // Get effect values
        float getSpeedMultiplier() const;
        float getFreezeSlowdown() const { return m_freezeSlowdown; }

        // Template method for applying effects
        template<typename Entity>
        void applyEffects(Entity& entity)
        {
            if (hasSpeedBoost())
            {
                entity.setSpeedMultiplier(m_speedBoostMultiplier);
            }
            if (hasShield())
            {
                entity.enableShield();
            }
        }

    private:
        struct ActivePowerUp
        {
            PowerUpType type = PowerUpType::ScoreDoubler;
            sf::Time remainingTime = sf::Time::Zero;
        };

        std::vector<ActivePowerUp> m_activePowerUps;

        // Use template to find power-up in vector
        template<typename Predicate>
        auto findPowerUp(Predicate pred)
        {
            return std::find_if(m_activePowerUps.begin(), m_activePowerUps.end(), pred);
        }

        template<typename Predicate>
        auto findPowerUp(Predicate pred) const
        {
            return std::find_if(m_activePowerUps.begin(), m_activePowerUps.end(), pred);
        }

        // Power-up effect values
        static constexpr float m_freezeSlowdown = 0.1f;        // 90% speed reduction
        static constexpr float m_shieldDuration = 10.0f;       // Shield lasts 10 seconds
        static constexpr float m_speedBoostMultiplier = 1.5f;  // 50% speed increase
        static constexpr float m_scoreDoubleMultiplier = 2.0f; // Double score
    };

    // Template specialization for power-up type traits
    template<PowerUpType Type>
    struct PowerUpTraits
    {
        static constexpr float duration = 10.0f;
        static constexpr const char* name = "Unknown";
    };

    // Specializations for each power-up type
    template<>
    struct PowerUpTraits<PowerUpType::ScoreDoubler>
    {
        static constexpr float duration = 10.0f;
        static constexpr const char* name = "Score Doubler";
    };

    template<>
    struct PowerUpTraits<PowerUpType::FrenzyStarter>
    {
        static constexpr float duration = 0.0f; // Instant effect
        static constexpr const char* name = "Frenzy Starter";
    };

    template<>
    struct PowerUpTraits<PowerUpType::SpeedBoost>
    {
        static constexpr float duration = 8.0f;
        static constexpr const char* name = "Speed Boost";
    };

    template<>
    struct PowerUpTraits<PowerUpType::Invincibility>
    {
        static constexpr float duration = 5.0f;
        static constexpr const char* name = "Invincibility";
    };

    template<>
    struct PowerUpTraits<PowerUpType::Freeze>
    {
        static constexpr float duration = 5.0f;
        static constexpr const char* name = "Freeze";
    };

    template<>
    struct PowerUpTraits<PowerUpType::ExtraLife>
    {
        static constexpr float duration = 0.0f; // Instant effect
        static constexpr const char* name = "Extra Life";
    };

    template<>
    struct PowerUpTraits<PowerUpType::Shield>
    {
        static constexpr float duration = 10.0f;
        static constexpr const char* name = "Shield";
    };

    // Template utility for creating power-ups by type
    template<PowerUpType Type>
    class TypedPowerUpFactory
    {
    public:
        static std::unique_ptr<PowerUp> create();
    };

    // Factory method for creating power-ups dynamically
    class PowerUpCreator
    {
    public:
        using CreatorFunc = std::function<std::unique_ptr<PowerUp>()>;

        static PowerUpCreator& getInstance()
        {
            static PowerUpCreator instance;
            return instance;
        }

        template<typename PowerUpClass>
        void registerPowerUp(PowerUpType type)
        {
            m_creators[type] = []() { return PowerUpFactory<PowerUpClass>::create(); };
        }

        std::unique_ptr<PowerUp> create(PowerUpType type) const
        {
            auto it = m_creators.find(type);
            if (it != m_creators.end())
            {
                return it->second();
            }
            return nullptr;
        }

    private:
        PowerUpCreator() = default;
        std::unordered_map<PowerUpType, CreatorFunc> m_creators;
    };
}