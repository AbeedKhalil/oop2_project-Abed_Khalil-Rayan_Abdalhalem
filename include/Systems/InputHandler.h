#pragma once

#include <SFML/Window/Event.hpp>
#include <functional>
#include <memory>
#include "InputStrategy.h"

namespace FishGame
{
    class InputHandler
    {
    public:
        InputHandler();

        void setReversed(bool reversed);
        void processEvent(sf::Event event, std::function<void(const sf::Event&)> callback);

    private:
        std::unique_ptr<IInputStrategy> m_strategy;
    };
}
