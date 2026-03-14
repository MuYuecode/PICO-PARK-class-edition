//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_UI_TRIANGLE_BUTTON_HPP
#define PICOPART_UI_TRIANGLE_BUTTON_HPP

#include "Character.hpp"
#include <string>

class UI_Triangle_Button : public Character {
public:
    // 傳入兩種狀態的圖片路徑
    UI_Triangle_Button(const std::string& normalImagePath, const std::string& pressedImagePath);

    // 必須在每一幀呼叫此方法，用來更新按鈕內部的計時器邏輯
    void UpdateButton();

    // 觸發「按下」狀態，並設定維持時間 (預設 200 毫秒)
    void Press(float durationMs = 200.0f);

    // 強制重置按鈕為未按下狀態 (用於選單切換時清空殘留狀態)
    void ResetState();

private:
    std::string m_NormalImagePath;
    std::string m_PressedImagePath;
    float m_PressTimer = 0.0f;
};

#endif //PICOPART_UI_TRIANGLE_BUTTON_HPP

