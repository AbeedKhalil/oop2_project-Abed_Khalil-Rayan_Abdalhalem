#include "PowerUp.h"
#include <cmath>

namespace FishGame
{
    // PowerUp base class implementation
    PowerUp::PowerUp(PowerUpType type, sf::Time duration)
        : BonusItem(BonusType::PowerUp, 0)
        , m_powerUpType(type)
        , m_duration(duration)
        , m_iconBackground(25.0f)
        , m_aura(35.0f)
        , m_pulseAnimation(0.0f)
    {
        m_radius = 25.0f;
        m_lifetime = sf::seconds(15.0f);

        // Setup visual components
        m_iconBackground.setOrigin(25.0f, 25.0f);
        m_iconBackground.setFillColor(sf::Color(50, 50, 50, 200));
        m_iconBackground.setOutlineColor(sf::Color::White);
        m_iconBackground.setOutlineThickness(2.0f);

        m_aura.setOrigin(35.0f, 35.0f);
        m_aura.setFillColor(sf::Color::Transparent);
        m_aura.setOutlineThickness(3.0f);
    }

    // Do NOT implement update() here - let derived classes handle it

    // ScoreDoublerPowerUp implementation
    ScoreDoublerPowerUp::ScoreDoublerPowerUp()
        : PowerUp(PowerUpType::ScoreDoubler, sf::seconds(m_doubleDuration))
        , m_icon()
    {
        // Setup "2X" icon text - font will be set later
        m_icon.setString("2X");
        m_icon.setCharacterSize(24);
        m_icon.setStyle(sf::Text::Bold);
        m_icon.setFillColor(sf::Color::Yellow);

        // Center the text
        sf::FloatRect bounds = m_icon.getLocalBounds();
        m_icon.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

        // Set aura color
        m_aura.setOutlineColor(getAuraColor());
    }

    void ScoreDoublerPowerUp::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update lifetime
        m_lifetimeElapsed += deltaTime;
        if (hasExpired())
        {
            destroy();
            return;
        }

        // Pulse animation
        m_pulseAnimation += deltaTime.asSeconds() * 3.0f;
        float pulse = 1.0f + 0.2f * std::sin(m_pulseAnimation);

        // Bobbing
        float bobOffset = std::sin(m_lifetimeElapsed.asSeconds() * m_bobFrequency) * m_bobAmplitude;
        m_position.y = m_baseY + bobOffset;

        // Update positions
        m_iconBackground.setPosition(m_position);
        m_iconBackground.setScale(pulse, pulse);

        m_aura.setPosition(m_position);
        m_aura.setScale(pulse * 1.1f, pulse * 1.1f);

        m_icon.setPosition(m_position);
        m_icon.setScale(pulse, pulse);

