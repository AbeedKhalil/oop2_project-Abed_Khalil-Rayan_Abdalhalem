#pragma once

#include "State.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <array>

namespace FishGame
{
    class IntroState;
    template<> struct is_state<IntroState> : std::true_type {};

    class IntroState : public State
    {
    public:
        explicit IntroState(Game& game);
        ~IntroState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        static constexpr float DISPLAY_DURATION = 3.0f;
        std::array<sf::Sprite, 2> m_sprites;
        std::size_t m_currentIndex;
        sf::Time m_elapsed;
    };
}