#pragma once

#include <stdexcept>
#include <string>

namespace FishGame
{
    class GameException : public std::runtime_error
    {
    public:
        explicit GameException(const std::string& message)
            : std::runtime_error(message) {}
    };

    class ResourceLoadException : public GameException
    {
    public:
        explicit ResourceLoadException(const std::string& message)
            : GameException(message) {}
    };

    class StateNotFoundException : public GameException
    {
    public:
        explicit StateNotFoundException(const std::string& message)
            : GameException(message) {}
    };
}
