//
// Created by cody2 on 2026/3/29.
//

#ifndef PLAYER_KEY_CONFIG_HPP
#define PLAYER_KEY_CONFIG_HPP

#include <vector>
#include "Util/Keycode.hpp"

struct PlayerKeyConfig {
    Util::Keycode up      = Util::Keycode::UNKNOWN;
    Util::Keycode down    = Util::Keycode::UNKNOWN;
    Util::Keycode left    = Util::Keycode::UNKNOWN;
    Util::Keycode right   = Util::Keycode::UNKNOWN;
    Util::Keycode jump    = Util::Keycode::UNKNOWN;
    Util::Keycode cancel  = Util::Keycode::UNKNOWN;
    Util::Keycode shot    = Util::Keycode::UNKNOWN;
    Util::Keycode menu    = Util::Keycode::UNKNOWN;
    Util::Keycode subMenu = Util::Keycode::UNKNOWN;

    [[nodiscard]] std::vector<Util::Keycode> AllKeys() const;
};

#endif // PLAYER_KEY_CONFIG_HPP
