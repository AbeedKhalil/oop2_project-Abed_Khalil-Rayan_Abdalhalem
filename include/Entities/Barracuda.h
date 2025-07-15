#pragma once

#include "AdvancedFish.h"
#include "SpriteManager.h"

namespace FishGame {

class Barracuda : public AdvancedFish
{
public:
    explicit Barracuda(int currentLevel = 1);
    ~Barracuda() override = default;

    EntityType getType() const override { return EntityType::LargeFish; }

    TextureID getTextureID() const override { return TextureID::Barracuda; }
    int getScorePoints() const override { return Constants::BARRACUDA_POINTS; }

    void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
                  const Entity* player, sf::Time deltaTime);

    void update(sf::Time deltaTime) override;
    void initializeSprite(SpriteManager& spriteManager);
    void playEatAnimation() override;
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void updateHuntingBehavior(const Entity* target, sf::Time /*deltaTime*/);

private:
    const Entity* m_currentTarget;
    sf::Time m_huntTimer;
    float m_dashSpeed;
    bool m_isDashing;

    std::unique_ptr<Animator> m_animator;
    std::string m_currentAnimation;
    bool m_facingRight{ false };
    bool m_turning{ false };
    sf::Time m_turnTimer{ sf::Time::Zero };

    static constexpr float m_turnDuration = 0.45f;
    static constexpr float m_huntRange = 250.0f;
    static constexpr float m_dashMultiplier = 2.5f;
    static constexpr float m_dashDuration = 1.0f;
};

} // namespace FishGame
