#include "InputStrategy.h"
#include <unordered_map>

namespace FishGame
{
    void NormalInputStrategy::process(sf::Event& /*event*/)
    {
        // No modification for normal controls
    }

    void ReversedInputStrategy::process(sf::Event& event)
    {
        if (event.type != sf::Event::KeyPressed)
            return;

        static const std::unordered_map<sf::Keyboard::Key, sf::Keyboard::Key> map = {
            {sf::Keyboard::W, sf::Keyboard::S}, {sf::Keyboard::S, sf::Keyboard::W},
            {sf::Keyboard::A, sf::Keyboard::D}, {sf::Keyboard::D, sf::Keyboard::A},
            {sf::Keyboard::Up, sf::Keyboard::Down}, {sf::Keyboard::Down, sf::Keyboard::Up},
            {sf::Keyboard::Left, sf::Keyboard::Right}, {sf::Keyboard::Right, sf::Keyboard::Left}
        };
        if (auto it = map.find(event.key.code); it != map.end())
            event.key.code = it->second;
    }
}
