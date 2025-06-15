#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <chrono>

namespace FishGame
{
    // Represents a single animation clip with frames and timing
    class AnimationClip
    {
    public:
        enum class PlayMode
        {
            Once,
            Loop
        };

        AnimationClip(const std::string& name, PlayMode mode = PlayMode::Loop);
        ~AnimationClip() = default;

        // Add a frame to the animation
        void addFrame(const sf::IntRect& frameRect, std::chrono::milliseconds duration);

        // Get frame data at specific index
        const sf::IntRect& getFrame(size_t index) const;
        std::chrono::milliseconds getFrameDuration(size_t index) const;

        // Animation properties
        size_t getFrameCount() const { return m_frames.size(); }
        const std::string& getName() const { return m_name; }
        PlayMode getPlayMode() const { return m_playMode; }
        void setPlayMode(PlayMode mode) { m_playMode = mode; }

        // Get total duration of the animation
        std::chrono::milliseconds getTotalDuration() const;

    private:
        struct FrameData
        {
            sf::IntRect rect;
            std::chrono::milliseconds duration;
        };

        std::string m_name;
        std::vector<FrameData> m_frames;
        PlayMode m_playMode;
    };
}