#include "CameraController.h"
#include <algorithm>

namespace FishGame
{
    CameraController::CameraController(const sf::View& view, sf::Vector2f worldSize,
        float smoothing)
        : m_view(view), m_worldSize(worldSize), m_smoothing(smoothing)
    {
    }

    void CameraController::update(const sf::Vector2f& targetPos)
    {
        if (m_frozen)
        {
            m_view.setCenter(m_frozenPos);
            return;
        }

        sf::Vector2f target = targetPos;
        sf::Vector2f halfSize = m_view.getSize() * 0.5f;

        if (m_worldSize.x > m_view.getSize().x)
            target.x = std::clamp(target.x, halfSize.x, m_worldSize.x - halfSize.x);
        else
            target.x = m_worldSize.x * 0.5f;

        if (m_worldSize.y > m_view.getSize().y)
            target.y = std::clamp(target.y, halfSize.y, m_worldSize.y - halfSize.y);
        else
            target.y = m_worldSize.y * 0.5f;

        sf::Vector2f current = m_view.getCenter();
        sf::Vector2f newCenter = current + (target - current) * m_smoothing;
        m_view.setCenter(newCenter);
    }

    void CameraController::freeze(const sf::Vector2f& position)
    {
        sf::Vector2f halfSize = m_view.getSize() * 0.5f;
        m_frozenPos.x = std::clamp(position.x, halfSize.x, m_worldSize.x - halfSize.x);
        m_frozenPos.y = std::clamp(position.y, halfSize.y, m_worldSize.y - halfSize.y);
        m_frozen = true;
    }

    void CameraController::unfreeze()
    {
        m_frozen = false;
    }
}

