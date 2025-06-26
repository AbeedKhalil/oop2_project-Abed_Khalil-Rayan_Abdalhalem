#include "MusicPlayer.h"
#include "GameExceptions.h"

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
    // Pre-load music files for smoother playback
    for (const auto& [id, file] : m_filenames)
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(file))
        {
            throw ResourceLoadException("Failed to load music: " + file);
        }
        music->setVolume(m_volume);
        m_musicTracks.emplace(id, std::move(music));
    }
}

void MusicPlayer::play(MusicID theme, bool loop)
{
    auto it = m_musicTracks.find(theme);
    if (it == m_musicTracks.end())
    {
        throw ResourceLoadException("Music ID not found");
    }

    // Stop previous track if playing
    if (m_current)
    {
        m_current->stop();
    }

    m_current = it->second.get();
    m_current->setLoop(loop);
    m_current->setVolume(m_volume);
    m_current->play();
}

void MusicPlayer::stop()
{
    if (m_current)
    {
        m_current->stop();
        m_current = nullptr;
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
