#ifndef APP_UTIL_HPP
#define APP_UTIL_HPP

#include <string>
#include "GameText.hpp"
#include "Util/Keycode.hpp"

namespace AppUtil {
    // 計算文字靠左對齊時的 X 座標
    inline float AlignLeft(const GameText& text, float boundaryX) {
        return boundaryX + text.GetSize().x / 2.0f;
    }

    // 計算文字靠右對齊時的 X 座標
    inline float AlignRight(const GameText& text, float boundaryX) {
        return boundaryX - text.GetSize().x / 2.0f;
    }

    // 將 Keycode 轉成玩家看得懂的顯示字串
    std::string KeycodeToString(Util::Keycode key);

    // 回傳這一幀剛被按下的第一個可綁定按鍵
    Util::Keycode GetAnyKeyDown();

    // 共用判斷，是否有滑鼠在物件上
    bool IsMouseHovering(const Util::GameObject& obj);

    // 共用判斷，物件是否被滑鼠點擊
    bool IsLeftClicked(const Util::GameObject& obj);

} // namespace AppUtil

#endif // APP_UTIL_HPP