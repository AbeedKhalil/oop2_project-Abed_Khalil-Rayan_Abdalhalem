#include "AnimationClip.h"
#include <numeric>
#include <stdexcept>

namespace FishGame
{
    AnimationClip::AnimationClip(const std::string& name, PlayMode mode)
        : m_name(name)
        , m_playMode(mode)
    {
    }

    void AnimationClip::addFrame(const sf::IntRect& frameRect, std::chrono::milliseconds duration)
    {
        m_frames.push_back({ frameRect, duration });
    }

    const sf::IntRect& AnimationClip::getFrame(size_t index) const
    {
        if (index >= m_frames.size())
        {
            throw std::out_of_range("Frame index out of range");
        }
        return m_frames[index].rect;
    }

    std::chrono::milliseconds AnimationClip::getFrameDuration(size_t index) const
    {
        if (index >= m_frames.size())
        {
            throw std::out_of_range("Frame index out of range");
        }
        return m_frames[index].duration;
    }

    std::chrono::milliseconds AnimationClip::getTotalDuration() const
    {
        return std::accumulate(m_frames.begin(), m_frames.end(),
            std::chrono::milliseconds(0),
            [](const auto& sum, const auto& frame) {
                return sum + frame.duration;
            });
    }
}