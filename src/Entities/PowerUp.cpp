#include "PowerUp.h"
#include "Utils/DrawHelpers.h"
#include "Systems/CollisionSystem.h"
#include <execution>

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

    void PowerUp::onCollide(Player& player, CollisionSystem& system)
    {
        onCollect();
        system.handlePowerUpCollision(player, *this);
    }

    // Do NOT implement update() here - let derived classes handle it

    // ScoreDoublerPowerUp implementation
    ScoreDoublerPowerUp::ScoreDoublerPowerUp()
        : PowerUp(PowerUpType::ScoreDoubler,
            sf::seconds(Constants::SCORE_DOUBLER_POWERUP_DURATION))
        , m_icon()
    {
        // Setup "2X" icon text - font will be set later
        m_icon.setString("2X");
        m_icon.setCharacterSize(Constants::HUD_FONT_SIZE);
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
        if (!updateLifetime(deltaTime))
            return;

        // Pulse animation
        m_pulseAnimation += deltaTime.asSeconds() * 3.0f;
        float pulse = 1.0f + 0.2f * std::sin(m_pulseAnimation);

        // Bobbing
        m_position.y = m_baseY + computeBobbingOffset();

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

void ScoreDoublerPowerUp::applyEffect(Player& /*player*/, CollisionSystem& system)
{
    system.m_powerUps.activatePowerUp(getPowerUpType(), getDuration());
    system.createParticle(getPosition(), Constants::SCORE_DOUBLER_COLOR);
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
        std::generate_n(std::back_inserter(m_lightningBolts), 4, [] {
            sf::CircleShape bolt(3.0f, 3);
            bolt.setFillColor(sf::Color::Magenta);
            bolt.setOrigin(3.0f, 3.0f);
            return bolt;
            });

        // Set aura color
        m_aura.setOutlineColor(getAuraColor());
    }

    void FrenzyStarterPowerUp::update(sf::Time deltaTime)
    {
        if (!updateLifetime(deltaTime))
            return;

        // Animation updates
        m_pulseAnimation += deltaTime.asSeconds() * 4.0f;
        m_sparkAnimation += deltaTime.asSeconds() * 10.0f;

        float pulse = 1.0f + 0.3f * std::sin(m_pulseAnimation);

        // Bobbing with faster frequency
        m_position.y = m_baseY + computeBobbingOffset(2.0f);

        // Update positions
        m_iconBackground.setPosition(m_position);
        m_iconBackground.setScale(pulse, pulse);

        m_aura.setPosition(m_position);
        m_aura.setScale(pulse * 1.2f, pulse * 1.2f);

        // Update lightning bolts - rotate around center using ranges
        auto boltIndices = std::views::iota(size_t{ 0 }, m_lightningBolts.size());
        std::for_each(std::execution::unseq, boltIndices.begin(), boltIndices.end(),
            [this](size_t i)
            {
                float angle = m_sparkAnimation + (i * 90.0f);
                float radius = 15.0f + 5.0f * std::sin(m_sparkAnimation * 2.0f);

                sf::Vector2f boltPos;
                boltPos.x = m_position.x + std::cos(angle * Constants::DEG_TO_RAD) * radius;
                boltPos.y = m_position.y + std::sin(angle * Constants::DEG_TO_RAD) * radius;

                m_lightningBolts[i].setPosition(boltPos);
                m_lightningBolts[i].setRotation(angle);
            });

        // Electric aura effect
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(100 + 155 * std::abs(std::sin(m_sparkAnimation)));
        m_aura.setOutlineColor(auraColor);
    }

void FrenzyStarterPowerUp::onCollect()
{
    destroy();
}

void FrenzyStarterPowerUp::applyEffect(Player& /*player*/, CollisionSystem& system)
{
    system.m_frenzySystem.forceFrenzy();
    system.createParticle(getPosition(), Constants::FRENZY_STARTER_COLOR);
}

    void FrenzyStarterPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_aura, states);
        target.draw(m_iconBackground, states);

        // Draw lightning bolts
        DrawUtils::drawContainer(m_lightningBolts, target, states);
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
            multiplier *= Constants::SCORE_DOUBLER_MULTIPLIER;
        }

        // Add other score-affecting power-ups here

        return multiplier;
    }

    float PowerUpManager::getSpeedMultiplier() const
    {
        return isActive(PowerUpType::SpeedBoost) ? Constants::SPEED_BOOST_MULTIPLIER : 1.0f;
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
