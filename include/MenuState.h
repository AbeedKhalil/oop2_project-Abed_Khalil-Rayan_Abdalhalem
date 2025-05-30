#pragma once

#include "State.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <functional>

namespace FishGame
{
    class MenuState : public State
    {
    public:
        explicit MenuState(Game& game);
        ~MenuState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;

    private:
        enum class MenuOption
        {
            Play,
            Options,
            Exit,
            Count
        };

        void updateOptionText();
        void selectOption();

    private:
        sf::Text m_titleText;
        std::array<sf::Text, static_cast<size_t>(MenuOption::Count)> m_options;
        MenuOption m_selectedOption;

        // Animation
        float m_animationTime;
        static constexpr float m_pulseSpeed = 2.0f;
    };
}