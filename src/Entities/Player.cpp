#include "Player.h"
#include "PlayerInput.h"
#include "PlayerGrowth.h"
#include "PlayerVisual.h"
#include "PlayerStatus.h"
#include "Hazard.h"


namespace FishGame
{
    // Static member initialization
    const sf::Time Player::m_eatAnimationDuration = sf::seconds(0.3f);
    const sf::Time Player::m_turnAnimationDuration = sf::seconds(0.45f);

    Player::~Player() = default;

    Player::Player()
        : Entity()
        , m_score(0)
        , m_currentStage(1)
        , m_growthProgress(0.0f)
        , m_autoOrient(true)
        , m_points(0)
        , m_targetPosition(0.0f, 0.0f)
        , m_controlsReversed(false)
        , m_poisonColorTimer(sf::Time::Zero)
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
        , m_spriteManager(nullptr)
        , m_soundPlayer(nullptr)
        , m_status(std::make_unique<PlayerStatus>(*this))
        , m_speedMultiplier(1.0f)
        , m_speedBoostTimer(sf::Time::Zero)
        , m_windowBounds(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT)
        , m_activeEffects()
        , m_eatAnimationScale(1.0f)
        , m_eatAnimationTimer(sf::Time::Zero)
        , m_turnAnimationTimer(sf::Time::Zero)
        , m_damageFlashColor(sf::Color::White)
        , m_damageFlashIntensity(0.0f)
        , m_animator(nullptr)
        , m_currentAnimation()
        , m_input(std::make_unique<PlayerInput>(*this))
        , m_growth(std::make_unique<PlayerGrowth>(*this))
        , m_visual(std::make_unique<PlayerVisual>(*this))
        , m_facingRight(false)
    {
        m_radius = m_baseRadius;

        // Start at center of screen
        m_position = sf::Vector2f(
            static_cast<float>(m_windowBounds.x) / 2.0f,
            static_cast<float>(m_windowBounds.y) / 2.0f);
        m_targetPosition = m_position;
    }

    void Player::initializeSystems(GrowthMeter* growthMeter, FrenzySystem* frenzySystem,
        IPowerUpManager* powerUpManager, IScoreSystem* scoreSystem)
    {
        m_growthMeter = growthMeter;
        m_frenzySystem = frenzySystem;
        m_powerUpManager = powerUpManager;
        m_scoreSystem = scoreSystem;

        if (m_growthMeter)
        {
            m_growthMeter->setStage(m_currentStage);
            m_growthMeter->setOnStageComplete([this]() { checkStageAdvancement(); });
        }
    }

    void Player::initializeSprite(SpriteManager& spriteManager)
    {
        m_spriteManager = &spriteManager;

        const sf::Texture& tex = spriteManager.getTexture(getTextureID());
        m_animator = std::make_unique<Animator>(createFishAnimator(tex));
        m_animator->setPosition(m_position);
        setRenderMode(RenderMode::Sprite);
        m_currentAnimation = "idleLeft";
        m_animator->play(m_currentAnimation);
    }

    TextureID Player::getTextureID() const
    {
        switch (getCurrentFishSize())
        {
        case FishSize::Small:
            return TextureID::PlayerSmall;
        case FishSize::Medium:
            return TextureID::PlayerMedium;
        case FishSize::Large:
            return TextureID::PlayerLarge;
        default:
            return TextureID::PlayerSmall;
        }
    }

    void Player::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update timers
        if (m_status)
            m_status->update(deltaTime);
        if (m_speedBoostTimer > sf::Time::Zero)
        {
            m_speedBoostTimer -= deltaTime;
            if (m_speedBoostTimer <= sf::Time::Zero && m_soundPlayer)
                m_soundPlayer->play(SoundEffectID::SpeedEnd);
        }

        // Handle input
        handleInput();


        // Limit maximum speed
        float maxSpeed = m_maxSpeed * (m_speedBoostTimer > sf::Time::Zero ? m_speedMultiplier : 1.0f);
        float currentSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
        if (currentSpeed > maxSpeed)
        {
            m_velocity = (m_velocity / currentSpeed) * maxSpeed;
        }

        // Update position
        updateMovement(deltaTime);

        // Keep player within window bounds
        constrainToWindow();

        // Auto-orient sprite based on velocity
        if (m_autoOrient && m_animator && m_renderMode == RenderMode::Sprite && m_eatAnimationTimer <= sf::Time::Zero)
        {
            if (std::abs(m_velocity.x) > m_orientationThreshold)
            {
                bool newFacingRight = m_velocity.x > 0;
                if (newFacingRight != m_facingRight)
                {
                    m_facingRight = newFacingRight;
                    m_turnAnimationTimer = m_turnAnimationDuration;
                    std::string turnAnim = m_facingRight ? "turnLeftToRight" : "turnRightToLeft";
                    m_animator->play(turnAnim);
                    m_currentAnimation = turnAnim;
                }
            }
        }

        // Rest of the update logic remains the same...
        checkStageAdvancement();
        updateVisualEffects(deltaTime);

