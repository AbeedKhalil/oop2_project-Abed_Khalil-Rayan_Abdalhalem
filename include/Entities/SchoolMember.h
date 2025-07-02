#pragma once

#include "Fish.h"
#include "CollisionDetector.h"
#include <type_traits>
#include <vector>
#include <algorithm>
#include <cmath>

namespace FishGame {

template<typename FishType>
class SchoolMember : public FishType
{
    static_assert(std::is_base_of_v<Fish, FishType>,
                  "SchoolMember can only be used with Fish types");

public:
    explicit SchoolMember(int currentLevel = 1)
        : FishType(currentLevel)
        , m_schoolId(-1)
        , m_neighborDistance(80.0f)
        , m_separationDistance(30.0f)
    {
    }

    void setSchoolId(int id) { m_schoolId = id; }
    int getSchoolId() const { return m_schoolId; }

    void updateSchooling(const std::vector<SchoolMember*>& schoolmates, sf::Time deltaTime)
    {
        if (schoolmates.empty()) return;

        sf::Vector2f separation(0, 0);
        sf::Vector2f alignment(0, 0);
        sf::Vector2f cohesion(0, 0);

        int separationCount = 0;
        int neighborCount = 0;

        std::for_each(schoolmates.begin(), schoolmates.end(),
            [this, &separation, &alignment, &cohesion,
            &separationCount, &neighborCount](const SchoolMember* mate)
            {
                if (mate == this || !mate->isAlive()) return;

                float distance = CollisionDetector::getDistance(
                    this->m_position, mate->getPosition());

                if (distance < m_separationDistance && distance > 0)
                {
                    sf::Vector2f diff = this->m_position - mate->getPosition();
                    diff /= distance;
                    separation += diff;
                    separationCount++;
                }

                if (distance < m_neighborDistance)
                {
                    alignment += mate->getVelocity();
                    cohesion += mate->getPosition();
                    neighborCount++;
                }
            });

        sf::Vector2f steer(0, 0);

        if (separationCount > 0)
        {
            separation /= static_cast<float>(separationCount);
            steer += separation * 1.5f;
        }

        if (neighborCount > 0)
        {
            alignment /= static_cast<float>(neighborCount);
            alignment = normalizeVector(alignment) * this->m_speed;
            steer += (alignment - this->m_velocity) * 0.5f;

            cohesion /= static_cast<float>(neighborCount);
            sf::Vector2f seekPos = cohesion - this->m_position;
            seekPos = normalizeVector(seekPos) * this->m_speed;
            steer += (seekPos - this->m_velocity) * 0.3f;
        }

        this->m_velocity += steer * deltaTime.asSeconds();

        float currentSpeed = std::sqrt(this->m_velocity.x * this->m_velocity.x +
                                       this->m_velocity.y * this->m_velocity.y);
        if (currentSpeed > this->m_speed)
        {
            this->m_velocity = (this->m_velocity / currentSpeed) * this->m_speed;
        }
    }

private:
    sf::Vector2f normalizeVector(const sf::Vector2f& vec)
    {
        float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        if (length > 0)
            return vec / length;
        return vec;
    }

private:
    int m_schoolId;
    float m_neighborDistance;
    float m_separationDistance;
};

} // namespace FishGame
