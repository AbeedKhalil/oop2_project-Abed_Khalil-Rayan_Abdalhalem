#pragma once

#include "PowerUp.h"
#include "GameConstants.h"
#include "Utils/SpriteDrawable.h"

namespace FishGame
{
    // Freeze Power-up - freezes all enemy fish temporarily
    class FreezePowerUp : public PowerUp, public SpriteDrawable<FreezePowerUp>
    {
    public:
        FreezePowerUp();
        ~FreezePowerUp() override = default;

        void update(sf::Time deltaTime) override;
        sf::Color getAuraColor() const override { return sf::Color::Cyan; }

        void setFont(const sf::Font& font) {}

    private:
        static constexpr float m_freezeDuration = Constants::FREEZE_POWERUP_DURATION;
    };

    // Extra Life Power-up - grants an additional life
    class ExtraLifePowerUp : public PowerUp, public SpriteDrawable<ExtraLifePowerUp>
    {
    public:
        ExtraLifePowerUp();
        ~ExtraLifePowerUp() override = default;

        void update(sf::Time deltaTime) override;
        sf::Color getAuraColor() const override { return sf::Color::Green; }

    private:
        float m_heartbeatAnimation;
        static constexpr float m_heartbeatSpeed = Constants::EXTRA_LIFE_HEARTBEAT_SPEED;
    };

    // Speed Boost Power-up - increases player speed
    class SpeedBoostPowerUp : public PowerUp, public SpriteDrawable<SpeedBoostPowerUp>
    {
    public:
        SpeedBoostPowerUp();
        ~SpeedBoostPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        sf::Color getAuraColor() const override { return sf::Color(0, 255, 255); }

    private:
        float m_lineAnimation;
        static constexpr float m_boostDuration = Constants::SPEEDBOOST_POWERUP_DURATION;
        static constexpr float m_speedMultiplier = Constants::SPEED_BOOST_MULTIPLIER;
    };
}