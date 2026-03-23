//
// Created by cody2 on 2026/3/13.
//

#include "GameText.hpp"
#include "AppUtil.hpp"

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
    return AppUtil::IsMouseHovering(*this);
}

bool GameText::IsLeftClicked() const {
    return AppUtil::IsLeftClicked(*this);
}