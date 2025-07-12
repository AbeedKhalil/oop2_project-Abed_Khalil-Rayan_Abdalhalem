#include "PlayerNameState.h"
#include "Game.h"
#include "States/GameOverState.h"

namespace FishGame {

PlayerNameState::PlayerNameState(Game& game)
    : State(game), m_backgroundSprite(), m_overlaySprite() {}

void PlayerNameState::onActivate(){
    auto& font = getGame().getFonts().get(Fonts::Main);
    auto& window = getGame().getWindow();
    auto& manager = getGame().getSpriteManager();
    m_input.clear();

    m_backgroundSprite.setTexture(manager.getTexture(TextureID::Background1));
    auto size = m_backgroundSprite.getTexture()->getSize();
    m_backgroundSprite.setScale(static_cast<float>(window.getSize().x)/size.x,
                               static_cast<float>(window.getSize().y)/size.y);

    m_overlaySprite.setTexture(manager.getTexture(TextureID::StageIntro));
    size = m_overlaySprite.getTexture()->getSize();
    m_overlaySprite.setScale(static_cast<float>(window.getSize().x)/size.x,
                             static_cast<float>(window.getSize().y)/size.y);
    m_prompt.setFont(font);
    m_prompt.setString("Enter Name:");
    m_prompt.setCharacterSize(36);
    auto pb=m_prompt.getLocalBounds();
    m_prompt.setOrigin(pb.width/2.f, pb.height/2.f);
    m_prompt.setPosition(window.getSize().x/2.f, window.getSize().y/2.f - 40.f);

    m_inputText.setFont(font);
    m_inputText.setCharacterSize(36);
    m_inputText.setPosition(window.getSize().x/2.f, window.getSize().y/2.f + 10.f);
    m_inputText.setOrigin(0.f, m_inputText.getLocalBounds().height/2.f);
}

void PlayerNameState::handleEvent(const sf::Event& event){
    if(event.type==sf::Event::TextEntered){
        if(event.text.unicode==8) { // backspace
            if(!m_input.empty()) m_input.pop_back();
        } else if(event.text.unicode==13){ // enter
            GameStats::getInstance().playerName = m_input.empty()?"Player":m_input;
            deferAction([this](){ requestStackPop(); requestStackPush(StateID::Menu); });
        } else if(event.text.unicode>=32 && event.text.unicode<127 && m_input.size()<12){
            m_input += static_cast<char>(event.text.unicode);
        }
        m_inputText.setString(m_input);
        auto b=m_inputText.getLocalBounds();
        m_inputText.setOrigin(b.width/2.f, b.height/2.f);
    }
}

bool PlayerNameState::update(sf::Time){ processDeferredActions(); return false; }

void PlayerNameState::render(){
    auto& window=getGame().getWindow();
    window.draw(m_backgroundSprite);
    window.draw(m_overlaySprite);
    window.draw(m_prompt);
    window.draw(m_inputText);
}

} // namespace FishGame
