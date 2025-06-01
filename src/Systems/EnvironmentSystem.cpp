#include "EnvironmentSystem.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>

namespace FishGame
{
    // BackgroundLayer implementation
    BackgroundLayer::BackgroundLayer(float scrollSpeed, const sf::Color& color)
        : m_elements()
        , m_scrollSpeed(scrollSpeed)
        , m_scrollOffset(0.0f)
        , m_baseColor(color)
        , m_currentEnvironment(EnvironmentType::OpenOcean)
    {
        m_elements.reserve(20);
        generateElements();
    }

    void BackgroundLayer::update(sf::Time deltaTime)
    {
        m_scrollOffset += m_scrollSpeed * deltaTime.asSeconds();

        if (m_scrollOffset > 100.0f)
        {
            m_scrollOffset -= 100.0f;
        }

        // Update element positions
        std::for_each(m_elements.begin(), m_elements.end(),
            [this](sf::RectangleShape& element) {
                sf::Vector2f pos = element.getPosition();
                pos.x += m_scrollSpeed * 0.1f;

                if (pos.x > 2000.0f)
                {
                    pos.x -= 2200.0f;
                }

                element.setPosition(pos);
            });
    }

    void BackgroundLayer::draw(sf::RenderTarget& target) const
    {
        std::for_each(m_elements.begin(), m_elements.end(),
            [&target](const sf::RectangleShape& element) {
                target.draw(element);
            });
    }

    void BackgroundLayer::setEnvironment(EnvironmentType type)
    {
        m_currentEnvironment = type;
        generateElements();
    }

    void BackgroundLayer::generateElements()
    {
        m_elements.clear();

        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> xDist(0.0f, 2000.0f);
        std::uniform_real_distribution<float> yDist(100.0f, 900.0f);
        std::uniform_real_distribution<float> sizeDist(20.0f, 100.0f);

        switch (m_currentEnvironment)
        {
        case EnvironmentType::CoralReef:
            // Generate coral shapes
            for (int i = 0; i < 15; ++i)
            {
                sf::RectangleShape coral(sf::Vector2f(sizeDist(rng), sizeDist(rng) * 1.5f));
                coral.setPosition(xDist(rng), yDist(rng));
                coral.setFillColor(sf::Color(
                    m_baseColor.r + (rng() % 50),
                    m_baseColor.g - (rng() % 30),
                    m_baseColor.b + (rng() % 40),
                    m_baseColor.a
                ));
                m_elements.push_back(coral);
            }
            break;

        case EnvironmentType::KelpForest:
            // Generate kelp strands
            for (int i = 0; i < 20; ++i)
            {
                sf::RectangleShape kelp(sf::Vector2f(10.0f, sizeDist(rng) * 3.0f));
                kelp.setPosition(xDist(rng), 1080.0f);
                kelp.setOrigin(5.0f, sizeDist(rng) * 3.0f);
                kelp.setFillColor(sf::Color(
                    0,
                    m_baseColor.g + (rng() % 50),
                    0,
                    m_baseColor.a
                ));
                m_elements.push_back(kelp);
            }
            break;

        case EnvironmentType::OpenOcean:
        default:
            // Generate subtle wave patterns
            for (int i = 0; i < 10; ++i)
            {
                sf::RectangleShape wave(sf::Vector2f(sizeDist(rng) * 2.0f, 5.0f));
                wave.setPosition(xDist(rng), yDist(rng));
                wave.setFillColor(sf::Color(
                    m_baseColor.r,
                    m_baseColor.g,
                    m_baseColor.b + (rng() % 30),
                    m_baseColor.a / 2
                ));
                m_elements.push_back(wave);
            }
            break;
        }
    }

    // OceanCurrentSystem implementation
    OceanCurrentSystem::OceanCurrentSystem()
        : m_currentDirection(1.0f, 0.0f)
        , m_currentStrength(50.0f)
        , m_waveOffset(0.0f)
        , m_particles()
    {
        m_particles.reserve(50);

        // Initialize current particles
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> xDist(0.0f, 1920.0f);
        std::uniform_real_distribution<float> yDist(0.0f, 1080.0f);

        for (int i = 0; i < 50; ++i)
        {
            CurrentParticle particle;
            particle.shape = sf::CircleShape(2.0f);
            particle.shape.setPosition(xDist(rng), yDist(rng));
            particle.shape.setFillColor(sf::Color(200, 200, 255, 100));
            particle.velocity = m_currentDirection * m_currentStrength;
            particle.lifetime = 5.0f;
            particle.alpha = 100.0f;
            m_particles.push_back(particle);
        }
    }

