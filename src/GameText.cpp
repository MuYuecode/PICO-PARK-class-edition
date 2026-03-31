//
// Created by cody2 on 2026/3/13.
//

#include "GameText.hpp"
#include "AppUtil.hpp"

const Util::Color kOrange = Util::Color::FromRGB(255, 140,   0, 255);

GameText::GameText(const std::string& text, int size)
    : GameObject(std::make_unique<Util::Text>(GA_RESOURCE_DIR"/Font/TerminusTTFWindows-Bold-4.49.3.ttf", size, text, kOrange), 100) {
    m_Visible = true;
}

GameText::GameText(const std::string& text, int size, const Util::Color& color)
    : GameObject(std::make_unique<Util::Text>(GA_RESOURCE_DIR"/Font/TerminusTTFWindows-Bold-4.49.3.ttf", size, text, color), 100) {
    m_Visible = true;
}

void GameText::SetColor(const Util::Color& color) const {
    if (auto t = std::dynamic_pointer_cast<Util::Text>(m_Drawable)) {
        t->SetColor(color);
    }
}

void GameText::SetText(const std::string& text) const {
    if (auto t = std::dynamic_pointer_cast<Util::Text>(m_Drawable)) {
        t->SetText(text);
    }
}

bool GameText::IsMouseHovering() const {
    return AppUtil::IsMouseHoveringByRect(GetPosition(), GetSize());
}

bool GameText::IsLeftClicked() const {
    return AppUtil::IsLeftClickedByRect(GetPosition(), GetSize());
}