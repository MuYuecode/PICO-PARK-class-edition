#ifndef APP_UTIL_HPP
#define APP_UTIL_HPP

#include <string>
#include "Character.hpp"
#include "GameText.hpp"
#include "Util/Keycode.hpp"

namespace AppUtil {

    /**
     * @brief 計算文字置左對齊時的中心 X 座標
     */
    inline float AlignLeft(const GameText& text, float boundaryX) {
        return boundaryX + text.GetSize().x / 2.0f;
    }

    /**
     * @brief 計算文字置右對齊時的中心 X 座標
     */
    inline float AlignRight(const GameText& text, float boundaryX) {
        return boundaryX - text.GetSize().x / 2.0f;
    }

    /**
     * @brief 將 Keycode 轉成玩家看得懂的顯示字串
     *        UNKNOWN → "-"
     */
    std::string KeycodeToString(Util::Keycode key);

    /**
     * @brief 回傳這一幀剛被按下的第一個可綁定按鍵
     *        若無任何按鍵被按下，回傳 Util::Keycode::UNKNOWN
     */
    Util::Keycode GetAnyKeyDown();

} // namespace AppUtil

#endif // APP_UTIL_HPP