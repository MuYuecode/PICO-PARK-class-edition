//
// Created by cody2 on 2026/3/14.
//

#include "ui/components/UITriangleButton.hpp"
#include "Util/Time.hpp"

UITriangleButton::UITriangleButton(std::string normalImagePath, std::string pressedImagePath)
    : Character(normalImagePath),
      m_NormalImagePath(std::move(normalImagePath)),
      m_PressedImagePath(std::move(pressedImagePath)) {
}

void UITriangleButton::Press(float durationMs) {
    SetImage(m_PressedImagePath);
    m_PressTimer = durationMs;
}

void UITriangleButton::UpdateButton() {
    if (m_PressTimer > 0.0f) {
        m_PressTimer -= Util::Time::GetDeltaTimeMs();

        if (m_PressTimer <= 0.0f) {
            m_PressTimer = 0.0f;
            SetImage(m_NormalImagePath);
        }
    }
}

void UITriangleButton::ResetState() {
    m_PressTimer = 0.0f;
    SetImage(m_NormalImagePath);
}
