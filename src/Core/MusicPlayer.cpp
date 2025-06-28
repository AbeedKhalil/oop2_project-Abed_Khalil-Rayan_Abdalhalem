#include "MusicPlayer.h"
#include "GameExceptions.h"
#include <chrono>
#include <future>
#include <vector>
#include <utility>

namespace FishGame {

MusicPlayer::MusicPlayer()
    : m_musicTracks()
    , m_filenames{
        {MusicID::MenuTheme, "MenuTheme.ogg"},
        {MusicID::InGame1, "InGame1.ogg"},
        {MusicID::InGame2, "InGame2.ogg"},
        {MusicID::InGame3, "InGame3.ogg"},
        {MusicID::BonusStage, "BonusStage.ogg"},
        {MusicID::InstructionsHelp, "InstructionsHelp.ogg"},
        {MusicID::ScoreSummary, "ScoreSummary.ogg"},
        {MusicID::StageCleared, "StageCleared.ogg"},
        {MusicID::PlayerDies, "PlayerDies.ogg"}}
    , m_current(nullptr)
    , m_volume(100.f)
{
    // Reserve to avoid rehashing during preload
    m_musicTracks.reserve(m_filenames.size());

    // Load music tracks asynchronously for faster startup
    std::vector<std::future<std::pair<MusicID, MusicPtr>>> futures;
    futures.reserve(m_filenames.size());

    for (const auto& [id, file] : m_filenames)
    {
        futures.emplace_back(std::async(std::launch::async,
            [id, file]() {
                auto music = std::make_unique<sf::Music>();
                if (!music->openFromFile(file))
                {
                    throw ResourceLoadException("Failed to load music: " + file);
                }
                return std::make_pair(id, std::move(music));
            }));
    }

    for (auto& fut : futures)
    {
        auto result = fut.get();
        result.second->setVolume(m_volume);
        m_musicTracks.emplace(result.first, std::move(result.second));
    }
}

void MusicPlayer::play(MusicID theme, bool loop)
{
    auto it = m_musicTracks.find(theme);
    if (it == m_musicTracks.end())
    {
        throw ResourceLoadException("Music ID not found");
    }

    auto next = it->second.get();

    // If same track requested, just ensure it is playing
    if (m_current == next)
    {
        next->setLoop(loop);
        if (next->getStatus() != sf::SoundSource::Playing)
        {
            next->setVolume(m_volume);
            next->play();
        }
        return;
    }

    // Cancel any existing fade thread
    m_fadeThread.request_stop();
    if (m_fadeThread.joinable())
    {
        m_fadeThread.join();
    }

    sf::Music* previous = m_current;
    m_current = next;

    // Start new track with volume 0 and fade in/out asynchronously
    m_current->setLoop(loop);
    m_current->setVolume(0.f);
    m_current->play();

    const float targetVolume = m_volume;
    m_fadeThread = std::jthread([this, previous, targetVolume](std::stop_token st)
    {
        const int steps = 20;
        const auto delay = std::chrono::milliseconds(
            static_cast<int>(FadeDuration * 1000 / steps));

        for (int i = 0; i < steps && !st.stop_requested(); ++i)
        {
            float t = static_cast<float>(i + 1) / static_cast<float>(steps);

            if (previous)
            {
                previous->setVolume(targetVolume * (1.f - t));
            }
            m_current->setVolume(targetVolume * t);
            std::this_thread::sleep_for(delay);
        }

        if (previous && !st.stop_requested())
        {
            previous->stop();
            previous->setVolume(targetVolume);
        }
    });
}

void MusicPlayer::stop()
{
    m_fadeThread.request_stop();
    if (m_fadeThread.joinable())
    {
        m_fadeThread.join();
    }


    // Stop any music that might still be playing
    if (m_current)
    {
        m_current->stop();
        m_current = nullptr;
    }

    for (auto& [_, music] : m_musicTracks)
    {
        if (music->getStatus() != sf::SoundSource::Stopped)
        {
            music->stop();
        }
    }
}

void MusicPlayer::setVolume(float volume)
{
    m_volume = volume;
    // Update volume on all tracks using STL algorithms
    for (auto& [_, music] : m_musicTracks)
    {
        music->setVolume(m_volume);
    }
}

} // namespace FishGame
