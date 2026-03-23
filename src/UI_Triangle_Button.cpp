//
// Created by cody2 on 2026/3/14.
//

#include "UI_Triangle_Button.hpp"
#include "Util/Time.hpp"

UI_Triangle_Button::UI_Triangle_Button(std::string normalImagePath, std::string pressedImagePath)
    : Character(normalImagePath), // 呼叫父類別建構子，預設載入 normal 圖片
      m_NormalImagePath(std::move(normalImagePath)),
      m_PressedImagePath(std::move(pressedImagePath)) {
}

void UI_Triangle_Button::Press(float durationMs) {
    SetImage(m_PressedImagePath);
    m_PressTimer = durationMs;
}

void UI_Triangle_Button::UpdateButton() {
    // 如果計時器大於 0，代表按鈕正處於「被按下」的發亮狀態
    if (m_PressTimer > 0.0f) {
        m_PressTimer -= Util::Time::GetDeltaTimeMs();

        // 時間到，切換回一般圖片
        if (m_PressTimer <= 0.0f) {
            m_PressTimer = 0.0f; // 防止出現負數
            SetImage(m_NormalImagePath);
        }
    }
}

void UI_Triangle_Button::ResetState() {
    m_PressTimer = 0.0f;
    SetImage(m_NormalImagePath);
}