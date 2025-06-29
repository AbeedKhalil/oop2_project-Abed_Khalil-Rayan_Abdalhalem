#include "Player.h"
#include "Fish.h"
#include "PlayerInput.h"
#include "PlayerGrowth.h"
#include "PlayerVisual.h"
#include "BonusItem.h"
#include "PowerUp.h"
#include "SpecialFish.h"
#include "Animator.h"
#include "SoundPlayer.h"
#include "CollisionDetector.h"
#include "GenericFish.h"
#include <SFML/Window.hpp>
#include <cmath>
#include <algorithm>

namespace FishGame
{
    // Static member initialization
    const sf::Time Player::m_invulnerabilityDuration = sf::seconds(2.0f);
    const sf::Time Player::m_damageCooldownDuration = sf::seconds(0.5f);
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
        , m_invulnerabilityTimer(sf::Time::Zero)
        , m_damageCooldown(sf::Time::Zero)
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
        , m_facingRight(false)
        , m_input(std::make_unique<PlayerInput>(*this))
        , m_growth(std::make_unique<PlayerGrowth>(*this))
        , m_visual(std::make_unique<PlayerVisual>(*this))
    {
        m_radius = m_baseRadius;

        // Start at center of screen
        m_position = sf::Vector2f(
            static_cast<float>(m_windowBounds.x) / 2.0f,
            static_cast<float>(m_windowBounds.y) / 2.0f);
        m_targetPosition = m_position;
    }

    void Player::initializeSystems(GrowthMeter* growthMeter, FrenzySystem* frenzySystem,
        PowerUpManager* powerUpManager, ScoreSystem* scoreSystem)
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
        updateInvulnerability(deltaTime);
        if (m_damageCooldown > sf::Time::Zero)
            m_damageCooldown -= deltaTime;
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

        if (m_thinkingTimer > sf::Time::Zero)
        {
            m_thinkingTimer -= deltaTime;
            sf::Vector2f cloudPos = m_position;
            cloudPos.y -= m_radius * 2.f;
            m_thinkingCloudSprite.setPosition(cloudPos);
            m_thinkingFishSprite.setPosition(cloudPos);
        }

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
        if (m_invulnerabilityTimer > sf::Time::Zero)
            return false;

        EntityType otherType = other.getType();
        if (otherType == EntityType::SmallFish ||
            otherType == EntityType::MediumFish ||
            otherType == EntityType::LargeFish)
        {
            const Fish* fish = dynamic_cast<const Fish*>(&other);
            if (!fish)
                return false;

            FishSize playerSize = getCurrentFishSize();
            FishSize fishSize = fish->getSize();

            return static_cast<int>(playerSize) >= static_cast<int>(fishSize);
        }

