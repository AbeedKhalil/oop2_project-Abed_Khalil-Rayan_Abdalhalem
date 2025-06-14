#include "BonusStageState.h"
#include "Game.h"
#include "GenericFish.h"
#include "SpecialFish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iterator>

namespace FishGame
{
    BonusStageState::BonusStageState(Game& game, BonusStageType type, int playerLevel)
        : State(game)
        , m_stageType(type)
        , m_playerLevel(playerLevel)
        , m_player(std::make_unique<Player>())
        , m_entities()
        , m_bonusItems()
        , m_environment(std::make_unique<EnvironmentSystem>())
        , m_timeLimit(sf::Time::Zero)
        , m_timeElapsed(sf::Time::Zero)
        , m_objective()
        , m_stageComplete(false)
        , m_bonusScore(0)
        , m_objectiveText()
        , m_timerText()
        , m_scoreText()
        , m_timerBar()
        , m_timerBackground()
        , m_randomEngine(std::chrono::steady_clock::now().time_since_epoch().count())
        , m_xDist(100.0f, 1820.0f)
        , m_yDist(100.0f, 980.0f)
    {
        // Initialize UI
        auto& font = getGame().getFonts().get(Fonts::Main);

        m_objectiveText.setFont(font);
        m_objectiveText.setCharacterSize(32);
        m_objectiveText.setFillColor(sf::Color::Yellow);
        m_objectiveText.setPosition(50.0f, 50.0f);

        m_timerText.setFont(font);
        m_timerText.setCharacterSize(24);
        m_timerText.setFillColor(sf::Color::White);
        m_timerText.setPosition(50.0f, 100.0f);

        m_scoreText.setFont(font);
        m_scoreText.setCharacterSize(28);
        m_scoreText.setFillColor(sf::Color::Green);
        m_scoreText.setPosition(50.0f, 150.0f);

        // Timer bar
        m_timerBackground.setSize(sf::Vector2f(400.0f, 20.0f));
        m_timerBackground.setPosition(50.0f, 130.0f);
        m_timerBackground.setFillColor(sf::Color(50, 50, 50));
        m_timerBackground.setOutlineColor(sf::Color::White);
        m_timerBackground.setOutlineThickness(2.0f);

        m_timerBar.setSize(sf::Vector2f(400.0f, 20.0f));
        m_timerBar.setPosition(50.0f, 130.0f);
        m_timerBar.setFillColor(sf::Color::Green);

        // Configure stage based on type
        switch (m_stageType)
        {
        case BonusStageType::TreasureHunt:
            m_timeLimit = sf::seconds(m_treasureHuntDuration);
            m_objective = { "Collect Pearl Oysters!", 10, 0, 100 };
            m_environment->setEnvironment(EnvironmentType::CoralReef);
            break;

        case BonusStageType::FeedingFrenzy:
            m_timeLimit = sf::seconds(m_feedingFrenzyDuration);
            m_objective = { "Eat Small Fish!", 20, 0, 50 };
            m_environment->setEnvironment(EnvironmentType::OpenOcean);
            break;

        case BonusStageType::SurvivalChallenge:
            m_timeLimit = sf::seconds(m_survivalDuration);
            m_objective = { "Survive the Predators!", 1, 0, 1000 };
            m_environment->setEnvironment(EnvironmentType::KelpForest);
            break;
        }

        // Initialize player
        m_player->setWindowBounds(getGame().getWindow().getSize());
        m_player->setPosition(960.0f, 540.0f);
        m_player->initializeSprite(getGame().getSpriteManager());

        // Reserve containers
        m_entities.reserve(50);
        m_bonusItems.reserve(30);
    }

