#include "StageSummaryState.h"
#include "Game.h"
#include "MusicPlayer.h"
#include "StageIntroState.h"

namespace {
using FishGame::TextureID;

sf::IntRect firstFrameRect(TextureID id) {
    using namespace FishGame;
    switch (id) {
    case TextureID::SmallFish:
    case TextureID::PoisonFish:
    case TextureID::Angelfish:
        return {1,1,66,44};
    case TextureID::MediumFish:
        return {1,1,172,108};
    case TextureID::LargeFish:
        return {1,1,201,148};
    case TextureID::PearlOysterClosed:
        return {1,1,101,101};
    case TextureID::PearlOysterOpen:
        return {1+4*101,1,101,101};
    case TextureID::Bomb:
        return {1,1,69,69};
    case TextureID::Pufferfish:
        return {5,5,187,131};
    case TextureID::PufferfishInflated:
        return {5,136,186,169};
    case TextureID::Jellyfish:
        return {1,1,75,197};
    case TextureID::Barracuda:
        return {1,1,270,122};
    default:
        return {};
    }
}
}

namespace FishGame {
StageSummaryState::StageSummaryState(Game& game)
    : State(game), m_overlaySprite(), m_scoreText(), m_nextButtonSprite(), m_nextText(), m_items() {}

void StageSummaryState::configure(int nextLevel, int levelScore,
                                  const std::unordered_map<TextureID, int>& counts,
                                  bool pushBonusStage) {
    auto& cfg = StageSummaryConfig::getInstance();
    cfg.nextLevel = nextLevel;
    cfg.levelScore = levelScore;
    cfg.pushBonusStage = pushBonusStage;
    cfg.counts = counts;
}

void StageSummaryState::onActivate() {
    getGame().getMusicPlayer().play(MusicID::ScoreSummary, false);
    auto& manager = getGame().getSpriteManager();
    auto& window = getGame().getWindow();
    m_overlaySprite.setTexture(manager.getTexture(TextureID::StageIntro));
    auto size = m_overlaySprite.getTexture()->getSize();
    m_overlaySprite.setScale(static_cast<float>(window.getSize().x)/size.x,
                             static_cast<float>(window.getSize().y)/size.y);

    auto& font = getGame().getFonts().get(Fonts::Main);
    m_scoreText.setFont(font);
    m_scoreText.setCharacterSize(48);
    m_scoreText.setFillColor(sf::Color::White);
    m_scoreText.setString("Score: " + std::to_string(StageSummaryConfig::getInstance().levelScore));
    auto bounds = m_scoreText.getLocalBounds();
    m_scoreText.setOrigin(bounds.width/2.f, bounds.height/2.f);
    m_scoreText.setPosition(window.getSize().x/2.f, 150.f);

    m_nextButtonSprite.setTexture(manager.getTexture(TextureID::Button));
    auto b = m_nextButtonSprite.getLocalBounds();
    m_nextButtonSprite.setOrigin(b.width/2.f, b.height/2.f);
    m_nextButtonSprite.setScale(Constants::MENU_BUTTON_SCALE, Constants::MENU_BUTTON_SCALE);
    m_nextButtonSprite.setPosition(window.getSize().x/2.f, window.getSize().y - 120.f);
    m_nextButtonSprite.setColor(sf::Color(128, 128, 128));

    m_nextText.setFont(font);
    m_nextText.setString("NEXT");
    m_nextText.setCharacterSize(36);
    auto nb = m_nextText.getLocalBounds();
    m_nextText.setOrigin(nb.width/2.f, nb.height/2.f + 10.0f);
    m_nextText.setPosition(m_nextButtonSprite.getPosition());
    m_nextText.setFillColor(sf::Color(128, 128, 128));
    m_buttonHovered = false;

    setupItems();
}

void StageSummaryState::setupItems() {
    m_items.clear();
    auto& cfg = StageSummaryConfig::getInstance();
    auto& manager = getGame().getSpriteManager();
    auto& font = getGame().getFonts().get(Fonts::Main);

    float startY = 250.f;
    float spacing = 80.f;
    float spriteX = getGame().getWindow().getSize().x/2.f - 100.f;
    float textX = getGame().getWindow().getSize().x/2.f + 60.f;

    int index = 0;
    for (const auto& kv : cfg.counts) {
        Item item;
        item.sprite.setTexture(manager.getTexture(kv.first));
        sf::IntRect rect = firstFrameRect(kv.first);
        if (rect.width > 0) item.sprite.setTextureRect(rect);
        auto b = item.sprite.getLocalBounds();
        item.sprite.setOrigin(b.width/2.f, b.height/2.f);
        item.sprite.setPosition(spriteX, startY + spacing*index);

        float scale = 0.5f;
        if (kv.first == TextureID::Starfish)
            scale = 0.02f;
        item.sprite.setScale(scale, scale);

        item.text.setFont(font);
        item.text.setString(std::to_string(kv.second));
        auto tb = item.text.getLocalBounds();
        item.text.setOrigin(tb.width/2.f, tb.height/2.f);
        item.text.setPosition(textX, startY + spacing*index);
        item.text.setCharacterSize(32);
        m_items.push_back(std::move(item));
        ++index;
    }
}

void StageSummaryState::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        exitState();
    } else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f pos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        bool hover = m_nextButtonSprite.getGlobalBounds().contains(pos);
        if (hover != m_buttonHovered) {
            m_buttonHovered = hover;
            auto &manager = getGame().getSpriteManager();
            m_nextButtonSprite.setTexture(manager.getTexture(hover ? TextureID::ButtonHover : TextureID::Button));
        }
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f pos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (m_nextButtonSprite.getGlobalBounds().contains(pos)) {
            exitState();
        }
    }
}

bool StageSummaryState::update(sf::Time) {
    processDeferredActions();
    return false;
}

void StageSummaryState::exitState() {
    deferAction([this]() {
        requestStackPop();
        auto &cfg = StageSummaryConfig::getInstance();
        if (cfg.pushBonusStage) {
            StageIntroState::configure(0, true, StateID::BonusStage);
        } else {
            StageIntroState::configure(cfg.nextLevel, false);
        }
        cfg.pushBonusStage = false;
        requestStackPush(StateID::StageIntro);
    });
}

void StageSummaryState::render() {
    auto& window = getGame().getWindow();
    window.draw(m_overlaySprite);
    window.draw(m_scoreText);
    for (auto& item : m_items) {
        window.draw(item.sprite);
        window.draw(item.text);
    }
    window.draw(m_nextButtonSprite);
    window.draw(m_nextText);
}

} // namespace FishGame
