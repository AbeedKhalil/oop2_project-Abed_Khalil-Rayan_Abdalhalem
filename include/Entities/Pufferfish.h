#pragma once

#include "AdvancedFish.h"
#include "SpriteManager.h"
#include <vector>
#include <SFML/Graphics/CircleShape.hpp>

namespace FishGame {

class Pufferfish : public AdvancedFish
{
public:
    explicit Pufferfish(int currentLevel = 1);
    ~Pufferfish() override = default;

    EntityType getType() const override { return EntityType::MediumFish; }
    TextureID getTextureID() const override
    {
        return isInflated() ? TextureID::PufferfishInflated : TextureID::Pufferfish;
    }
    int getScorePoints() const override { return Constants::PUFFERFISH_POINTS; }

    void update(sf::Time deltaTime) override;
    void initializeSprite(SpriteManager& spriteManager);
    bool canEat(const Entity& other) const;

    bool isInflated() const { return m_isPuffed; }

    void pushEntity(Entity& entity);
    bool canPushEntity(const Entity& entity) const;

    void onCollide(Player& player, CollisionSystem& system) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void updateCycleState(sf::Time deltaTime);
    void transitionToInflated();
    void transitionToNormal();

    enum class PuffPhase { None, Inflating, Holding, Deflating };

private:
    bool m_isPuffed;
    sf::Time m_stateTimer;
    float m_inflationLevel;
    float m_normalRadius;
    std::vector<sf::CircleShape> m_spikes;

    static constexpr float m_pushDistance = 10.0f;
    static constexpr float m_pushForce = 500.0f;
    static constexpr float m_normalStateDuration = 5.0f;
    static constexpr float m_puffedStateDuration = 5.0f;
    static constexpr float m_inflationSpeed = 3.0f;
    static constexpr float m_deflationSpeed = 3.0f;
    static constexpr float m_inflatedRadiusMultiplier = 2.0f;
    static constexpr int m_spikeCount = 8;

    bool m_isPuffing{ false };
    sf::Time m_puffTimer{ sf::Time::Zero };
    static constexpr float m_puffAnimDuration = 0.6f;
    PuffPhase m_puffPhase{ PuffPhase::None };
};

} // namespace FishGame
