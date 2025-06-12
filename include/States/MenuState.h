#pragma once

#include "State.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <array>
#include <functional>
#include <optional>
#include <algorithm>
#include <vector>
#include <random>

namespace FishGame
{
    // Template trait specialization
    class MenuState;
    template<> struct is_state<MenuState> : std::true_type {};

    class MenuState : public State
    {
    public:
        explicit MenuState(Game& game);
        ~MenuState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        enum class MenuOption : size_t
        {
            Play = 0,
            Settings,
            Credits,
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
        };

        using MenuAction = std::function<void()>;
        using MenuItemType = MenuItem<MenuAction>;

        // Core methods
        void initializeMenu();
        void updateOptionText();
        void selectOption();

        // Event handlers
        void handleKeyPress(const sf::Keyboard::Key& key);
        void handleMouseMove(const sf::Vector2f& mousePos);
        void handleMouseClick(const sf::Vector2f& mousePos);

        // Animation utilities
        void updateAnimations(sf::Time deltaTime);

        // Background helpers
        void initializeBackground();
        void updateBackground(sf::Time deltaTime);

    private:
        struct BackgroundFish
        {
            sf::CircleShape shape;
            sf::Vector2f velocity;
        };
        // UI elements
        sf::Sprite m_titleSprite;
        std::array<MenuItemType, static_cast<size_t>(MenuOption::Count)> m_menuItems;

        // State
        MenuOption m_selectedOption;
        MenuOption m_previousOption;

        // Animation data
        float m_animationTime;
        float m_transitionAlpha;
        bool m_isTransitioning;

        // Background
        sf::Sprite m_backgroundSprite;
        std::vector<BackgroundFish> m_backgroundFish;
        std::mt19937 m_randomEngine;
        // Animation parameters from constants
        static constexpr float m_pulseSpeed = Constants::MENU_PULSE_SPEED;
        static constexpr float m_pulseAmplitude = Constants::MENU_PULSE_AMPLITUDE;
        static constexpr float m_fadeSpeed = Constants::MENU_FADE_SPEED;
    };
}