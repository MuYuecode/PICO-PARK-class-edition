//
// Created by cody2 on 2026/3/12.
//

#ifndef GAMETEXT_HPP
#define GAMETEXT_HPP

#include <string>
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Color.hpp"

class GameText : public Util::GameObject {
public:
    GameText(const std::string& text, int size, const Util::Color& color)
        : GameObject(std::make_unique<Util::Text>(GA_RESOURCE_DIR"/Font/TerminusTTFWindows-Bold-4.49.3.ttf", size, text, color), 100) {
        m_Visible = true;
    }

    [[nodiscard]] bool GetVisibility() const { return m_Visible; }

    void SetVisible(bool visible) { m_Visible = visible; }

    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
};

#endif //GAMETEXT_HPP