    void OceanCurrentSystem::update(sf::Time deltaTime)
    {
        m_waveOffset += deltaTime.asSeconds();
        updateParticles(deltaTime);
    }

    void OceanCurrentSystem::setDirection(const sf::Vector2f& direction)
    {
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0)
        {
            m_currentDirection = direction / length;
        }
    }

    sf::Vector2f OceanCurrentSystem::getCurrentForce(const sf::Vector2f& position) const
    {
        // Add wave-like variation to current
        float waveInfluence = std::sin(position.x * 0.01f + m_waveOffset) * 0.3f;
        float verticalWave = std::sin(position.y * 0.01f + m_waveOffset * 0.7f) * 0.2f;

        sf::Vector2f force = m_currentDirection * m_currentStrength;
        force.y += verticalWave * m_currentStrength;
        force.x *= (1.0f + waveInfluence);

        return force;
    }

    void OceanCurrentSystem::drawDebug(sf::RenderTarget& target) const
    {
        std::for_each(m_particles.begin(), m_particles.end(),
            [&target](const CurrentParticle& particle) {
                target.draw(particle.shape);
            });
    }

    void OceanCurrentSystem::updateParticles(sf::Time deltaTime)
    {
        std::for_each(m_particles.begin(), m_particles.end(),
            [this, deltaTime](CurrentParticle& particle) {
                // Update position with current force
                sf::Vector2f force = getCurrentForce(particle.shape.getPosition());
                particle.shape.move(force * deltaTime.asSeconds());

                // Wrap around screen
                sf::Vector2f pos = particle.shape.getPosition();
                if (pos.x > 1920.0f) pos.x = 0.0f;
                if (pos.x < 0.0f) pos.x = 1920.0f;
                if (pos.y > 1080.0f) pos.y = 0.0f;
                if (pos.y < 0.0f) pos.y = 1080.0f;
                particle.shape.setPosition(pos);

                // Update lifetime
                particle.lifetime -= deltaTime.asSeconds();
                if (particle.lifetime <= 0.0f)
                {
                    particle.lifetime = 5.0f;
                }
            });
    }

    // EnvironmentSystem implementation
    EnvironmentSystem::EnvironmentSystem()
        : m_currentEnvironment(EnvironmentType::OpenOcean)
        , m_currentTimeOfDay(TimeOfDay::Day)
        , m_farLayer(std::make_unique<BackgroundLayer>(10.0f, sf::Color(0, 50, 100, 50)))
        , m_midLayer(std::make_unique<BackgroundLayer>(20.0f, sf::Color(0, 70, 120, 70)))
        , m_nearLayer(std::make_unique<BackgroundLayer>(30.0f, sf::Color(0, 90, 140, 90)))
        , m_oceanCurrents(std::make_unique<OceanCurrentSystem>())
        , m_lightingOverlay(sf::Vector2f(1920.0f, 1080.0f))
        , m_dayNightTimer(sf::Time::Zero)
        , m_transitionTimer(sf::Time::Zero)
        , m_predatorAggressionBase(1.0f)
        , m_isTransitioning(false)
        , m_dayNightCyclePaused(true)  // Start paused by default
        , m_onEnvironmentChange(nullptr)
        , m_randomEngine(std::chrono::steady_clock::now().time_since_epoch().count())
        , m_timeDist(0, 3)
    {
        m_lightingOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    }

    void EnvironmentSystem::update(sf::Time deltaTime)
    {
        // Update layers
        m_farLayer->update(deltaTime);
        m_midLayer->update(deltaTime);
        m_nearLayer->update(deltaTime);

        // Update ocean currents
        m_oceanCurrents->update(deltaTime);

        // Update day/night cycle only if not paused
        if (!m_dayNightCyclePaused)
        {
            updateDayNightCycle(deltaTime);
        }

        // Handle environment transitions
        if (m_isTransitioning)
        {
            m_transitionTimer += deltaTime;
            if (m_transitionTimer.asSeconds() >= m_transitionDuration)
            {
                m_isTransitioning = false;
                applyEnvironmentEffects();
            }
        }
    }

    void EnvironmentSystem::setEnvironment(EnvironmentType type)
    {
        if (m_currentEnvironment != type)
        {
            transitionEnvironment(type);
        }
    }

    void EnvironmentSystem::setTimeOfDay(TimeOfDay time)
    {
        m_currentTimeOfDay = time;

        // Update lighting overlay
        sf::Color overlayColor = getAmbientLightColor();
        m_lightingOverlay.setFillColor(overlayColor);
    }

    void EnvironmentSystem::setRandomTimeOfDay()
    {
        // Array of all possible times
        const TimeOfDay times[] = {
            TimeOfDay::Day,
            TimeOfDay::Dusk,
            TimeOfDay::Night,
            TimeOfDay::Dawn
        };

        // Select a random time
        setTimeOfDay(times[m_timeDist(m_randomEngine)]);
    }

    float EnvironmentSystem::getPredatorAggressionMultiplier() const
    {
        float timeMultiplier = 1.0f;

        switch (m_currentTimeOfDay)
        {
        case TimeOfDay::Night:
            timeMultiplier = 1.5f; // More aggressive at night
            break;
        case TimeOfDay::Dusk:
        case TimeOfDay::Dawn:
            timeMultiplier = 1.2f; // Slightly more aggressive during transitions
            break;
        default:
            timeMultiplier = 1.0f;
            break;
        }

        return m_predatorAggressionBase * timeMultiplier;
    }

    sf::Color EnvironmentSystem::getAmbientLightColor() const
    {
        switch (m_currentTimeOfDay)
        {
        case TimeOfDay::Day:
            return sf::Color(0, 0, 0, 0); // No overlay
        case TimeOfDay::Dusk:
            return sf::Color(255, 100, 0, 50); // Orange tint
        case TimeOfDay::Night:
            return sf::Color(0, 0, 50, 150); // Dark blue
        case TimeOfDay::Dawn:
            return sf::Color(255, 200, 100, 30); // Light orange
        default:
            return sf::Color(0, 0, 0, 0);
        }
    }

    sf::Vector2f EnvironmentSystem::getOceanCurrentForce(const sf::Vector2f& position) const
    {
        return m_oceanCurrents->getCurrentForce(position);
    }

    void EnvironmentSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw background layers (far to near)
        m_farLayer->draw(target);
        m_midLayer->draw(target);
        m_nearLayer->draw(target);

        // Draw ocean current particles
        m_oceanCurrents->drawDebug(target);

        // Draw lighting overlay
        target.draw(m_lightingOverlay, states);
    }

    void EnvironmentSystem::updateDayNightCycle(sf::Time deltaTime)
    {
        // Only update if cycle is not paused
        if (m_dayNightCyclePaused)
            return;

        m_dayNightTimer += deltaTime;

        float cycleProgress = m_dayNightTimer.asSeconds() / m_dayDuration;
        cycleProgress = std::fmod(cycleProgress, 1.0f);

        // Determine time of day based on cycle progress
        TimeOfDay newTime;
        if (cycleProgress < 0.25f)
            newTime = TimeOfDay::Day;
        else if (cycleProgress < 0.35f)
            newTime = TimeOfDay::Dusk;
        else if (cycleProgress < 0.65f)
            newTime = TimeOfDay::Night;
        else if (cycleProgress < 0.75f)
            newTime = TimeOfDay::Dawn;
        else
            newTime = TimeOfDay::Day;

        if (newTime != m_currentTimeOfDay)
        {
            setTimeOfDay(newTime);
        }
    }

    void EnvironmentSystem::applyEnvironmentEffects()
    {
        switch (m_currentEnvironment)
        {
        case EnvironmentType::CoralReef:
            m_predatorAggressionBase = 0.8f; // Less aggressive in coral reef
            m_oceanCurrents->setStrength(30.0f);
            m_oceanCurrents->setDirection(sf::Vector2f(1.0f, 0.2f));
            break;

        case EnvironmentType::OpenOcean:
            m_predatorAggressionBase = 1.0f; // Normal aggression
            m_oceanCurrents->setStrength(50.0f);
            m_oceanCurrents->setDirection(sf::Vector2f(1.0f, 0.0f));
            break;

        case EnvironmentType::KelpForest:
            m_predatorAggressionBase = 1.2f; // More aggressive in kelp forest
            m_oceanCurrents->setStrength(20.0f);
            m_oceanCurrents->setDirection(sf::Vector2f(0.5f, -0.5f));
            break;
        }
    }

    void EnvironmentSystem::transitionEnvironment(EnvironmentType newType)
    {
        m_currentEnvironment = newType;
        m_isTransitioning = true;
        m_transitionTimer = sf::Time::Zero;

        // Update all layers
        m_farLayer->setEnvironment(newType);
        m_midLayer->setEnvironment(newType);
        m_nearLayer->setEnvironment(newType);

        if (m_onEnvironmentChange)
        {
            m_onEnvironmentChange(newType);
        }
    }
}