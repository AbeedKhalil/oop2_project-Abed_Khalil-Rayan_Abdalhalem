#pragma once

#include "AdvancedFish.h"
#include "SpriteManager.h"

namespace FishGame {

class Angelfish : public AdvancedFish
{
public:
    explicit Angelfish(int currentLevel = 1);
    ~Angelfish() override = default;

    EntityType getType() const override { return EntityType::SmallFish; }
    int getPointValue() const { return m_bonusPoints; }
    TextureID getTextureID() const override { return TextureID::Angelfish; }
    int getScorePoints() const override { return Constants::ANGELFISH_POINTS; }

    void update(sf::Time deltaTime) override;

    void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
                  const Entity* player, sf::Time deltaTime) override;

    void onCollide(Player& player, CollisionSystem& system) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void updateErraticMovement(sf::Time deltaTime);
    void updateEvasiveMovement(const std::vector<std::unique_ptr<Entity>>& entities,
                               const Entity* player);
    sf::Vector2f calculateEscapeVector(const std::vector<const Entity*>& threats);

private:
    int m_bonusPoints;
    float m_colorShift;
    std::vector<sf::CircleShape> m_fins;
    sf::Time m_directionChangeTimer;

    const Entity* m_currentThreat;
    bool m_isEvading;
    sf::Time m_evasionTimer;

    static constexpr float m_baseSpeed = 280.0f;
    static constexpr float m_evadeSpeed = 400.0f;
    static constexpr float m_directionChangeInterval = 0.3f;
    static constexpr float m_threatDetectionRange = 150.0f;
    static constexpr float m_panicRange = 80.0f;
    static constexpr int m_baseBonus = 50;
};

} // namespace FishGame
