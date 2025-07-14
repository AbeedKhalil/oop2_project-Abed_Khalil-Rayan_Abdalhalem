#pragma once

#include <SFML/Window/Event.hpp>

namespace FishGame
{
    class IInputStrategy
    {
    public:
        virtual ~IInputStrategy() = default;
        virtual void process(sf::Event& event) = 0;
    };

    class NormalInputStrategy : public IInputStrategy
    {
    public:
        void process(sf::Event& event) override;
    };

    class ReversedInputStrategy : public IInputStrategy
    {
    public:
        void process(sf::Event& event) override;
    };
}
