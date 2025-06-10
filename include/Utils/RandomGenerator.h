#pragma once

#include <random>
#include <algorithm>
#include <iterator>
#include <type_traits>

namespace FishGame
{
    // Template-based random number generator following OOP guidelines
    template<typename T>
    class RandomGenerator
    {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

    public:
        RandomGenerator(T min, T max)
            : m_engine(std::random_device{}())
            , m_distribution(min, max)
        {
        }

        T generate() { return m_distribution(m_engine); }

        // Template method for generating multiple values
        template<typename Container>
        void generateN(Container& container, size_t count)
        {
            std::generate_n(std::back_inserter(container), count,
                [this]() { return generate(); });
        }

        // Reset range
        void setRange(T min, T max)
        {
            m_distribution = std::conditional_t<
                std::is_integral_v<T>,
                std::uniform_int_distribution<T>,
                std::uniform_real_distribution<T>
            >(min, max);
        }

    private:
        std::mt19937 m_engine;
        std::conditional_t<
            std::is_integral_v<T>,
            std::uniform_int_distribution<T>,
            std::uniform_real_distribution<T>
        > m_distribution;
    };

    // Convenience type aliases
    using IntRandom = RandomGenerator<int>;
    using FloatRandom = RandomGenerator<float>;
}