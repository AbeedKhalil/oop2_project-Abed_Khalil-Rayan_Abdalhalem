#pragma once

#include <SFML/Window/Event.hpp>

namespace FishGame
{
    class InputHandler
    {
    public:
        void setReversed(bool reversed) { m_reversed = reversed; }
        void processEvent(sf::Event event, std::function<void(const sf::Event&)> callback);
    private:
        bool m_reversed{false};
    };
}