        if (m_renderMode == RenderMode::Sprite && m_animator)
        {
            m_animator->update(deltaTime);

            float stageScale = 1.0f;
            if (m_spriteManager)
            {
                const auto& cfg = m_spriteManager->getScaleConfig();
                switch (getCurrentFishSize())
                {
                case FishSize::Small:
                    stageScale = cfg.small;
                    break;
                case FishSize::Medium:
                    stageScale = (cfg.medium) + 0.18f;
                    break;
                case FishSize::Large:
                    stageScale = (cfg.large) + 0.4f;
                    break;
                default:
                    stageScale = 1.0f;
                    break;
                }
            }

            m_animator->setScale(sf::Vector2f(stageScale * m_eatAnimationScale,
                stageScale * m_eatAnimationScale));

            std::string desired;
            float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
            if (speed > 10.f)
                desired = m_facingRight ? "swimRight" : "swimLeft";
            else
                desired = m_facingRight ? "idleRight" : "idleLeft";

            if (m_turnAnimationTimer <= sf::Time::Zero && m_eatAnimationTimer <= sf::Time::Zero && desired != m_currentAnimation)
            {
                m_animator->play(desired);
                m_currentAnimation = desired;
            }

            m_animator->setPosition(m_position);
        }
    }

    void Player::handleInput()
    {
        if (m_input)
            m_input->handleInput();
    }

    sf::FloatRect Player::getBounds() const
    {
        return sf::FloatRect(m_position.x - m_radius, m_position.y - m_radius,
            m_radius * 2.0f, m_radius * 2.0f);
    }

    void Player::grow(int scoreValue)
    {
        if (m_growth)
            m_growth->grow(scoreValue);
    }

    void Player::addPoints(int points)
    {
        if (m_growth)
            m_growth->addPoints(points);
    }

    void Player::checkStageAdvancement()
    {
        if (m_growth)
            m_growth->checkStageAdvancement();
    }
    void Player::resetSize()
    {
        if (m_growth)
            m_growth->resetSize();
    }
    void Player::fullReset()
    {
        if (m_growth)
            m_growth->fullReset();
    }


bool Player::canEat(const Entity& other) const
{
    return m_status ? m_status->canEat(other) : false;
}

bool Player::attemptEat(Entity& other)
{
    return m_status ? m_status->attemptEat(other) : false;
}

bool Player::canTailBite(const Entity& other) const
{
    return m_status ? m_status->canTailBite(other) : false;
}

bool Player::attemptTailBite(Entity& other)
{
    return m_status ? m_status->attemptTailBite(other) : false;
}

    FishSize Player::getCurrentFishSize() const
    {
        switch (m_currentStage)
        {
        case 1:
            return FishSize::Small;
        case 2:
            return FishSize::Medium;
        case 3:
            return FishSize::Large;
        default:
            return FishSize::Small;
        }
    }

void Player::takeDamage()
{
    if (m_status)
        m_status->takeDamage();
}

void Player::die()
{
    if (m_status)
        m_status->die();
}

void Player::respawn()
{
    if (m_status)
        m_status->respawn();
}

bool Player::isInvulnerable() const
{
    return m_status ? m_status->isInvulnerable() : false;
}

bool Player::hasRecentlyTakenDamage() const
{
    return m_status ? m_status->hasRecentlyTakenDamage() : false;
}

sf::Time Player::getInvulnerabilityTimer() const
{
    return m_status ? m_status->getInvulnerabilityTimer() : sf::Time::Zero;
}

void Player::applySpeedBoost(float multiplier, sf::Time duration)
{
    m_speedMultiplier = multiplier;
    m_speedBoostTimer = duration;
    if (m_soundPlayer)
        m_soundPlayer->play(SoundEffectID::SpeedStart);
}

void Player::applyPoisonEffect(sf::Time duration)
{
    m_poisonColorTimer = duration;
    m_controlsReversed = true;
    if (m_soundPlayer)
        m_soundPlayer->play(SoundEffectID::PlayerPoison);
}

void Player::triggerEatEffect()
{
    if (m_visual)
        m_visual->triggerEatEffect();
}

void Player::triggerDamageEffect()
{
    if (m_visual)
        m_visual->triggerDamageEffect();
}

void Player::setWindowBounds(const sf::Vector2u& windowSize)
{
    m_windowBounds = windowSize;
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (m_visual)
        m_visual->draw(target, states);
}
    void Player::constrainToWindow()
    {
        m_position.x = std::clamp(m_position.x, m_radius,
            static_cast<float>(m_windowBounds.x) - m_radius);
        m_position.y = std::clamp(m_position.y, m_radius,
            static_cast<float>(m_windowBounds.y) - m_radius);
    }


void Player::updateVisualEffects(sf::Time deltaTime)
{
    if (m_visual)
        m_visual->update(deltaTime);
}

    void Player::onCollideWith(Entity& other, CollisionSystem& system)
    {
        other.onCollide(*this, system);
    }

    void Player::onCollideWith(Fish& fish, CollisionSystem& system)
    {
        fish.onCollide(*this, system);
    }

    void Player::onCollideWith(Hazard& hazard, CollisionSystem& system)
    {
        hazard.onCollide(*this, system);
    }
}

