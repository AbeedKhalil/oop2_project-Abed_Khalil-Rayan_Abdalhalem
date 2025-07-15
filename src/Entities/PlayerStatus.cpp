#include "PlayerStatus.h"
#include "Player.h"
#include "Fish.h"
#include "CollisionDetector.h"
#include "FrenzySystem.h"
#include "IPowerUpManager.h"
#include "IScoreSystem.h"
#include "Animator.h"
#include <cmath>

namespace FishGame {

const sf::Time PlayerStatus::m_invulnerabilityDuration = sf::seconds(2.0f);
const sf::Time PlayerStatus::m_damageCooldownDuration = sf::seconds(0.5f);

PlayerStatus::PlayerStatus(Player& player)
    : m_player(player)
    , m_invulnerabilityTimer(sf::Time::Zero)
    , m_damageCooldown(sf::Time::Zero)
{
}

void PlayerStatus::update(sf::Time deltaTime)
{
    if (m_invulnerabilityTimer > sf::Time::Zero) {
        m_invulnerabilityTimer -= deltaTime;
        if (m_invulnerabilityTimer < sf::Time::Zero)
            m_invulnerabilityTimer = sf::Time::Zero;
    }

    if (m_damageCooldown > sf::Time::Zero)
        m_damageCooldown -= deltaTime;
}

bool PlayerStatus::canEat(const Entity& other) const
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

        FishSize playerSize = m_player.getCurrentFishSize();
        FishSize fishSize = fish->getSize();

        return static_cast<int>(playerSize) >= static_cast<int>(fishSize);
    }

    return false;
}

bool PlayerStatus::attemptEat(Entity& other)
{
    if (!canEat(other))
        return false;

    sf::Vector2f mouthOffset(m_player.getRadius(), 0.f);
    mouthOffset.x = m_player.isFacingRight() ? mouthOffset.x : -mouthOffset.x;

    sf::Vector2f mouthPos = m_player.getPosition() + mouthOffset * 0.8f;
    float mouthRadius = m_player.getRadius() * 0.5f;

    float distance = CollisionDetector::getDistance(mouthPos, other.getPosition());
    if (distance > mouthRadius + other.getRadius())
        return false;

    const Fish* fish = dynamic_cast<const Fish*>(&other);
    if (fish)
    {
        m_player.addPoints(fish->getScorePoints());
        m_player.grow(fish->getPointValue());

        if (m_player.m_scoreSystem)
        {
            m_player.m_scoreSystem->registerHit();

            int frenzyMultiplier = m_player.m_frenzySystem ? m_player.m_frenzySystem->getMultiplier() : 1;
            float powerUpMultiplier = m_player.m_powerUpManager ? m_player.m_powerUpManager->getScoreMultiplier() : 1.f;

            m_player.m_scoreSystem->addScore(ScoreEventType::FishEaten, fish->getPointValue(),
                                             other.getPosition(), frenzyMultiplier, powerUpMultiplier);
            m_player.m_scoreSystem->recordFish(fish->getTextureID());
        }

        if (m_player.m_frenzySystem)
        {
            m_player.m_frenzySystem->registerFishEaten();
        }

        if (m_player.getAnimator())
        {
            std::string eatAnim = m_player.isFacingRight() ? "eatRight" : "eatLeft";
            m_player.getAnimator()->play(eatAnim);
            m_player.setCurrentAnimation(eatAnim);
        }

        return true;
    }

    return false;
}

bool PlayerStatus::canTailBite(const Entity& other) const
{
    const Fish* fish = dynamic_cast<const Fish*>(&other);
    if (!fish)
        return false;

    int sizeDifference = static_cast<int>(fish->getSize()) - static_cast<int>(m_player.getCurrentFishSize());
    return sizeDifference >= 2;
}

bool PlayerStatus::attemptTailBite(Entity& other)
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

            float dx = m_player.getPosition().x - tailPos.x;
            float dy = m_player.getPosition().y - tailPos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < m_player.getRadius() + 10.f)
            {
                if (m_player.m_scoreSystem)
                {
                    int frenzyMultiplier = m_player.m_frenzySystem ? m_player.m_frenzySystem->getMultiplier() : 1;
                    float powerUpMultiplier = m_player.m_powerUpManager ? m_player.m_powerUpManager->getScoreMultiplier() : 1.f;

                    m_player.m_scoreSystem->registerTailBite(m_player.getPosition(), frenzyMultiplier, powerUpMultiplier);
                }

                return true;
            }
        }
    }

    return false;
}

void PlayerStatus::takeDamage()
{
    if (m_invulnerabilityTimer > sf::Time::Zero)
        return;

    m_damageCooldown = m_damageCooldownDuration;

    if (m_player.m_scoreSystem)
    {
        m_player.m_scoreSystem->registerMiss();
    }

    m_player.triggerDamageEffect();
}

void PlayerStatus::die()
{
    m_player.m_isAlive = false;
    m_player.m_position = {static_cast<float>(m_player.m_windowBounds.x) / 2.f,
                           static_cast<float>(m_player.m_windowBounds.y) / 2.f};
    m_player.m_velocity = {0.f, 0.f};
    m_player.m_targetPosition = m_player.m_position;

    m_invulnerabilityTimer = m_invulnerabilityDuration;

    m_player.m_growthProgress = std::max(0.f, m_player.m_growthProgress - 20.f);

    if (m_player.m_growthMeter)
    {
        m_player.m_growthMeter->setPoints(m_player.m_points);
    }

    m_player.m_eatAnimationScale = 1.f;
    m_player.m_damageFlashIntensity = 0.f;
    m_player.m_controlsReversed = false;
    m_player.m_poisonColorTimer = sf::Time::Zero;
}

void PlayerStatus::respawn()
{
    m_player.m_isAlive = true;
    m_player.m_position = {static_cast<float>(m_player.m_windowBounds.x) / 2.f,
                           static_cast<float>(m_player.m_windowBounds.y) / 2.f};
    m_player.m_velocity = {0.f, 0.f};
    m_player.m_targetPosition = m_player.m_position;
    m_invulnerabilityTimer = m_invulnerabilityDuration;
    m_player.m_controlsReversed = false;
    m_player.m_poisonColorTimer = sf::Time::Zero;
    if (m_player.m_soundPlayer)
        m_player.m_soundPlayer->play(SoundEffectID::PlayerSpawn);
}

} // namespace FishGame
