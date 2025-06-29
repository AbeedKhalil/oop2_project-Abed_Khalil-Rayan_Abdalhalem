#include "InputHandler.h"
#include <unordered_map>

namespace FishGame
{
    void InputHandler::processEvent(sf::Event event, std::function<void(const sf::Event&)> callback)
    {
        if (m_reversed && event.type == sf::Event::KeyPressed)
        {
            static const std::unordered_map<sf::Keyboard::Key, sf::Keyboard::Key> map = {
                {sf::Keyboard::W, sf::Keyboard::S}, {sf::Keyboard::S, sf::Keyboard::W},
                {sf::Keyboard::A, sf::Keyboard::D}, {sf::Keyboard::D, sf::Keyboard::A},
                {sf::Keyboard::Up, sf::Keyboard::Down}, {sf::Keyboard::Down, sf::Keyboard::Up},
                {sf::Keyboard::Left, sf::Keyboard::Right}, {sf::Keyboard::Right, sf::Keyboard::Left}
            };
            if (auto it = map.find(event.key.code); it != map.end())
                event.key.code = it->second;
        }
        callback(event);
    }
}
