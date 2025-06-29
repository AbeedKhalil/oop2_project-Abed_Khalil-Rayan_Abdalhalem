#pragma once

#include <type_traits>

namespace FishGame
{
    // Generic timer for controlling spawn intervals
    template <typename TimeType>
    class SpawnTimer
    {
    public:
        explicit SpawnTimer(TimeType interval = TimeType{})
            : m_interval(interval), m_elapsed(TimeType{}) {}

        // Update elapsed time and return true if the interval has elapsed 
        bool update(TimeType delta)
        {
            m_elapsed += delta;
            if (m_elapsed >= m_interval && m_interval > TimeType{})
            {
                m_elapsed -= m_interval;
                return true;
            }
            return false;
        }

        void reset(TimeType newElapsed = TimeType{}) { m_elapsed = newElapsed; }
        void setInterval(TimeType interval) { m_interval = interval; }
        TimeType getInterval() const { return m_interval; }

    private:
        TimeType m_interval;
        TimeType m_elapsed;
    };
}
