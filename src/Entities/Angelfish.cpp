#include "Angelfish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpriteManager.h"
#include "Animator.h"
#include "Systems/CollisionSystem.h"
#include "Pufferfish.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <ranges>
#include <execution>

namespace FishGame
{
    // Angelfish implementation
    Angelfish::Angelfish(int currentLevel)
        : AdvancedFish(FishSize::Small, m_baseSpeed, currentLevel, MovementPattern::ZigZag)
        , m_bonusPoints(m_baseBonus* currentLevel)
        , m_colorShift(0.0f)
        , m_directionChangeTimer(sf::Time::Zero)
        , m_currentThreat(nullptr)
        , m_isEvading(false)
        , m_evasionTimer(sf::Time::Zero)
    {
        // Create decorative fins
        m_fins.reserve(3);
        std::generate_n(std::back_inserter(m_fins), 3, [] {
            sf::CircleShape fin(10.0f, 3);
            fin.setFillColor(sf::Color(255, 200, 100, 150));
            fin.setOrigin(10.0f, 10.0f);
            return fin;
            });
    }

    void Angelfish::update(sf::Time deltaTime)
    {
        // Call base class update (which handles frozen state)
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        // Skip AI movement updates when frozen
        if (!m_isFrozen)
        {
            // Update evasion timer
            if (m_isEvading)
            {
                m_evasionTimer -= deltaTime;
                if (m_evasionTimer <= sf::Time::Zero)
                {
                    m_isEvading = false;
                    m_currentThreat = nullptr;
                }
            }

            // Update erratic movement only when not evading or frozen
            if (!m_isEvading)
            {
                updateErraticMovement(deltaTime);
            }
        }

        // Update visual effects (continue even when frozen)
        m_colorShift += deltaTime.asSeconds() * (m_isFrozen ? 0.5f : 2.0f);

        // Update fin positions with more dynamic movement
        auto finIdx = std::views::iota(size_t{ 0 }, m_fins.size());
        std::for_each(std::execution::unseq, finIdx.begin(), finIdx.end(),
            [this](size_t i)
            {
                float finAngle = (m_colorShift + static_cast<float>(i) * 120.0f) * 3.14159f / 180.0f;
                float finRadius = 20.0f + (m_isEvading ? 10.0f * std::sin(m_colorShift * 5.0f) : 0.0f);

                sf::Vector2f finPos(
                    m_position.x + std::cos(finAngle) * finRadius,
                    m_position.y + std::sin(finAngle) * finRadius);

                m_fins[i].setPosition(finPos);
                m_fins[i].setRotation(finAngle * Constants::RAD_TO_DEG);

                // Pulse fins when evading (unless frozen)
                if (m_isEvading && !m_isFrozen)
                {
                    float scale = 1.0f + 0.3f * std::sin(m_colorShift * 10.0f);
                    m_fins[i].setScale(scale, scale);
                }
            });
    }

    void Angelfish::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
        (void)deltaTime;
        if (!m_isAlive || m_isFrozen || m_isStunned)
            return;

        // Collect all potential threats
        std::vector<const Entity*> threats;
        threats.reserve(entities.size() + 1);

        // Check player as threat
        if (player && player->isAlive())
        {
            float distance = EntityUtils::distance(*this, *player);

            if (distance < m_threatDetectionRange)
            {
                const Player* playerPtr = dynamic_cast<const Player*>(player);
                if (playerPtr && playerPtr->canEat(*this))
                {
                    threats.push_back(player);

                    // Panic mode if very close
                    if (distance < m_panicRange)
                    {
                        m_isEvading = true;
                        m_evasionTimer = sf::seconds(2.0f);
                        m_currentThreat = player;
                    }
                }
            }
        }

        // Check other fish as threats
        std::for_each(entities.begin(), entities.end(),
            [this, &threats](const std::unique_ptr<Entity>& entity)
            {
                if (entity.get() == this || !entity->isAlive())
                    return;

                const Fish* fish = dynamic_cast<const Fish*>(entity.get());
                if (!fish)
                    return;

                float distance = EntityUtils::distance(*this, *entity);

                if (distance < m_threatDetectionRange && fish->canEat(*this))
                {
                    threats.push_back(entity.get());

                    // Special case: avoid puffed pufferfish at longer range
                    if (const Pufferfish* puffer = dynamic_cast<const Pufferfish*>(fish))
                    {
                        if (puffer->isInflated() && distance < m_threatDetectionRange * 1.5f)
                        {
                            m_isEvading = true;
                            m_evasionTimer = sf::seconds(1.5f);
                            m_currentThreat = entity.get();
                        }
                    }
                    else if (distance < m_panicRange)
                    {
                        m_isEvading = true;
                        m_evasionTimer = sf::seconds(2.0f);
                        m_currentThreat = entity.get();
                    }
                }
            });

