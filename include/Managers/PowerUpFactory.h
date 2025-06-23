#pragma once

#include "PowerUp.h"
#include "ExtendedPowerUps.h"
#include <memory>
#include <type_traits>

namespace FishGame
{
    namespace detail
    {
        template<typename T, typename = void>
        struct has_set_font : std::false_type {};

        template<typename T>
        struct has_set_font<T, std::void_t<decltype(std::declval<T>().setFont(std::declval<const sf::Font&>()))>> : std::true_type {};

        template<typename Derived>
        struct FactoryHelper
        {
            static std::unique_ptr<PowerUp> create(const sf::Font* font)
            {
                auto ptr = std::make_unique<Derived>();
                if constexpr (has_set_font<Derived>::value)
                {
                    if (font)
                        ptr->setFont(*font);
                }
                return ptr;
            }
        };
    }

    template<PowerUpType T>
    struct PowerUpFactory;

    template<>
    struct PowerUpFactory<PowerUpType::ScoreDoubler> : detail::FactoryHelper<ScoreDoublerPowerUp> {};

    template<>
    struct PowerUpFactory<PowerUpType::FrenzyStarter> : detail::FactoryHelper<FrenzyStarterPowerUp> {};

    template<>
    struct PowerUpFactory<PowerUpType::SpeedBoost> : detail::FactoryHelper<SpeedBoostPowerUp> {};

    template<>
    struct PowerUpFactory<PowerUpType::Freeze> : detail::FactoryHelper<FreezePowerUp> {};

    template<>
    struct PowerUpFactory<PowerUpType::ExtraLife> : detail::FactoryHelper<ExtraLifePowerUp> {};

    template<>
    struct PowerUpFactory<PowerUpType::AddTime> : detail::FactoryHelper<AddTimePowerUp> {};
}

