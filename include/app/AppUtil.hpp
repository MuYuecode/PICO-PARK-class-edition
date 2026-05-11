#ifndef APP_UTIL_HPP
#define APP_UTIL_HPP

#include <cmath>
#include <string>

#include "gameplay/Character.hpp"
#include "ui/GameText.hpp"
#include "Util/Keycode.hpp"

namespace AppUtil {
    inline float AlignLeft(const GameText& text, float boundaryX) {
        return boundaryX + text.GetSize().x / 2.0f;
    }

    inline float AlignRight(const GameText& text, float boundaryX) {
        return boundaryX - text.GetSize().x / 2.0f;
    }

    inline float SpriteHalfW(const Character& sprite) {
        return std::abs(sprite.GetScaledSize().x) * 0.5f;
    }

    inline float SpriteHalfH(const Character& sprite) {
        return std::abs(sprite.GetScaledSize().y) * 0.5f;
    }

    inline float LeftEdge(const Character& sprite) {
        return sprite.GetPosition().x - SpriteHalfW(sprite);
    }

    inline float RightEdge(const Character& sprite) {
        return sprite.GetPosition().x + SpriteHalfW(sprite);
    }

    inline float TopEdge(const Character& sprite) {
        return sprite.GetPosition().y + SpriteHalfH(sprite);
    }

    inline float BottomEdge(const Character& sprite) {
        return sprite.GetPosition().y - SpriteHalfH(sprite);
    }

    inline float AlignSpriteLeft(const Character& sprite, float boundaryX) {
        return boundaryX + SpriteHalfW(sprite);
    }

    inline float AlignSpriteRight(const Character& sprite, float boundaryX) {
        return boundaryX - SpriteHalfW(sprite);
    }

    inline float AlignSpriteTop(const Character& sprite, float boundaryY) {
        return boundaryY - SpriteHalfH(sprite);
    }

    inline float AlignSpriteBottom(const Character& sprite, float boundaryY) {
        return boundaryY + SpriteHalfH(sprite);
    }

    inline float AlignSpriteBelow(const Character& sprite, const Character& anchor) {
        return BottomEdge(anchor) - SpriteHalfH(sprite);
    }

    inline float AlignSpriteAbove(const Character& sprite, const Character& anchor) {
        return TopEdge(anchor) + SpriteHalfH(sprite);
    }

    std::string KeycodeToString(Util::Keycode key);

    Util::Keycode GetAnyKeyDown();

    bool IsMouseHoveringByRect(const glm::vec2& center, const glm::vec2& size);

    bool IsLeftClickedByRect(const glm::vec2& center, const glm::vec2& size);

}

#endif // APP_UTIL_HPP
