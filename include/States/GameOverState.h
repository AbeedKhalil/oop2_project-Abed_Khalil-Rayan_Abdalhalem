#pragma once

#include "State.h"
#include "Game.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <memory>
#include <functional>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>

namespace FishGame
{
    // Template trait specialization
    class GameOverState;
    template<> struct is_state<GameOverState> : std::true_type {};

    // Global game statistics (simple solution for passing data between states)
    struct GameStats
    {
        int finalScore = 0;
        int highScore = 0;
        int fishEaten = 0;
        int levelReached = 1;
        float survivalTime = 0.0f;
        bool newHighScore = false;

        static GameStats& getInstance()
        {
            static GameStats instance;
            return instance;
        }
    };

    class GameOverState : public State
    {
    public:
        explicit GameOverState(Game& game);
        ~GameOverState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        enum class MenuOption : size_t
        {
            Retry = 0,
            MainMenu,
            Exit,
            Count
        };

        // Template for menu option handling
        template<typename Action>
        struct MenuItem
        {
            std::string text;
            Action action;
            sf::Text textObject;
            sf::RectangleShape background;
        };

        using MenuAction = std::function<void()>;
        using MenuItemType = MenuItem<MenuAction>;

        // Particle effect for background
        struct Particle
        {
            sf::CircleShape shape;
            sf::Vector2f velocity;
            float lifetime = 0.f;
            float maxLifetime = 0.f;
        };

        // Core methods
        void initializeUI();
        void initializeStats();
        void initializeMenu();
        void initializeParticles();
        void createStatText(const std::string& label, const std::string& value, float yPos);

        // Update methods
        void updateAnimations(sf::Time deltaTime);
        void updateParticles(sf::Time deltaTime);
        void updateMenuSelection(sf::Time deltaTime);
        void updateTransitions(sf::Time deltaTime);

        // Event handlers
        void handleKeyPress(const sf::Keyboard::Key& key);
        void handleMouseMove(const sf::Vector2f& mousePos);
        void handleMouseClick(const sf::Vector2f& mousePos);

        // Menu actions
        void selectOption();
        void navigateMenu(int direction);
        void updateMenuVisuals();

        // Render helpers
        void renderBackground();
        void renderStats();
        void renderMenu();
        void renderParticles();

        // Utility methods
        void centerText(sf::Text& text, float yPosition);
        sf::Color interpolateColor(const sf::Color& start, const sf::Color& end, float t);
        float easeInOutQuad(float t);

        // Template helper for spawning particles
        template<typename Generator>
        void spawnParticle(Generator& gen)
        {
            if (m_particles.size() < m_maxParticles)
            {
                Particle particle;
                particle.shape.setRadius(std::uniform_real_distribution<float>(2.0f, 6.0f)(gen));
                particle.shape.setFillColor(sf::Color(255, 255, 255, 40));
                particle.shape.setPosition(
                    std::uniform_real_distribution<float>(0, getGame().getWindow().getSize().x)(gen),
                    getGame().getWindow().getSize().y + 20.0f
                );
                particle.velocity = sf::Vector2f(
                    std::uniform_real_distribution<float>(-20.0f, 20.0f)(gen),
                    std::uniform_real_distribution<float>(-60.0f, -30.0f)(gen)
                );
                particle.lifetime = 0.0f;
                particle.maxLifetime = std::uniform_real_distribution<float>(3.0f, 6.0f)(gen);
                m_particles.push_back(particle);
            }
        }

    private:
        // UI elements
        sf::Text m_titleText;
        sf::Text m_gameOverText;
        std::vector<sf::Text> m_statTexts;
        std::array<MenuItemType, static_cast<size_t>(MenuOption::Count)> m_menuItems;

        // Background elements
        sf::RectangleShape m_backgroundOverlay;
        std::vector<Particle> m_particles;
        static constexpr size_t m_maxParticles = 50;

        // State
        MenuOption m_selectedOption;
        bool m_isTransitioning;
        float m_transitionAlpha;
        float m_animationTime;
        float m_fadeInTime;

        // Animation parameters
        static constexpr float m_fadeInDuration = 1.5f;
        static constexpr float m_pulseSpeed = Constants::MENU_PULSE_SPEED;
        static constexpr float m_pulseAmplitude = Constants::MENU_PULSE_AMPLITUDE;
        static constexpr float m_transitionSpeed = 3.0f;

        // Random number generation
        std::mt19937 m_randomEngine;
    };
}
