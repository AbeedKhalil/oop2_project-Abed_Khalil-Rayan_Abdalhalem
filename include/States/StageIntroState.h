#pragma once

#include "State.h"
#include "GameConstants.h"
#include "StateUtils.h"
#include <vector>
#include <string>

namespace FishGame
{
    class StageIntroState;
    template<> struct is_state<StageIntroState> : std::true_type {};

    struct StageIntroConfig
    {
        int level = 1;
        bool pushPlay = true;
        static StageIntroConfig& getInstance()
        {
            static StageIntroConfig instance;
            return instance;
        }
    };

    class StageIntroState : public State
    {
    public:
        explicit StageIntroState(Game& game);
        ~StageIntroState() override = default;

        static void configure(int level, bool pushPlay);

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        struct Item
        {
            sf::Sprite sprite;
            sf::Text text;
        };

        void setupItems();
        void exitState();

        sf::Sprite m_backgroundSprite;
        std::vector<Item> m_items;
        sf::Time m_elapsed;
        int m_level;
        bool m_pushPlay;
        static constexpr float DISPLAY_TIME = 3.0f;
    };
}
