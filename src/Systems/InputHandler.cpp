#include "InputHandler.h"
#include "InputStrategy.h"

namespace FishGame
{
    InputHandler::InputHandler()
        : m_strategy(std::make_unique<NormalInputStrategy>())
    {
    }

    void InputHandler::setReversed(bool reversed)
    {
        if (reversed)
            m_strategy = std::make_unique<ReversedInputStrategy>();
        else
            m_strategy = std::make_unique<NormalInputStrategy>();
    }

    void InputHandler::processEvent(sf::Event event, std::function<void(const sf::Event&)> callback)
    {
        if (m_strategy)
            m_strategy->process(event);
        callback(event);
    }
}
