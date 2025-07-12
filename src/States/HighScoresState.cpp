#include "HighScoresState.h"
#include "Game.h"
#include <sstream>

namespace FishGame {

HighScoresState::HighScoresState(Game& game)
    : State(game) {}

void HighScoresState::onActivate() {
    auto& fonts = getGame().getFonts();
    auto& manager = getGame().getSpriteManager();
    auto& window = getGame().getWindow();

    m_titleText.setFont(fonts.get(Fonts::Main));
    m_titleText.setString("HIGH SCORES");
    m_titleText.setCharacterSize(48);
    auto tb = m_titleText.getLocalBounds();
    m_titleText.setOrigin(tb.width/2.f, tb.height/2.f);
    m_titleText.setPosition(window.getSize().x/2.f, 150.f);

    m_backButton.setTexture(manager.getTexture(TextureID::Button));
    auto bb = m_backButton.getLocalBounds();
    m_backButton.setOrigin(bb.width/2.f, bb.height/2.f);
    m_backButton.setScale(Constants::MENU_BUTTON_SCALE, Constants::MENU_BUTTON_SCALE);
    m_backButton.setPosition(window.getSize().x/2.f, window.getSize().y - 120.f);

    loadScores();
}

void HighScoresState::loadScores() {
    m_scores = loadHighScores("highscores.txt");
    auto& font = getGame().getFonts().get(Fonts::Main);
    m_scoreTexts.clear();
    float startY = 250.f;
    float spacing = 40.f;
    for (std::size_t i=0;i<m_scores.size();++i) {
        sf::Text text;
        text.setFont(font);
        std::ostringstream ss; ss<<i+1<<". "<<m_scores[i].name<<" - "<<m_scores[i].score;
        text.setString(ss.str());
        text.setCharacterSize(32);
        auto b = text.getLocalBounds();
        text.setOrigin(b.width/2.f, b.height/2.f);
        text.setPosition(getGame().getWindow().getSize().x/2.f, startY + spacing*i);
        m_scoreTexts.push_back(text);
    }
}

void HighScoresState::handleEvent(const sf::Event& event) {
    if(event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::Escape){
        deferAction([this](){ requestStackPop(); });
    } else if(event.type==sf::Event::MouseMoved) {
        sf::Vector2f pos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        bool h = m_backButton.getGlobalBounds().contains(pos);
        if(h!=m_hover){
            m_hover=h;
            m_backButton.setTexture(getGame().getSpriteManager().getTexture(h?TextureID::ButtonHover:TextureID::Button));
        }
    } else if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left){
        sf::Vector2f pos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if(m_backButton.getGlobalBounds().contains(pos))
            deferAction([this](){ requestStackPop(); });
    }
}

bool HighScoresState::update(sf::Time) {
    processDeferredActions();
    return false;
}

void HighScoresState::render() {
    auto& window = getGame().getWindow();
    window.draw(m_titleText);
    for(const auto& t : m_scoreTexts) window.draw(t);
    window.draw(m_backButton);
}

} // namespace FishGame
