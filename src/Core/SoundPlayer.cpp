#include "SoundPlayer.h"
#include "GameExceptions.h"
#include <future>
#include <vector>
#include <utility>

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

    // Load buffers in parallel using std::async for faster startup
    std::vector<std::future<std::pair<SoundEffectID, BufferPtr>>> futures;
    futures.reserve(m_filenames.size());

    for (const auto& [id, file] : m_filenames) {
        futures.emplace_back(std::async(std::launch::async,
            [id, file]() {
                auto buffer = std::make_unique<sf::SoundBuffer>();
                if (!buffer->loadFromFile(file)) {
                    throw ResourceLoadException("Failed to load sound: " + file);
                }
                return std::make_pair(id, std::move(buffer));
            }));
    }

    for (auto& fut : futures) {
        auto result = fut.get();
        m_soundBuffers.emplace(result.first, std::move(result.second));
    }

    for (auto& sound : m_sounds) {
        sound.setVolume(m_volume);
    }
}

void SoundPlayer::play(SoundEffectID effect)
{
    auto it = m_soundBuffers.find(effect);
    if (it == m_soundBuffers.end())
        return;

    for (auto& sound : m_sounds) {
        if (sound.getStatus() != sf::Sound::Playing) {
            sound.setBuffer(*it->second);
            sound.setVolume(m_volume);
            sound.play();
            return;
        }
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
    for (auto& sound : m_sounds) {
        sound.setVolume(m_volume);
    }
}

} // namespace FishGame