        // Calculate evasive movement
        if (!threats.empty())
        {
            updateEvasiveMovement(entities, player);
        }
    }

    void Angelfish::updateEvasiveMovement(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player)
    {
        std::vector<const Entity*> threats;

        // Collect nearby threats for composite escape vector
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr && playerPtr->canEat(*this))
            {
                float distance = EntityUtils::distance(*this, *player);
                if (distance < m_threatDetectionRange)
                {
                    threats.push_back(player);
                }
            }
        }

        std::for_each(entities.begin(), entities.end(),
            [this, &threats](const std::unique_ptr<Entity>& entity)
            {
                if (entity.get() == this || !entity->isAlive())
                    return;

                const Fish* fish = dynamic_cast<const Fish*>(entity.get());
                if (fish && fish->canEat(*this))
                {
                    float distance = EntityUtils::distance(*this, *entity);
                    if (distance < m_threatDetectionRange)
                    {
                        threats.push_back(entity.get());
                    }
                }
            });

        if (!threats.empty())
        {
            sf::Vector2f escapeVector = calculateEscapeVector(threats);

            // Apply speed based on threat proximity
            float speed = m_isEvading ? m_evadeSpeed : m_baseSpeed;

            // Add some randomness to make movement less predictable
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> noiseDist(-30.0f, 30.0f);

            float noise = noiseDist(rng) * Constants::DEG_TO_RAD;
            float cos_n = std::cos(noise);
            float sin_n = std::sin(noise);

            sf::Vector2f noisyEscape(
                escapeVector.x * cos_n - escapeVector.y * sin_n,
                escapeVector.x * sin_n + escapeVector.y * cos_n
            );

            m_velocity = noisyEscape * speed;
        }
    }

    sf::Vector2f Angelfish::calculateEscapeVector(const std::vector<const Entity*>& threats)
    {
        sf::Vector2f compositeEscape(0.0f, 0.0f);

        // Calculate weighted escape vector based on threat proximity
        std::for_each(threats.begin(), threats.end(),
            [this, &compositeEscape](const Entity* threat)
            {
                sf::Vector2f toThreat = threat->getPosition() - m_position;
                float distance = std::sqrt(toThreat.x * toThreat.x + toThreat.y * toThreat.y);

                if (distance > 0.0f)
                {
                    // Weight by inverse distance (closer = stronger repulsion)
                    float weight = 1.0f / (distance / m_panicRange);
                    weight = std::min(weight, 3.0f); // Cap the weight

                    sf::Vector2f escapeDir = -toThreat / distance; // Normalize and reverse
                    compositeEscape += escapeDir * weight;
                }
            });

        // Normalize composite escape vector
        float length = std::sqrt(compositeEscape.x * compositeEscape.x +
            compositeEscape.y * compositeEscape.y);

        if (length > 0.0f)
        {
            compositeEscape /= length;
        }
        else
        {
            // No clear escape direction - pick random
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
            float angle = angleDist(rng) * Constants::DEG_TO_RAD;
            compositeEscape = sf::Vector2f(std::cos(angle), std::sin(angle));
        }

        return compositeEscape;
    }

    void Angelfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw fins first
        std::for_each(m_fins.begin(), m_fins.end(),
            [&target, &states](const sf::CircleShape& fin)
            {
                target.draw(fin, states);
            });

        Fish::draw(target, states);
    }

    void Angelfish::updateErraticMovement(sf::Time deltaTime)
    {
        m_directionChangeTimer += deltaTime;

        if (m_directionChangeTimer.asSeconds() > m_directionChangeInterval)
        {
            m_directionChangeTimer = sf::Time::Zero;

            // Random direction change
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> angleDist(-60.0f, 60.0f);

            float angleChange = angleDist(rng) * Constants::DEG_TO_RAD;

            // Rotate velocity
            float cos_a = std::cos(angleChange);
            float sin_a = std::sin(angleChange);

            sf::Vector2f newVelocity(
                m_velocity.x * cos_a - m_velocity.y * sin_a,
                m_velocity.x * sin_a + m_velocity.y * cos_a
            );

            m_velocity = newVelocity;
        }
    }

    void Angelfish::onCollide(Player& player, CollisionSystem& system)
    {
        if (player.isInvulnerable() || system.m_playerStunned)
            return;

        if (player.canEat(*this) && player.attemptEat(*this))
        {
            system.m_levelCounts[getTextureID()]++;
            system.m_sounds.play(SoundEffectID::Bite1);
            system.createParticle(getPosition(), Constants::ANGELFISH_PARTICLE_COLOR,
                                 Constants::ANGELFISH_PARTICLE_COUNT);
            destroy();
        }
    }
}

