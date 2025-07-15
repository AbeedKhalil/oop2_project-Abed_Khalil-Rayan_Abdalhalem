#include "PlayerGrowth.h"
#include "Player.h"


namespace FishGame {

PlayerGrowth::PlayerGrowth(Player& player) : m_player(player) {}

void PlayerGrowth::grow(int scoreValue)
{
    float growthPoints = 0.f;
    if (scoreValue <= 3)
        growthPoints = Player::tinyFishGrowth();
    else if (scoreValue <= 6)
        growthPoints = Player::smallFishGrowth();
    else if (scoreValue <= 9)
        growthPoints = Player::mediumFishGrowth();
    else
        growthPoints = static_cast<float>(scoreValue);

    m_player.addGrowthProgress(growthPoints);

    if (auto meter = m_player.getGrowthMeter())
    {
        meter->setPoints(m_player.getPoints());
    }

    m_player.triggerEatEffect();
}

void PlayerGrowth::addPoints(int points)
{
    m_player.incrementPoints(points);
    if (auto meter = m_player.getGrowthMeter())
    {
        meter->setPoints(m_player.getPoints());
    }
}

void PlayerGrowth::checkStageAdvancement()
{
    if (m_player.getCurrentStage() == 1 && m_player.getPoints() >= Constants::POINTS_FOR_STAGE_2)
    {
        m_player.setCurrentStage(2);
        updateStage();
    }
    else if (m_player.getCurrentStage() == 2 && m_player.getPoints() >= Constants::POINTS_FOR_STAGE_3)
    {
        m_player.setCurrentStage(3);
        updateStage();
    }
}

void PlayerGrowth::resetSize()
{
    m_player.setScore(0);
    m_player.setCurrentStage(1);
    m_player.setGrowthProgress(0.f);
    m_player.setRadius(Player::baseRadius());

    if (auto meter = m_player.getGrowthMeter())
    {
        meter->reset();
        meter->setStage(1);
    }

    updateStage();
}

void PlayerGrowth::fullReset()
{
    resetSize();
    m_player.setPoints(0);
    m_player.setControlsReversed(false);
    m_player.setPoisonColorTimer(sf::Time::Zero);
}

void PlayerGrowth::updateStage()
{
    if (m_player.getSoundPlayer() && m_player.getCurrentStage() == 1)
        m_player.getSoundPlayer()->play(SoundEffectID::StageIntro);
    else if (m_player.getSoundPlayer())
        m_player.getSoundPlayer()->play(SoundEffectID::PlayerGrow);

    m_player.setRadius(static_cast<float>(Player::baseRadius() *
        std::pow(Player::growthFactor(), static_cast<float>(m_player.getCurrentStage() - 1))));

    if (auto meter = m_player.getGrowthMeter())
    {
        meter->setStage(m_player.getCurrentStage());
    }

    if (m_player.getAnimator() && m_player.getRenderMode() == Entity::RenderMode::Sprite && m_player.getSpriteManager())
    {
        float stageScale = 1.f;
        const auto& cfg = m_player.getSpriteManager()->getScaleConfig();
        switch (m_player.getCurrentFishSize())
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
            stageScale = 1.f;
            break;
        }
        m_player.getAnimator()->setScale(sf::Vector2f(stageScale, stageScale));
    }

    m_player.getActiveEffects().push_back({1.5f, 0.f, sf::Color::Cyan, sf::seconds(0.5f)});
}

} // namespace FishGame
