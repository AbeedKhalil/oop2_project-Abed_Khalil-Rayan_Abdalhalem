#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <random>
#include <functional>
#include <algorithm>

namespace FishGame
{
    enum class EnvironmentType
    {
        CoralReef,
        OpenOcean,
        KelpForest
    };

    enum class TimeOfDay
    {
        Day,
        Dusk,
        Night,
        Dawn
    };

    // Ocean current particle for visual effect
    struct CurrentParticle
    {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        float lifetime;
        float alpha;
    };

    // Background layer for parallax effect
    class BackgroundLayer
    {
    public:
        BackgroundLayer(float scrollSpeed, const sf::Color& color);

        void update(sf::Time deltaTime);
        void draw(sf::RenderTarget& target) const;
        void setEnvironment(EnvironmentType type);

    private:
        std::vector<sf::RectangleShape> m_elements;
        float m_scrollSpeed;
        float m_scrollOffset;
        sf::Color m_baseColor;
        EnvironmentType m_currentEnvironment;

        void generateElements();
    };

    // Ocean current system affecting movement
    class OceanCurrentSystem
    {
    public:
        OceanCurrentSystem();

        void update(sf::Time deltaTime);
        void setStrength(float strength) { m_currentStrength = strength; }
        void setDirection(const sf::Vector2f& direction);

        sf::Vector2f getCurrentForce(const sf::Vector2f& position) const;
        void drawDebug(sf::RenderTarget& target) const;

    private:
        sf::Vector2f m_currentDirection;
        float m_currentStrength;
        float m_waveOffset;
        std::vector<CurrentParticle> m_particles;

        void updateParticles(sf::Time deltaTime);
    };

    // Main environment system managing all environmental features
    class EnvironmentSystem : public sf::Drawable
    {
    public:
        EnvironmentSystem();
        ~EnvironmentSystem() = default;

        // Delete copy operations
        EnvironmentSystem(const EnvironmentSystem&) = delete;
        EnvironmentSystem& operator=(const EnvironmentSystem&) = delete;

        // Allow move operations
        EnvironmentSystem(EnvironmentSystem&&) = default;
        EnvironmentSystem& operator=(EnvironmentSystem&&) = default;

        void update(sf::Time deltaTime);
        void setEnvironment(EnvironmentType type);
        void setTimeOfDay(TimeOfDay time);

        EnvironmentType getCurrentEnvironment() const { return m_currentEnvironment; }
        TimeOfDay getCurrentTimeOfDay() const { return m_currentTimeOfDay; }

        float getPredatorAggressionMultiplier() const;
        sf::Color getAmbientLightColor() const;
        sf::Vector2f getOceanCurrentForce(const sf::Vector2f& position) const;

        // Day/night cycle control methods
        void pauseDayNightCycle() { m_dayNightCyclePaused = true; }
        void resumeDayNightCycle() { m_dayNightCyclePaused = false; }
        void setRandomTimeOfDay();

        // Callbacks
        void setOnEnvironmentChange(std::function<void(EnvironmentType)> callback)
        {
            m_onEnvironmentChange = callback;
        }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateDayNightCycle(sf::Time deltaTime);
        void applyEnvironmentEffects();
        void transitionEnvironment(EnvironmentType newType);

    private:
        EnvironmentType m_currentEnvironment;
        TimeOfDay m_currentTimeOfDay;

        std::unique_ptr<BackgroundLayer> m_farLayer;
        std::unique_ptr<BackgroundLayer> m_midLayer;
        std::unique_ptr<BackgroundLayer> m_nearLayer;

        std::unique_ptr<OceanCurrentSystem> m_oceanCurrents;

        sf::RectangleShape m_lightingOverlay;
        sf::Time m_dayNightTimer;
        sf::Time m_transitionTimer;

        float m_predatorAggressionBase;
        bool m_isTransitioning;
        bool m_dayNightCyclePaused;

        std::function<void(EnvironmentType)> m_onEnvironmentChange;

        // Random number generation for time of day
        std::mt19937 m_randomEngine;
        std::uniform_int_distribution<int> m_timeDist;

        // Day/night cycle configuration
        static constexpr float m_dayDuration = 60.0f; // 60 seconds per full cycle
        static constexpr float m_transitionDuration = 3.0f;
    };
}