#include "gameplay/PlayerKeyConfig.hpp"

std::vector<Util::Keycode> PlayerKeyConfig::AllKeys() const {
    std::vector<Util::Keycode> result;
    for (auto k : {up, down, left, right, jump, cancel, shot, menu, subMenu}) {
        if (k != Util::Keycode::UNKNOWN) result.push_back(k);
    }
    return result;
}