    void BonusStageState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                deferAction([this]() {
                    requestStackPop();
                    });
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            m_player->followMouse(sf::Vector2f(
                static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y)
            ));
        }
    }

    bool BonusStageState::update(sf::Time deltaTime)
    {
        if (m_stageComplete)
        {
            return false;
        }

        m_timeElapsed += deltaTime;

        // Update environment
        m_environment->update(deltaTime);

        // Apply ocean currents to player
        sf::Vector2f currentForce = m_environment->getOceanCurrentForce(m_player->getPosition());
        m_player->setVelocity(m_player->getVelocity() + currentForce * deltaTime.asSeconds());

        // Update player
        m_player->update(deltaTime);

        // Stage-specific updates
        switch (m_stageType)
        {
        case BonusStageType::TreasureHunt:
            updateTreasureHunt(deltaTime);
            break;
        case BonusStageType::FeedingFrenzy:
            updateFeedingFrenzy(deltaTime);
            break;
        case BonusStageType::SurvivalChallenge:
            updateSurvivalChallenge(deltaTime);
            break;
        }

        // Update entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [deltaTime, this](auto& entity) {
                entity->update(deltaTime);

                // Apply ocean currents to fish
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    sf::Vector2f force = m_environment->getOceanCurrentForce(entity->getPosition());
                    entity->setVelocity(entity->getVelocity() + force * deltaTime.asSeconds() * 0.5f);
                }
            });

        // Update bonus items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [deltaTime](auto& item) {
                item->update(deltaTime);
            });

        // Remove dead entities
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                [](const auto& entity) { return !entity->isAlive(); }),
            m_entities.end()
        );

        m_bonusItems.erase(
            std::remove_if(m_bonusItems.begin(), m_bonusItems.end(),
                [](const auto& item) { return !item->isAlive(); }),
            m_bonusItems.end()
        );

        // Check completion
        checkCompletion();

        // Update UI
        std::ostringstream timerStream;
        float timeRemaining = std::max(0.0f, m_timeLimit.asSeconds() - m_timeElapsed.asSeconds());
        timerStream << "Time: " << std::fixed << std::setprecision(1) << timeRemaining << "s";
        m_timerText.setString(timerStream.str());

        float timerPercent = 1.0f - (m_timeElapsed.asSeconds() / m_timeLimit.asSeconds());
        m_timerBar.setSize(sf::Vector2f(400.0f * timerPercent, 20.0f));

        if (timerPercent < 0.3f)
        {
            m_timerBar.setFillColor(sf::Color::Red);
        }
        else if (timerPercent < 0.6f)
        {
            m_timerBar.setFillColor(sf::Color::Yellow);
        }

        std::ostringstream scoreStream;
        scoreStream << "Bonus Score: " << m_bonusScore;
        m_scoreText.setString(scoreStream.str());

        processDeferredActions();
        return false;
    }

    void BonusStageState::render()
    {
        auto& window = getGame().getWindow();

        // Draw environment
        window.draw(*m_environment);

        // Draw entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [&window](const auto& entity) {
                window.draw(*entity);
            });

        // Draw bonus items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [&window](const auto& item) {
                window.draw(*item);
            });

        // Draw player - cast to drawable
        window.draw(static_cast<const sf::Drawable&>(*m_player));

        // Draw UI
        window.draw(m_objectiveText);
        window.draw(m_timerText);
        window.draw(m_scoreText);
        window.draw(m_timerBackground);
        window.draw(m_timerBar);

        // Draw completion message
        if (m_stageComplete)
        {
            sf::Text completeText;
            completeText.setFont(getGame().getFonts().get(Fonts::Main));
            completeText.setCharacterSize(48);
            completeText.setFillColor(sf::Color::Yellow);
            completeText.setString("BONUS STAGE COMPLETE!");

            sf::FloatRect bounds = completeText.getLocalBounds();
            completeText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            completeText.setPosition(960.0f, 540.0f);

            window.draw(completeText);
        }
    }

    void BonusStageState::onActivate()
    {
        // Initial spawn based on stage type
        switch (m_stageType)
        {
        case BonusStageType::TreasureHunt:
            spawnTreasureItems();
            break;
        case BonusStageType::FeedingFrenzy:
            spawnBonusFish();
            break;
        case BonusStageType::SurvivalChallenge:
            spawnPredatorWave();
            break;
        }

        // Update initial objective text
        std::ostringstream objStream;
        objStream << m_objective.description << " (" << m_objective.currentCount
            << "/" << m_objective.targetCount << ")";
        m_objectiveText.setString(objStream.str());
    }

    void BonusStageState::updateTreasureHunt(sf::Time deltaTime)
    {
        // Spawn more oysters periodically
        static sf::Time spawnTimer = sf::Time::Zero;
        spawnTimer += deltaTime;

        if (spawnTimer.asSeconds() > 3.0f && m_bonusItems.size() < 5)
        {
            spawnTimer = sf::Time::Zero;
            spawnTreasureItems();
        }

        // Check collisions with oysters
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [this](auto& item) {
                if (PearlOyster* oyster = dynamic_cast<PearlOyster*>(item.get()))
                {
                    if (oyster->isOpen() && CollisionDetector::checkCircleCollision(*m_player, *oyster))
                    {
                        oyster->onCollect();
                        m_objective.currentCount++;
                        m_bonusScore += m_objective.pointsPerItem;

                        // Update objective text
                        std::ostringstream objStream;
                        objStream << m_objective.description << " (" << m_objective.currentCount
                            << "/" << m_objective.targetCount << ")";
                        m_objectiveText.setString(objStream.str());
                    }
                }
            });
    }

    void BonusStageState::updateFeedingFrenzy(sf::Time deltaTime)
    {
        // Spawn more fish periodically
        static sf::Time spawnTimer = sf::Time::Zero;
        spawnTimer += deltaTime;

        if (spawnTimer.asSeconds() > 2.0f && m_entities.size() < 20)
        {
            spawnTimer = sf::Time::Zero;
            spawnBonusFish();
        }

        // Check collisions with fish
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](auto& entity) {
                if (SmallFish* fish = dynamic_cast<SmallFish*>(entity.get()))
                {
                    if (m_player->canEat(*fish) && CollisionDetector::checkCircleCollision(*m_player, *fish))
                    {
                        if (m_player->attemptEat(*fish))
                        {
                            fish->destroy();
                            m_objective.currentCount++;
                            m_bonusScore += m_objective.pointsPerItem;

                            // Update objective text
                            std::ostringstream objStream;
                            objStream << m_objective.description << " (" << m_objective.currentCount
                                << "/" << m_objective.targetCount << ")";
                            m_objectiveText.setString(objStream.str());
                        }
                    }
                }
            });
    }

    void BonusStageState::updateSurvivalChallenge(sf::Time deltaTime)
    {
        // Spawn predator waves
        static sf::Time waveTimer = sf::Time::Zero;
        waveTimer += deltaTime;

        if (waveTimer.asSeconds() > 10.0f)
        {
            waveTimer = sf::Time::Zero;
            spawnPredatorWave();
        }

        // Check if player got eaten
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](auto& entity) {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    if (fish->canEat(*m_player) && CollisionDetector::checkCircleCollision(*m_player, *fish))
                    {
                        // Player dies - stage failed
                        m_objective.currentCount = 0;
                        completeStage();
                    }
                }
            });

        // Award survival points over time
        static sf::Time survivalTimer = sf::Time::Zero;
        survivalTimer += deltaTime;

        if (survivalTimer.asSeconds() > 1.0f)
        {
            survivalTimer = sf::Time::Zero;
            m_bonusScore += 10;
        }
    }

    void BonusStageState::spawnTreasureItems()
    {
        std::generate_n(std::back_inserter(m_bonusItems), 3, [this] {
            auto oyster = std::make_unique<PearlOyster>();
            oyster->setPosition(m_xDist(m_randomEngine), m_yDist(m_randomEngine));
            return oyster;
            });
    }

    void BonusStageState::spawnBonusFish()
    {
        std::generate_n(std::back_inserter(m_entities), 5, [this] {
            auto fish = std::make_unique<SmallFish>(m_playerLevel);
            bool fromLeft = m_randomEngine() % 2 == 0;
            float x = fromLeft ? -50.0f : 1970.0f;
            float y = m_yDist(m_randomEngine);
            fish->setPosition(x, y);
            fish->setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);
            fish->setWindowBounds(getGame().getWindow().getSize());
            fish->initializeSprite(getGame().getSpriteManager());
            return fish;
            });
    }

    void BonusStageState::spawnPredatorWave()
    {
        // Spawn barracudas
        std::generate_n(std::back_inserter(m_entities), 2, [this, i = 0]() mutable {
            auto barracuda = std::make_unique<Barracuda>(m_playerLevel);
            float angle = (360.0f / 2.0f) * i * Constants::DEG_TO_RAD;
            float x = 960.0f + std::cos(angle) * 500.0f;
            float y = 540.0f + std::sin(angle) * 300.0f;
            barracuda->setPosition(x, y);
            barracuda->setWindowBounds(getGame().getWindow().getSize());
            barracuda->initializeSprite(getGame().getSpriteManager());
            ++i;
            return barracuda;
            });
    }

    void BonusStageState::checkCompletion()
    {
        if (m_timeElapsed >= m_timeLimit || m_objective.currentCount >= m_objective.targetCount)
        {
            completeStage();
        }
    }

    void BonusStageState::completeStage()
    {
        m_stageComplete = true;

        // Calculate final bonus
        if (m_stageType == BonusStageType::SurvivalChallenge)
        {
            if (m_objective.currentCount > 0) // Survived
            {
                m_bonusScore += m_objective.pointsPerItem;
            }
        }

        // Return to play state after delay
        deferAction([this]() {
            requestStackPop();
            });
    }

    int BonusStageState::calculateBonus() const
    {
        return m_bonusScore;
    }
}