        // Aura glow effect
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(128 + 127 * std::sin(m_pulseAnimation * 0.5f));
        m_aura.setOutlineColor(auraColor);
    }

    void ScoreDoublerPowerUp::onCollect()
    {
        destroy();
    }

    void ScoreDoublerPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_aura, states);
        target.draw(m_iconBackground, states);
        target.draw(m_icon, states);
    }

    // FrenzyStarterPowerUp implementation
    FrenzyStarterPowerUp::FrenzyStarterPowerUp()
        : PowerUp(PowerUpType::FrenzyStarter, sf::Time::Zero)
        , m_lightningBolts()
        , m_sparkAnimation(0.0f)
    {
        // Create lightning bolt shapes
        m_lightningBolts.reserve(4);
        for (int i = 0; i < 4; ++i)
        {
            sf::CircleShape bolt(3.0f, 3);
            bolt.setFillColor(sf::Color::Magenta);
            bolt.setOrigin(3.0f, 3.0f);
            m_lightningBolts.push_back(std::move(bolt));
        }

        // Set aura color
        m_aura.setOutlineColor(getAuraColor());
    }

    void FrenzyStarterPowerUp::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update lifetime
        m_lifetimeElapsed += deltaTime;
        if (hasExpired())
        {
            destroy();
            return;
        }

        // Animation updates
        m_pulseAnimation += deltaTime.asSeconds() * 4.0f;
        m_sparkAnimation += deltaTime.asSeconds() * 10.0f;

        float pulse = 1.0f + 0.3f * std::sin(m_pulseAnimation);

        // Bobbing with faster frequency
        float bobOffset = std::sin(m_lifetimeElapsed.asSeconds() * m_bobFrequency * 2.0f) * m_bobAmplitude;
        m_position.y = m_baseY + bobOffset;

        // Update positions
        m_iconBackground.setPosition(m_position);
        m_iconBackground.setScale(pulse, pulse);

        m_aura.setPosition(m_position);
        m_aura.setScale(pulse * 1.2f, pulse * 1.2f);

        // Update lightning bolts - rotate around center
        for (size_t i = 0; i < m_lightningBolts.size(); ++i)
        {
            float angle = m_sparkAnimation + (i * 90.0f);
            float radius = 15.0f + 5.0f * std::sin(m_sparkAnimation * 2.0f);

            sf::Vector2f boltPos;
            boltPos.x = m_position.x + std::cos(angle * 3.14159f / 180.0f) * radius;
            boltPos.y = m_position.y + std::sin(angle * 3.14159f / 180.0f) * radius;

            m_lightningBolts[i].setPosition(boltPos);
            m_lightningBolts[i].setRotation(angle);
        }

        // Electric aura effect
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(100 + 155 * std::abs(std::sin(m_sparkAnimation)));
        m_aura.setOutlineColor(auraColor);
    }

    void FrenzyStarterPowerUp::onCollect()
    {
        destroy();
    }

    void FrenzyStarterPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_aura, states);
        target.draw(m_iconBackground, states);

        // Draw lightning bolts
        std::for_each(m_lightningBolts.begin(), m_lightningBolts.end(),
            [&target, &states](const sf::CircleShape& bolt) {
                target.draw(bolt, states);
            });
    }

    // PowerUpManager implementation
    PowerUpManager::PowerUpManager()
        : m_activePowerUps()
    {
        m_activePowerUps.reserve(4); // Pre-allocate for typical number of simultaneous power-ups
    }

    void PowerUpManager::activatePowerUp(PowerUpType type, sf::Time duration)
    {
        // Check if power-up already active
        auto existing = findPowerUp([type](const ActivePowerUp& p) { return p.type == type; });

        if (existing != m_activePowerUps.end())
        {
            // Extend duration
            existing->remainingTime = std::max(existing->remainingTime, duration);
        }
        else
        {
            // Add new power-up
            m_activePowerUps.push_back({ type, duration });
        }
    }

    void PowerUpManager::update(sf::Time deltaTime)
    {
        // Update all active power-ups
        std::for_each(m_activePowerUps.begin(), m_activePowerUps.end(),
            [deltaTime](ActivePowerUp& powerUp) {
                powerUp.remainingTime -= deltaTime;
            });

        // Remove expired power-ups using erase-remove idiom
        m_activePowerUps.erase(
            std::remove_if(m_activePowerUps.begin(), m_activePowerUps.end(),
                [](const ActivePowerUp& powerUp) {
                    return powerUp.remainingTime <= sf::Time::Zero;
                }),
            m_activePowerUps.end()
        );
    }

    void PowerUpManager::reset()
    {
        m_activePowerUps.clear();
    }

    bool PowerUpManager::isActive(PowerUpType type) const
    {
        return std::any_of(m_activePowerUps.begin(), m_activePowerUps.end(),
            [type](const ActivePowerUp& powerUp) {
                return powerUp.type == type;
            });
    }

    sf::Time PowerUpManager::getRemainingTime(PowerUpType type) const
    {
        auto it = std::find_if(m_activePowerUps.begin(), m_activePowerUps.end(),
            [type](const ActivePowerUp& powerUp) {
                return powerUp.type == type;
            });

        return (it != m_activePowerUps.end()) ? it->remainingTime : sf::Time::Zero;
    }

    float PowerUpManager::getScoreMultiplier() const
    {
        float multiplier = 1.0f;

        if (isActive(PowerUpType::ScoreDoubler))
        {
            multiplier *= m_scoreDoubleMultiplier;
        }

        // Add other score-affecting power-ups here

        return multiplier;
    }

    float PowerUpManager::getSpeedMultiplier() const
    {
        return isActive(PowerUpType::SpeedBoost) ? m_speedBoostMultiplier : 1.0f;
    }

    std::vector<PowerUpType> PowerUpManager::getActivePowerUps() const
    {
        std::vector<PowerUpType> activeTypes;
        activeTypes.reserve(m_activePowerUps.size());

        std::transform(m_activePowerUps.begin(), m_activePowerUps.end(),
            std::back_inserter(activeTypes),
            [](const ActivePowerUp& powerUp) { return powerUp.type; });

        return activeTypes;
    }
}