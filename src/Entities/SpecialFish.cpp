#include "SpecialFish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpriteManager.h"
#include "BarracudaAnimator.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iterator>

namespace FishGame
{
    // AdvancedFish implementation
    AdvancedFish::AdvancedFish(FishSize size, float speed, int currentLevel, MovementPattern pattern)
        : Fish(size, speed, currentLevel)
        , m_movementPattern(pattern)
        , m_patternTimer(0.0f)
        , m_baseY(0.0f)
        , m_amplitude(30.0f)
        , m_frequency(2.0f)
    {
    }

    void AdvancedFish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // When frozen simply defer to base update so the sprite stays synced
        if (m_isFrozen)
        {
            Fish::update(deltaTime);
            return;
        }

        m_patternTimer += deltaTime.asSeconds();

        // Update movement pattern which may modify velocity/position
        updateMovementPattern(deltaTime);

        // Let the base class handle movement, sprite sync and bounds checks
        Fish::update(deltaTime);
    }

    void AdvancedFish::updateMovementPattern(sf::Time deltaTime)
    {
        switch (m_movementPattern)
        {
        case MovementPattern::Sinusoidal:
        {
            // Store base Y on first frame
            if (m_baseY == 0.0f)
                m_baseY = m_position.y;

            // Apply sinusoidal movement
            float yOffset = m_amplitude * std::sin(m_patternTimer * m_frequency);
            m_position.y = m_baseY + yOffset;
            break;
        }

        case MovementPattern::ZigZag:
        {
            // Change Y direction periodically
            float zigzagPeriod = 1.0f;
            if (std::fmod(m_patternTimer, zigzagPeriod) < zigzagPeriod / 2)
            {
                m_velocity.y = m_speed * 0.5f;
            }
            else
            {
                m_velocity.y = -m_speed * 0.5f;
            }
            break;
        }

        case MovementPattern::Circular:
        {
            float radius = 50.0f;
            float angle = m_patternTimer * 2.0f;
            sf::Vector2f circularOffset(
                radius * std::cos(angle),
                radius * std::sin(angle)
            );

            // Apply circular motion around current trajectory
            m_velocity.x = m_speed * std::cos(angle);
            m_velocity.y = m_speed * std::sin(angle);
            break;
        }

        case MovementPattern::Linear:
        default:
            // No special movement
            break;
        }
    }

    // Barracuda implementation
    Barracuda::Barracuda(int currentLevel)
        : AdvancedFish(FishSize::Large, 180.0f, currentLevel, MovementPattern::Aggressive)
        , m_currentTarget(nullptr)
        , m_huntTimer(sf::Time::Zero)
        , m_dashSpeed(450.0f)
        , m_isDashing(false)
        , m_animator(nullptr)
        , m_currentAnimation()
        , m_facingRight(false)
        , m_turning(false)
        , m_turnTimer(sf::Time::Zero)
    {
        // Barracuda appearance
        m_pointValue = getPointValue(m_size, m_currentLevel) * 2;  // Double points

        // Make Barracuda larger than default large fish
        m_radius = 50.0f;
    }

    void Barracuda::initializeSprite(SpriteManager& spriteManager)
    {
        const sf::Texture& tex = spriteManager.getTexture(getTextureID());
        m_animator = std::make_unique<BarracudaAnimator>(tex);

        float scale = spriteManager.getScaleConfig().large * 1.5f;
        m_animator->setScale({ scale, scale });
        m_animator->setPosition(m_position);
        setRenderMode(RenderMode::Sprite);

        m_facingRight = m_velocity.x > 0.f;
        m_currentAnimation = m_facingRight ? "swimRight" : "swimLeft";
        m_animator->play(m_currentAnimation);
    }

    void Barracuda::playEatAnimation()
    {
        if (!m_animator)
            return;

        std::string eat = m_facingRight ? "eatRight" : "eatLeft";
        m_animator->play(eat);
        m_currentAnimation = eat;
    }

    void Barracuda::update(sf::Time deltaTime)
    {
        AdvancedFish::update(deltaTime);

        if (m_animator && getRenderMode() == RenderMode::Sprite)
        {
            bool newFacingRight = m_velocity.x > 0.f;
            if (newFacingRight != m_facingRight)
            {
                m_facingRight = newFacingRight;
                std::string turn = m_facingRight ? "turnLeftToRight" : "turnRightToLeft";
                m_animator->play(turn);
                m_currentAnimation = turn;
                m_turning = true;
                m_turnTimer = sf::Time::Zero;
            }

            m_animator->update(deltaTime);

            if (m_turning)
            {
                m_turnTimer += deltaTime;
                if (m_turnTimer.asSeconds() >= m_turnDuration)
                {
                    std::string swim = m_facingRight ? "swimRight" : "swimLeft";
                    m_animator->play(swim);
                    m_currentAnimation = swim;
                    m_turning = false;
                }
            }

            m_animator->setPosition(m_position);
        }
    }

    void Barracuda::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (m_animator)
            target.draw(*m_animator, states);
        else
            Fish::draw(target, states);
    }

    void Barracuda::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
        if (!m_isAlive || m_isFrozen || m_isStunned)
            return;

        // Rest of the AI logic remains the same...
        m_huntTimer += deltaTime;

        // Find closest prey
        const Entity* closestPrey = nullptr;
        float closestDistance = m_huntRange;

        // Check player first - Barracuda can hunt player if player is smaller
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr && canEat(*player))
            {
                float distance = EntityUtils::distance(*this, *player);
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closestPrey = player;
                }
            }
        }

        // Check other fish
        std::for_each(entities.begin(), entities.end(),
            [this, &closestPrey, &closestDistance](const std::unique_ptr<Entity>& entity)
            {
                if (entity.get() == this || !entity->isAlive())
                    return;

                if (canEat(*entity))
                {
                    float distance = EntityUtils::distance(*this, *entity);
                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        closestPrey = entity.get();
                    }
                }
            });

        // Update hunting behavior
        if (closestPrey)
        {
            m_currentTarget = closestPrey;
            updateHuntingBehavior(closestPrey, deltaTime);
        }
        else
        {
            m_isDashing = false;
            m_currentTarget = nullptr;
        }
    }

    void Barracuda::updateHuntingBehavior(const Entity* target, sf::Time deltaTime)
    {
        sf::Vector2f direction = target->getPosition() - m_position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0)
        {
            direction /= distance;  // Normalize

            // Start dash if close enough
            if (distance < 150.0f && !m_isDashing)
            {
                m_isDashing = true;
                m_huntTimer = sf::Time::Zero;
            }

            // Apply speed based on state
            float currentSpeed = m_isDashing ? m_dashSpeed : m_speed;

            // End dash after duration
            if (m_isDashing && m_huntTimer.asSeconds() > m_dashDuration)
            {
                m_isDashing = false;
            }

            m_velocity = direction * currentSpeed;
        }
    }

    // Pufferfish implementation
    Pufferfish::Pufferfish(int currentLevel)
        : AdvancedFish(FishSize::Medium, 100.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_isPuffed(false)
        , m_stateTimer(sf::Time::Zero)
        , m_inflationLevel(0.0f)
        , m_normalRadius(25.0f)
        , m_frame(0)
        , m_frameTimer(sf::Time::Zero)
        , m_texture(nullptr)
        , m_frameWidth(0)
        , m_frameHeight(0)
        , m_playPuff(false)
        , m_puffPlayed(false)
    {
        m_radius = m_normalRadius;

        // Create spikes
        m_spikes.reserve(m_spikeCount);
        std::generate_n(std::back_inserter(m_spikes), m_spikeCount, [] {
            sf::CircleShape spike(3.0f, 3);  // Triangle
            spike.setFillColor(sf::Color(150, 100, 50));
            spike.setOrigin(3.0f, 3.0f);
            return spike;
            });
    }

    void Pufferfish::initializeSprite(SpriteManager& spriteManager)
    {
        Fish::initializeSprite(spriteManager);

        m_texture = &spriteManager.getTexture(getTextureID());
        if (m_texture)
        {
            m_frameWidth = 190;
            m_frameHeight = static_cast<int>(m_texture->getSize().y) - 2;
            int x = 1 + m_swimStart * m_frameWidth;
            getSpriteComponent()->setTextureRect(sf::IntRect(x, 1, m_frameWidth, m_frameHeight));
        }
    }

    void Pufferfish::update(sf::Time deltaTime)
    {
        // Check base frozen state first
        if (m_isFrozen)
        {
            // Just update position with frozen velocity
            updateMovement(deltaTime);

            // Still update visual elements but not state transitions
            for (size_t i = 0; i < m_spikes.size(); ++i)
            {
                float angle = (360.0f / m_spikeCount) * i * Constants::DEG_TO_RAD;
                float spikeRadius = m_radius + (m_inflationLevel * 10.0f);

                sf::Vector2f spikePos(
                    m_position.x + std::cos(angle) * spikeRadius,
                    m_position.y + std::sin(angle) * spikeRadius
                );

                m_spikes[i].setPosition(spikePos);
                m_spikes[i].setRotation(angle * Constants::RAD_TO_DEG);
            }
            return;
        }

        // Call base class update (which now also checks frozen state)
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        // Update automatic cycle between normal and puffed states
        updateCycleState(deltaTime);

        // Update spike positions
        for (size_t i = 0; i < m_spikes.size(); ++i)
        {
            float angle = (360.0f / m_spikeCount) * i * Constants::DEG_TO_RAD;
            float spikeRadius = m_radius + (m_inflationLevel * 10.0f);

            sf::Vector2f spikePos(
                m_position.x + std::cos(angle) * spikeRadius,
                m_position.y + std::sin(angle) * spikeRadius
            );

            m_spikes[i].setPosition(spikePos);
            m_spikes[i].setRotation(angle * Constants::DEG_TO_RAD);
        }
    }

    bool Pufferfish::canEat(const Entity& other) const
    {
        // Pufferfish can't eat when inflated
        if (isInflated())
            return false;

        return Fish::canEat(other);
    }

    void Pufferfish::pushEntity(Entity& entity)
    {
        if (!isInflated() || !canPushEntity(entity))
            return;

        // Calculate push direction
        sf::Vector2f pushDirection = entity.getPosition() - m_position;
        float distance = std::sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);

        if (distance > 0.0f)
        {
            // Normalize and apply push
            pushDirection /= distance;
            sf::Vector2f pushVelocity = pushDirection * m_pushForce;

            // Set entity velocity to push velocity
            entity.setVelocity(pushVelocity);

            // Move entity immediately by push distance
            sf::Vector2f newPosition = entity.getPosition() + pushDirection * m_pushDistance;
            entity.setPosition(newPosition);
        }
    }

    bool Pufferfish::canPushEntity(const Entity& entity) const
    {
        if (!isInflated())
            return false;

        float distance = EntityUtils::distance(*this, entity);
        float pushRadius = m_radius + 10.0f; // Slightly larger than actual radius for push effect

        return distance < pushRadius;
    }

    void Pufferfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        Fish::draw(target, states);

        // Draw spikes when inflating
        if (m_inflationLevel > 0.2f)
        {
            std::for_each(m_spikes.begin(), m_spikes.end(),
                [&target, &states](const sf::CircleShape& spike)
                {
                    target.draw(spike, states);
                });
        }
    }

    void Pufferfish::updateSpriteEffects(sf::Time deltaTime)
    {
        if (getRenderMode() != RenderMode::Sprite || !getSpriteComponent() || !m_texture)
            return;

        m_frameTimer += deltaTime;
        if (m_frameTimer.asSeconds() >= m_frameTime)
        {
            m_frameTimer -= sf::seconds(m_frameTime);

            if (m_playPuff && !m_puffPlayed)
            {
                ++m_frame;
                if (m_frame >= m_puffStart + m_puffCount)
                {
                    m_puffPlayed = true;
                    m_playPuff = false;
                    m_frame = m_puffStart + m_puffCount - 1;
                }
            }
            else if (!m_isPuffed)
            {
                ++m_frame;
                if (m_frame >= m_swimStart + m_swimCount)
                    m_frame = m_swimStart;
            }

            int x = 1 + m_frame * m_frameWidth;
            getSpriteComponent()->setTextureRect(sf::IntRect(x, 1, m_frameWidth, m_frameHeight));
        }
    }

    void Pufferfish::updateCycleState(sf::Time deltaTime)
    {
        m_stateTimer += deltaTime;

        if (!m_isPuffed)
        {
            // Normal state
            if (m_stateTimer.asSeconds() >= m_normalStateDuration)
            {
                // Transition to puffed state
                transitionToInflated();
            }
            else
            {
                // Deflate if needed
                if (m_inflationLevel > 0.0f)
                {
                    m_inflationLevel = std::max(0.0f, m_inflationLevel - m_deflationSpeed * deltaTime.asSeconds());

                    // Update radius
                    m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
                }
            }
        }
        else
        {
            // Puffed state
            if (m_stateTimer.asSeconds() >= m_puffedStateDuration)
            {
                // Transition to normal state
                transitionToNormal();
            }
            else
            {
                // Inflate if needed
                if (m_inflationLevel < 1.0f)
                {
                    m_inflationLevel = std::min(1.0f, m_inflationLevel + m_inflationSpeed * deltaTime.asSeconds());

                    // Update radius
                    m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
                }
            }
        }

        // Slow down when inflated
        float speedMultiplier = 1.0f - (m_inflationLevel * 0.7f);
        m_speed = 100.0f * speedMultiplier;
    }

    void Pufferfish::transitionToInflated()
    {
        m_isPuffed = true;
        m_stateTimer = sf::Time::Zero;
        m_playPuff = true;
        m_puffPlayed = false;
        m_frame = m_puffStart;
    }

    void Pufferfish::transitionToNormal()
    {
        m_isPuffed = false;
        m_stateTimer = sf::Time::Zero;
        m_frame = m_swimStart;
    }

    // PoisonFish implementation
    PoisonFish::PoisonFish(int currentLevel)
        : AdvancedFish(FishSize::Small, 130.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_poisonBubbles()
        , m_wobbleAnimation(0.0f)
        , m_poisonDuration(sf::seconds(m_poisonEffectDuration))
        , m_poisonPoints(m_basePoisonPoints* currentLevel)
    {
        // Purple poisonous appearance
        m_pointValue = 0;

        // Set amplitude for sinusoidal movement - reduced for smaller fish
        m_amplitude = 15.0f;
        m_frequency = 3.0f;

        // Create poison bubble effects - smaller bubbles for small fish
        m_poisonBubbles.reserve(m_bubbleCount);
        std::generate_n(std::back_inserter(m_poisonBubbles), m_bubbleCount, [] {
            sf::CircleShape bubble(2.0f);
            bubble.setFillColor(sf::Color(200, 100, 255, 150));
            bubble.setOrigin(2.0f, 2.0f);
            return bubble;
            });
    }

    void PoisonFish::update(sf::Time deltaTime)
    {
        // Call base class update (which handles frozen state)
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        // Update visual effects even when frozen (but slower)
        if (m_isFrozen)
        {
            // Slow down animation when frozen
            m_wobbleAnimation += deltaTime.asSeconds() * 0.3f;
        }
        else
        {
            // Normal animation speed
            m_wobbleAnimation += deltaTime.asSeconds() * 3.0f;
        }

        // Update poison bubbles
        updatePoisonBubbles(deltaTime);
    }

    void PoisonFish::updatePoisonBubbles(sf::Time deltaTime)
    {
        for (size_t i = 0; i < m_poisonBubbles.size(); ++i)
        {
            float angle = (60.0f * i + m_wobbleAnimation * 30.0f) * Constants::DEG_TO_RAD;
            float radius = 18.0f + 3.0f * std::sin(m_wobbleAnimation + i);

            sf::Vector2f bubblePos(
                m_position.x + std::cos(angle) * radius,
                m_position.y + std::sin(angle) * radius
            );

            m_poisonBubbles[i].setPosition(bubblePos);

            // Pulsing effect for bubbles
            float scale = 1.0f + 0.2f * std::sin(m_wobbleAnimation * 2.0f + i);
            m_poisonBubbles[i].setScale(scale, scale);
        }
    }

    void PoisonFish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw poison bubbles first
        std::for_each(m_poisonBubbles.begin(), m_poisonBubbles.end(),
            [&target, &states](const sf::CircleShape& bubble) {
                target.draw(bubble, states);
            });

        // Draw the fish
        Fish::draw(target, states);
    }

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
        for (size_t i = 0; i < m_fins.size(); ++i)
        {
            float finAngle = (m_colorShift + i * 120.0f) * 3.14159f / 180.0f;
            float finRadius = 20.0f + (m_isEvading ? 10.0f * std::sin(m_colorShift * 5.0f) : 0.0f);

            sf::Vector2f finPos(
                m_position.x + std::cos(finAngle) * finRadius,
                m_position.y + std::sin(finAngle) * finRadius
            );

            m_fins[i].setPosition(finPos);
            m_fins[i].setRotation(finAngle * Constants::DEG_TO_RAD);

            // Pulse fins when evading (unless frozen)
            if (m_isEvading && !m_isFrozen)
            {
                float scale = 1.0f + 0.3f * std::sin(m_colorShift * 10.0f);
                m_fins[i].setScale(scale, scale);
            }
        }
    }

    void Angelfish::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
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
            updateEvasiveMovement(entities, player, deltaTime);
        }
    }

    void Angelfish::updateEvasiveMovement(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
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
}