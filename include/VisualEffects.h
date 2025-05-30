#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

namespace FishGame
{
    class VisualEffect
    {
    public:
        VisualEffect(sf::Time duration);
        virtual ~VisualEffect() = default;

        virtual void update(sf::Time deltaTime) = 0;
        virtual void draw(sf::RenderTarget& target) const = 0;

        bool isActive() const { return m_timeRemaining > sf::Time::Zero; }

    protected:
        sf::Time m_timeRemaining;
        sf::Time m_totalDuration;
    };

    class FlashingText : public VisualEffect
    {
    public:
        FlashingText(const sf::Text& text, sf::Time duration, float flashSpeed = 5.0f);

        void update(sf::Time deltaTime) override;
        void draw(sf::RenderTarget& target) const override;

    private:
        mutable sf::Text m_text;  // Make mutable for color changes
        float m_flashSpeed;
        float m_currentAlpha;
    };

    class ScorePopup : public VisualEffect
    {
    public:
        ScorePopup(const sf::Vector2f& position, int points, const sf::Font& font);

        void update(sf::Time deltaTime) override;
        void draw(sf::RenderTarget& target) const override;

    private:
        sf::Text m_text;
        sf::Vector2f m_velocity;
        float m_fadeSpeed;
    };

    // Fixed EffectManager template
    class EffectManager
    {
    public:
        template<typename EffectType, typename... Args>
        void createEffect(Args&&... args)
        {
            m_effects.push_back(std::make_unique<EffectType>(std::forward<Args>(args)...));
        }

        void update(sf::Time deltaTime)
        {
            std::for_each(m_effects.begin(), m_effects.end(),
                [deltaTime](const std::unique_ptr<VisualEffect>& effect)
                {
                    effect->update(deltaTime);
                });

            // Remove inactive effects
            m_effects.erase(
                std::remove_if(m_effects.begin(), m_effects.end(),
                    [](const std::unique_ptr<VisualEffect>& effect)
                    {
                        return !effect->isActive();
                    }),
                m_effects.end()
            );
        }

        void draw(sf::RenderTarget& target) const
        {
            std::for_each(m_effects.begin(), m_effects.end(),
                [&target](const std::unique_ptr<VisualEffect>& effect)
                {
                    effect->draw(target);
                });
        }

        void clear()
        {
            m_effects.clear();
        }

    private:
        std::vector<std::unique_ptr<VisualEffect>> m_effects;
    };
}