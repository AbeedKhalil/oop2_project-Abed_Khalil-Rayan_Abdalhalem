#pragma once

#include "AdvancedFish.h"
#include <vector>
#include <SFML/Graphics/CircleShape.hpp>

namespace FishGame {

class PoisonFish : public AdvancedFish
{
public:
    explicit PoisonFish(int currentLevel = 1);
    ~PoisonFish() override = default;

    TextureID getTextureID() const override { return TextureID::PoisonFish; }
    EntityType getType() const override { return EntityType::SmallFish; }
    int getPointValue() const override { return m_poisonPoints; }
    int getScorePoints() const override { return m_poisonPoints; }

    void update(sf::Time deltaTime) override;

    sf::Time getPoisonDuration() const { return m_poisonDuration; }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void updatePoisonBubbles(sf::Time /*deltaTime*/);

private:
    std::vector<sf::CircleShape> m_poisonBubbles;
    float m_wobbleAnimation;
    sf::Time m_poisonDuration;
    int m_poisonPoints;

    static constexpr float m_poisonEffectDuration = 5.0f;
    static constexpr int m_basePoisonPoints = -10;
    static constexpr int m_bubbleCount = 6;
};

} // namespace FishGame
