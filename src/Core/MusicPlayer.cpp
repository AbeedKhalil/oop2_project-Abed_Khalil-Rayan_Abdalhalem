#include "MusicPlayer.h"
#include "GameExceptions.h"

namespace FishGame {

MusicPlayer::MusicPlayer()
    : m_music()
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
    , m_volume(100.f)
{}

void MusicPlayer::play(MusicID theme, bool loop)
{
    auto it = m_filenames.find(theme);
    if (it == m_filenames.end()) {
        throw ResourceLoadException("Music ID not found");
    }

    if (!m_music.openFromFile(it->second)) {
        throw ResourceLoadException("Failed to load music: " + it->second);
    }

    m_music.setLoop(loop);
    m_music.setVolume(m_volume);
    m_music.play();
}

void MusicPlayer::stop()
{
    m_music.stop();
}

void MusicPlayer::setVolume(float volume)
{
    m_volume = volume;
    m_music.setVolume(m_volume);
}

} // namespace FishGame
