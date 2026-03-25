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
    [[nodiscard]] glm::vec2 GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetSize() const { return GetScaledSize(); }

    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    void SetColor(const Util::Color& color) const ;
    void SetText(const std::string& text) const ;

    [[nodiscard]] bool IsMouseHovering() const;
    [[nodiscard]] bool IsLeftClicked() const;
};

#endif //GAMETEXT_HPP