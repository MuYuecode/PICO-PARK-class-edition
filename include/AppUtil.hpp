#ifndef APP_UTIL_HPP
#define APP_UTIL_HPP

#include "Character.hpp"
#include "GameText.hpp"

namespace AppUtil {

    /**
     * @brief 計算文字置左對齊時的中心 X 座標
     *
     * PTSD 使用中心點定位，此函式將「文字左邊緣 = boundaryX」
     * 反推回應傳入 SetPosition() 的中心 X 值。
     *
     * @param text      要對齊的 GameText 物件
     * @param boundaryX 左邊界的 X 座標
     * @return 應傳入 SetPosition() 的中心 X 座標
     */
    inline float AlignLeft(const GameText& text, float boundaryX) {
        return boundaryX + text.GetSize().x / 2.0f;
    }

    /**
     * @brief 計算文字置右對齊時的中心 X 座標
     *
     * 將「文字右邊緣 = boundaryX」反推回中心 X 值。
     *
     * @param text      要對齊的 GameText 物件
     * @param boundaryX 右邊界的 X 座標
     * @return 應傳入 SetPosition() 的中心 X 座標
     */
    inline float AlignRight(const GameText& text, float boundaryX) {
        return boundaryX - text.GetSize().x / 2.0f;
    }

} // namespace AppUtil

#endif // APP_UTIL_HPP