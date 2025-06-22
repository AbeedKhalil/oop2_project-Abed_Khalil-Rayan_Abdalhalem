#pragma once

#include "State.h"
#include "GameConstants.h"
#include "Levels/LevelTable.h"
#include <vector>
#include <string>

namespace FishGame
{
    // Store the definition for the upcoming level so the state can access it
    void setUpcomingLevelDef(LevelDef def);
    LevelDef takeUpcomingLevelDef();

    class BetweenLevelState : public State
    {
    public:
        explicit BetweenLevelState(Game& game, LevelDef upcoming);
        ~BetweenLevelState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        LevelDef m_def;
        sf::Text m_headerText;
        sf::Text m_continueText;
        std::vector<sf::Text> m_entityTexts;
        sf::RectangleShape m_background;
    };
}

