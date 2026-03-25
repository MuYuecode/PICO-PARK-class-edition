//
// Created by cody2 on 2026/3/14.
//

#include "UI_Triangle_Button.hpp"
#include "Util/Time.hpp"

UI_Triangle_Button::UI_Triangle_Button(std::string normalImagePath, std::string pressedImagePath)
    : Character(normalImagePath),
      m_NormalImagePath(std::move(normalImagePath)),
      m_PressedImagePath(std::move(pressedImagePath)) {
}

void UI_Triangle_Button::Press(float durationMs) {
    SetImage(m_PressedImagePath);
    m_PressTimer = durationMs;
}

void UI_Triangle_Button::UpdateButton() {
    if (m_PressTimer > 0.0f) {
        m_PressTimer -= Util::Time::GetDeltaTimeMs();

        if (m_PressTimer <= 0.0f) {
            m_PressTimer = 0.0f;
            SetImage(m_NormalImagePath);
        }
    }
}

void UI_Triangle_Button::ResetState() {
    m_PressTimer = 0.0f;
    SetImage(m_NormalImagePath);
}