        return false;
    }

    bool Player::attemptEat(Entity& other)
    {
        if (!canEat(other))
            return false;

        // Only eat if the target intersects with the player's mouth area
        sf::Vector2f mouthOffset(m_radius, 0.0f);
        mouthOffset.x = m_facingRight ? mouthOffset.x : -mouthOffset.x;

        sf::Vector2f mouthPos = m_position + mouthOffset * 0.8f;
        float mouthRadius = m_radius * 0.5f;

        float distance = CollisionDetector::getDistance(mouthPos, other.getPosition());
        if (distance > mouthRadius + other.getRadius())
            return false;

        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (fish)
        {
            // Use polymorphic score value
            addPoints(fish->getScorePoints());

            // Visual growth
            grow(fish->getPointValue());

            // Register hit for chain bonus
            if (m_scoreSystem)
            {
                m_scoreSystem->registerHit();

                int frenzyMultiplier = m_frenzySystem ? m_frenzySystem->getMultiplier() : 1;
                float powerUpMultiplier = m_powerUpManager ? m_powerUpManager->getScoreMultiplier() : 1.0f;

                m_scoreSystem->addScore(ScoreEventType::FishEaten, fish->getPointValue(),
                    other.getPosition(), frenzyMultiplier, powerUpMultiplier);
                m_scoreSystem->recordFish(fish->getTextureID());
            }

            if (m_frenzySystem)
            {
                m_frenzySystem->registerFishEaten();
            }

            if (m_animator)
            {
                std::string eatAnim = m_facingRight ? "eatRight" : "eatLeft";
                m_animator->play(eatAnim);
                m_currentAnimation = eatAnim;
            }

            return true;
        }

        return false;
    }

    bool Player::canTailBite(const Entity& other) const
    {
        const Fish* fish = dynamic_cast<const Fish*>(&other);
        if (!fish)
            return false;

        int sizeDifference = static_cast<int>(fish->getSize()) - static_cast<int>(getCurrentFishSize());
        return sizeDifference >= 2;
    }

    bool Player::attemptTailBite(Entity& other)
    {
        if (canTailBite(other) && !hasRecentlyTakenDamage())
        {
            sf::Vector2f fishPos = other.getPosition();
            sf::Vector2f fishVelocity = other.getVelocity();

            sf::Vector2f tailOffset = -fishVelocity;
            float length = std::sqrt(tailOffset.x * tailOffset.x + tailOffset.y * tailOffset.y);
            if (length > 0)
            {
                tailOffset = (tailOffset / length) * other.getRadius() * 0.8f;
                sf::Vector2f tailPos = fishPos + tailOffset;

                float dx = m_position.x - tailPos.x;
                float dy = m_position.y - tailPos.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < m_radius + 10.0f)
                {
                    if (m_scoreSystem)
                    {
                        int frenzyMultiplier = m_frenzySystem ? m_frenzySystem->getMultiplier() : 1;
                        float powerUpMultiplier = m_powerUpManager ? m_powerUpManager->getScoreMultiplier() : 1.0f;

                        m_scoreSystem->registerTailBite(m_position, frenzyMultiplier, powerUpMultiplier);
                    }

                    return true;
                }
            }
        }

        return false;
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
        if (m_invulnerabilityTimer > sf::Time::Zero)
            return;

        m_damageCooldown = m_damageCooldownDuration;

        if (m_scoreSystem)
        {
            m_scoreSystem->registerMiss();
        }

        triggerDamageEffect();
    }

    void Player::die()
    {
        // Mark the player as dead so update() ignores further actions
        m_isAlive = false;
        m_position = sf::Vector2f(
            static_cast<float>(m_windowBounds.x) / 2.0f,
            static_cast<float>(m_windowBounds.y) / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_targetPosition = m_position;

        m_invulnerabilityTimer = m_invulnerabilityDuration;

        m_growthProgress = std::max(0.0f, m_growthProgress - 20.0f);

        if (m_growthMeter)
        {
            m_growthMeter->setPoints(m_points);
        }

        m_eatAnimationScale = 1.0f;
        m_damageFlashIntensity = 0.0f;
        m_controlsReversed = false;
        m_poisonColorTimer = sf::Time::Zero;
    }

    void Player::respawn()
    {
        m_isAlive = true;
        m_position = sf::Vector2f(
            static_cast<float>(m_windowBounds.x) / 2.0f,
            static_cast<float>(m_windowBounds.y) / 2.0f);
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_targetPosition = m_position;
        m_invulnerabilityTimer = m_invulnerabilityDuration;
        m_controlsReversed = false;
        m_poisonColorTimer = sf::Time::Zero;
        if (m_soundPlayer)
            m_soundPlayer->play(SoundEffectID::PlayerSpawn);

        startThinking(FishSize::Small);
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

    void Player::startThinking(FishSize size)
    {
        if (!m_spriteManager)
            return;

        m_thinkingCloudSprite.setTexture(m_spriteManager->getTexture(TextureID::ThinkingCloud));
        m_thinkingCloudSprite.setScale(0.5f, 0.5f);
        sf::FloatRect cb = m_thinkingCloudSprite.getLocalBounds();
        m_thinkingCloudSprite.setOrigin(cb.width / 2.f, cb.height);

        if (m_animator)
        {
            m_thinkingFishSprite.setTexture(m_animator->getTexture());
            m_thinkingFishSprite.setTextureRect(m_animator->getCurrentFrame());
        }
        else
        {
            TextureID fishTex = TextureID::SmallFish;
            switch (size)
            {
            case FishSize::Medium: fishTex = TextureID::MediumFish; break;
            case FishSize::Large: fishTex = TextureID::LargeFish; break;
            default: fishTex = TextureID::SmallFish; break;
            }
            m_thinkingFishSprite.setTexture(m_spriteManager->getTexture(fishTex));
        }
        sf::FloatRect fb = m_thinkingFishSprite.getLocalBounds();
        m_thinkingFishSprite.setOrigin(fb.width / 2.f, fb.height / 2.f);
        float scaleX = m_facingRight ? -0.5f : 0.5f;
        m_thinkingFishSprite.setScale(scaleX, 0.5f);

        m_thinkingTimer = sf::seconds(3.f);
    }

void Player::setWindowBounds(const sf::Vector2u& windowSize)
{
    m_windowBounds = windowSize;
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (m_visual)
        m_visual->draw(target, states);

    if (m_thinkingTimer > sf::Time::Zero)
    {
        target.draw(m_thinkingCloudSprite, states);
        target.draw(m_thinkingFishSprite, states);
    }
}
    void Player::constrainToWindow()
    {
        m_position.x = std::clamp(m_position.x, m_radius,
            static_cast<float>(m_windowBounds.x) - m_radius);
        m_position.y = std::clamp(m_position.y, m_radius,
            static_cast<float>(m_windowBounds.y) - m_radius);
    }

    void Player::updateInvulnerability(sf::Time deltaTime)
    {
        if (m_invulnerabilityTimer > sf::Time::Zero)
        {
            m_invulnerabilityTimer -= deltaTime;
            if (m_invulnerabilityTimer < sf::Time::Zero)
            {
                m_invulnerabilityTimer = sf::Time::Zero;
            }
        }
    }


void Player::updateVisualEffects(sf::Time deltaTime)
{
    if (m_visual)
        m_visual->update(deltaTime);
}
}

