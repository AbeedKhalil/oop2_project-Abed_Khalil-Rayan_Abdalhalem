#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame
{
    class CameraController
    {
    public:
        CameraController() = default;
        CameraController(const sf::View& view, sf::Vector2f worldSize,
            float smoothing = 0.1f);

        void setView(const sf::View& view) { m_view = view; }
        sf::View& getView() { return m_view; }
        const sf::View& getView() const { return m_view; }

        void setWorldSize(sf::Vector2f size) { m_worldSize = size; }
        sf::Vector2f getWorldSize() const { return m_worldSize; }

        void setSmoothing(float smoothing) { m_smoothing = smoothing; }
        float getSmoothing() const { return m_smoothing; }

        void update(const sf::Vector2f& targetPos);

        void freeze(const sf::Vector2f& position);
        void unfreeze();
        bool isFrozen() const { return m_frozen; }

    private:
        sf::View m_view{};
        sf::Vector2f m_worldSize{};
        bool m_frozen{false};
        sf::Vector2f m_frozenPos{};
        float m_smoothing{0.1f};
    };
}

