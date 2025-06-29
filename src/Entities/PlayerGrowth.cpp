#include "PlayerGrowth.h"
#include "Player.h"
#include "PlayerVisual.h"
#include <cmath>

namespace FishGame {

PlayerGrowth::PlayerGrowth(Player& player) : m_player(player) {}

void PlayerGrowth::grow(int scoreValue)
{
    float growthPoints = 0.f;
    if (scoreValue <= 3)
        growthPoints = Player::m_tinyFishGrowth;
    else if (scoreValue <= 6)
        growthPoints = Player::m_smallFishGrowth;
    else if (scoreValue <= 9)
        growthPoints = Player::m_mediumFishGrowth;
    else
        growthPoints = static_cast<float>(scoreValue);

    m_player.m_growthProgress += growthPoints;

    if (m_player.m_growthMeter)
    {
        m_player.m_growthMeter->setPoints(m_player.m_points);
    }

    if (m_player.m_visual)
        m_player.m_visual->triggerEatEffect();
}

void PlayerGrowth::addPoints(int points)
{
    m_player.m_points += points;
    if (m_player.m_growthMeter)
    {
        m_player.m_growthMeter->setPoints(m_player.m_points);
    }
}

void PlayerGrowth::checkStageAdvancement()
{
    if (m_player.m_currentStage == 1 && m_player.m_points >= Constants::POINTS_FOR_STAGE_2)
    {
        m_player.m_currentStage = 2;
        updateStage();
    }
    else if (m_player.m_currentStage == 2 && m_player.m_points >= Constants::POINTS_FOR_STAGE_3)
    {
        m_player.m_currentStage = 3;
        updateStage();
    }
}

void PlayerGrowth::resetSize()
{
    m_player.m_score = 0;
    m_player.m_currentStage = 1;
    m_player.m_growthProgress = 0.f;
    m_player.m_radius = Player::m_baseRadius;

    if (m_player.m_growthMeter)
    {
        m_player.m_growthMeter->reset();
        m_player.m_growthMeter->setStage(1);
    }

    updateStage();
}

void PlayerGrowth::fullReset()
{
    resetSize();
    m_player.m_points = 0;
    m_player.m_controlsReversed = false;
    m_player.m_poisonColorTimer = sf::Time::Zero;
}

void PlayerGrowth::updateStage()
{
    if (m_player.m_soundPlayer && m_player.m_currentStage == 1)
        m_player.m_soundPlayer->play(SoundEffectID::StageIntro);
    else if (m_player.m_soundPlayer)
        m_player.m_soundPlayer->play(SoundEffectID::PlayerGrow);

    m_player.m_radius = static_cast<float>(Player::m_baseRadius *
        std::pow(Player::m_growthFactor, static_cast<float>(m_player.m_currentStage - 1)));

    if (m_player.m_growthMeter)
    {
        m_player.m_growthMeter->setStage(m_player.m_currentStage);
    }

    if (m_player.m_currentStage == 2)
        m_player.startThinking(FishSize::Medium);
    else if (m_player.m_currentStage == 3)
        m_player.startThinking(FishSize::Large);

    if (m_player.m_animator && m_player.m_renderMode == Entity::RenderMode::Sprite && m_player.m_spriteManager)
    {
        float stageScale = 1.f;
        const auto& cfg = m_player.m_spriteManager->getScaleConfig();
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
        m_player.m_animator->setScale(sf::Vector2f(stageScale, stageScale));
    }

    m_player.m_activeEffects.push_back({1.5f, 0.f, sf::Color::Cyan, sf::seconds(0.5f)});
}

} // namespace FishGame
