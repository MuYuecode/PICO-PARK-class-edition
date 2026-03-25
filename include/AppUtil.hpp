#ifndef APP_UTIL_HPP
#define APP_UTIL_HPP

#include <string>
#include "GameText.hpp"
#include "Util/Keycode.hpp"

namespace AppUtil {
    inline float AlignLeft(const GameText& text, float boundaryX) {
        return boundaryX + text.GetSize().x / 2.0f;
    }

    inline float AlignRight(const GameText& text, float boundaryX) {
        return boundaryX - text.GetSize().x / 2.0f;
    }

    std::string KeycodeToString(Util::Keycode key);

    Util::Keycode GetAnyKeyDown();

    bool IsMouseHovering(const Util::GameObject& obj);

    bool IsLeftClicked(const Util::GameObject& obj);

}

#endif // APP_UTIL_HPP