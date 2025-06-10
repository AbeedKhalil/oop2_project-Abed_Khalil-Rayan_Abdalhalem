#pragma once

#include "PowerUp.h"
#include "GameConstants.h"
#include <chrono>
#include <random>

namespace FishGame
{
    // Freeze Power-up - freezes all enemy fish temporarily
    class FreezePowerUp : public PowerUp
    {
    public:
        FreezePowerUp();
        ~FreezePowerUp() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;
        sf::Color getAuraColor() const override { return sf::Color::Cyan; }

        void setFont(const sf::Font& font) { m_icon.setFont(font); }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_icon;
        std::vector<sf::RectangleShape> m_iceShards;
        static constexpr float m_freezeDuration = Constants::FREEZE_POWERUP_DURATION;
    };

    // Extra Life Power-up - grants an additional life
    class ExtraLifePowerUp : public PowerUp
    {
    public:
        ExtraLifePowerUp();
        ~ExtraLifePowerUp() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;
        sf::Color getAuraColor() const override { return sf::Color::Green; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::CircleShape m_heart;
        float m_heartbeatAnimation;
        static constexpr float m_heartbeatSpeed = Constants::EXTRA_LIFE_HEARTBEAT_SPEED;
    };

    // Speed Boost Power-up - increases player speed
    class SpeedBoostPowerUp : public PowerUp
    {
    public:
        SpeedBoostPowerUp();
        ~SpeedBoostPowerUp() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;
        sf::Color getAuraColor() const override { return sf::Color(0, 255, 255); }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        std::vector<sf::ConvexShape> m_speedLines;
        float m_lineAnimation;
        static constexpr float m_boostDuration = Constants::SPEEDBOOST_POWERUP_DURATION;
        static constexpr float m_speedMultiplier = Constants::SPEED_BOOST_MULTIPLIER;
    };
}