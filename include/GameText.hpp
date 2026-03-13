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

    [[nodiscard]] const glm::vec2 GetPosition() const { return m_Transform.translation; }

    [[nodiscard]] const glm::vec2 GetSize() const { return GetScaledSize(); }

    // ==========================================
    // 新增：偵測滑鼠是否懸停在文字範圍內
    // ==========================================
    [[nodiscard]] bool IsMouseHovering() const;

    // ==========================================
    // 新增：偵測文字是否被滑鼠左鍵點擊
    // ==========================================
    [[nodiscard]] bool IsLeftClicked() const;
};

#endif //GAMETEXT_HPP