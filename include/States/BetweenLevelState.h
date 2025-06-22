#pragma once

#include "State.h"
#include "GameConstants.h"
#include <vector>
#include <string>

namespace FishGame
{
    void setBetweenLevelEntities(std::vector<std::string> entities);

    class BetweenLevelState : public State
    {
    public:
        explicit BetweenLevelState(Game& game);
        ~BetweenLevelState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        std::vector<std::string> m_entities;
        sf::Text m_headerText;
        sf::Text m_continueText;
        std::vector<sf::Text> m_entityTexts;
        sf::RectangleShape m_background;
    };
}

