//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_UI_TRIANGLE_BUTTON_HPP
#define PICOPART_UI_TRIANGLE_BUTTON_HPP

#include "Character.hpp"
#include <string>

class UITriangleButton : public Character {
public:
    UITriangleButton(std::string normalImagePath, std::string pressedImagePath);

    void UpdateButton();

    void Press(float durationMs = 75.0f);

    void ResetState();

private:
    std::string m_NormalImagePath;
    std::string m_PressedImagePath;
    float m_PressTimer = 0.0f;
};

#endif //PICOPART_UI_TRIANGLE_BUTTON_HPP

