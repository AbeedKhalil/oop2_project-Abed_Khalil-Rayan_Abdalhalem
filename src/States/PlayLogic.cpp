#include "PlayLogic.h"
#include "PlayState.h"
#include "Game.h"
#include "StageIntroState.h"

namespace FishGame {

PlayLogic::PlayLogic(PlayState& state) : m_state(state) {}

void PlayLogic::handleEvent(const sf::Event& event)
{
    if (m_state.m_isPlayerStunned || m_state.getGame().getCurrentState<StageIntroState>())
        return;

    m_state.m_inputHandler.setReversed(m_state.m_hasControlsReversed);
    m_state.m_inputHandler.processEvent(event, [this](const sf::Event& processedEvent)
    {
        switch (processedEvent.type)
        {
        case sf::Event::KeyPressed:
            switch (processedEvent.key.code)
            {
            case sf::Keyboard::Escape:
                m_state.deferAction([this]() {
                    m_state.requestStackPop();
                    m_state.requestStackPush(StateID::Menu);
                });
                break;
            case sf::Keyboard::P:
                m_state.deferAction([this]() {
                    StageIntroState::configure(m_state.m_gameState.currentLevel, false);
                    m_state.requestStackPush(StateID::StageIntro);
                });
                break;
            case sf::Keyboard::W:
            case sf::Keyboard::S:
            case sf::Keyboard::A:
            case sf::Keyboard::D:
            case sf::Keyboard::Up:
            case sf::Keyboard::Down:
            case sf::Keyboard::Left:
            case sf::Keyboard::Right:
                break;
            default:
                break;
            }
            break;
        case sf::Event::MouseMoved:
            break;
        case sf::Event::MouseButtonPressed:
            break;
        default:
            break;
        }
    });
}

bool PlayLogic::update(sf::Time deltaTime)
{
    m_state.updateGameplay(deltaTime);
    m_state.processDeferredActions();
    return false;
}

} // namespace FishGame
