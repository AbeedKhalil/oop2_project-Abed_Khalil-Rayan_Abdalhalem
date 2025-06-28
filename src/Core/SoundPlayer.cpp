#include "SoundPlayer.h"
#include "GameExceptions.h"
#include <algorithm>

namespace FishGame {

SoundPlayer::SoundPlayer()
    : m_soundBuffers()
    , m_filenames{
        {SoundEffectID::Bite1, "Bite1.wav"},
        {SoundEffectID::Bite2, "Bite2.wav"},
        {SoundEffectID::Bite3, "Bite3.wav"},
        {SoundEffectID::Bite4, "Bite4.wav"},
        {SoundEffectID::FreezePowerup, "FreezePowerup.wav"},
        {SoundEffectID::LifePowerup, "LifePowerup.wav"},
        {SoundEffectID::MineExplode, "MineExplode.wav"},
        {SoundEffectID::MouseDown, "MouseDown.wav"},
        {SoundEffectID::MouseOver, "MouseOver.wav"},
        {SoundEffectID::OysterPearl, "OysterPearl.wav"},
        {SoundEffectID::PlayerGrow, "PlayerGrow.wav"},
        {SoundEffectID::PlayerPoison, "PlayerPoison.wav"},
        {SoundEffectID::PlayerSpawn, "PlayerSpawn.wav"},
        {SoundEffectID::PlayerStunned, "PlayerStunned.wav"},
        {SoundEffectID::PufferBounce, "PufferBounce.wav"},
        {SoundEffectID::SpeedEnd, "SpeedEnd.wav"},
        {SoundEffectID::SpeedStart, "SpeedStart.wav"},
        {SoundEffectID::StageIntro, "StageIntro.wav"},
        {SoundEffectID::StarPickup, "StarPickup.wav"},
        {SoundEffectID::FeedingFrenzy, "FeedingFrenzy.wav"},
        {SoundEffectID::SuperFrenzy, "SuperFrenzy.wav"}}
    , m_sounds(16)
    , m_volume(100.f)
{
    m_soundBuffers.reserve(m_filenames.size());
    std::for_each(m_filenames.begin(), m_filenames.end(), [this](const auto& p) {
        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (!buffer->loadFromFile(p.second)) {
            throw ResourceLoadException("Failed to load sound: " + p.second);
        }
        m_soundBuffers.emplace(p.first, std::move(buffer));
    });

    std::for_each(m_sounds.begin(), m_sounds.end(), [this](sf::Sound& sound) {
        sound.setVolume(m_volume);
    });
}

void SoundPlayer::play(SoundEffectID effect)
{
    auto it = m_soundBuffers.find(effect);
    if (it == m_soundBuffers.end())
        return;

    auto soundIt = std::find_if(m_sounds.begin(), m_sounds.end(),
        [](const sf::Sound& s) { return s.getStatus() != sf::Sound::Playing; });
    if (soundIt != m_sounds.end()) {
        soundIt->setBuffer(*it->second);
        soundIt->setVolume(m_volume);
        soundIt->play();
        return;
    }

    // If all sounds are playing, restart the first one
    m_sounds.front().stop();
    m_sounds.front().setBuffer(*it->second);
    m_sounds.front().setVolume(m_volume);
    m_sounds.front().play();
}

void SoundPlayer::setVolume(float volume)
{
    m_volume = volume;
    std::for_each(m_sounds.begin(), m_sounds.end(), [this](sf::Sound& sound) {
        sound.setVolume(m_volume);
    });
}

} // namespace FishGame

