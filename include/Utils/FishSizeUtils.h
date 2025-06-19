#pragma once

#include "Entities/Fish.h"

namespace FishGame::FishSizeUtils {

    template<typename T>
    constexpr T selectBySize(FishSize size, T smallValue, T mediumValue, T largeValue)
    {
        switch (size)
        {
        case FishSize::Small:
            return smallValue;
        case FishSize::Medium:
            return mediumValue;
        case FishSize::Large:
            return largeValue;
        default:
            return smallValue;
        }
    }

    template<typename Config>
    constexpr auto valueFromConfig(const Config& cfg, FishSize size)
        -> decltype(cfg.small)
    {
        return selectBySize(size, cfg.small, cfg.medium, cfg.large);
    }

} // namespace FishGame::FishSizeUtils
