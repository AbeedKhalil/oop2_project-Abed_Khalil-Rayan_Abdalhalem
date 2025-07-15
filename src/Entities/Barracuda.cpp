#include "Barracuda.h"
#include "Player.h"

namespace FishGame
{

    // Barracuda implementation
    Barracuda::Barracuda(int currentLevel)
        : AdvancedFish(FishSize::Large, 180.0f, currentLevel, MovementPattern::Linear)
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
        m_animator = std::make_unique<Animator>(createBarracudaAnimator(tex));

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
        m_eating = true;
        m_eatTimer = sf::seconds(m_eatDuration);
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

    void Barracuda::updateHuntingBehavior(const Entity* target, sf::Time /*deltaTime*/)
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

}

